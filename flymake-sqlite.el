;;; flymake-sqlite.el --- Flymake backend for SQLite -*- lexical-binding: t; -*-

;; Copyright (C) 2026  gemmaro

;; Author: gemmaro <gemmaro.dev@gmail.com>
;; Package-Requires: ((emacs "30.2") (flymake "1.4.5"))
;; Keywords: languages, sql, flymake

;; This program is free software: you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.
;;
;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with this program.  If not, see <https://www.gnu.org/licenses/>.

;;; Commentary:

;; This is a Flymake backend for SQLite (version 3).
;;
;; You can optionally set `flymake-sqlite-database-filename' variable
;; via file local variables, directory local variables, or custom
;; variables.  Please see the docstring for details.
;;
;; You can hook this backend to the SQL mode as follows:
;;
;;   (add-hook 'sql-mode-hook #'flymake-sqlite-setup)
;;   (add-hook 'sql-mode-hook #'flymake-mode)
;;
;; Troubleshooting:
;;
;; * "Dynamic module not found" --- Please make sure you have compiled
;;   the dynamic module and locate it under the
;;   `flymake-sqlite-module-path'.

;;; Code:

(require 'flymake)

(defgroup flymake-sqlite nil
  "Flymake checker for SQL files backed by SQLite's own parser."
  :group 'flymake
  :prefix "flymake-sqlite-")

(defcustom flymake-sqlite-database-filename nil
  "Database filename which is used for SQL files.

If unset, an empty in-memory database will be used, so unknown tables
might cause errors, for instance.  The path of the database filename
should be relative to the SQL file."
  :type '(choice (const :tag "unspecified" nil)
                 (file :tag "database filename"))
  :group 'flymake-sqlite)

(defcustom flymake-sqlite-module-path
  (expand-file-name
   "flymake-sqlite-module.so"
   (file-name-directory load-file-name))
  "Built dynamic module file path for this package."
  :type '(file :tag "module filepath")
  :group 'flymake-sqlite)

(defun flymake-sqlite--ensure-module ()
  "Load the dynamic module if not yet."
  (unless (fboundp 'flymake-sqlite--check)
    (unless (file-exists-p flymake-sqlite-module-path)
      (error "flymake-sqlite: Dynamic module not found: %s"
             flymake-sqlite-module-path))
    (module-load flymake-sqlite-module-path)))

(defun flymake-sqlite--resolve-database ()
  "Resolve the database filename to be absolute according to the buffer."
  (when (and flymake-sqlite-database-filename
             (stringp flymake-sqlite-database-filename))
    (expand-file-name flymake-sqlite-database-filename default-directory)))

(declare-function flymake-sqlite--check "flymake-sqlite-module")
(defun flymake-sqlite-checker (report-fn &rest _args)
  "Flymake backend to check the SQL.
The diagnostic results are passed to REPORT-FN."
  (flymake-sqlite--ensure-module)
  (funcall report-fn
           (mapcar (pcase-lambda (`(,offset . ,message))
                     (let* ((source (current-buffer))
                            (beg (+ (point-min)  offset))
                            (end (min (1+ beg) (point-max))))
                       (flymake-make-diagnostic source beg end
                                                :error message)))
                   (condition-case err
                       (flymake-sqlite--check
                        (buffer-substring-no-properties (point-min) (point-max))
                        (flymake-sqlite--resolve-database))
                     (error
                      `((1 . ,(format "flymake-sqlite: %s"
                                      (error-message-string err)))))))))

;;;###autoload
(defun flymake-sqlite-setup ()
  "Enable this backend in the current buffer."
  (add-hook 'flymake-diagnostic-functions #'flymake-sqlite-checker nil t))

(provide 'flymake-sqlite)
;;; flymake-sqlite.el ends here
