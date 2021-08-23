# -*- coding: utf-8 -*-
###############################################################################
#
# CSW Client
# ---------------------------------------------------------
# QGIS Catalog Service client.
#
# Copyright (C) 2010 NextGIS (http://nextgis.org),
#                    Alexander Bruy (alexander.bruy@gmail.com),
#                    Maxim Dubinin (sim@gis-lab.info)
#
# Copyright (C) 2017 Tom Kralidis (tomkralidis@gmail.com)
#
# This source is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This code is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
###############################################################################

from qgis.core import QgsSettings
from qgis.PyQt.QtWidgets import QDialog, QMessageBox

from MetaSearch.util import get_ui_class
from MetaSearch.search_backend import CATALOG_TYPES

BASE_CLASS = get_ui_class('newconnectiondialog.ui')


class NewConnectionDialog(QDialog, BASE_CLASS):
    """Dialogue to add a new CSW entry"""

    def __init__(self, conn_name=None):
        """init"""

        QDialog.__init__(self)
        self.setupUi(self)
        self.settings = QgsSettings()
        self.conn_name = None
        self.conn_name_orig = conn_name
        self.username = None
        self.password = None

        self.cmbCatalogType.addItems(CATALOG_TYPES)

    def accept(self):
        """add CSW entry"""

        conn_name = self.leName.text().strip()
        conn_url = self.leURL.text().strip()
        conn_username = self.leUsername.text().strip()
        conn_password = self.lePassword.text().strip()
        conn_catalog_type = self.cmbCatalogType.currentText()

        if any([conn_name == '', conn_url == '']):
            QMessageBox.warning(self, self.tr('Save Connection'),
                                self.tr('Both Name and URL must be provided.'))
            return

        if '/' in conn_name:
            QMessageBox.warning(self, self.tr('Save Connection'),
                                self.tr('Name cannot contain \'/\'.'))
            return

        if conn_name is not None:
            key = '/MetaSearch/%s' % conn_name
            keyurl = '%s/url' % key
            key_orig = '/MetaSearch/%s' % self.conn_name_orig

            # warn if entry was renamed to an existing connection
            if all([self.conn_name_orig != conn_name,
                    self.settings.contains(keyurl)]):
                res = QMessageBox.warning(
                    self, self.tr('Save Connection'),
                    self.tr('Overwrite {0}?').format(conn_name),
                    QMessageBox.Ok | QMessageBox.Cancel)
                if res == QMessageBox.Cancel:
                    return

            # on rename delete original entry first
            if all([self.conn_name_orig is not None,
                    self.conn_name_orig != conn_name]):
                self.settings.remove(key_orig)

            self.settings.setValue(keyurl, conn_url)
            self.settings.setValue('/MetaSearch/selected', conn_name)

            self.settings.setValue('%s/username' % key, conn_username)
            self.settings.setValue('%s/password' % key, conn_password)

            self.settings.setValue('%s/catalog-type' % key, conn_catalog_type)

            QDialog.accept(self)

    def reject(self):
        """back out of dialogue"""

        QDialog.reject(self)
