all: run-release-cmd

CmdTest.class: CmdTest.java
	javac CmdTest.java

run-release-cmd: CmdTest.class
	./launcher/java CmdTest < CmdTest.input1
	./launcher/java CmdTest < CmdTest.input2

run-gdb-cmd: CmdTest.class
	./launcher/java-gdb CmdTest < CmdTest.input1

run-mmap: TestMMap.cpp
	g++ -c TestMMap.cpp
	g++ -o TestMMap TestMMap.cpp
	./TestMMap

clean:
	rm -f *.class *.log core *.o TestMMap
