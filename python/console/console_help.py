# -*- coding: utf-8 -*-

"""
***************************************************************************
    help.py
    ---------------------
    Date                 : September 2012
    Copyright            : (C) 2012 by Salvatore Larosa
    Email                : lrssvtml at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Salvatore Larosa'
__date__ = 'September 2012'
__copyright__ = '(C) 2012, Salvatore Larosa'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from ui_console_help import Ui_Help
from qgis.core import QgsApplication
import os

class HelpDialog(QDialog, Ui_Help):
    def __init__(self, parent):
        QDialog.__init__(self, parent)
        self.setupUi(self)

        self.setWindowTitle(QCoreApplication.translate("PythonConsole","Help Python Console"))
        self.setMaximumSize(530, 300)

        qgisDataDir = QgsApplication.pkgDataPath()
        listFile = os.listdir(qgisDataDir + "/python/console/help/i18n")
        localeFullName = QSettings().value( "locale/userLocale", QVariant( "" ) ).toString()
        locale = "en_US"
        for i in listFile:
            lang = i[0:5]
            if localeFullName in (lang[0:2], lang):
                locale = lang

        filename = qgisDataDir + "/python/console/help/help.htm? \
                                                lang=" + locale \
                                                + "&pkgDir=" + qgisDataDir

        url = QUrl(filename)
        self.webView.load(url)
