.PHONY: debug_sym clean

check_symbols:
	python check_symbols.py

debug:
	cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build && $(MAKE) -C build

debug_sym: check_symbols
	cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build && $(MAKE) -C build

release: clean
	cmake -DCMAKE_BUILD_TYPE=Release -S . -B build && ninja -C build

clean:
	rm -r build || true
