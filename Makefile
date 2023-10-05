bin/seml: seml.c
	clang -o $@ -Ofast $^

clean:
	rm bin/seml
