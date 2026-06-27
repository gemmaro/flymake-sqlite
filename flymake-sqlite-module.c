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

#include <emacs-module.h>
#include <sqlite3.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int plugin_is_GPL_compatible;

static emacs_value nil;

static char *
copy_emacs_string (emacs_env *env, emacs_value val, ptrdiff_t *len)
{
  /* Ref. (elisp) Module Values */
  if (!env->copy_string_contents (env, val, NULL, len))
    return NULL;

  char *buf = malloc (*len);
  if (!buf)
    return NULL;
  if (!env->copy_string_contents (env, val, buf, len))
    goto free_and_fail;
  (*len)--; /* length contained the last null character */
  return buf;

free_and_fail:
  free (buf);
  return NULL;
}

static ptrdiff_t
offset_to_codepoints (const char *s, ptrdiff_t offset)
{
  ptrdiff_t codepoints = 0;
  for (ptrdiff_t i = 0; i < offset; i++)
    {
      /* 0xC0 : 1100 0000
       * 0x80 : 1000 0000
       * ----------------
       *        0000 0000 : ASCII
       *        1100 0000 : 2 bytes char head
       *        1110 0000 : 3 bytes char head
       *        1111 0000 : 4 bytes char head
       *        1000 0000 : continuation byte
       */
      if ((s[i] & 0xC0) != 0x80)
        codepoints++;
    }
  return codepoints;
}

static emacs_value
cons (emacs_env *env, emacs_value car, emacs_value cdr)
{
  emacs_value args[2] = { car, cdr };
  return env->funcall (env, env->intern (env, "cons"), 2, args);
}

static emacs_value
from_cstr (emacs_env *env, const char *str)
{
  return env->make_string (env, str, strlen (str));
}

static emacs_value
check (emacs_env *env, ptrdiff_t nargs, emacs_value args[], void *data)
{
  const emacs_value sql_val = args[0], dbfile_val = args[1];
  (void)data;

  /* This cannot be in the module initialization for some reason. */
  nil = env->intern (env, "nil");

  char *sql = NULL, *dbfile = NULL;
  ptrdiff_t sql_len = 0;
  emacs_value result = nil;
  sqlite3 *db = NULL;
  sql = copy_emacs_string (env, sql_val, &sql_len);
  if (!sql)
    goto done;

  ptrdiff_t dbfile_len = 0;
  if (nargs > 1 && !env->eq (env, dbfile_val, nil))
    dbfile = copy_emacs_string (env, dbfile_val, &dbfile_len);

  int oresult;
  if (dbfile)
    oresult = sqlite3_open_v2 (dbfile, &db, SQLITE_OPEN_READONLY, NULL);
  else
    oresult = sqlite3_open_v2 (
        ":memory:", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
  if (oresult != SQLITE_OK)
    goto failed_to_open;

  ptrdiff_t offset = 0;
  while (offset < sql_len)
    {
      /* Skip whitespace as in SQLite Tokenizer Requirements[1].
       *
       * > One of these five characters: u0009, u000a, u000c, u000d, or u0020
       *
       * u0009 | \t
       * u000a | \n
       * u000c | \f
       * u000d | \r
       * u0020 | ' '
       *
       * [1] https://sqlite.org/draft/tokenreq.html
       */
      uint8_t c;
      while ((c = *(sql + offset)))
        {
          if (c == '\t' || c == '\n' || c == '\f' || c == '\r' || c == ' ')
            offset++;
          else
            break;
        }

      sqlite3_stmt *stmt = NULL;
      const char *tail = NULL;

      /* Parse only and do not actually execute it. */
      const int presult = sqlite3_prepare_v2 (db, sql + offset,
                                              sql_len - offset, &stmt, &tail);
      if (presult == SQLITE_OK)
        {
          sqlite3_finalize (stmt);
          offset = tail - sql;
          continue;
        }
      const char *err_msg = sqlite3_errmsg (db);
      const int err_offset = sqlite3_error_offset (db);
      long beg, end;
      const bool is_logic_err = err_offset < 0;
      if (is_logic_err)
        {
          beg = offset;
          if (tail)
            end = tail - sql;
          else
            end = -1;
        }
      else /* syntax error */
        {
          beg = offset + offset_to_codepoints (sql, err_offset);
          end = beg + 1;
        }
      result = cons (env,
                     cons (env, env->make_integer (env, beg),
                           cons (env, env->make_integer (env, end),
                                 from_cstr (env, err_msg))),
                     result);
      if (is_logic_err && tail)
        {
          offset = tail - sql;
          continue;
        }
      break;
    }
  goto done;

failed_to_open:
  {
    char msg[1 << 9];
    snprintf (msg, sizeof (msg), "failed to open database (filename=%s): %s",
              dbfile, db ? sqlite3_errmsg (db) : sqlite3_errstr (oresult));
    result = cons (env, env->make_integer (env, 0), from_cstr (env, msg));
  }

done:
  if (db)
    sqlite3_close (db);
  free (sql);
  free (dbfile);
  return result;
}

int
emacs_module_init (struct emacs_runtime *ert)
{
  emacs_env *env = ert->get_environment (ert);

  emacs_value fargs[] = {
    env->intern (env, "flymake-sqlite--check"),
    env->make_function (
        env, 1, 2, check,
        "Check STRING with the SQLite parser.\n"
        "If DB-FILE is given, SQLite checks against the database.\n"
        "Otherwise it uses an empty in-memory database.\n"
        "Return found errors as (BEG . END . MESSAGE) if any, or return nil.\n"
        "Note that BEG and END here are numbers of Unicode codepoints offset "
        "based on UTF-8.\n"
        "END can be negative if no error offset nor first byte of the end of "
        "statement cannot be detected.\n"
        "\n(fn STRING &optional DB-FILE)",
        NULL)
  };
  env->funcall (env, env->intern (env, "fset"), 2, fargs);

  emacs_value pargs[] = { env->intern (env, "flymake-sqlite-module") };
  env->funcall (env, env->intern (env, "provide"), 1, pargs);

  return 0;
}
