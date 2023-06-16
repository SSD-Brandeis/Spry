
ifneq ($(USE_RTTI), 1)
	CXXFLAGS += -fno-rtti
endif

CXX = g++
# CXX_FLAGS = -std=c++1y -O0 -ggdb -g3
CXX_FLAGS = -std=c++1y -O0 -ggdb -g3 #-lSegFault
I_PATH = -I . -I ./tree_builder


all: emu_runner 

# CXXFLAGS += -std=c++1y
# $(info PRINT FLAGS $(CXXFLAGS))
# PLATFORM_CXXFLAGS := $(filter-out -std=c++11,$(PLATFORM_CXXFLAGS))
# $(info PRINT FLAGS $(PLATFORM_CXXFLAGS))
# PLATFORM_CXXFLAGS += -std=c++1y
# $(info PRINT FLAGS $(PLATFORM_CXXFLAGS))
# CFLAGS += -std=c++1y
# $(info PRINT FLAGS $(CFLAGS))

SRC=$(wildcard *.cc)	
SRC2=$(wildcard */*.cc)	

#emu_runner: emu_runner.cc emu_environment.cc tree_builder.cc workload_executor.cc workload_generator.cc query_runner.cc instruction_decoder.cc
#	$(CXX) $(CXX_FLAGS)  $@.cc -o$@ emu_environment.cc tree_builder.cc workload_executor.cc workload_generator.cc query_runner.cc instruction_decoder.cc

emu_runner: emu_runner.cc $(SRC) $(SRC2)
	$(CXX) $(CXX_FLAGS)    -o$@ $(I_PATH) $(SRC) $(SRC2)

test_compact: test_compact.cpp $(SRC) $(SRC2)
	$(CXX) $(CXX_FLAGS)    -o$@ $(I_PATH) $(SRC) $(SRC2)

clean:
	rm -rf emu_runner  


