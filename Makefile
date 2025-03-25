all:
	g++ src/lexer.cpp src/tokens.cpp src/main.cpp -o bin/axscript

clean:
	rm -f bin/axscript