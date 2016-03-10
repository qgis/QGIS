# -*- coding: utf-8 -*-

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

__author__ = 'Alexander Bruy'
__date__ = 'October 2014'
__copyright__ = '(C) 2014, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from PyQt4 import uic
from PyQt4.QtCore import QUrl
from PyQt4.QtGui import QDesktopServices, QDockWidget

from qgis.utils import iface

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'DlgMessage.ui'))


class MessageDialog(BASE, WIDGET):

    def __init__(self):
        super(MessageDialog, self).__init__(None)
        self.setupUi(self)

        self.txtMessage.anchorClicked.connect(self.openLink)

    def setTitle(self, title):
        self.setWindowTitle(title)

    def setMessage(self, message):
        self.txtMessage.setHtml(message)

    def openLink(self, url):
        if url.toString() == "log":
            self.close()
            logDock = iface.mainWindow().findChild(QDockWidget, 'MessageLog')
            logDock.show()
        else:
            QDesktopServices.openUrl(url)
