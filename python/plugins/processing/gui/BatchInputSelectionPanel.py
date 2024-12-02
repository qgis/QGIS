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

__author__ = "Victor Olaya"
__date__ = "August 2012"
__copyright__ = "(C) 2012, Victor Olaya"

import os
from pathlib import Path

from qgis.PyQt.QtCore import pyqtSignal, QCoreApplication
from qgis.PyQt.QtWidgets import (
    QWidget,
    QHBoxLayout,
    QMenu,
    QPushButton,
    QLineEdit,
    QSizePolicy,
    QAction,
    QFileDialog,
)
from qgis.PyQt.QtGui import QCursor

from qgis.core import (
    QgsMapLayer,
    QgsRasterLayer,
    QgsSettings,
    QgsProject,
    QgsProcessing,
    QgsProcessingUtils,
    QgsProcessingParameterMultipleLayers,
    QgsProcessingParameterRasterLayer,
    QgsProcessingParameterVectorLayer,
    QgsProcessingParameterMeshLayer,
    QgsProcessingParameterPointCloudLayer,
    QgsProcessingParameterFeatureSource,
    QgsProcessingParameterMapLayer,
)

from processing.gui.MultipleInputDialog import MultipleInputDialog

from processing.tools import dataobjects


class BatchInputSelectionPanel(QWidget):
    valueChanged = pyqtSignal()

    def __init__(self, param, row, col, dialog):
        super().__init__(None)
        self.param = param
        self.dialog = dialog
        self.row = row
        self.col = col
        self.horizontalLayout = QHBoxLayout(self)
        self.horizontalLayout.setSpacing(0)
        self.horizontalLayout.setMargin(0)
        self.text = QLineEdit()
        self.text.setObjectName("text")
        self.text.setMinimumWidth(300)
        self.setValue("")
        self.text.editingFinished.connect(self.textEditingFinished)
        self.text.setSizePolicy(
            QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Expanding
        )
        self.horizontalLayout.addWidget(self.text)
        self.pushButton = QPushButton()
        self.pushButton.setText("…")
        self.pushButton.clicked.connect(self.showPopupMenu)
        self.horizontalLayout.addWidget(self.pushButton)
        self.setLayout(self.horizontalLayout)

    def _panel(self):
        return self.dialog.mainWidget()

    def _table(self):
        return self._panel().tblParameters

    def showPopupMenu(self):
        popupmenu = QMenu()

        if not (
            isinstance(self.param, QgsProcessingParameterMultipleLayers)
            and self.param.layerType == dataobjects.TYPE_FILE
        ):
            selectLayerAction = QAction(
                QCoreApplication.translate(
                    "BatchInputSelectionPanel", "Select from Open Layers…"
                ),
                self.pushButton,
            )
            selectLayerAction.triggered.connect(self.showLayerSelectionDialog)
            popupmenu.addAction(selectLayerAction)

        selectFileAction = QAction(
            QCoreApplication.translate("BatchInputSelectionPanel", "Select Files…"),
            self.pushButton,
        )
        selectFileAction.triggered.connect(self.showFileSelectionDialog)
        popupmenu.addAction(selectFileAction)

        selectDirectoryAction = QAction(
            QCoreApplication.translate("BatchInputSelectionPanel", "Select Directory…"),
            self.pushButton,
        )
        selectDirectoryAction.triggered.connect(self.showDirectorySelectionDialog)
        popupmenu.addAction(selectDirectoryAction)

        popupmenu.exec(QCursor.pos())

    def showLayerSelectionDialog(self):
        layers = []
        if isinstance(self.param, QgsProcessingParameterRasterLayer) or (
            isinstance(self.param, QgsProcessingParameterMultipleLayers)
            and self.param.layerType() == QgsProcessing.SourceType.TypeRaster
        ):
            layers = QgsProcessingUtils.compatibleRasterLayers(QgsProject.instance())
        elif isinstance(self.param, QgsProcessingParameterVectorLayer):
            layers = QgsProcessingUtils.compatibleVectorLayers(QgsProject.instance())
        elif isinstance(self.param, QgsProcessingParameterMapLayer):
            layers = QgsProcessingUtils.compatibleLayers(QgsProject.instance())
        elif isinstance(self.param, QgsProcessingParameterMeshLayer) or (
            isinstance(self.param, QgsProcessingParameterMultipleLayers)
            and self.param.layerType() == QgsProcessing.SourceType.TypeMesh
        ):
            layers = QgsProcessingUtils.compatibleMeshLayers(QgsProject.instance())
        elif isinstance(self.param, QgsProcessingParameterPointCloudLayer) or (
            isinstance(self.param, QgsProcessingParameterMultipleLayers)
            and self.param.layerType() == QgsProcessing.SourceType.TypePointCloud
        ):
            layers = QgsProcessingUtils.compatiblePointCloudLayers(
                QgsProject.instance()
            )
        else:
            datatypes = [QgsProcessing.SourceType.TypeVectorAnyGeometry]
            if isinstance(self.param, QgsProcessingParameterFeatureSource):
                datatypes = self.param.dataTypes()
            elif isinstance(self.param, QgsProcessingParameterMultipleLayers):
                datatypes = [self.param.layerType()]

            if QgsProcessing.SourceType.TypeVectorAnyGeometry not in datatypes:
                layers = QgsProcessingUtils.compatibleVectorLayers(
                    QgsProject.instance(), datatypes
                )
            else:
                layers = QgsProcessingUtils.compatibleVectorLayers(
                    QgsProject.instance()
                )

        dlg = MultipleInputDialog([layer.name() for layer in layers])
        dlg.exec()

        def generate_layer_id(layer):
            # prefer layer name if unique
            if (
                len([l for l in layers if l.name().lower() == layer.name().lower()])
                == 1
            ):
                return layer.name()
            else:
                # otherwise fall back to layer id
                return layer.id()

        if dlg.selectedoptions is not None:
            selected = dlg.selectedoptions
            if len(selected) == 1:
                self.setValue(generate_layer_id(layers[selected[0]]))
            else:
                if isinstance(self.param, QgsProcessingParameterMultipleLayers):
                    self.text.setText(";".join(layers[idx].id() for idx in selected))
                else:
                    rowdif = len(selected) - (self._table().rowCount() - self.row)
                    for i in range(rowdif):
                        self._panel().addRow()
                    for i, layeridx in enumerate(selected):
                        self._table().cellWidget(i + self.row, self.col).setValue(
                            generate_layer_id(layers[layeridx])
                        )

    def showFileSelectionDialog(self):
        self.showFileDialog(seldir=False)

    def showDirectorySelectionDialog(self):
        self.showFileDialog(seldir=True)

    def showFileDialog(self, seldir):
        settings = QgsSettings()
        text = str(self.text.text())
        if os.path.isdir(text):
            path = text
        elif not seldir and os.path.isdir(os.path.dirname(text)):
            path = os.path.dirname(text)
        elif settings.contains("/Processing/LastInputPath"):
            path = str(settings.value("/Processing/LastInputPath"))
        else:
            path = ""

        if not seldir:
            ret, selected_filter = QFileDialog.getOpenFileNames(
                self, self.tr("Select Files"), path, self.param.createFileFilter()
            )
        else:
            ret = QFileDialog.getExistingDirectory(
                self, self.tr("Select Directory"), path
            )

        if ret:
            if seldir:
                settings.setValue("/Processing/LastInputPath", ret)

                files = []
                for pp in Path(ret).rglob("*"):
                    if not pp.is_file():
                        continue

                    p = pp.as_posix()

                    if (
                        isinstance(self.param, QgsProcessingParameterRasterLayer)
                        or (
                            isinstance(self.param, QgsProcessingParameterMultipleLayers)
                            and self.param.layerType()
                            == QgsProcessing.SourceType.TypeRaster
                        )
                    ) and not QgsRasterLayer.isValidRasterFileName(p):
                        continue

                    files.append(p)

                if not files:
                    return

            else:
                files = list(ret)
                settings.setValue(
                    "/Processing/LastInputPath", os.path.dirname(str(files[0]))
                )

            for i, filename in enumerate(files):
                files[i] = dataobjects.getRasterSublayer(filename, self.param)
            if len(files) == 1:
                self.text.setText(files[0])
                self.textEditingFinished()
            else:
                if isinstance(self.param, QgsProcessingParameterMultipleLayers):
                    self.text.setText(";".join(str(f) for f in files))
                else:
                    rowdif = len(files) - (self._table().rowCount() - self.row)
                    for i in range(rowdif):
                        self._panel().addRow()
                    for i, f in enumerate(files):
                        self._table().cellWidget(i + self.row, self.col).setValue(f)

    def textEditingFinished(self):
        self._value = self.text.text()
        self.valueChanged.emit()

    def getValue(self):
        return self._value if self._value else None

    def setValue(self, value):
        self._value = value
        if isinstance(value, QgsMapLayer):
            self.text.setText(value.name())
        else:  # should be basestring
            self.text.setText(value)
        self.valueChanged.emit()
