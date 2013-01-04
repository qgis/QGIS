# -*- coding: utf-8 -*-

"""
***************************************************************************
    CrsSelectionDialog.py
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

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

class CrsSelectionDialog(QDialog):

    def __init__(self):
        QDialog.__init__(self)
        self.authid = None
        layout = QVBoxLayout()
        self.selector = QgsProjectionSelector(self)
        buttonBox = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Close)
        layout.addWidget(self.selector)
        layout.addWidget(buttonBox)
        self.setLayout(layout)

        self.connect(buttonBox, SIGNAL("accepted()"), self.okPressed)
        self.connect(buttonBox, SIGNAL("rejected()"), self.cancelPressed)

    def okPressed(self):
        self.authid = self.selector.selectedAuthId()
        self.close()

    def cancelPressed(self):
        self.authid = None
        self.close()
