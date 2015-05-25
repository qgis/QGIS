# -*- coding: utf-8 -*-

"""
***************************************************************************
    InputLayerSelectorPanel.py
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
from PyQt4.QtCore import QSettings
from PyQt4.QtGui import QIcon, QFileDialog
from processing.tools import dataobjects

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'widgetLayerSelector.ui'))


class InputLayerSelectorPanel(BASE, WIDGET):

    def __init__(self, options, param):
        super(InputLayerSelectorPanel, self).__init__(None)
        self.setupUi(self)

        self.btnIterate.setIcon(
            QIcon(os.path.join(pluginPath, 'images', 'iterate.png')))
        self.btnIterate.hide()

        self.param = param

        for (name, value) in options:
            self.cmbText.addItem(name, value)

        self.btnSelect.clicked.connect(self.showSelectionDialog)

    def showSelectionDialog(self):
        settings = QSettings()
        text = unicode(self.cmbText.currentText())
        if os.path.isdir(text):
            path = text
        elif os.path.isdir(os.path.dirname(text)):
            path = os.path.dirname(text)
        elif settings.contains('/Processing/LastInputPath'):
            path = unicode(settings.value('/Processing/LastInputPath'))
        else:
            path = ''

        filename = QFileDialog.getOpenFileName(self, self.tr('Select file'),
            path, self.tr('All files (*.*);;') + self.param.getFileFilter())
        if filename:
            settings.setValue('/Processing/LastInputPath',
                              os.path.dirname(unicode(filename)))
            filename = dataobjects.getRasterSublayer(filename, self.param)
            self.cmbText.addItem(filename, filename)
            self.cmbText.setCurrentIndex(self.cmbText.count() - 1)


    def getValue(self):
        return self.cmbText.itemData(self.cmbText.currentIndex())
