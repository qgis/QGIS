# -*- coding: utf-8 -*-

"""
***************************************************************************
    RenderingStyleFilePanel.py
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

from PyQt4.QtGui import *

from processing.core.ProcessingConfig import ProcessingConfig
from processing.tools.system import *

from processing.ui.ui_widgetBaseSelector import Ui_Form


class RenderingStyleFilePanel(QWidget, Ui_Form):

    def __init__(self):
        QWidget.__init__(self)
        self.setupUi(self)

        self.btnSelect.clicked.connect(self.showSelectionDialog)


    def showSelectionDialog(self):
        filename = QFileDialog.getOpenFileName(self,
            self.tr('Select style file'), '',
            self.tr('QGIS Layer Style File (*.qml *.QML)'))
        if filename:
            self.leText.setText(filename)

    def setText(self, text):
        self.leText.setText(text)

    def getValue(self):
        s = self.leText.text()
        if isWindows():
            s = s.replace('\\', '/')
        return s
