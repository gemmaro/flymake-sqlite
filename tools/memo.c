/*
 * Copyright (C) 2026  gemmaro
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <sqlite3.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

int
main (void)
{
  sqlite3 *db = NULL;
  int open_result = sqlite3_open_v2 (
      ":memory:", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
  assert (open_result == SQLITE_OK);

  sqlite3_stmt *stmt = NULL;
  const char *tail = NULL;

  const char *query = "SELECT * FROM do_not_exist";
  const int prepare_result
      = sqlite3_prepare_v2 (db, query, strlen (query), &stmt, &tail);
  if (prepare_result == SQLITE_OK)
    {
      fprintf (stderr, "[info] prepared.\n");
      sqlite3_finalize (stmt);
    }
  else
    {
      const int offset = sqlite3_error_offset (db);
      fprintf (stderr, "[offset] %d\n", offset);

      const char *msg = sqlite3_errmsg (db);
      fprintf (stderr, "[msg] %s\n", msg);

      const char *str = sqlite3_errstr (prepare_result);
      fprintf (stderr, "[str] %s\n", str);
    }
  sqlite3_close (db);
}
