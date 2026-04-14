torch: src/torch.c src/generation.h src/parser.h src/token.h
	mkdir -p build
	cc src/torch.c src/generation.h src/parser.h src/token.h -o build/torch

ember: src/ember/lexer.h src/ember/lexer.c src/ember/main.c
	mkdir -p build
	cc src/ember/lexer.c src/ember/main.c -o build/ember

clean:
	rm -rf build/
