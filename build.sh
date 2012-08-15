JAVA_HOME=/usr/lib/jvm/java-7-openjdk-amd64 # Ununtu 12.04 64-bit
TIGHTDB_JAVA_HOME="$HOME/tightdb_java2"


# Build libtightdb-jni.so
cd "$TIGHTDB_JAVA_HOME/tightdb_jni/src" || exit 1
make clean || exit 1
make -j8 EXTRA_CFLAGS="-I\"$JAVA_HOME/include\"" || exit 1

# Build tightdb.jar
cd "$TIGHTDB_JAVA_HOME/src/main/java" || exit 1
find com/ -type f -name '*.class' -delete || exit 1
export CLASSPATH=/usr/share/java/commons-io.jar:/usr/share/java/commons-lang.jar:/usr/share/java/freemarker.jar:.
javac $(find com/ -type f -name '*.java' | fgrep -v /doc/ | fgrep -v /example/) || exit 1
jar cf tightdb.jar $(find com/ -type f -name '*.class') || exit 1

# Build and run example
cd "$TIGHTDB_JAVA_HOME/tightdb-example/src/main/java" || exit 1
find com/ -type f -name '*.class' -delete || exit 1
export CLASSPATH="$TIGHTDB_JAVA_HOME/src/main/java/tightdb.jar:."
javac com/tightdb/example/Example.java com/tightdb/example/generated/*.java || exit 1
java -Djava.library.path="$TIGHTDB_JAVA_HOME/tightdb_jni/src" com.tightdb.example.Example || exit 1

# Build and run test suite
cd "$TIGHTDB_JAVA_HOME/src/test/java" || exit 1
find com/ -type f -name '*.class' -delete || exit 1
test_sources="$(find * -type f -name '*Test.java')"
test_classes="$(echo "$test_sources" | sed 's/\.java$/.class/')"
export CLASSPATH="$TIGHTDB_JAVA_HOME/src/main/java/tightdb.jar:$TIGHTDB_JAVA_HOME/tightdb-example/src/main/java:/usr/share/java/testng.jar:/usr/share/java/qdox.jar:/usr/share/java/bsh.jar:."
javac $test_sources || exit 1
java -Djava.library.path="$TIGHTDB_JAVA_HOME/tightdb_jni/src" org.testng.TestNG -d "$TIGHTDB_JAVA_HOME/test_output" -testclass $test_classes || exit 1
