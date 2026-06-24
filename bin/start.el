(require 'flymake-sqlite)
(add-hook 'sql-mode-hook #'flymake-sqlite-setup)
(add-hook 'sql-mode-hook #'flymake-mode)
