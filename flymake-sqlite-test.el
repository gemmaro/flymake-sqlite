;;; flymake-sqlite-test.el --- Tests for flymake-sqlite  -*- lexical-binding: t; -*-

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

;; This test can be run by "./bin/check".

;;; Code:

(require 'ert)
(require 'flymake-sqlite-module)

(ert-deftest flymake-sqlite-test-check ()
  "Test check."
  (should (equal
           (flymake-sqlite--check "SELECT * FROM do_not_exist;")
           ;;                      012345678901234567890123456
           ;;                                1         2
           '((:warning 0 27 . "no such table: do_not_exist"))))
  (should (equal
           (flymake-sqlite--check "SELECT 1 +;")
           ;;                      01234567890
           ;;                                1
           '((:error 10 11 . "near \";\": syntax error"))))
  (should (equal
           ;;                                1         2
           ;;                      012345678901234567890123456
           (flymake-sqlite--check "-- This is multiline case.
SELECT ??;")
;;90123456
;; 3
           '((:error 35 36 . "near \"?\": syntax error"))))
  (should (equal
           ;;                      0123456789
           (flymake-sqlite--check "SELECT 1;
SELECT ??;")
;;23456789
           '((:error 18 19 . "near \"?\": syntax error"))))
  (should (equal
           ;;                                1
           ;;                      01234567890
           (flymake-sqlite--check "SELECT ??;
SELECT !!;")
;;34567890
;;       2
           '((:error 8 9 . "near \"?\": syntax error"))))
  (should (equal
           ;;                                1         2         3         4         5
           ;;                      012345678901234567890123456789012345678901234567890123456789
           (flymake-sqlite--check "INSERT INTO do_not_exist_1 VALUES (1, 'Alice'), (2, 'Bob');
INSERT INTO do_not_exist_2 VALUES (1, 'foo'), (2, 'bar');")
;;2345678901234567890123456789012345678901234567890123456
;;        7         8         9         0         1
           '((:warning 60 117 . "no such table: do_not_exist_2")
	         (:warning 0 59 . "no such table: do_not_exist_1")))))

(provide 'flymake-sqlite-test)
;;; flymake-sqlite-test.el ends here
