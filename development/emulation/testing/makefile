SRC0=$(wildcard ../*.cc)	
SRC1=$(filter-out ../emu_runner.cc, $(SRC0) )	
SRC2=$(wildcard ../tree_builder/*.cc)
CXX = g++
CXX_FLAGS = -std=c++1y -O0 -ggdb -g3  #-lSegFault
I_PATH = -I ./ -I ../tree_builder

test_insert_entry_list: test_insert_entry_list.cpp $(SRC1) $(SRC2) 
	echo $@
	$(CXX) $(CXX_FLAGS) -o$@  test_insert_entry_list.cpp $(SRC1) $(SRC2)

test_compact: test_compact.cpp $(SRC1) $(SRC2) 
	echo $@
	$(CXX) $(CXX_FLAGS) -o$@  test_compact.cpp $(SRC1) $(SRC2)
