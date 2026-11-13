"""Consumer-facing safeguards against misleading benchmark comparisons."""

import copy
import unittest

from run import compare


def report():
    return {
        "schema_version": 1, "generator_version": 1, "seed": 42,
        "results": [{
            "case": {"id": "example", "mode": "NW"},
            "input_hash": "123abc", "query_length": 32, "target_length": 33,
            "reference_distance": 2, "k": -1, "skipped": False,
            "median_ns": 1000, "peak_rss_bytes": 4096,
        }],
    }


class ComparisonTests(unittest.TestCase):
    def test_speedup_direction_and_memory_are_preserved(self):
        baseline = report()
        current = copy.deepcopy(baseline)
        current["results"][0]["median_ns"] = 2000
        current["results"][0]["peak_rss_bytes"] = 8192
        row, = compare(current, baseline)
        self.assertEqual(row["speedup"], 0.5)
        self.assertEqual(row["baseline_peak_rss_bytes"], 4096)
        self.assertEqual(row["current_peak_rss_bytes"], 8192)

    def test_different_inputs_or_alignment_results_cannot_look_like_speedups(self):
        for key, value in (("input_hash", "different"), ("reference_distance", 3), ("k", 1)):
            with self.subTest(key=key):
                baseline = report()
                current = copy.deepcopy(baseline)
                current["results"][0][key] = value
                with self.assertRaisesRegex(ValueError, key):
                    compare(current, baseline)

    def test_missing_or_duplicate_cases_are_rejected(self):
        baseline = report()
        current = copy.deepcopy(baseline)
        current["results"] = []
        with self.assertRaisesRegex(ValueError, "same cases"):
            compare(current, baseline)
        current["results"] = baseline["results"] * 2
        with self.assertRaisesRegex(ValueError, "duplicate"):
            compare(current, baseline)

    def test_seed_or_generator_change_invalidates_comparison(self):
        for key in ("seed", "generator_version"):
            with self.subTest(key=key):
                baseline = report()
                current = copy.deepcopy(baseline)
                current[key] += 1
                with self.assertRaisesRegex(ValueError, key):
                    compare(current, baseline)

    def test_unmeasurable_zero_distance_rejection_is_not_reported_as_a_speedup(self):
        baseline = report()
        baseline["results"][0]["skipped"] = True
        del baseline["results"][0]["median_ns"]
        self.assertEqual(compare(copy.deepcopy(baseline), baseline), [])


if __name__ == "__main__":
    unittest.main()
