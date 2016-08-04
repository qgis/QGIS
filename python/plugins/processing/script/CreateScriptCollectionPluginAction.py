# -*- coding: utf-8 -*-

"""
***************************************************************************
    CreateScriptCollectionPluginAction.py
    ---------------------
    Date                 : May 2016
    Copyright            : (C) 2016 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'May 2016'
__copyright__ = '(C) 2016, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon
from processing.gui.ToolboxAction import ToolboxAction
from processing.script.ScriptSelector import ScriptSelector
from processing.tools.system import mkdir

pluginPath = os.path.split(os.path.dirname(__file__))[0]

initTemplate = '''from .plugin import ProcessingScriptCollectionPlugin

def classFactory(iface):
    return ProcessingScriptCollectionPlugin()
'''
metadataTemplate = '''[general]
name=$name$
description=$description$
category=Analysis
version=1.0
qgisMinimumVersion=2.0

author=$author$
email=$email$

tags=analysis,processing

homepage=http://qgis.org
tracker=https://hub.qgis.org/projects/QGIS/issues
repository=https://github.com/qgis/QGIS
'''

pluginTemplate = '''import os

from processing.core.Processing import Processing

class ProcessingScriptCollectionPlugin:

    def initGui(self):
        Processing.addScripts(os.path.join(os.path.dirname(__file__), "scripts"))

    def unload(self):
        Processing.removeScripts(os.path.join(os.path.dirname(__file__), "scripts"))
'''


class CreateScriptCollectionPluginAction(ToolboxAction):

    def __init__(self):
        self.name, self.i18n_name = self.trAction('Create script collection plugin')
        self.group, self.i18n_group = self.trAction('Tools')

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'script.png'))

    def execute(self):
        dlg = ScriptSelector()
        dlg.exec_()
        if dlg.scripts:
            mkdir(dlg.folder)
            initFile = os.path.join(dlg.folder, "__init__.py")
            with open(initFile, "w") as f:
                f.write(initTemplate)
            metadataFile = os.path.join(dlg.folder, "metadata.txt")
            with open(metadataFile, "w") as f:
                f.write(metadataTemplate.replace("$name$", dlg.name).replace("$description$", dlg.description)
                        .replace("$author$", dlg.author).replace("$email$", dlg.email))
            pluginFile = os.path.join(dlg.folder, "plugin.py")
            with open(pluginFile, "w") as f:
                f.write(pluginTemplate)
            scriptsFolder = os.path.join(dlg.folder, "scripts")
            mkdir(scriptsFolder)
            for script in dlg.scripts:
                scriptFile = os.path.join(scriptsFolder, os.path.basename(script.descriptionFile))
                with open(scriptFile, "w") as f:
                    f.write(script.script)
