# coding=utf-8
"""Plugin dependencies selection dialog

.. note:: This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

"""

__author__ = 'elpaso@itopen.it'
__date__ = '2018-09-19'
__copyright__ = 'Copyright 2018, GISCE-TI S.L.'


import os

from qgis.PyQt import QtWidgets, QtCore
from .ui_qgsplugindependenciesdialogbase import Ui_QgsPluginDependenciesDialogBase
from qgis.utils import iface


class QgsPluginDependenciesDialog(QtWidgets.QDialog, Ui_QgsPluginDependenciesDialogBase):
    """A dialog that shows plugin dependencies and offers a way to install or upgrade the
    dependencies.
    """

    def __init__(self, plugin_name, to_install, to_upgrade, not_found, parent=None):
        """Creates the dependencies dialog

        :param plugin_name: the name of the parent plugin
        :type plugin_name: str
        :param to_install: list of plugin IDs that needs to be installed
        :type to_install: list
        :param to_upgrade: list of plugin IDs that needs to be upgraded
        :type to_upgrade: list
        :param not_found: list of plugin IDs that are not found (unavailable)
        :type not_found: list
        :param parent: parent object, defaults to None
        :param parent: QWidget, optional
        """

        super().__init__(parent)
        self.setupUi(self)
        self.setWindowTitle(self.tr("Plugin Dependencies Manager"))
        self.mPluginDependenciesLabel.setText(self.tr("Plugin dependencies for <b>%s</b>") % plugin_name)
        self.setStyleSheet("QTableView { padding: 20px;}")
        # Name, Version Installed, Version Required, Version Available, Action Checkbox
        self.pluginList.setColumnCount(5)
        self.pluginList.setHorizontalHeaderLabels([self.tr('Name'), self.tr('Installed'), self.tr('Required'), self.tr('Available'), self.tr('Action')])
        self.pluginList.setRowCount(len(not_found) + len(to_install) + len(to_upgrade))
        self.__actions = {}

        def _display(txt):
            if txt is None:
                return ""
            return txt

        def _make_row(data, i, name):
            widget = QtWidgets.QLabel("<b>%s</b>" % name)
            widget.p_id = data['id']
            widget.action = data['action']
            self.pluginList.setCellWidget(i, 0, widget)
            widget = QtWidgets.QTableWidgetItem(_display(data['version_installed']))
            widget.setTextAlignment(QtCore.Qt.AlignHCenter | QtCore.Qt.AlignVCenter)
            self.pluginList.setItem(i, 1, widget)
            widget = QtWidgets.QTableWidgetItem(_display(data['version_required']))
            widget.setTextAlignment(QtCore.Qt.AlignHCenter | QtCore.Qt.AlignVCenter)
            self.pluginList.setItem(i, 2, widget)
            widget = QtWidgets.QTableWidgetItem(_display(data['version_available']))
            widget.setTextAlignment(QtCore.Qt.AlignHCenter | QtCore.Qt.AlignVCenter)
            self.pluginList.setItem(i, 3, widget)

        i = 0
        for name, data in to_install.items():
            _make_row(data, i, name)
            widget = QtWidgets.QCheckBox(self.tr("Install"))
            widget.setChecked(True)
            self.pluginList.setCellWidget(i, 4, widget)
            i += 1

        for name, data in to_upgrade.items():
            _make_row(data, i, name)
            widget = QtWidgets.QCheckBox(self.tr("Upgrade"))
            widget.setChecked(True)
            self.pluginList.setCellWidget(i, 4, widget)
            i += 1

        for name, data in not_found.items():
            _make_row(data, i, name)
            widget = QtWidgets.QLabel(self.tr("Fix manually"))
            self.pluginList.setCellWidget(i, 4, widget)
            i += 1

    def actions(self):
        """Returns the list of actions

        :return: dict of actions
        :rtype: dict
        """

        return self.__actions

    def accept(self):
        self.__actions = {}
        for i in range(self.pluginList.rowCount()):
            try:
                if self.pluginList.cellWidget(i, 4).isChecked():
                    self.__actions[self.pluginList.cellWidget(i, 0).p_id] = self.pluginList.cellWidget(i, 0).action
            except:
                pass
        super().accept()
