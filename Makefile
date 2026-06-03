.PHONY: debug_sym clean

check_symbols:
	python check_symbols.py

debug: format
	cmake -DCMAKE_BUILD_TYPE=Debug -DDEBUG=TRUE -S . -B build && $(MAKE) -C build

debug_sym: format check_symbols
	cmake -DCMAKE_BUILD_TYPE=Debug -DDEBUG=TRUE -S . -B build && $(MAKE) -C build

release: clean format
	cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DDEBUG=FALSE -S . -B build && $(MAKE) -C build
	python ./make-release/release.py

format:
	clear
	find src include lib/custom -name "*.c*" -o -name "*.h*" | xargs clang-format -i
	find src include -name "*.c*" -o -name "*.h*"  | xargs python3 config/check_includes.py --fix


clean:
	rm -rf build
