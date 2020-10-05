# -*- coding:utf-8 -*-
"""
/***************************************************************************
Python Console for QGIS
                             -------------------
begin                : 2020-06-04
copyright            : (C) 2020 by Richard Duivenvoorde
email                : Richard Duivenvoorde (at) duif (dot) net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
Some portions of code were taken from https://code.google.com/p/pydee/
"""

from qgis.PyQt.QtCore import Qt, QUrl
from qgis.PyQt.QtGui import QColor, QFontDatabase, QDesktopServices
from qgis.PyQt.Qsci import QsciLexerPython, QsciAPIs
from qgis.core import QgsApplication, Qgis
from qgis.gui import QgsCodeEditorPython, QgsCodeEditor
import os


class QgsPythonConsoleBase(QgsCodeEditorPython):

    def __init__(self, parent=None):
        super().__init__(parent)

        # Margin 0 is used for line numbers (editor and output)
        self.setMarginWidth(0, "00000")
        # Margin 1 is used for the '>>>' prompt (console input)
        self.setMarginWidth(1, "0")
        self.setMarginType(1, 5)  # TextMarginRightJustified=5
        # Margin 2 is used for the 'folding' (editor)
        self.setMarginWidth(2, "0")

        self.iconRun = QgsApplication.getThemeIcon("console/mIconRunConsole.svg")
        self.iconRunScript = QgsApplication.getThemeIcon("mActionStart.svg")
        self.iconUndo = QgsApplication.getThemeIcon("mActionUndo.svg")
        self.iconRedo = QgsApplication.getThemeIcon("mActionRedo.svg")
        self.iconCodePad = QgsApplication.getThemeIcon("console/iconCodepadConsole.svg")
        self.iconCommentEditor = QgsApplication.getThemeIcon("console/iconCommentEditorConsole.svg")
        self.iconUncommentEditor = QgsApplication.getThemeIcon("console/iconUncommentEditorConsole.svg")
        self.iconSettings = QgsApplication.getThemeIcon("console/iconSettingsConsole.svg")
        self.iconFind = QgsApplication.getThemeIcon("console/iconSearchEditorConsole.svg")
        self.iconSyntaxCk = QgsApplication.getThemeIcon("console/iconSyntaxErrorConsole.svg")
        self.iconObjInsp = QgsApplication.getThemeIcon("console/iconClassBrowserConsole.svg")
        self.iconCut = QgsApplication.getThemeIcon("mActionEditCut.svg")
        self.iconCopy = QgsApplication.getThemeIcon("mActionEditCopy.svg")
        self.iconPaste = QgsApplication.getThemeIcon("mActionEditPaste.svg")
        self.iconClear = QgsApplication.getThemeIcon("console/iconClearConsole.svg")
        self.iconHideTool = QgsApplication.getThemeIcon("console/iconHideToolConsole.svg")
        self.iconShowEditor = QgsApplication.getThemeIcon("console/iconShowEditorConsole.svg")
        self.iconPyQGISHelp = QgsApplication.getThemeIcon("console/iconHelpConsole.svg")

    def setLexers(self):
        self.api = QsciAPIs(self.lexer())
        checkBoxAPI = self.settings.value("pythonConsole/preloadAPI", True, type=bool)
        checkBoxPreparedAPI = self.settings.value("pythonConsole/usePreparedAPIFile", False, type=bool)
        if checkBoxAPI:
            pap = os.path.join(QgsApplication.pkgDataPath(), "python", "qsci_apis", "pyqgis.pap")
            self.api.loadPrepared(pap)
        elif checkBoxPreparedAPI:
            self.api.loadPrepared(self.settings.value("pythonConsole/preparedAPIFile"))
        else:
            apiPath = self.settings.value("pythonConsole/userAPI", [])
            for i in range(0, len(apiPath)):
                self.api.load(apiPath[i])
            self.api.prepare()
            self.lexer().setAPIs(self.api)


if __name__ == "__main__":
    pass
