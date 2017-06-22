all: Test.class

Test.class: Test.java
	javac Test.java

CmdTest.class: CmdTest.java
	javac CmdTest.java

run-release: Test.class
	./launcher/java Test

run-gdb: Test.class
	./launcher/java-gdb Test

run-release-cmd: CmdTest.class
	./launcher/java CmdTest < CmdTest.input

run-gdb-cmd: CmdTest.class
	./launcher/java-gdb CmdTest < CmdTest.input

run-stats:
	./launcher/jvm-stats Test

run-gc:
	./launcher/jvm-gc Test

run-mmap: TestMMap.cpp
	g++ -c TestMMap.cpp
	g++ -o TestMMap TestMMap.cpp
	./TestMMap

clean:
	rm -f *.class *.log core *.o TestMMap
