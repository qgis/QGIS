# -*- coding: utf-8 -*-

"""
***************************************************************************
    CrsSelectionPanel.py
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
from PyQt4.QtCore import *

from qgis.gui import *
from qgis.core import *

from processing.ui.ui_widgetCRSSelector import Ui_widgetCRSSelector

class CrsSelectionPanel(QWidget, Ui_widgetCRSSelector):

    def __init__(self, default):
        QWidget.__init__(self)
        self.setupUi(self)

        self.btnBrowse.clicked.connect(self.browseCRS)
        self.authId = QgsCoordinateReferenceSystem(default).authid()
        self.updateText()

    def setAuthId(self, authid):
        self.authId = authid
        self.updateText()

    def browseCRS(self):
        selector = QgsGenericProjectionSelector()
        selector.setSelectedAuthId(self.authId)
        if selector.exec_():
            self.authId = selector.selectedAuthId()
            self.updateText()

    def updateText(self):
        if self.authId is not None:
            self.leCRS.setText(self.authId)

    def getValue(self):
        return self.authId
