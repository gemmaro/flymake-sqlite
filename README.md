# Flymake SQLite

This is a Flymake backend for SQLite.

It can parse and check the SQL files; basically it checks the syntax
and, if the database filename is specified, it checks whether tables
and columns do exist or not.

See also the commentary section and docstrings in the package file.

## Install

It requires SQLite C interface (at least version 3.38.0 or later[^1]) to
build the dynamic module.

[^1]: Because this package requires `sqlite3_error_offset()`
    introduced in the [3.38.0 on 2022-02-22][3-38-0].

To build the module, run `make`.  If you use GCC, `make CC=gcc` to
compile the dynamic module.  Some platform might requires additional
adjustments, so please refer to the `Makefile`.

[3-38-0]: https://sqlite.org/releaselog/3_38_0.html#:~:text=Added%20the%20sqlite3%5Ferror%5Foffset%28%29%20interface%2C%20which%20can%20sometimes%20help%20to%20localize%20an%20SQL%20error%20to%20a%20specific%20character%20in%20the%20input%20SQL%20text%2C%20so%20that%20applications%20can%20provide%20better%20error%20messages%2E

## Change Log

* 0.1.0 - 2026-06-25: Initial release.

## Development

There are some fixture files under the `fixtures` directory.  To
create a dummy database for the okay case (`fixtures/ok1.sql`), run
`./bin/setup`.  To check if the package works, run `./bin/start` and
open fixtures.

Currently no unit tests available.  I manually test these in addition
to the basic check with `make check`:

* Open fixture files for expected errors/no-errors.
* Try edit a bit for okay files.  Sometimes this causes unexpected
  error reporting.
* Try to edit & save files.

If you are using Guix and Direnv, run `direnv allow .` to install
necessary packages.

References: [SQLite C Interface][s], [My previous attempt to write a
Flymake backend][pc]

[pc]: https://github.com/flymake/emacs-flymake-perlcritic
[s]: https://sqlite.org/c3ref/funclist.html

## License

Copyright (C) 2026  gemmaro

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
