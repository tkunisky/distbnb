#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
OUTPUT_DIR="$SCRIPT_DIR/../test_data"

rm $OUTPUT_DIR/*.csv

for n in 10 20 30 40 50 60 70 80 90 100
do
  for i in 1 2 3 4 5
  do
    python $SCRIPT_DIR/erdos_renyi_laplacian.py \
      --N="${n}" --p=0.5 --seed="${i}" --output_file="test_data/er__0_5__${n}__${i}.csv"
  done
done
