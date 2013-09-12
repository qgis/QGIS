# -*- coding: utf-8 -*-

"""
***************************************************************************
    SilentProgress.py
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

from processing import interface
from PyQt4.QtCore import *
from PyQt4 import QtGui

class MessageBarProgress():
    
    def __init__(self):
        self.progressMessageBar = interface.iface.messageBar().createMessage("Executing algorithm")
        self.progress = QtGui.QProgressBar()
        self.progress.setMaximum(100)
        self.progress.setAlignment(Qt.AlignLeft|Qt.AlignVCenter)
        self.progressMessageBar.layout().addWidget(self.progress) 
        interface.iface.messageBar().pushWidget(self.progressMessageBar, interface.iface.messageBar().INFO)  

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
        interface.iface.messageBar().clearWidgets()

         