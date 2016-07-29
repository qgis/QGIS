# -*- coding: utf-8 -*-

"""
***************************************************************************
    PreconfiguredAlgorithmDialog.py
    ---------------------
    Date                 : April 2016
    Copyright            : (C) 2012 by Victor Olaya
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
__date__ = 'April 2016'
__copyright__ = '(C) 2016, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


import os
import json

from processing.preconfigured.PreconfiguredUtils import algAsDict
from processing.preconfigured.PreconfiguredUtils import preconfiguredAlgorithmsFolder
from processing.gui.AlgorithmDialogBase import AlgorithmDialogBase
from processing.gui.AlgorithmDialog import AlgorithmDialog
from processing.core.alglist import algList

from PyQt4.QtGui import QMessageBox, QPalette, QColor, QVBoxLayout, QLabel, \
    QLineEdit, QWidget


class PreconfiguredAlgorithmDialog(AlgorithmDialog):

    def __init__(self, alg, toolbox):
        AlgorithmDialog.__init__(self, alg)
        self.toolbox = toolbox
        self.cornerWidget.setVisible(False)
        self.btnRun.setText(self.tr("OK"))
        self.tabWidget.removeTab(1)
        self.settingsPanel = SettingsPanel()
        self.tabWidget.addTab(self.settingsPanel, "Description")

    def accept(self):
        try:
            self.setParamValues()
            msg = self.alg._checkParameterValuesBeforeExecuting()
            if msg:
                QMessageBox.warning(
                    self, self.tr('Unable to execute algorithm'), msg)
                return
            description = algAsDict(self.alg)
            description["name"] = self.settingsPanel.txtName.text().strip()
            description["group"] = self.settingsPanel.txtGroup.text().strip()
            if not (description["name"] and description["group"]):
                self.tabWidget.setCurrentIndex(self.tabWidget.count() - 1)
                return
            validChars = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:'
            filename = ''.join(c for c in description["name"] if c in validChars).lower() + ".json"
            filepath = os.path.join(preconfiguredAlgorithmsFolder(), filename)
            with open(filepath, "w") as f:
                json.dump(description, f)
            algList.reloadProvider('preconfigured')
        except AlgorithmDialogBase.InvalidParameterValue as e:
            try:
                self.buttonBox.accepted.connect(lambda:
                                                e.widget.setPalette(QPalette()))
                palette = e.widget.palette()
                palette.setColor(QPalette.Base, QColor(255, 255, 0))
                e.widget.setPalette(palette)
                self.lblProgress.setText(
                    self.tr('<b>Missing parameter value: %s</b>') % e.parameter.description)
                return
            except:
                QMessageBox.critical(self,
                                     self.tr('Unable to execute algorithm'),
                                     self.tr('Wrong or missing parameter values'))
        self.close()


class SettingsPanel(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        layout = QVBoxLayout()
        labelName = QLabel("Name")
        labelGroup = QLabel("Group")
        self.txtName = QLineEdit()
        self.txtGroup = QLineEdit()
        layout.addWidget(labelName)
        layout.addWidget(self.txtName)
        layout.addWidget(labelGroup)
        layout.addWidget(self.txtGroup)
        layout.addStretch()
        self.setLayout(layout)
