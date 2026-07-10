"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QGIS
Date                 : May 23, 2011
copyright            : (C) 2011 by Giuseppe Sucameli
email                : brush.tyler@gmail.com

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

from qgis.core import (
    Qgis,
)
from qgis.PyQt import sip
from qgis.PyQt.QtWidgets import QApplication, QPushButton


class DBManagerPlugin:
    def __init__(self, iface):
        self.iface = iface
        self._item = None

    def initGui(self):
        def install_community(_):
            if self._item and not sip.isdeleted(self._item):
                self.iface.messageBar().popWidget(self._item)
                self._item = None

            self.iface.showPluginManager(
                tabIndex=0, searchTerm=r"DB Manager \\(community\\)"
            )

        if self.iface.messageBar() is not None:
            message_widget = self.iface.messageBar().createMessage(
                QApplication.translate("DBManagerPlugin", "DB Manager"),
                QApplication.translate(
                    "DBManagerPlugin",
                    'The DB Manager plugin is no longer installed with QGIS. Please install the "DB Manager (community)" replacement plugin instead.',
                ),
            )
            install_button = QPushButton("Install Now")
            install_button.clicked.connect(install_community)
            message_widget.layout().addWidget(install_button)
            self._item = self.iface.messageBar().pushWidget(
                message_widget, Qgis.MessageLevel.Warning, 0
            )

    def unload(self):
        pass
