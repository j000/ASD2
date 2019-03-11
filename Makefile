# Copyright (C) 2019 Jarosław Rymut
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>
SHELL := /bin/sh

# pliki
# HELPERS nie tworzą plików wykonywalnych
TESTS := $(wildcard *test.cpp)
GENERATORS := $(wildcard gen*.cpp)
HELPERS := $(TESTS) $(GENERATORS)
FILES := $(filter-out $(HELPERS),$(wildcard *.cpp))

####################
ifneq (,$(wildcard zestaw.mk))
include zestaw.mk
endif

####################
# numer zestawu
zestaw := $(notdir $(lastword $(CURDIR)))
# nazwa spakowanego pliku
tar := Rymut_$(zestaw).tar.gz

####################
# flagi
CFLAGS := -O3 -march=native -pipe -Wall -Wextra -Weffc++ -Wconversion -Wpedantic
# PIC > PIE > no-pie
CFLAGS += -fPIC
# and for linker:
LDFLAGS += -pie
# ścieżka do bibliotek
LDLIBS += -Wl,-R.
# link time optimization
CFLAGS += -flto
# debugowanie
# CFLAGS += -ggdb3
# standardy
CXXFLAGS := -std=c++17 $(CFLAGS)
CFLAGS += -std=c11

CPPFLAGS := -MMD -MP

####################
# private jest dostępne od GNU make 3.82
ifneq (3.82,$(firstword $(sort $(MAKE_VERSION) 3.82)))
  $(error "*** PLEASE USE AT LEAST GNU MAKE 3.82 FROM 2010 ***")
endif

####################
# wszystko zależy od Makefile
$(FILES:.cpp=.o): Makefile
$(HELPERS:.cpp=.o): Makefile
ifneq (,$(wildcard zestaw.mk))
$(FILES:.cpp=.o): zestaw.mk
$(HELPERS:.cpp=.o): zestaw.mk
endif

# make może automatycznie usunąć te pliki
.INTERMEDIATE: $(HELPERS:.cpp=.o) $(FILES:.cpp=.o)

####################
# domyślne cele
.DEFAULT_GOAL := all
.PHONY: all
all: $(FILES:.cpp=.x)

.PHONY: clean
clean: mostlyclean
	$(RM) $(FILES:.cpp=.x) $(tar) lib*

.PHONY: mostlyclean
mostlyclean:
	$(RM) $(HELPERS:.cpp=.x) $(FILES:.cpp=.d) $(HELPERS:.cpp=.d)

.PHONY: tar
tar:
# poprawka na błąd 'file changed as we read it'
	@[ -f $(tar) ] || touch $(tar)
	tar -caf ./$(tar) \
		--dereference \
		--exclude=*.d \
		--exclude=*.o \
		--exclude=*.x \
		--exclude=*.tar.* \
		-C .. \
		$(zestaw)

# kopia wbudowanej reguły, z nowym rozszerzeniem
%.x: %.o
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) $(OUTPUT_OPTION)

lib%.so: private LDFLAGS += -shared
lib%.so: %.o
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) $(OUTPUT_OPTION)

%.d: %.cpp
	$(CXX) $(CPPFLAGS:MD=M) -MF $@ $<

ifneq (,$(wildcard $(FILES:.cpp=.d)))
-include $(wildcard $(FILES:.cpp=.d))
endif
ifneq (,$(wildcard $(HELPERS:.cpp=.d)))
-include $(wildcard $(HELPERS:.cpp=.d))
endif

# usuń domyślne reguły
%: %.o
%: %.cpp
.SUFFIXES:
.SUFFIXES: .o .cpp

####################
# inne cele
.PHONY: check
check:
	@clang-tidy \
		-checks="bugprone-*,\
	cppcoreguidelines-*,\
	clang-analyzer-*,\
	misc-*,\
	modernize-*,\
	performance-*,\
	portability-*,\
	readability-*,\
	-clang-analyzer-cplusplus.NewDeleteLeaks,\
	-misc-unused-parameters,\
	-readability-braces-around-statements,\
	-readability-named-parameter,\
	-readability-implicit-bool-conversion,\
	-cppcoreguidelines-no-malloc,\
	-cppcoreguidelines-owning-memory,\
	-cppcoreguidelines-pro-bounds-*"\
		-header-filter='.*,-rapidcheck.hpp' \
		-config="{CheckOptions: [ \
			{ key: readability-identifier-naming.NamespaceCase, value: CamelCase }, \
			{ key: readability-identifier-naming.ClassCase, value: CamelCase  }, \
			{ key: readability-identifier-naming.StructCase, value: CamelCase  }, \
			{ key: readability-identifier-naming.FunctionCase, value: lower_case }, \
			{ key: readability-identifier-naming.VariableCase, value: lower_case }, \
			{ key: readability-identifier-naming.GlobalConstantCase, value: UPPER_CASE } \
		]}" \
		$(FILES) \
		-- $(filter-out -flto,$(CXXFLAGS))
# lto does something bad to clang-tidy

.PHONY: format
format:
	@clang-format -i -style=file $(FILES) $(HELPERS)

####################
# testy
N := 350

.PHONY: test
test:
	@printf "\033"'[32m   /`"'\''-,__'"\n"
	@printf '   \/\)`   `'\''-.'"\n"
	@printf '  // \ .--.\   '\''.'"\n"
	@printf ' //|-.  \_o `-.  \---._'"\n"
	@printf ' || \_o\  _.-.\`'\''-'\''    `-.'"\n"
	@printf ' || |\.-'\''`    |           `.'"\n"
	@printf ' || | \  \    |             `\\'"\n"
	@printf ' \| /  \ ,\'\'' /                \\'"\n"
	@printf '  `'\''    `---'\''                  ;'"\n"
	@printf '         `))          .-'\''      |'"\n"
	@printf '      .-.// .-.     .'\''        ,;=D'"\n"
	@printf '     /  // /   \  .'\''          ||'"\n"
	@printf '    |..-'\'' |     '\''-'\''          //'"\n"
	@printf '    ((    \         .===. _,//                    All tests are successfull!'"\n"
	@printf '     '\''`'\''--`'\''---'\'''\'''\'',--\_/-;-'\''`'"\n"
	@printf 'jgs                `~/^\`'"\n"
	@printf '                    '\''==='\'"\033"'[0m'"\n"

.PHONY: FORCE
FORCE:

test.gen%.txt: gen%.x FORCE
	./$(filter %.x,$^) $(N) >$@

test.%.txt: %.x
	/usr/bin/time -f "%C: %E (mem: %MKB, cpu: %P)" ./$(filter %.x,$^) <$(filter test.gen%,$^) >$@

test_%: test.%.txt
	@diff -sq $^

%test: %test.x
	./$^

test: $(subst .cpp,,$(TESTS))
