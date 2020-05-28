# -*- coding: utf-8 -*-

"""
***************************************************************************
    AddScriptFromTemplateAction.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2018 by Matteo Ghetta
    Email                : matteo dot ghetta at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Matteo Ghetta'
__date__ = 'March 2018'
__copyright__ = '(C) 2018, Matteo Ghetta'

import os
import codecs

from qgis.PyQt.QtCore import QCoreApplication

from qgis.core import QgsApplication

from processing.gui.ToolboxAction import ToolboxAction

from processing.script.ScriptEditorDialog import ScriptEditorDialog


class AddScriptFromTemplateAction(ToolboxAction):

    def __init__(self):
        self.name = QCoreApplication.translate("AddScriptFromTemplate", "Create New Script from Template…")
        self.group = self.tr("Tools")

    def execute(self):
        dlg = ScriptEditorDialog(None)

        pluginPath = os.path.split(os.path.dirname(__file__))[0]
        templatePath = os.path.join(
            pluginPath, 'script', 'ScriptTemplate.py')

        with codecs.open(templatePath, 'r', encoding='utf-8') as f:
            templateTxt = f.read()
            dlg.editor.setText(templateTxt)

        dlg.show()
