SRC0=$(wildcard ../*.cc)	
SRC00=$(wildcard ./*.cc)
SRC01=$(wildcard ./*.h)	
SRC1=$(filter-out ../emu_runner.cc, $(SRC0) )	
SRC2=$(wildcard ../tree_builder/*.cc)
CXX = g++
CXX_FLAGS = -std=c++1y -O0 -ggdb -g3 -lSegFault
CXX_DEBUG_FLAG = -D DEBUG
I_PATH = -I ./ -I ../tree_builder
 
test_perlevel_range_delete_filter: test_perlevel_range_delete_filter.cpp $(SRC1) $(SRC2) $(SRC00) $(SRC01)
	echo $@
	$(CXX) $(CXX_FLAGS) $(CXX_DEBUG_FLAG) -o$@  test_perlevel_range_delete_filter.cpp $(SRC1) $(SRC2)

