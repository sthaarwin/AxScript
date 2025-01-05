all:
	g++ src/lexer.cpp src/tokens.cpp src/main.cpp -o src/output/axscript

clean:
	rm -f src/output/axscript