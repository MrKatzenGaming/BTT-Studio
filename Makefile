.PHONY: debug_sym clean

check_symbols:
	python check_symbols.py

debug:
	cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build && $(MAKE) -C build

debug_sym: check_symbols
	cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build && $(MAKE) -C build

release: clean
	cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build && ninja -C build
	python ./make-Release/release.py

clean:
	rm -r build || true
