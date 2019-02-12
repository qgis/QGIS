# -*- coding: utf-8 -*-

"""
***************************************************************************
    EditModelAction.py
    ---------------------
    Date                 : February 2019
    Copyright            : (C) 2019 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nyall Dawson'
__date__ = 'February 2019'
__copyright__ = '(C) 2019, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import QgsProcessingModelAlgorithm, QgsProcessing, QgsApplication
from processing.gui.ContextAction import ContextAction
from processing.script.ScriptEditorDialog import ScriptEditorDialog


class ExportModelAsPythonScriptAction(ContextAction):

    def __init__(self):
        super().__init__()
        self.name = QCoreApplication.translate('ExportModelAsPythonScriptAction', 'Export Model as Python Algorithm…')

    def isEnabled(self):
        return isinstance(self.itemData, QgsProcessingModelAlgorithm)

    def icon(self):
        return QgsApplication.getThemeIcon('/mActionSaveAsPython.svg')

    def execute(self):
        alg = self.itemData
        dlg = ScriptEditorDialog(None)

        dlg.editor.setText('\n'.join(alg.asPythonCode(QgsProcessing.PythonQgsProcessingAlgorithmSubclass, 4)))
        dlg.show()
