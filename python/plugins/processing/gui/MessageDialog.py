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

from PyQt4.QtCore import *
from PyQt4.QtGui import *


from processing.ui.ui_DlgMessage import Ui_Dialog


class MessageDialog(QDialog, Ui_Dialog):

    def __init__(self):
        QDialog.__init__(self)
        self.setupUi(self)

        self.txtMessage.anchorClicked.connect(self.openLink)

    def setTitle(self, title):
        self.setWindowTitle(title)

    def setMessage(self, message):
        self.txtMessage.setHtml(message)

    def openLink(self, url):
        QDesktopServices.openUrl(QUrl(url))
