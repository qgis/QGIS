"""
***************************************************************************
    MessageDialog.py
    ---------------------
    Date                 : October 2014
    Copyright            : (C) 2014 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Alexander Bruy"
__date__ = "October 2014"
__copyright__ = "(C) 2014, Alexander Bruy"

import os
import warnings

from qgis.PyQt import uic
from qgis.PyQt.QtGui import QDesktopServices
from qgis.PyQt.QtWidgets import QDockWidget

from qgis.utils import iface

pluginPath = os.path.split(os.path.dirname(__file__))[0]
with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(os.path.join(pluginPath, "ui", "DlgMessage.ui"))


class MessageDialog(BASE, WIDGET):

    def __init__(self):
        super().__init__(None)
        self.setupUi(self)

        self.txtMessage.anchorClicked.connect(self.openLink)

    def setTitle(self, title):
        self.setWindowTitle(title)

    def setMessage(self, message):
        self.txtMessage.setHtml(message)

    def openLink(self, url):
        if url.toString() == "log":
            self.close()
            logDock = iface.mainWindow().findChild(QDockWidget, "MessageLog")
            logDock.show()
        else:
            QDesktopServices.openUrl(url)
