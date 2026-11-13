#!/usr/bin/env python3
"""Run deterministic Edlib workloads in isolated processes; compare JSON reports."""

import argparse
import csv
import datetime
import hashlib
import itertools
import json
import math
import pathlib
import platform
import statistics
import subprocess
import sys


SCHEMA_VERSION = 1
GENERATOR_VERSION = 1


def cases(suite):
    if suite == "smoke":
        sizes = [(32, 32), (65, 4096)]
        alphabets = [4, 256]
        mutations = [0, 30]
    else:
        sizes = [(n, n) for n in (32, 63, 64, 65, 128, 1024, 10000, 100000)]
        sizes += [(128, 100000), (1024, 100000)]
        alphabets = [4, 20, 256]
        mutations = [0, 5, 30]
    for (query, target), alphabet, mutation, mode, task, cutoff in itertools.product(
        sizes, alphabets, mutations, ("NW", "SHW", "HW"),
        ("distance", "locations", "path"), ("auto", "bounded", "reject")
    ):
        yield {
            "id": f"q{query}-t{target}-a{alphabet}-e{mutation}-{mode}-{task}-{cutoff}",
            "query": query, "target": target, "alphabet": alphabet,
            "mutation": mutation, "mode": mode, "task": task, "cutoff": cutoff,
        }


def compare(current, baseline):
    for key in ("schema_version", "generator_version", "seed"):
        if current[key] != baseline[key]:
            raise ValueError(f"baseline has different {key}")
    old = {row["case"]["id"]: row for row in baseline["results"]}
    new = {row["case"]["id"]: row for row in current["results"]}
    if len(old) != len(baseline["results"]) or len(new) != len(current["results"]):
        raise ValueError("report contains duplicate cases")
    if old.keys() != new.keys():
        raise ValueError("baseline and current report must contain exactly the same cases")
    comparisons = []
    for case_id, row in new.items():
        previous = old[case_id]
        for key in ("case", "input_hash", "query_length", "target_length", "reference_distance", "k", "skipped"):
            if row[key] != previous[key]:
                raise ValueError(f"{case_id}: baseline differs in {key}")
        if row["skipped"]:
            continue
        comparisons.append({
            "case": case_id,
            "baseline_ns": previous["median_ns"],
            "current_ns": row["median_ns"],
            "speedup": previous["median_ns"] / row["median_ns"],
            "baseline_peak_rss_bytes": previous["peak_rss_bytes"],
            "current_peak_rss_bytes": row["peak_rss_bytes"],
        })
    return comparisons


def positive(text):
    value = int(text)
    if value < 1:
        raise argparse.ArgumentTypeError("must be positive")
    return value


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=pathlib.Path, help="path to edlib-benchmark")
    parser.add_argument("--suite", choices=("smoke", "full"), default="full")
    parser.add_argument("--filter", default="", help="only case IDs containing this substring")
    parser.add_argument("--seed", type=int, default=42)
    parser.add_argument("--samples", type=positive, default=5)
    parser.add_argument("--sample-ms", type=positive, default=10, help="minimum calibration batch duration")
    parser.add_argument("--timeout", type=positive, default=300, help="maximum seconds per case")
    parser.add_argument("--label", default="", help="revision/build flags or other build description")
    parser.add_argument("--output", type=pathlib.Path, help="write a JSON report (required for a run)")
    parser.add_argument("--baseline", type=pathlib.Path, help="compare against this JSON report")
    parser.add_argument("--compare", type=pathlib.Path, help="compare an existing report, without running benchmarks")
    parser.add_argument("--list", action="store_true", help="list selected cases without running")
    args = parser.parse_args()
    if args.compare:
        if not args.baseline:
            parser.error("--compare requires --baseline")
        current = json.loads(args.compare.read_text())
    else:
        selected = [case for case in cases(args.suite) if args.filter in case["id"]]
        if not selected:
            parser.error("no cases match --filter")
        if args.list:
            for case in selected:
                print(case["id"])
            return
        if args.binary is None or args.output is None:
            parser.error("a benchmark run requires --binary and --output")
        if not 0 <= args.seed <= 2147483647 or args.samples > 1000 or args.sample_ms > 60000:
            parser.error("seed must be 0..2147483647; samples <= 1000; sample-ms <= 60000")
        if args.baseline and args.output.resolve() == args.baseline.resolve():
            parser.error("--output must not overwrite --baseline")
        binary = args.binary.resolve(strict=True)
        current = {
            "schema_version": SCHEMA_VERSION,
            "generator_version": GENERATOR_VERSION,
            "seed": args.seed,
            "metadata": {
                "label": args.label,
                "created_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                "platform": platform.platform(),
                "machine": platform.machine(),
                "processor": platform.processor(),
                "binary": str(binary),
                "binary_sha256": hashlib.sha256(binary.read_bytes()).hexdigest(),
                "suite": args.suite,
                "filter": args.filter,
                "samples": args.samples,
                "sample_ms": args.sample_ms,
                "command": sys.argv,
            },
            "results": [],
        }
        for index, case in enumerate(selected, 1):
            command = [str(binary)] + [str(case[key]) for key in (
                "query", "target", "alphabet", "mutation", "mode", "task", "cutoff"
            )] + [str(args.seed), str(args.samples), str(args.sample_ms)]
            print(f"[{index}/{len(selected)}] {case['id']}", file=sys.stderr, flush=True)
            completed = subprocess.run(command, capture_output=True, text=True, timeout=args.timeout)
            if completed.returncode:
                raise RuntimeError(f"{case['id']}: benchmark exited {completed.returncode}: {completed.stderr}")
            result = json.loads(completed.stdout)
            result["case"] = case
            if not result["skipped"]:
                result["median_ns"] = statistics.median(result["samples_ns"])
                result["min_ns"] = min(result["samples_ns"])
                result["max_ns"] = max(result["samples_ns"])
            current["results"].append(result)
        args.output.write_text(json.dumps(current, indent=2) + "\n")
        measured = sum(not row["skipped"] for row in current["results"])
        print(f"Wrote {args.output}: {measured} measured cases; "
              f"{len(selected) - measured} zero-distance rejection cases skipped.", file=sys.stderr)
    if args.baseline:
        baseline = json.loads(args.baseline.read_text())
        comparisons = compare(current, baseline)
        if comparisons:
            writer = csv.DictWriter(sys.stdout, fieldnames=list(comparisons[0]))
            writer.writeheader()
            writer.writerows(comparisons)
            speedup = math.exp(statistics.mean(math.log(row["speedup"]) for row in comparisons))
            print(f"Geometric mean speedup: {speedup:.3f}x (>1 is faster). "
                  "Inspect per-case results; aggregate gains can hide regressions.", file=sys.stderr)


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, KeyError, RuntimeError, subprocess.TimeoutExpired) as error:
        print(f"error: {error}", file=sys.stderr)
        sys.exit(1)
