1. Compile RocksDB first by executing `make static_lib` in parent dir
2. Compile all examples: `cd examples/; make all`
Set --gen_workload 1 if the workload hasn't been generated yet
To Run:
	./simple_example -i 100000 --RD 100 -P 64 -B 4 -E 1024 -T 5 --selectivity 0.001 --workload_filename "workload/workload1.txt" --insert_before_range_delete 0.9 --gen_workload 0 --max_open_files 999 --skip_reading_RD_blocks 0 --number_of_PQ 5000

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






./simple_example -i 100000 --RD 100 -P 128 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload21.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log21
./simple_example -i 100000 --RD 500 -P 128 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload22.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log22
./simple_example -i 100000 --RD 900 -P 128 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload23.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log23

./simple_example -i 100000 --RD 10 -P 128 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload24.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log24
./simple_example -i 100000 --RD 50 -P 128 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload25.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log25
./simple_example -i 100000 --RD 90 -P 128 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload26.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log26

./simple_example -i 100000 --RD 1 -P 128 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload27.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log27
./simple_example -i 100000 --RD 5 -P 128 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload28.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log28
./simple_example -i 100000 --RD 9 -P 128 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload29.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log29







./simple_example -i 100000 --RD 100 -P 2 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload31.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log31
./simple_example -i 100000 --RD 500 -P 2 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload32.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log32
./simple_example -i 100000 --RD 900 -P 2 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload33.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log33

./simple_example -i 100000 --RD 10 -P 2 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload34.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log34
./simple_example -i 100000 --RD 50 -P 2 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload35.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log35
./simple_example -i 100000 --RD 90 -P 2 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload36.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log36

./simple_example -i 100000 --RD 1 -P 2 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload37.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log37
./simple_example -i 100000 --RD 5 -P 2 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload38.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log38
./simple_example -i 100000 --RD 9 -P 2 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload39.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log39








./simple_example -i 100000 --RD 100 -P 4 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload41.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log41
./simple_example -i 100000 --RD 500 -P 4 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload42.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log42
./simple_example -i 100000 --RD 900 -P 4 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload43.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log43

./simple_example -i 100000 --RD 10 -P 4 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload44.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log44
./simple_example -i 100000 --RD 50 -P 4 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload45.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log45
./simple_example -i 100000 --RD 90 -P 4 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload46.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log46

./simple_example -i 100000 --RD 1 -P 4 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload47.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log47
./simple_example -i 100000 --RD 5 -P 4 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload48.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log48
./simple_example -i 100000 --RD 9 -P 4 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload49.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log49





















./simple_example -i 100000 --RD 100 -P 8 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload51.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log51
./simple_example -i 100000 --RD 500 -P 8 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload52.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log52
./simple_example -i 100000 --RD 900 -P 8 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload53.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log53

./simple_example -i 100000 --RD 10 -P 8 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload54.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log54
./simple_example -i 100000 --RD 50 -P 8 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload55.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log55
./simple_example -i 100000 --RD 90 -P 8 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload56.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log56

./simple_example -i 100000 --RD 1 -P 8 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload57.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log57
./simple_example -i 100000 --RD 5 -P 8 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload58.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log58
./simple_example -i 100000 --RD 9 -P 8 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload59.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log59






./simple_example -i 100000 --RD 100 -P 16 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload61.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log61
./simple_example -i 100000 --RD 500 -P 16 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload62.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log62
./simple_example -i 100000 --RD 900 -P 16 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload63.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log63

./simple_example -i 100000 --RD 10 -P 16 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload64.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log64
./simple_example -i 100000 --RD 50 -P 16 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload65.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log65
./simple_example -i 100000 --RD 90 -P 16 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload66.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log66

./simple_example -i 100000 --RD 1 -P 16 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload67.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log67
./simple_example -i 100000 --RD 5 -P 16 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload68.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log68
./simple_example -i 100000 --RD 9 -P 16 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload69.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log69




./simple_example -i 100000 --RD 100 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload71.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log71
./simple_example -i 100000 --RD 500 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload72.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log72
./simple_example -i 100000 --RD 900 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload73.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log73

./simple_example -i 100000 --RD 10 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload74.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log74
./simple_example -i 100000 --RD 50 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload75.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log75
./simple_example -i 100000 --RD 90 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload76.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log76

./simple_example -i 100000 --RD 1 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload77.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log77
./simple_example -i 100000 --RD 5 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload78.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log78
./simple_example -i 100000 --RD 9 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload79.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log79



./simple_example -i 100000 --RD 100 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload71_1.txt" --insert_before_range_delete 0.1 --gen_workload 0 > log71_1

./simple_example -i 100000 --RD 100 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload71_2.txt" --insert_before_range_delete 0.5 --gen_workload 0 > log71_2


















#
./simple_example -i 100000 --RD 100 -P 64 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload11disk.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log11disk
./simple_example -i 100000 --RD 500 -P 64 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload12disk.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log12disk
./simple_example -i 100000 --RD 900 -P 64 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload13disk.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log13disk

./simple_example -i 100000 --RD 10 -P 64 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload14disk.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log14disk
./simple_example -i 100000 --RD 50 -P 64 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload15disk.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log15disk
./simple_example -i 100000 --RD 90 -P 64 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload16.disktxt" --insert_before_range_delete 0.9 --gen_workload 0 > log16disk

./simple_example -i 100000 --RD 1 -P 64 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload17disk.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log17disk
./simple_example -i 100000 --RD 5 -P 64 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload18disk.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log18disk
./simple_example -i 100000 --RD 9 -P 64 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload19disk.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log19disk







#### 1xx,  B x E = 4096 kB





./simple_example -i 100000 --RD 100 -P 32 -B 8 -E 512 -T 2 --selectivity 0.001 --workload_filename "workload/workload111.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log111
./simple_example -i 100000 --RD 500 -P 32 -B 8 -E 512 -T 2 --selectivity 0.001 --workload_filename "workload/workload112.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log112
./simple_example -i 100000 --RD 900 -P 32 -B 8 -E 512 -T 2 --selectivity 0.001 --workload_filename "workload/workload113.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log113

./simple_example -i 100000 --RD 10 -P 32 -B 8 -E 512 -T 2 --selectivity 0.01 --workload_filename "workload/workload114.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log114
./simple_example -i 100000 --RD 50 -P 32 -B 8 -E 512 -T 2 --selectivity 0.01 --workload_filename "workload/workload115.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log115
./simple_example -i 100000 --RD 90 -P 32 -B 8 -E 512 -T 2 --selectivity 0.01 --workload_filename "workload/workload116.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log116

./simple_example -i 100000 --RD 1 -P 32 -B 8 -E 512 -T 2 --selectivity 0.1 --workload_filename "workload/workload117.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log117
./simple_example -i 100000 --RD 5 -P 32 -B 8 -E 512 -T 2 --selectivity 0.1 --workload_filename "workload/workload118.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log118
./simple_example -i 100000 --RD 9 -P 32 -B 8 -E 512 -T 2 --selectivity 0.1 --workload_filename "workload/workload119.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log119





./simple_example -i 100000 --RD 100 -P 32 -B 16 -E 256 -T 2 --selectivity 0.001 --workload_filename "workload/workload121.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log121
./simple_example -i 100000 --RD 500 -P 32 -B 16 -E 256 -T 2 --selectivity 0.001 --workload_filename "workload/workload122.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log122
./simple_example -i 100000 --RD 900 -P 32 -B 16 -E 256 -T 2 --selectivity 0.001 --workload_filename "workload/workload123.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log123

./simple_example -i 100000 --RD 10 -P 32 -B 16 -E 256 -T 2 --selectivity 0.01 --workload_filename "workload/workload124.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log124
./simple_example -i 100000 --RD 50 -P 32 -B 16 -E 256 -T 2 --selectivity 0.01 --workload_filename "workload/workload125.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log125
./simple_example -i 100000 --RD 90 -P 32 -B 16 -E 256 -T 2 --selectivity 0.01 --workload_filename "workload/workload126.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log126

./simple_example -i 100000 --RD 1 -P 32 -B 16 -E 256 -T 2 --selectivity 0.1 --workload_filename "workload/workload127.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log127
./simple_example -i 100000 --RD 5 -P 32 -B 16 -E 256 -T 2 --selectivity 0.1 --workload_filename "workload/workload128.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log128
./simple_example -i 100000 --RD 9 -P 32 -B 16 -E 256 -T 2 --selectivity 0.1 --workload_filename "workload/workload129.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log129







./simple_example -i 100000 --RD 100 -P 32 -B 32 -E 128 -T 2 --selectivity 0.001 --workload_filename "workload/workload131.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log131
./simple_example -i 100000 --RD 500 -P 32 -B 32 -E 128 -T 2 --selectivity 0.001 --workload_filename "workload/workload132.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log132
./simple_example -i 100000 --RD 900 -P 32 -B 32 -E 128 -T 2 --selectivity 0.001 --workload_filename "workload/workload133.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log133

./simple_example -i 100000 --RD 10 -P 32 -B 32 -E 128 -T 2 --selectivity 0.01 --workload_filename "workload/workload134.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log134
./simple_example -i 100000 --RD 50 -P 32 -B 32 -E 128 -T 2 --selectivity 0.01 --workload_filename "workload/workload135.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log135
./simple_example -i 100000 --RD 90 -P 32 -B 32 -E 128 -T 2 --selectivity 0.01 --workload_filename "workload/workload136.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log136

./simple_example -i 100000 --RD 1 -P 32 -B 32 -E 128 -T 2 --selectivity 0.1 --workload_filename "workload/workload137.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log137
./simple_example -i 100000 --RD 5 -P 32 -B 32 -E 128 -T 2 --selectivity 0.1 --workload_filename "workload/workload138.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log138
./simple_example -i 100000 --RD 9 -P 32 -B 32 -E 128 -T 2 --selectivity 0.1 --workload_filename "workload/workload139.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log139











./simple_example -i 100000 --RD 100 -P 32 -B 64 -E 64 -T 2 --selectivity 0.001 --workload_filename "workload/workload141.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log141
./simple_example -i 100000 --RD 500 -P 32 -B 64 -E 64 -T 2 --selectivity 0.001 --workload_filename "workload/workload142.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log142
./simple_example -i 100000 --RD 900 -P 32 -B 64 -E 64 -T 2 --selectivity 0.001 --workload_filename "workload/workload143.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log143

./simple_example -i 100000 --RD 10 -P 32 -B 64 -E 64 -T 2 --selectivity 0.01 --workload_filename "workload/workload144.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log144
./simple_example -i 100000 --RD 50 -P 32 -B 64 -E 64 -T 2 --selectivity 0.01 --workload_filename "workload/workload145.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log145
./simple_example -i 100000 --RD 90 -P 32 -B 64 -E 64 -T 2 --selectivity 0.01 --workload_filename "workload/workload146.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log146

./simple_example -i 100000 --RD 1 -P 32 -B 64 -E 64 -T 2 --selectivity 0.1 --workload_filename "workload/workload147.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log147
./simple_example -i 100000 --RD 5 -P 32 -B 64 -E 64 -T 2 --selectivity 0.1 --workload_filename "workload/workload148.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log148
./simple_example -i 100000 --RD 9 -P 32 -B 64 -E 64 -T 2 --selectivity 0.1 --workload_filename "workload/workload149.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log149









### 3xx, (-P 32 -B 4 -E 1024 -T 2), insert_before_range_delete = 0.8, 0.7, 0.6, ...
./simple_example -i 100000 --RD 100 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload311.txt" --insert_before_range_delete 0.8 --gen_workload 0 > log311
./simple_example -i 100000 --RD 500 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload312.txt" --insert_before_range_delete 0.8 --gen_workload 0 > log312
./simple_example -i 100000 --RD 900 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload313.txt" --insert_before_range_delete 0.8 --gen_workload 0 > log313
./simple_example -i 100000 --RD 10 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload314.txt" --insert_before_range_delete 0.8 --gen_workload 0 > log314
./simple_example -i 100000 --RD 50 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload315.txt" --insert_before_range_delete 0.8 --gen_workload 0 > log315
./simple_example -i 100000 --RD 90 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload316.txt" --insert_before_range_delete 0.8 --gen_workload 0 > log316
./simple_example -i 100000 --RD 1 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload317.txt" --insert_before_range_delete 0.8 --gen_workload 0 > log317
./simple_example -i 100000 --RD 5 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload318.txt" --insert_before_range_delete 0.8 --gen_workload 0 > log318
./simple_example -i 100000 --RD 9 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload319.txt" --insert_before_range_delete 0.8 --gen_workload 0 > log319

./simple_example -i 100000 --RD 100 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload321.txt" --insert_before_range_delete 0.5 --gen_workload 0 > log321
./simple_example -i 100000 --RD 500 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload322.txt" --insert_before_range_delete 0.5 --gen_workload 0 > log322
./simple_example -i 100000 --RD 900 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload323.txt" --insert_before_range_delete 0.5 --gen_workload 0 > log323
./simple_example -i 100000 --RD 10 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload324.txt" --insert_before_range_delete 0.5 --gen_workload 0 > log324
./simple_example -i 100000 --RD 50 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload325.txt" --insert_before_range_delete 0.5 --gen_workload 0 > log325
./simple_example -i 100000 --RD 90 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload326.txt" --insert_before_range_delete 0.5 --gen_workload 0 > log326
./simple_example -i 100000 --RD 1 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload327.txt" --insert_before_range_delete 0.5 --gen_workload 0 > log327
./simple_example -i 100000 --RD 5 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload328.txt" --insert_before_range_delete 0.5 --gen_workload 0 > log328
./simple_example -i 100000 --RD 9 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload329.txt" --insert_before_range_delete 0.5 --gen_workload 0 > log329

./simple_example -i 100000 --RD 100 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload331.txt" --insert_before_range_delete 0.1 --gen_workload 0 > log331
./simple_example -i 100000 --RD 500 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload332.txt" --insert_before_range_delete 0.1 --gen_workload 0 > log332
./simple_example -i 100000 --RD 900 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload333.txt" --insert_before_range_delete 0.1 --gen_workload 0 > log333
./simple_example -i 100000 --RD 10 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload334.txt" --insert_before_range_delete 0.1 --gen_workload 0 > log334
./simple_example -i 100000 --RD 50 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload335.txt" --insert_before_range_delete 0.1 --gen_workload 0 > log335
./simple_example -i 100000 --RD 90 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload336.txt" --insert_before_range_delete 0.1 --gen_workload 0 > log336
./simple_example -i 100000 --RD 1 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload337.txt" --insert_before_range_delete 0.1 --gen_workload 0 > log337
./simple_example -i 100000 --RD 5 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload338.txt" --insert_before_range_delete 0.1 --gen_workload 0 > log338
./simple_example -i 100000 --RD 9 -P 32 -B 4 -E 1024 -T 2 --selectivity 0.1 --workload_filename "workload/workload339.txt" --insert_before_range_delete 0.1 --gen_workload 0 > log339

















./simple_example -i 100000 --RD 1 -P 512 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload411.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log411
./simple_example -i 100000 --RD 5 -P 512 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload412.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log412
./simple_example -i 100000 --RD 10 -P 512 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload413.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log413
./simple_example -i 100000 --RD 50 -P 512 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload414.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log414
./simple_example -i 100000 --RD 100 -P 512 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload415.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log415
./simple_example -i 100000 --RD 500 -P 512 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload416.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log416
./simple_example -i 100000 --RD 900 -P 512 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload417.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log417
<!-- ./simple_example -i 100000 --RD 9000 -P 512 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload418.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log418 -->


./simple_example -i 100000 --RD 1 -P 512 -B 16 -E 256 -T 2 --selectivity 0.001 --workload_filename "workload/workload421.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log421
./simple_example -i 100000 --RD 5 -P 512 -B 16 -E 256 -T 2 --selectivity 0.001 --workload_filename "workload/workload422.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log422
./simple_example -i 100000 --RD 10 -P 512 -B 16 -E 256 -T 2 --selectivity 0.001 --workload_filename "workload/workload423.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log423
./simple_example -i 100000 --RD 50 -P 512 -B 16 -E 256 -T 2 --selectivity 0.001 --workload_filename "workload/workload424.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log424
./simple_example -i 100000 --RD 100 -P 512 -B 16 -E 256 -T 2 --selectivity 0.001 --workload_filename "workload/workload425.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log425
./simple_example -i 100000 --RD 500 -P 512 -B 16 -E 256 -T 2 --selectivity 0.001 --workload_filename "workload/workload426.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log426
./simple_example -i 100000 --RD 900 -P 512 -B 16 -E 256 -T 2 --selectivity 0.001 --workload_filename "workload/workload427.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log427
<!-- ./simple_example -i 100000 --RD 9000 -P 512 -B 16 -E 256 -T 2 --selectivity 0.001 --workload_filename "workload/workload428.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log428 -->


./simple_example -i 100000 --RD 1 -P 512 -B 64 -E 64 -T 2 --selectivity 0.001 --workload_filename "workload/workload431.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log431
./simple_example -i 100000 --RD 5 -P 512 -B 64 -E 64 -T 2 --selectivity 0.001 --workload_filename "workload/workload432.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log432
./simple_example -i 100000 --RD 10 -P 512 -B 64 -E 64 -T 2 --selectivity 0.001 --workload_filename "workload/workload433.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log433
./simple_example -i 100000 --RD 50 -P 512 -B 64 -E 64 -T 2 --selectivity 0.001 --workload_filename "workload/workload434.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log434
./simple_example -i 100000 --RD 100 -P 512 -B 64 -E 64 -T 2 --selectivity 0.001 --workload_filename "workload/workload435.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log435
./simple_example -i 100000 --RD 500 -P 512 -B 64 -E 64 -T 2 --selectivity 0.001 --workload_filename "workload/workload436.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log436
./simple_example -i 100000 --RD 900 -P 512 -B 64 -E 64 -T 2 --selectivity 0.001 --workload_filename "workload/workload437.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log437
<!-- ./simple_example -i 100000 --RD 9000 -P 512 -B 64 -E 64 -T 2 --selectivity 0.001 --workload_filename "workload/workload438.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log438 -->


./simple_example -i 100000 --RD 1 -P 512 -B 256 -E 16 -T 2 --selectivity 0.001 --workload_filename "workload/workload441.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log441
./simple_example -i 100000 --RD 5 -P 512 -B 256 -E 16 -T 2 --selectivity 0.001 --workload_filename "workload/workload442.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log442
./simple_example -i 100000 --RD 10 -P 512 -B 256 -E 16 -T 2 --selectivity 0.001 --workload_filename "workload/workload443.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log443
./simple_example -i 100000 --RD 50 -P 512 -B 256 -E 16 -T 2 --selectivity 0.001 --workload_filename "workload/workload444.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log444
./simple_example -i 100000 --RD 100 -P 512 -B 256 -E 16 -T 2 --selectivity 0.001 --workload_filename "workload/workload445.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log445
./simple_example -i 100000 --RD 500 -P 512 -B 256 -E 16 -T 2 --selectivity 0.001 --workload_filename "workload/workload446.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log446
./simple_example -i 100000 --RD 900 -P 512 -B 256 -E 16 -T 2 --selectivity 0.001 --workload_filename "workload/workload447.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log447
<!-- ./simple_example -i 100000 --RD 9000 -P 512 -B 256 -E 16 -T 2 --selectivity 0.001 --workload_filename "workload/workload448.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log448 -->










./simple_example -i 100000 --RD 100 -P 512 -B 4 -E 1024 -T 2 --selectivity 0.0001 --workload_filename "workload/workload511.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log511
./simple_example -i 100000 --RD 100 -P 512 -B 4 -E 1024 -T 2 --selectivity 0.0005 --workload_filename "workload/workload512.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log512
./simple_example -i 100000 --RD 100 -P 512 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload513.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log513
./simple_example -i 100000 --RD 100 -P 512 -B 4 -E 1024 -T 2 --selectivity 0.005 --workload_filename "workload/workload514.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log514
./simple_example -i 100000 --RD 100 -P 512 -B 4 -E 1024 -T 2 --selectivity 0.009 --workload_filename "workload/workload515.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log515




./simple_example -i 100000 --RD 100 -P 512 -B 16 -E 256 -T 2 --selectivity 0.0001 --workload_filename "workload/workload521.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log521
./simple_example -i 100000 --RD 100 -P 512 -B 16 -E 256 -T 2 --selectivity 0.0005 --workload_filename "workload/workload522.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log522
./simple_example -i 100000 --RD 100 -P 512 -B 16 -E 256 -T 2 --selectivity 0.001 --workload_filename "workload/workload523.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log523
./simple_example -i 100000 --RD 100 -P 512 -B 16 -E 256 -T 2 --selectivity 0.005 --workload_filename "workload/workload524.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log524
./simple_example -i 100000 --RD 100 -P 512 -B 16 -E 256 -T 2 --selectivity 0.009 --workload_filename "workload/workload525.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log525


./simple_example -i 100000 --RD 100 -P 512 -B 64 -E 64 -T 2 --selectivity 0.0001 --workload_filename "workload/workload531.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log531
./simple_example -i 100000 --RD 100 -P 512 -B 64 -E 64 -T 2 --selectivity 0.0005 --workload_filename "workload/workload532.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log532
./simple_example -i 100000 --RD 100 -P 512 -B 64 -E 64 -T 2 --selectivity 0.001 --workload_filename "workload/workload533.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log533
./simple_example -i 100000 --RD 100 -P 512 -B 64 -E 64 -T 2 --selectivity 0.005 --workload_filename "workload/workload534.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log534
./simple_example -i 100000 --RD 100 -P 512 -B 64 -E 64 -T 2 --selectivity 0.009 --workload_filename "workload/workload535.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log535


./simple_example -i 100000 --RD 100 -P 512 -B 256 -E 16 -T 2 --selectivity 0.0001 --workload_filename "workload/workload541.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log541
./simple_example -i 100000 --RD 100 -P 512 -B 256 -E 16 -T 2 --selectivity 0.0005 --workload_filename "workload/workload542.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log542
./simple_example -i 100000 --RD 100 -P 512 -B 256 -E 16 -T 2 --selectivity 0.001 --workload_filename "workload/workload543.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log543
./simple_example -i 100000 --RD 100 -P 512 -B 256 -E 16 -T 2 --selectivity 0.005 --workload_filename "workload/workload544.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log544
./simple_example -i 100000 --RD 100 -P 512 -B 256 -E 16 -T 2 --selectivity 0.009 --workload_filename "workload/workload545.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log545














./simple_example -i 100000 --RD 10 -P 512 -B 4 -E 1024 -T 2 --selectivity 0.001 --workload_filename "workload/workload611.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log611
./simple_example -i 100000 --RD 10 -P 512 -B 4 -E 1024 -T 2 --selectivity 0.005 --workload_filename "workload/workload612.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log612
./simple_example -i 100000 --RD 10 -P 512 -B 4 -E 1024 -T 2 --selectivity 0.009 --workload_filename "workload/workload613.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log613
./simple_example -i 100000 --RD 10 -P 512 -B 4 -E 1024 -T 2 --selectivity 0.01 --workload_filename "workload/workload614.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log614
./simple_example -i 100000 --RD 10 -P 512 -B 4 -E 1024 -T 2 --selectivity 0.05 --workload_filename "workload/workload615.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log615
./simple_example -i 100000 --RD 10 -P 512 -B 4 -E 1024 -T 2 --selectivity 0.09 --workload_filename "workload/workload616.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log616


./simple_example -i 100000 --RD 10 -P 512 -B 16 -E 256 -T 2 --selectivity 0.001 --workload_filename "workload/workload621.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log621
./simple_example -i 100000 --RD 10 -P 512 -B 16 -E 256 -T 2 --selectivity 0.005 --workload_filename "workload/workload622.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log622
./simple_example -i 100000 --RD 10 -P 512 -B 16 -E 256 -T 2 --selectivity 0.009 --workload_filename "workload/workload623.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log623
./simple_example -i 100000 --RD 10 -P 512 -B 16 -E 256 -T 2 --selectivity 0.01 --workload_filename "workload/workload624.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log624
./simple_example -i 100000 --RD 10 -P 512 -B 16 -E 256 -T 2 --selectivity 0.05 --workload_filename "workload/workload625.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log625
./simple_example -i 100000 --RD 10 -P 512 -B 16 -E 256 -T 2 --selectivity 0.09 --workload_filename "workload/workload626.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log626


./simple_example -i 100000 --RD 10 -P 512 -B 64 -E 64 -T 2 --selectivity 0.001 --workload_filename "workload/workload631.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log631
./simple_example -i 100000 --RD 10 -P 512 -B 64 -E 64 -T 2 --selectivity 0.005 --workload_filename "workload/workload632.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log632
./simple_example -i 100000 --RD 10 -P 512 -B 64 -E 64 -T 2 --selectivity 0.009 --workload_filename "workload/workload633.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log633
./simple_example -i 100000 --RD 10 -P 512 -B 64 -E 64 -T 2 --selectivity 0.01 --workload_filename "workload/workload634.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log634
./simple_example -i 100000 --RD 10 -P 512 -B 64 -E 64 -T 2 --selectivity 0.05 --workload_filename "workload/workload635.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log635
./simple_example -i 100000 --RD 10 -P 512 -B 64 -E 64 -T 2 --selectivity 0.09 --workload_filename "workload/workload636.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log636


./simple_example -i 100000 --RD 10 -P 512 -B 256 -E 16 -T 2 --selectivity 0.001 --workload_filename "workload/workload641.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log641
./simple_example -i 100000 --RD 10 -P 512 -B 256 -E 16 -T 2 --selectivity 0.005 --workload_filename "workload/workload642.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log642
./simple_example -i 100000 --RD 10 -P 512 -B 256 -E 16 -T 2 --selectivity 0.009 --workload_filename "workload/workload643.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log643
./simple_example -i 100000 --RD 10 -P 512 -B 256 -E 16 -T 2 --selectivity 0.01 --workload_filename "workload/workload644.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log644
./simple_example -i 100000 --RD 10 -P 512 -B 256 -E 16 -T 2 --selectivity 0.05 --workload_filename "workload/workload645.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log645
./simple_example -i 100000 --RD 10 -P 512 -B 256 -E 16 -T 2 --selectivity 0.09 --workload_filename "workload/workload646.txt" --insert_before_range_delete 0.9 --gen_workload 0 > log646












