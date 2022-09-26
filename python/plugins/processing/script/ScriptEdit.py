# -*- coding: utf-8 -*-

"""
***************************************************************************
    ScriptEdit.py
    ---------------------
    Date                 : April 2013
    Copyright            : (C) 2013 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'April 2013'
__copyright__ = '(C) 2013, Alexander Bruy'


from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtGui import QKeySequence
from qgis.PyQt.QtWidgets import QShortcut
from qgis.gui import QgsCodeEditorPython

from qgis.PyQt.Qsci import QsciScintilla


class ScriptEdit(QgsCodeEditorPython):

    def __init__(self, parent=None):
        super().__init__(parent)

        self.initShortcuts()

    def initShortcuts(self):
        (ctrl, shift) = (self.SCMOD_CTRL << 16, self.SCMOD_SHIFT << 16)

        # Disable some shortcuts
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('D') + ctrl)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('L') + ctrl)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('L') + ctrl
                           + shift)
        self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord('T') + ctrl)

        # self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord("Z") + ctrl)
        # self.SendScintilla(QsciScintilla.SCI_CLEARCMDKEY, ord("Y") + ctrl)

        # Use Ctrl+Space for autocompletion
        self.shortcutAutocomplete = QShortcut(QKeySequence(Qt.CTRL
                                                           + Qt.Key_Space), self)
        self.shortcutAutocomplete.setContext(Qt.WidgetShortcut)
        self.shortcutAutocomplete.activated.connect(self.autoComplete)

    def toggleComment(self):
        self.beginUndoAction()
        if self.hasSelectedText():
            start_line, start_pos, end_line, end_pos = self.getSelection()
        else:
            start_line, start_pos = self.getCursorPosition()
            end_line, end_pos = start_line, start_pos

        # Special case, only empty lines
        if not any(self.text(line).strip() for line in range(start_line, end_line + 1)):
            return

        all_commented = all(
            self.text(line).strip().startswith("#")
            for line in range(start_line, end_line + 1)
            if self.text(line).strip()
        )
        min_indentation = min(
            self.indentation(line)
            for line in range(start_line, end_line + 1)
            if self.text(line).strip()
        )

        for line in range(start_line, end_line + 1):
            # Empty line
            if not self.text(line).strip():
                continue

            delta = 0

            if not all_commented:
                self.insertAt("# ", line, min_indentation)
                delta = -2
            else:
                if not self.text(line).strip().startswith("#"):
                    continue
                if self.text(line).strip().startswith("# "):
                    delta = 2
                else:
                    delta = 1

                self.setSelection(
                    line, self.indentation(line), line, self.indentation(line) + delta,
                )
                self.removeSelectedText()

        self.endUndoAction()

        self.setSelection(start_line, start_pos - delta, end_line, end_pos - delta)
