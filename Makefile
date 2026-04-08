torch: src/torch.c src/parser.h src/token.h
	mkdir -p build
	cc src/torch.c src/parser.h src/token.h -o build/torch
