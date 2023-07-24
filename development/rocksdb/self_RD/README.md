1. Compile RocksDB first by executing `make static_lib` in parent dir
2. Compile all examples: `cd examples/; make all`
To Run:
	./simple_example -i 100000 --RD 100 -P 64 -B 4 -E 1024 -T 5 --selectivity 0.001 --workload_filename "workload/workload1.txt" --insert_before_range_delete 0.9 --gen_workload 0

#
./simple_example -i 100000 --RD 100 -P 64 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload11.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log11
./simple_example -i 100000 --RD 500 -P 64 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload12.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log12
./simple_example -i 100000 --RD 900 -P 64 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload13.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log13

./simple_example -i 100000 --RD 10 -P 64 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload14.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log14
./simple_example -i 100000 --RD 50 -P 64 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload15.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log15
./simple_example -i 100000 --RD 90 -P 64 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload16.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log16

./simple_example -i 100000 --RD 1 -P 64 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload17.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log17
./simple_example -i 100000 --RD 5 -P 64 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload18.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log18
./simple_example -i 100000 --RD 9 -P 64 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload19.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log19