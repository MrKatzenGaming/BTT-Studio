.PHONY: debug_sym clean

PROJNAME ?= BTT-Studio

CONTENTPATH := package/atmosphere/contents/0100000000010000

CMAKE_FLAGS = -G "Ninja"\
			  -DCMAKE_C_COMPILER_LAUNCHER=ccache \
			  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache 

debug: format
	cmake $(CMAKE_FLAGS) -DCMAKE_BUILD_TYPE=Debug -DDEBUG=TRUE -S . -B build/debug
	ln -sf build/debug/compile_commands.json .
	cmake --build build/debug

release: clean format
	cmake $(CMAKE_FLAGS) -DCMAKE_BUILD_TYPE=RelWithDebInfo -DDEBUG=FALSE -S . -B build/release
	ln -sf build/release/compile_commands.json .
	cmake --build build/release
	$(MAKE) package

check_symbols:
	python check_symbols.py

format:
	-clear
	python3 config/check_includes.py
	find src include lib/custom  -type f -regex ".*\.[ch]p*" -exec clang-format -i {} '+'

clean:
	rm -rf build package compile_commands.json

package:
	@echo -e "\n\e[34m\e[1m═══════════Packaging Files══════════\e[0m\e[36m\n"

	@echo - Creating exefs folders
	@mkdir -p $(CONTENTPATH)/exefs/

	@echo - Copying subsdk binaries
	@cp build/release/$(PROJNAME).nso $(CONTENTPATH)/exefs/subsdk4 

	@echo - Copying npdm file
	@cp build/release/main.npdm $(CONTENTPATH)/exefs/main.npdm 

	@echo - Copying NSS debug symbols
	@cp build/release/$(PROJNAME).nss package/$(PROJNAME).nss
	
	@echo - Copying romfs data
	@cp -R romfs/ $(CONTENTPATH)

	@echo - Ziping files
	@zip -rq package/$(PROJNAME)-Switch.zip package/atmosphere
	@mv package/atmosphere package/$(PROJNAME)
	@zip -rq package/$(PROJNAME)-SimpleModManager.zip package/$(PROJNAME)

	@echo -e "\n\e[32m════════════════════════════════════"
	@echo -e "\e[1m         ✓ Build complete!\e[0m"
	@echo -e "\e[32m════════════════════════════════════\e[0m"