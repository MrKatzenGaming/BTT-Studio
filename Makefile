.PHONY: debug_sym clean

check_symbols:
	python check_symbols.py

debug: format
	cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build && $(MAKE) -C build

debug_sym: format check_symbols
	cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build && $(MAKE) -C build

release: clean format
	cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build && ninja -C build
	python ./make-Release/release.py

format:
	find ./src -name "*.*" | xargs clang-format -i

clean:
	rm -r build || true
