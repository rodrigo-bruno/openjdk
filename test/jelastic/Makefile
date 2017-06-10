all: Test.class

Test.class: Test.java
	javac Test.java

run-release: Test.class
	./launcher/java Test

run-gdb: Test.class
	./launcher/java-gdb Test

run-stats:
	./launcher/jvm-stats Test

run-gc:
	./launcher/jvm-gc Test

clean:
	rm -f *.class *.log core
