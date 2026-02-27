#This is for YCSB workload Simulation

1. Make sure zstd is installed. apt-get install zstd

2. Compile RocksDB first by executing `make static_lib -j8` in parent dir

3. Compile all examples: `make simple_example -j8`

Run it with script:
	python run_task_256B_string_key_YCSB.py
