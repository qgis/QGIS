# -*- coding: utf-8 -*-

"""
***************************************************************************
    MessageBarProgress.py
    ---------------------
    Date                 : April 2013
    Copyright            : (C) 2013 by Victor Olaya
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
__date__ = 'April 2013'
__copyright__ = '(C) 2013, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from PyQt4.QtCore import *
from PyQt4 import QtGui
from qgis.utils import iface
from qgis.gui import *


class MessageBarProgress:

    def __init__(self):
        self.progressMessageBar = \
            iface.messageBar().createMessage('Executing algorithm')
        self.progress = QtGui.QProgressBar()
        self.progress.setMaximum(100)
        self.progress.setAlignment(Qt.AlignLeft | Qt.AlignVCenter)
        self.progressMessageBar.layout().addWidget(self.progress)
        iface.messageBar().pushWidget(self.progressMessageBar,
                                      iface.messageBar().INFO)

    def error(self, msg):
        iface.messageBar().clearWidgets()
        iface.messageBar().pushMessage("Error", msg,
                                                  level = QgsMessageBar.CRITICAL,
                                                  duration = 3)

    def setText(self, text):
        pass

    def setPercentage(self, i):
        self.progress.setValue(i)

    def setInfo(self, _):
        pass

    def setCommand(self, _):
        pass

    def setDebugInfo(self, _):
        pass

    def setConsoleInfo(self, _):
        pass

    def close(self):
        iface.messageBar().clearWidgets()
