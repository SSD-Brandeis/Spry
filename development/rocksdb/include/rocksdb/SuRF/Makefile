SOURCES := $(wildcard ./include/*.cc)
OBJECTS := $(SOURCES:.cc=.o)



all: 
	# remove simple_example if it exists
	rm -f simple_example
	make simple_example


simple_example: ${OBJECTS}
	#g++ -mpopcnt -std=c++11 -g -DDEBUG_LOUDS_SPARSE -DDEBUG_LOUDS_DENSE -DDEBUG_BUILDER simple_example_self.cpp -o simple_example
	#cat ${OBJECTS}
	g++ -mpopcnt -std=c++11 -g simple_example_self.cpp ${OBJECTS} -o simple_example


clean:
	rm -f $(OBJECTS) out

