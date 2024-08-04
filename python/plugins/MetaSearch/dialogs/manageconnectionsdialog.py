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
# Copyright (C) 2014 Tom Kralidis (tomkralidis@gmail.com)
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

import xml.etree.ElementTree as etree

from qgis.core import QgsSettings
from qgis.PyQt.QtWidgets import QDialog, QDialogButtonBox, QFileDialog, QListWidgetItem, QMessageBox  # noqa

from MetaSearch.util import (get_connections_from_file, get_ui_class,
                             prettify_xml)

BASE_CLASS = get_ui_class('manageconnectionsdialog.ui')


class ManageConnectionsDialog(QDialog, BASE_CLASS):
    """manage connections"""

    def __init__(self, mode):
        """init dialog"""

        QDialog.__init__(self)
        self.setupUi(self)
        self.settings = QgsSettings()
        self.filename = None
        self.mode = mode  # 0 - save, 1 - load
        self.btnBrowse.clicked.connect(self.select_file)
        self.manage_gui()

    def manage_gui(self):
        """manage interface"""

        if self.mode == 1:
            self.label.setText(self.tr('Load from file'))
            self.buttonBox.button(QDialogButtonBox.StandardButton.Ok).setText(self.tr('Load'))
        else:
            self.label.setText(self.tr('Save to file'))
            self.buttonBox.button(QDialogButtonBox.StandardButton.Ok).setText(self.tr('Save'))
            self.populate()

        self.buttonBox.button(QDialogButtonBox.StandardButton.Ok).setEnabled(False)

    def select_file(self):
        """select file ops"""

        label = self.tr('eXtensible Markup Language (*.xml *.XML)')

        if self.mode == 0:
            slabel = self.tr('Save Connections')
            self.filename, filter = QFileDialog.getSaveFileName(self, slabel,
                                                                '.', label)
        else:
            slabel = self.tr('Load Connections')
            self.filename, selected_filter = QFileDialog.getOpenFileName(
                self, slabel, '.', label)

        if not self.filename:
            return

        # ensure the user never omitted the extension from the file name
        if not self.filename.lower().endswith('.xml'):
            self.filename = '%s.xml' % self.filename

        self.leFileName.setText(self.filename)

        if self.mode == 1:
            self.populate()

        self.buttonBox.button(QDialogButtonBox.StandardButton.Ok).setEnabled(True)

    def populate(self):
        """populate connections list from settings"""

        if self.mode == 0:
            self.settings.beginGroup('/MetaSearch/')
            keys = self.settings.childGroups()
            for key in keys:
                item = QListWidgetItem(self.listConnections)
                item.setText(key)
            self.settings.endGroup()

        else:  # populate connections list from file
            doc = get_connections_from_file(self, self.filename)
            if doc is None:
                self.filename = None
                self.leFileName.clear()
                self.listConnections.clear()
                return

            for catalog in doc.findall('csw'):
                item = QListWidgetItem(self.listConnections)
                item.setText(catalog.attrib.get('name'))

    def save(self, connections):
        """save connections ops"""

        doc = etree.Element('qgsCSWConnections')
        doc.attrib['version'] = '1.0'

        for conn in connections:
            url = self.settings.value('/MetaSearch/%s/url' % conn)
            type_ = self.settings.value('/MetaSearch/%s/catalog-type' % conn)
            if url is not None:
                connection = etree.SubElement(doc, 'csw')
                connection.attrib['name'] = conn
                connection.attrib['type'] = type_ or 'OGC CSW 2.0.2'
                connection.attrib['url'] = url

        # write to disk
        with open(self.filename, 'w') as fileobj:
            fileobj.write(prettify_xml(etree.tostring(doc)))
        QMessageBox.information(self, self.tr('Save Connections'),
                                self.tr('Saved to {0}.').format(self.filename))
        self.reject()

    def load(self, items):
        """load connections"""

        self.settings.beginGroup('/MetaSearch/')
        keys = self.settings.childGroups()
        self.settings.endGroup()

        exml = etree.parse(self.filename).getroot()

        for catalog in exml.findall('csw'):
            conn_name = catalog.attrib.get('name')

            # process only selected connections
            if conn_name not in items:
                continue

            # check for duplicates
            if conn_name in keys:
                label = self.tr('File {0} exists. Overwrite?').format(
                    conn_name)
                res = QMessageBox.warning(self, self.tr('Loading Connections'),
                                          label,
                                          QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No)
                if res != QMessageBox.StandardButton.Yes:
                    continue

            # no dups detected or overwrite is allowed
            url = '/MetaSearch/%s/url' % conn_name
            type_ = '/MetaSearch/%s/catalog-type' % conn_name
            self.settings.setValue(url, catalog.attrib.get('url'))
            self.settings.setValue(type_, catalog.attrib.get('catalog-type', 'OGC CSW 2.0.2'))

    def accept(self):
        """accept connections"""

        selection = self.listConnections.selectedItems()
        if len(selection) == 0:
            return

        items = []
        for sel in selection:
            items.append(sel.text())

        if self.mode == 0:  # save
            self.save(items)
        else:  # load
            self.load(items)

        self.filename = None
        self.leFileName.clear()
        self.listConnections.clear()
        self.buttonBox.button(QDialogButtonBox.StandardButton.Ok).setEnabled(False)

    def reject(self):
        """back out of manage connections dialogue"""

        QDialog.reject(self)
