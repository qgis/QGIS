# -*- coding: utf-8 -*-

"""
***************************************************************************
    AddScriptFromTemplateAction.py
    ---------------------
    Date                 : August 2022 (QGIS Hackfest in Firenze)
    Copyright            : (C) 2022 by Germán Carrillo
    Email                : gcarrillo at linuxmail dot org
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Germán Carrillo'
__date__ = 'August 2022'
__copyright__ = '(C) 2022, Germán Carrillo'

import os
import codecs

from qgis.PyQt.QtCore import QCoreApplication

from processing.gui.ToolboxAction import ToolboxAction

from processing.script.ScriptEditorDialog import ScriptEditorDialog


class AddScriptFromTemplateWithDecoratorsAction(ToolboxAction):

    def __init__(self):
        self.name = QCoreApplication.translate("AddScriptFromTemplateWithDecoratorsAction",
                                               "Create New Script from Template with Python Decorators…")
        self.group = self.tr("Tools")

    def execute(self):
        dlg = ScriptEditorDialog(None)

        pluginPath = os.path.split(os.path.dirname(__file__))[0]
        templatePath = os.path.join(
            pluginPath, 'script', 'ScriptTemplateWithDecorators.py')

        with codecs.open(templatePath, 'r', encoding='utf-8') as f:
            templateTxt = f.read()
            dlg.editor.setText(templateTxt)

        dlg.show()
