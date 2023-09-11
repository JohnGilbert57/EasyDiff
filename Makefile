CXX=g++
CPPFLAGS=-Wall -Werror

TARGET=ediff
SRCS=$(TARGET).cc

all: $(TARGET) testfiles

$(TARGET): $(SRCS) 

# make sure that the testing files are all executable
testfiles:
	chmod 700 ./test.[0-9] ./test.[0-9][0-9]

tests: test
test: $(TARGET) testfiles
	-./test.0
	-./test.1
	-./test.2
	-./test.3
	-./test.4
	-./test.5
	-./test.6
	-./test.7
	-./test.8
	-./test.9
	-./test.10
	-./test.11
	-./test.12
	-./test.13

clean:
	rm -f $(TARGET) *.o test.*.* .unreadable_test_file 
