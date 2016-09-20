# -*- coding: utf-8 -*-

"""
***************************************************************************
    BatchInputSelectionPanel.py
    ---------------------
    Date                 : August 2012
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
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtCore import QSettings, pyqtSignal
from qgis.PyQt.QtWidgets import QWidget, QHBoxLayout, QMenu, QPushButton, QLineEdit, QSizePolicy, QAction, QFileDialog
from qgis.PyQt.QtGui import QCursor

from qgis.core import QgsMapLayer

from processing.gui.MultipleInputDialog import MultipleInputDialog

from processing.core.parameters import ParameterMultipleInput
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTable

from processing.tools import dataobjects


class BatchInputSelectionPanel(QWidget):

    valueChanged = pyqtSignal()

    def __init__(self, param, row, col, dialog):
        super(BatchInputSelectionPanel, self).__init__(None)
        self.param = param
        self.dialog = dialog
        self.row = row
        self.col = col
        self.horizontalLayout = QHBoxLayout(self)
        self.horizontalLayout.setSpacing(0)
        self.horizontalLayout.setMargin(0)
        self.text = QLineEdit()
        self.text.setObjectName('text')
        self.text.setMinimumWidth(300)
        self.setValue('')
        self.text.editingFinished.connect(self.textEditingFinished)
        self.text.setSizePolicy(QSizePolicy.Expanding,
                                QSizePolicy.Expanding)
        self.horizontalLayout.addWidget(self.text)
        self.pushButton = QPushButton()
        self.pushButton.setText('...')
        self.pushButton.clicked.connect(self.showPopupMenu)
        self.horizontalLayout.addWidget(self.pushButton)
        self.setLayout(self.horizontalLayout)

    def _panel(self):
        return self.dialog.mainWidget

    def _table(self):
        return self._panel().tblParameters

    def showPopupMenu(self):
        popupmenu = QMenu()

        if not (isinstance(self.param, ParameterMultipleInput) and
                self.param.datatype == dataobjects.TYPE_FILE):
            selectLayerAction = QAction(
                self.tr('Select from open layers'), self.pushButton)
            selectLayerAction.triggered.connect(self.showLayerSelectionDialog)
            popupmenu.addAction(selectLayerAction)

        selectFileAction = QAction(
            self.tr('Select from filesystem'), self.pushButton)
        selectFileAction.triggered.connect(self.showFileSelectionDialog)
        popupmenu.addAction(selectFileAction)

        popupmenu.exec_(QCursor.pos())

    def showLayerSelectionDialog(self):
        if (isinstance(self.param, ParameterRaster) or
                (isinstance(self.param, ParameterMultipleInput) and
                 self.param.datatype == dataobjects.TYPE_RASTER)):
            layers = dataobjects.getRasterLayers()
        elif isinstance(self.param, ParameterTable):
            layers = dataobjects.getTables()
        else:
            if isinstance(self.param, ParameterVector):
                datatype = self.param.datatype
            else:
                datatype = [self.param.datatype]
            layers = dataobjects.getVectorLayers(datatype)

        dlg = MultipleInputDialog([layer.name() for layer in layers])
        dlg.exec_()
        if dlg.selectedoptions is not None:
            selected = dlg.selectedoptions
            if len(selected) == 1:
                self.setValue(layers[selected[0]])
            else:
                if isinstance(self.param, ParameterMultipleInput):
                    self.text.setText(';'.join(layers[idx].name() for idx in selected))
                else:
                    rowdif = len(selected) - (self._table().rowCount() - self.row)
                    for i in range(rowdif):
                        self._panel().addRow()
                    for i, layeridx in enumerate(selected):
                        self._table().cellWidget(i + self.row,
                                                 self.col).setValue(layers[layeridx])

    def showFileSelectionDialog(self):
        settings = QSettings()
        text = unicode(self.text.text())
        if os.path.isdir(text):
            path = text
        elif os.path.isdir(os.path.dirname(text)):
            path = os.path.dirname(text)
        elif settings.contains('/Processing/LastInputPath'):
            path = unicode(settings.value('/Processing/LastInputPath'))
        else:
            path = ''

        ret, selected_filter = QFileDialog.getOpenFileNames(self, self.tr('Open file'), path,
                                                            self.tr('All files(*.*);;') + self.param.getFileFilter())
        if ret:
            files = list(ret)
            settings.setValue('/Processing/LastInputPath',
                              os.path.dirname(unicode(files[0])))
            for i, filename in enumerate(files):
                files[i] = dataobjects.getRasterSublayer(filename, self.param)
            if len(files) == 1:
                self.text.setText(files[0])
                self.textEditingFinished()
            else:
                if isinstance(self.param, ParameterMultipleInput):
                    self.text.setText(';'.join(unicode(f) for f in files))
                else:
                    rowdif = len(files) - (self._table().rowCount() - self.row)
                    for i in range(rowdif):
                        self._panel().addRow()
                    for i, f in enumerate(files):
                        self._table().cellWidget(i + self.row,
                                                 self.col).setValue(f)

    def textEditingFinished(self):
        self._value = self.text.text()
        self.valueChanged.emit()

    def value(self):
        return self._value

    def setValue(self, value):
        self._value = value
        if isinstance(value, QgsMapLayer):
            self.text.setText(value.name())
        else:  # should be basestring
            self.text.setText(value)
        self.valueChanged.emit()
