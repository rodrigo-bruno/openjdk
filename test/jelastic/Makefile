all: Test.class

Test.class: Test.java
	javac Test.java

run-release: Test.class
	./launcher/java Test

run-gdb: Test.class
	./launcher/java-gdb Test

clean:
	rm -f *.class *.log core
