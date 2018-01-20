#!/usr/bin/env bash

docker build -t martinsos/edlib-performance-comparison:latest .
nextflow run performance-comparison.nf -with-docker martinsos/edlib-performance-comparison:latest
