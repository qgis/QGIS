# -*- coding: utf-8 -*-

"""
***************************************************************************
    FileSelectionPanel.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
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
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from PyQt4 import uic
from PyQt4.QtGui import QFileDialog
from PyQt4.QtCore import QSettings

from processing.tools.system import isWindows

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'widgetBaseSelector.ui'))


class FileSelectionPanel(BASE, WIDGET):

    def __init__(self, isFolder, ext=None):
        super(FileSelectionPanel, self).__init__(None)
        self.setupUi(self)

        self.ext = ext or '*'
        self.isFolder = isFolder

        self.btnSelect.clicked.connect(self.showSelectionDialog)

    def showSelectionDialog(self):
        # Find the file dialog's working directory
        settings = QSettings()
        text = self.leText.text()
        if os.path.isdir(text):
            path = text
        elif os.path.isdir(os.path.dirname(text)):
            path = os.path.dirname(text)
        elif settings.contains('/Processing/LastInputPath'):
            path = settings.value('/Processing/LastInputPath')
        else:
            path = ''

        if self.isFolder:
            folder = QFileDialog.getExistingDirectory(self,
                                                      self.tr('Select folder'), path)
            if folder:
                self.leText.setText(folder)
                settings.setValue('/Processing/LastInputPath',
                                  os.path.dirname(folder))
        else:
            filenames = QFileDialog.getOpenFileNames(self,
                                                     self.tr('Select file'), path, '*.' + self.ext)
            if filenames:
                self.leText.setText(u';'.join(filenames))
                settings.setValue('/Processing/LastInputPath',
                                  os.path.dirname(filenames[0]))

    def getValue(self):
        s = self.leText.text()
        if isWindows():
            s = s.replace('\\', '/')
        return s

    def setText(self, text):
        self.leText.setText(text)
