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

from qgis.core import QgsApplication
from qgis.PyQt.Qsci import QsciScintilla


class QgsQsciScintillaBase(QsciScintilla):

    MARKER_NUM = 6
    DEFAULT_COLOR = "#4d4d4c"
    KEYWORD_COLOR = "#8959a8"
    CLASS_COLOR = "#4271ae"
    METHOD_COLOR = "#4271ae"
    DECORATION_COLOR = "#3e999f"
    NUMBER_COLOR = "#c82829"
    COMMENT_COLOR = "#8e908c"
    COMMENT_BLOCK_COLOR = "#8e908c"
    BACKGROUND_COLOR = "#ffffff"
    CURSOR_COLOR = "#636363"
    CARET_LINE_COLOR = "#efefef"
    SINGLE_QUOTE_COLOR = "#718c00"
    DOUBLE_QUOTE_COLOR = "#718c00"
    TRIPLE_SINGLE_QUOTE_COLOR = "#eab700"
    TRIPLE_DOUBLE_QUOTE_COLOR = "#eab700"
    MARGIN_BACKGROUND_COLOR = "#efefef"
    MARGIN_FOREGROUND_COLOR = "#636363"
    SELECTION_BACKGROUND_COLOR = "#d7d7d7"
    SELECTION_FOREGROUND_COLOR = "#303030"
    MATCHED_BRACE_BACKGROUND_COLOR = "#b7f907"
    MATCHED_BRACE_FOREGROUND_COLOR = "#303030"
    EDGE_COLOR = "#efefef"
    FOLD_COLOR = "#efefef"

    def __init__(self, parent=None):
        super(QgsQsciScintillaBase, self).__init__(parent)

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


if __name__ == "__main__":
    pass
