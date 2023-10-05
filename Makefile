bin/seml: seml.c
	clang -o $@ -Os $^

clean:
	rm bin/seml
