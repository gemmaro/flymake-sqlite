# Copyright (C) 2026  gemmaro
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

CC      ?= cc
CFLAGS  ?= -O2 -Wall -Wextra -fPIC

SQLITE_LIBS := -lsqlite3
EXT			:= $(shell emacs --batch --eval '(princ module-file-suffix)')
LDFLAGS		:= -shared
MODULE		:= flymake-sqlite-module$(EXT)

all: $(MODULE)

$(MODULE): flymake-sqlite-module.c
	$(CC) $(CFLAGS) $(LDFLAGS) \
		-o $@ \
		$< $(SQLITE_LIBS)

check: $(MODULE)
	./bin/check

clean:
	rm -f $(MODULE)

.PHONY: all check clean
