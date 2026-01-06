#!/bin/bash
# Generate workload6111.txt matching run_task_12B_string_key.py parameters

./K-V-Workload-Generator-master/load_gen \
  --insert=100000 \
  --range_delete=100 \
  --range_delete_selectivity=0.001 \
  --entry_size=1024 \
  --range_delete_threshold=0.999 \
  --key_size=12 \
  --using_string_key=1

mv workload.txt workload/workload6111.txt
echo "Generated workload/workload6111.txt"
