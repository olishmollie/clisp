SOURCEPATH=src
CLASSPATH=dist
EXECUTABLE=jfig.Fig

build: src/jfig/*.java
	javac -sourcepath $(SOURCEPATH) -d $(CLASSPATH) $^

run:
	java -classpath $(CLASSPATH) $(EXECUTABLE) $(ARGS)

