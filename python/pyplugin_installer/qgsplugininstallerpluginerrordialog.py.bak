# -*- coding:utf-8 -*-
"""
/***************************************************************************
                           qgsplugininstallerpluginerrordialog.py
                           Plugin Installer module
                             -------------------
    Date                 : June 2013
    Copyright            : (C) 2013 by Borys Jurgiel
    Email                : info at borysjurgiel dot pl

    This module is based on former plugin_installer plugin:
      Copyright (C) 2007-2008 Matthew Perry
      Copyright (C) 2008-2013 Borys Jurgiel

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""

from PyQt4.QtGui import QDialog

from ui_qgsplugininstallerpluginerrorbase import Ui_QgsPluginInstallerPluginErrorDialogBase


class QgsPluginInstallerPluginErrorDialog(QDialog, Ui_QgsPluginInstallerPluginErrorDialogBase):
    # ----------------------------------------- #

    def __init__(self, parent, errorMessage):
        QDialog.__init__(self, parent)
        self.setupUi(self)
        if not errorMessage:
            errorMessage = self.tr("no error message received")
        self.textBrowser.setText(errorMessage)
