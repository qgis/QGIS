# -*- coding: utf-8 -*-

"""
***************************************************************************
    dialogSRS.py
    ---------------------
    Date                 : June 2010
    Copyright            : (C) 2010 by Giuseppe Sucameli
    Email                : brush dot tyler at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Giuseppe Sucameli'
__date__ = 'June 2010'
__copyright__ = '(C) 2010, Giuseppe Sucameli'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.PyQt.QtWidgets import QDialog, QVBoxLayout, QDialogButtonBox
from qgis.gui import QgsProjectionSelector


class GdalToolsSRSDialog(QDialog):

    def __init__(self, title, parent=None):
        QDialog.__init__(self, parent)
        self.setWindowTitle(title)

        layout = QVBoxLayout()
        self.selector = QgsProjectionSelector(self)
        buttonBox = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Close)

        layout.addWidget(self.selector)
        layout.addWidget(buttonBox)
        self.setLayout(layout)

        buttonBox.accepted.connect(self.accept)
        buttonBox.rejected.connect(self.reject)

    def authid(self):
        return unicode(self.selector.selectedAuthId())

    def proj4string(self):
        return self.selector.selectedProj4String()

    def getProjection(self):
        if self.authid().upper().startswith("EPSG:"):
            return self.authid()

        if self.selector.selectedProj4String():
            return self.proj4string()

        return ''
