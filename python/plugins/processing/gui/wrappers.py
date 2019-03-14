# -*- coding: utf-8 -*-

"""
***************************************************************************
    wrappers.py - Standard parameters widget wrappers
    ---------------------
    Date                 : May 2016
    Copyright            : (C) 2016 by Arnaud Morvan, Victor Olaya
    Email                : arnaud dot morvan at camptocamp dot com
                           volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Arnaud Morvan'
__date__ = 'May 2016'
__copyright__ = '(C) 2016, Arnaud Morvan'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import locale
import os
import re
from functools import cmp_to_key
from inspect import isclass
from copy import deepcopy

from qgis.core import (
    QgsApplication,
    QgsUnitTypes,
    QgsCoordinateReferenceSystem,
    QgsExpression,
    QgsExpressionContextGenerator,
    QgsFieldProxyModel,
    QgsMapLayerProxyModel,
    QgsWkbTypes,
    QgsSettings,
    QgsProject,
    QgsMapLayer,
    QgsVectorLayer,
    QgsProcessing,
    QgsProcessingUtils,
    QgsProcessingParameterDefinition,
    QgsProcessingParameterBoolean,
    QgsProcessingParameterCrs,
    QgsProcessingParameterExtent,
    QgsProcessingParameterPoint,
    QgsProcessingParameterFile,
    QgsProcessingParameterMultipleLayers,
    QgsProcessingParameterNumber,
    QgsProcessingParameterDistance,
    QgsProcessingParameterRasterLayer,
    QgsProcessingParameterEnum,
    QgsProcessingParameterString,
    QgsProcessingParameterExpression,
    QgsProcessingParameterVectorLayer,
    QgsProcessingParameterMeshLayer,
    QgsProcessingParameterField,
    QgsProcessingParameterFeatureSource,
    QgsProcessingParameterMapLayer,
    QgsProcessingParameterBand,
    QgsProcessingParameterMatrix,
    QgsProcessingParameterDistance,
    QgsProcessingFeatureSourceDefinition,
    QgsProcessingOutputRasterLayer,
    QgsProcessingOutputVectorLayer,
    QgsProcessingOutputMapLayer,
    QgsProcessingOutputMultipleLayers,
    QgsProcessingOutputFile,
    QgsProcessingOutputString,
    QgsProcessingOutputNumber,
    QgsProcessingModelChildParameterSource,
    QgsProcessingModelAlgorithm,
    QgsRasterDataProvider,
    NULL,
    Qgis)

from qgis.PyQt.QtWidgets import (
    QCheckBox,
    QComboBox,
    QLabel,
    QDialog,
    QFileDialog,
    QHBoxLayout,
    QVBoxLayout,
    QLineEdit,
    QPlainTextEdit,
    QToolButton,
    QWidget,
)
from qgis.gui import (
    QgsGui,
    QgsExpressionLineEdit,
    QgsExpressionBuilderDialog,
    QgsFieldComboBox,
    QgsFieldExpressionWidget,
    QgsProjectionSelectionDialog,
    QgsMapLayerComboBox,
    QgsProjectionSelectionWidget,
    QgsRasterBandComboBox,
    QgsProcessingGui,
    QgsAbstractProcessingParameterWidgetWrapper
)
from qgis.PyQt.QtCore import pyqtSignal, QObject, QVariant, Qt
from qgis.utils import iface

from processing.core.ProcessingConfig import ProcessingConfig
from processing.modeler.MultilineTextPanel import MultilineTextPanel

from processing.gui.NumberInputPanel import NumberInputPanel, ModelerNumberInputPanel, DistanceInputPanel
from processing.gui.RangePanel import RangePanel
from processing.gui.PointSelectionPanel import PointSelectionPanel
from processing.gui.FileSelectionPanel import FileSelectionPanel
from processing.gui.CheckboxesPanel import CheckboxesPanel
from processing.gui.MultipleInputPanel import MultipleInputPanel
from processing.gui.BatchInputSelectionPanel import BatchInputSelectionPanel
from processing.gui.FixedTablePanel import FixedTablePanel
from processing.gui.ExtentSelectionPanel import ExtentSelectionPanel
from processing.gui.ParameterGuiUtils import getFileFilter

from processing.tools import dataobjects

DIALOG_STANDARD = QgsProcessingGui.Standard
DIALOG_BATCH = QgsProcessingGui.Batch
DIALOG_MODELER = QgsProcessingGui.Modeler


class InvalidParameterValue(Exception):
    pass


dialogTypes = {"AlgorithmDialog": DIALOG_STANDARD,
               "ModelerParametersDialog": DIALOG_MODELER,
               "BatchAlgorithmDialog": DIALOG_BATCH}


def getExtendedLayerName(layer):
    authid = layer.crs().authid()
    if ProcessingConfig.getSetting(ProcessingConfig.SHOW_CRS_DEF) and authid is not None:
        return u'{} [{}]'.format(layer.name(), authid)
    else:
        return layer.name()


class WidgetWrapper(QgsAbstractProcessingParameterWidgetWrapper):

    NOT_SET_OPTION = '~~~~!!!!NOT SET!!!!~~~~~~~'

    def __init__(self, param, dialog, row=0, col=0, **kwargs):
        self.dialogType = dialogTypes.get(dialog.__class__.__name__, QgsProcessingGui.Standard)
        super().__init__(param, self.dialogType)

        self.dialog = dialog
        self.row = row
        self.col = col

        self.widget = self.createWidget(**kwargs)
        self.label = self.createLabel()
        if param.defaultValue() is not None:
            self.setValue(param.defaultValue())

    def comboValue(self, validator=None, combobox=None):
        if combobox is None:
            combobox = self.widget
        idx = combobox.findText(combobox.currentText())
        if idx < 0:
            v = combobox.currentText().strip()
            if validator is not None and not validator(v):
                raise InvalidParameterValue()
            return v
        if combobox.currentData() == self.NOT_SET_OPTION:
            return None
        elif combobox.currentData() is not None:
            return combobox.currentData()
        else:
            return combobox.currentText()

    def createWidget(self, **kwargs):
        pass

    def createLabel(self):
        if self.dialogType == DIALOG_BATCH:
            return None
        desc = self.parameterDefinition().description()
        if isinstance(self.parameterDefinition(), QgsProcessingParameterExtent):
            desc += self.tr(' (xmin, xmax, ymin, ymax)')
        if isinstance(self.parameterDefinition(), QgsProcessingParameterPoint):
            desc += self.tr(' (x, y)')
        if self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional:
            desc += self.tr(' [optional]')

        label = QLabel(desc)
        label.setToolTip(self.parameterDefinition().name())
        return label

    def setValue(self, value):
        pass

    def value(self):
        return None

    def widgetValue(self):
        return self.value()

    def setWidgetValue(self, value, context):
        self.setValue(value)

    def setComboValue(self, value, combobox=None):
        if combobox is None:
            combobox = self.widget
        if isinstance(value, list):
            if value:
                value = value[0]
            else:
                value = None
        values = [combobox.itemData(i) for i in range(combobox.count())]
        try:
            idx = values.index(value)
            combobox.setCurrentIndex(idx)
            return
        except ValueError:
            pass
        if combobox.isEditable():
            if value is not None:
                combobox.setEditText(str(value))
        else:
            combobox.setCurrentIndex(0)

    def refresh(self):
        pass

    def getFileName(self, initial_value=''):
        """Shows a file open dialog"""
        settings = QgsSettings()
        if os.path.isdir(initial_value):
            path = initial_value
        elif os.path.isdir(os.path.dirname(initial_value)):
            path = os.path.dirname(initial_value)
        elif settings.contains('/Processing/LastInputPath'):
            path = str(settings.value('/Processing/LastInputPath'))
        else:
            path = ''

        # TODO: should use selectedFilter argument for default file format
        filename, selected_filter = QFileDialog.getOpenFileName(self.widget, self.tr('Select File'),
                                                                path, getFileFilter(self.parameterDefinition()))
        if filename:
            settings.setValue('/Processing/LastInputPath',
                              os.path.dirname(str(filename)))
        return filename, selected_filter


class BasicWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        return QLineEdit()

    def setValue(self, value):
        self.widget.setText(value)

    def value(self):
        return self.widget.text()


class BooleanWidgetWrapper(WidgetWrapper):

    def createLabel(self):
        if self.dialogType == DIALOG_STANDARD:
            return None
        else:
            return super().createLabel()

    def createWidget(self):
        if self.dialogType == DIALOG_STANDARD:
            return QCheckBox()
        elif self.dialogType == DIALOG_BATCH:
            widget = QComboBox()
            widget.addItem(self.tr('Yes'), True)
            widget.addItem(self.tr('No'), False)
            return widget
        else:
            widget = QComboBox()
            widget.addItem(self.tr('Yes'), True)
            widget.addItem(self.tr('No'), False)
            bools = self.dialog.getAvailableValuesOfType(QgsProcessingParameterBoolean, None)
            for b in bools:
                widget.addItem(self.dialog.resolveValueDescription(b), b)
            return widget

    def setValue(self, value):
        if value is None or value == NULL:
            return
        if self.dialogType == DIALOG_STANDARD:
            self.widget.setChecked(value)
        else:
            self.setComboValue(value)

    def value(self):
        if self.dialogType == DIALOG_STANDARD:
            return self.widget.isChecked()
        else:
            return self.comboValue()


class CrsWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        if self.dialogType == DIALOG_MODELER:
            self.combo = QComboBox()
            widget = QWidget()
            layout = QHBoxLayout()
            layout.setMargin(0)
            layout.setContentsMargins(0, 0, 0, 0)
            layout.setSpacing(1)
            layout.addWidget(self.combo)
            btn = QToolButton()
            btn.setIcon(QgsApplication.getThemeIcon("mActionSetProjection.svg"))
            btn.setToolTip(self.tr("Select CRS"))
            btn.clicked.connect(self.selectProjection)
            layout.addWidget(btn)

            widget.setLayout(layout)
            self.combo.setEditable(True)
            crss = self.dialog.getAvailableValuesOfType((QgsProcessingParameterCrs, QgsProcessingParameterString), QgsProcessingOutputString)
            for crs in crss:
                self.combo.addItem(self.dialog.resolveValueDescription(crs), crs)
            layers = self.dialog.getAvailableValuesOfType([QgsProcessingParameterRasterLayer,
                                                           QgsProcessingParameterVectorLayer,
                                                           QgsProcessingParameterMeshLayer,
                                                           QgsProcessingParameterFeatureSource],
                                                          [QgsProcessingOutputVectorLayer,
                                                           QgsProcessingOutputRasterLayer,
                                                           QgsProcessingOutputMapLayer])
            for l in layers:
                self.combo.addItem("Crs of layer " + self.dialog.resolveValueDescription(l), l)
            if self.parameterDefinition().defaultValue():
                self.combo.setEditText(self.parameterDefinition().defaultValue())
            return widget
        else:
            widget = QgsProjectionSelectionWidget()
            if self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional:
                widget.setOptionVisible(QgsProjectionSelectionWidget.CrsNotSet, True)

            if self.parameterDefinition().defaultValue():
                if self.parameterDefinition().defaultValue() == 'ProjectCrs':
                    crs = QgsProject.instance().crs()
                else:
                    crs = QgsCoordinateReferenceSystem(self.parameterDefinition().defaultValue())
                widget.setCrs(crs)
            else:
                widget.setOptionVisible(QgsProjectionSelectionWidget.CrsNotSet, True)

            widget.crsChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
            return widget

    def selectProjection(self):
        dialog = QgsProjectionSelectionDialog(self.widget)
        current_crs = QgsCoordinateReferenceSystem(self.combo.currentText())
        if current_crs.isValid():
            dialog.setCrs(current_crs)

        if dialog.exec_():
            self.setValue(dialog.crs().authid())

    def setValue(self, value):
        if value is None or value == NULL:
            return

        if self.dialogType == DIALOG_MODELER:
            self.setComboValue(value, self.combo)
        elif value == 'ProjectCrs':
            self.widget.setCrs(QgsProject.instance().crs())
        else:
            self.widget.setCrs(QgsCoordinateReferenceSystem(value))

    def value(self):
        if self.dialogType == DIALOG_MODELER:
            return self.comboValue(combobox=self.combo)
        else:
            crs = self.widget.crs()
            if crs.isValid():
                return self.widget.crs().authid()
            else:
                return None


class ExtentWidgetWrapper(WidgetWrapper):
    USE_MIN_COVERING_EXTENT = "[Use min covering extent]"

    def createWidget(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            widget = ExtentSelectionPanel(self.dialog, self.parameterDefinition())
            widget.hasChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
            return widget
        else:
            widget = QComboBox()
            widget.setEditable(True)
            extents = self.dialog.getAvailableValuesOfType(QgsProcessingParameterExtent, (QgsProcessingOutputString))
            if self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional:
                widget.addItem(self.USE_MIN_COVERING_EXTENT, None)
            layers = self.dialog.getAvailableValuesOfType([QgsProcessingParameterFeatureSource,
                                                           QgsProcessingParameterRasterLayer,
                                                           QgsProcessingParameterVectorLayer,
                                                           QgsProcessingParameterMeshLayer],
                                                          [QgsProcessingOutputRasterLayer,
                                                           QgsProcessingOutputVectorLayer,
                                                           QgsProcessingOutputMapLayer])
            for ex in extents:
                widget.addItem(self.dialog.resolveValueDescription(ex), ex)
            for l in layers:
                widget.addItem("Extent of " + self.dialog.resolveValueDescription(l), l)
            if not self.parameterDefinition().defaultValue():
                widget.setEditText(self.parameterDefinition().defaultValue())
            return widget

    def setValue(self, value):
        if value is None or value == NULL:
            return

        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            self.widget.setExtentFromString(value)
        else:
            self.setComboValue(value)

    def value(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            return self.widget.getValue()
        else:
            idx = self.widget.findText(self.widget.currentText())
            if idx < 0:
                s = str(self.widget.currentText()).strip()
                if s:
                    try:
                        tokens = s.split(',')
                        if len(tokens) != 4:
                            raise InvalidParameterValue()
                        for token in tokens:
                            float(token)
                    except:
                        raise InvalidParameterValue()
                elif self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional:
                    s = None
                else:
                    raise InvalidParameterValue()
                return s
            else:
                return self.widget.currentData()


class PointWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            return PointSelectionPanel(self.dialog, self.parameterDefinition().defaultValue())
        else:
            item = QComboBox()
            item.setEditable(True)
            points = self.dialog.getAvailableValuesOfType((QgsProcessingParameterPoint, QgsProcessingParameterString), (QgsProcessingOutputString))
            for p in points:
                item.addItem(self.dialog.resolveValueDescription(p), p)
            item.setEditText(str(self.parameterDefinition().defaultValue()))
            return item

    def setValue(self, value):
        if value is None or value == NULL:
            return

        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            self.widget.setPointFromString(value)
        else:
            self.setComboValue(value)

    def value(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            return self.widget.getValue()
        else:
            idx = self.widget.findText(self.widget.currentText())
            if idx < 0:
                s = str(self.widget.currentText()).strip()
                if s:
                    try:
                        tokens = s.split(',')
                        if len(tokens) != 2:
                            raise InvalidParameterValue()
                        for token in tokens:
                            float(token)
                    except:
                        raise InvalidParameterValue()
                elif self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional:
                    s = None
                else:
                    raise InvalidParameterValue()
                return s
            else:
                return self.widget.currentData()


class FileWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            return FileSelectionPanel(self.parameterDefinition().behavior() == QgsProcessingParameterFile.Folder,
                                      self.parameterDefinition().extension())
        else:
            self.combo = QComboBox()
            self.combo.setEditable(True)
            files = self.dialog.getAvailableValuesOfType(QgsProcessingParameterFile, (QgsProcessingOutputRasterLayer, QgsProcessingOutputVectorLayer, QgsProcessingOutputMapLayer, QgsProcessingOutputFile, QgsProcessingOutputString))
            for f in files:
                self.combo.addItem(self.dialog.resolveValueDescription(f), f)
            if self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional:
                self.combo.setEditText("")
            widget = QWidget()
            layout = QHBoxLayout()
            layout.setMargin(0)
            layout.setContentsMargins(0, 0, 0, 0)
            layout.setSpacing(6)
            layout.addWidget(self.combo)
            btn = QToolButton()
            btn.setText('…')
            btn.setToolTip(self.tr("Select file"))
            btn.clicked.connect(self.selectFile)
            layout.addWidget(btn)
            widget.setLayout(layout)
            return widget

    def selectFile(self):
        settings = QgsSettings()
        if os.path.isdir(os.path.dirname(self.combo.currentText())):
            path = os.path.dirname(self.combo.currentText())
        if settings.contains('/Processing/LastInputPath'):
            path = settings.value('/Processing/LastInputPath')
        else:
            path = ''

        if self.parameterDefinition().extension():
            filter = self.tr('{} files').format(
                self.parameterDefinition().extension().upper()) + ' (*.' + self.parameterDefinition().extension() + self.tr(
                ');;All files (*.*)')
        else:
            filter = self.tr('All files (*.*)')

        filename, selected_filter = QFileDialog.getOpenFileName(self.widget,
                                                                self.tr('Select File'), path,
                                                                filter)
        if filename:
            self.combo.setEditText(filename)

    def setValue(self, value):
        if value is None or value == NULL:
            return

        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            self.widget.setText(value)
        else:
            self.setComboValue(value, combobox=self.combo)

    def value(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            return self.widget.getValue()
        else:
            return self.comboValue(combobox=self.combo)


class FixedTableWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            return FixedTablePanel(self.parameterDefinition())
        else:
            self.combobox = QComboBox()
            values = self.dialog.getAvailableValuesOfType(QgsProcessingParameterMatrix)
            for v in values:
                self.combobox.addItem(self.dialog.resolveValueDescription(v), v)
            return self.combobox

    def setValue(self, value):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            self.widget.setValue(value)
        else:
            self.setComboValue(value, combobox=self.combobox)

    def value(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            return self.widget.table
        else:
            return self.comboValue(combobox=self.combobox)


class MultipleLayerWidgetWrapper(WidgetWrapper):

    def _getOptions(self):
        if self.parameterDefinition().layerType() == QgsProcessing.TypeVectorAnyGeometry:
            options = self.dialog.getAvailableValuesOfType((QgsProcessingParameterFeatureSource,
                                                            QgsProcessingParameterVectorLayer,
                                                            QgsProcessingParameterMultipleLayers),
                                                           [QgsProcessingOutputVectorLayer,
                                                            QgsProcessingOutputMapLayer,
                                                            QgsProcessingOutputMultipleLayers])
        elif self.parameterDefinition().layerType() == QgsProcessing.TypeVector:
            options = self.dialog.getAvailableValuesOfType((QgsProcessingParameterFeatureSource,
                                                            QgsProcessingParameterVectorLayer,
                                                            QgsProcessingParameterMultipleLayers),
                                                           [QgsProcessingOutputVectorLayer,
                                                            QgsProcessingOutputMapLayer,
                                                            QgsProcessingOutputMultipleLayers],
                                                           [QgsProcessing.TypeVector])
        elif self.parameterDefinition().layerType() == QgsProcessing.TypeVectorPoint:
            options = self.dialog.getAvailableValuesOfType((QgsProcessingParameterFeatureSource,
                                                            QgsProcessingParameterVectorLayer,
                                                            QgsProcessingParameterMultipleLayers),
                                                           [QgsProcessingOutputVectorLayer,
                                                            QgsProcessingOutputMapLayer,
                                                            QgsProcessingOutputMultipleLayers],
                                                           [QgsProcessing.TypeVectorPoint,
                                                            QgsProcessing.TypeVectorAnyGeometry])
        elif self.parameterDefinition().layerType() == QgsProcessing.TypeVectorLine:
            options = self.dialog.getAvailableValuesOfType((QgsProcessingParameterFeatureSource,
                                                            QgsProcessingParameterVectorLayer,
                                                            QgsProcessingParameterMultipleLayers),
                                                           [QgsProcessingOutputVectorLayer,
                                                            QgsProcessingOutputMapLayer,
                                                            QgsProcessingOutputMultipleLayers],
                                                           [QgsProcessing.TypeVectorLine,
                                                            QgsProcessing.TypeVectorAnyGeometry])
        elif self.parameterDefinition().layerType() == QgsProcessing.TypeVectorPolygon:
            options = self.dialog.getAvailableValuesOfType((QgsProcessingParameterFeatureSource,
                                                            QgsProcessingParameterVectorLayer,
                                                            QgsProcessingParameterMultipleLayers),
                                                           [QgsProcessingOutputVectorLayer,
                                                            QgsProcessingOutputMapLayer,
                                                            QgsProcessingOutputMultipleLayers],
                                                           [QgsProcessing.TypeVectorPolygon,
                                                            QgsProcessing.TypeVectorAnyGeometry])
        elif self.parameterDefinition().layerType() == QgsProcessing.TypeRaster:
            options = self.dialog.getAvailableValuesOfType(
                (QgsProcessingParameterRasterLayer, QgsProcessingParameterMultipleLayers),
                [QgsProcessingOutputRasterLayer,
                 QgsProcessingOutputMapLayer,
                 QgsProcessingOutputMultipleLayers])
        elif self.parameterDefinition().layerType() == QgsProcessing.TypeMesh:
            options = self.dialog.getAvailableValuesOfType(
                (QgsProcessingParameterMeshLayer, QgsProcessingParameterMultipleLayers),
                [])
        elif self.parameterDefinition().layerType() == QgsProcessing.TypeMapLayer:
            options = self.dialog.getAvailableValuesOfType((QgsProcessingParameterRasterLayer,
                                                            QgsProcessingParameterFeatureSource,
                                                            QgsProcessingParameterVectorLayer,
                                                            QgsProcessingParameterMeshLayer,
                                                            QgsProcessingParameterMultipleLayers),
                                                           [QgsProcessingOutputRasterLayer,
                                                            QgsProcessingOutputVectorLayer,
                                                            QgsProcessingOutputMapLayer,
                                                            QgsProcessingOutputMultipleLayers])
        else:
            options = self.dialog.getAvailableValuesOfType(QgsProcessingParameterFile, QgsProcessingOutputFile)
        options = sorted(options, key=lambda opt: self.dialog.resolveValueDescription(opt))
        return options

    def createWidget(self):
        if self.dialogType == DIALOG_STANDARD:
            if self.parameterDefinition().layerType() == QgsProcessing.TypeFile:
                return MultipleInputPanel(datatype=QgsProcessing.TypeFile)
            else:
                if self.parameterDefinition().layerType() == QgsProcessing.TypeRaster:
                    options = QgsProcessingUtils.compatibleRasterLayers(QgsProject.instance(), False)
                elif self.parameterDefinition().layerType() == QgsProcessing.TypeMesh:
                    options = QgsProcessingUtils.compatibleMeshLayers(QgsProject.instance(), False)
                elif self.parameterDefinition().layerType() in (QgsProcessing.TypeVectorAnyGeometry, QgsProcessing.TypeVector):
                    options = QgsProcessingUtils.compatibleVectorLayers(QgsProject.instance(), [], False)
                elif self.parameterDefinition().layerType() == QgsProcessing.TypeMapLayer:
                    options = QgsProcessingUtils.compatibleVectorLayers(QgsProject.instance(), [], False)
                    options.extend(QgsProcessingUtils.compatibleRasterLayers(QgsProject.instance(), False))
                    options.extend(QgsProcessingUtils.compatibleMeshLayers(QgsProject.instance(), False))
                else:
                    options = QgsProcessingUtils.compatibleVectorLayers(QgsProject.instance(), [self.parameterDefinition().layerType()],
                                                                        False)
                opts = [getExtendedLayerName(opt) for opt in options]
                return MultipleInputPanel(opts, datatype=self.parameterDefinition().layerType())
        elif self.dialogType == DIALOG_BATCH:
            widget = BatchInputSelectionPanel(self.parameterDefinition(), self.row, self.col, self.dialog)
            widget.valueChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
            return widget
        else:
            options = [self.dialog.resolveValueDescription(opt) for opt in self._getOptions()]
            return MultipleInputPanel(options, datatype=self.parameterDefinition().layerType())

    def refresh(self):
        if self.parameterDefinition().layerType() != QgsProcessing.TypeFile:
            if self.parameterDefinition().layerType() == QgsProcessing.TypeRaster:
                options = QgsProcessingUtils.compatibleRasterLayers(QgsProject.instance(), False)
            elif self.parameterDefinition().layerType() == QgsProcessing.TypeMesh:
                options = QgsProcessingUtils.compatibleMeshLayers(QgsProject.instance(), False)
            elif self.parameterDefinition().layerType() in (QgsProcessing.TypeVectorAnyGeometry, QgsProcessing.TypeVector):
                options = QgsProcessingUtils.compatibleVectorLayers(QgsProject.instance(), [], False)
            elif self.parameterDefinition().layerType() == QgsProcessing.TypeMapLayer:
                options = QgsProcessingUtils.compatibleVectorLayers(QgsProject.instance(), [], False)
                options.extend(QgsProcessingUtils.compatibleRasterLayers(QgsProject.instance(), False))
                options.extend(QgsProcessingUtils.compatibleMeshLayers(QgsProject.instance(), False))
            else:
                options = QgsProcessingUtils.compatibleVectorLayers(QgsProject.instance(), [self.parameterDefinition().layerType()],
                                                                    False)
            opts = [getExtendedLayerName(opt) for opt in options]
            self.widget.updateForOptions(opts)

    def setValue(self, value):
        if value is None or value == NULL:
            return

        if self.dialogType == DIALOG_STANDARD:
            pass  # TODO
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.setValue(value)
        else:
            options = self._getOptions()

            if not isinstance(value, (tuple, list)):
                value = [value]

            selected_options = []
            for sel in value:
                if sel in options:
                    selected_options.append(options.index(sel))
                elif isinstance(sel, QgsProcessingModelChildParameterSource):
                    selected_options.append(sel.staticValue())
                else:
                    selected_options.append(sel)

            self.widget.setSelectedItems(selected_options)

    def value(self):
        if self.dialogType == DIALOG_STANDARD:
            if self.parameterDefinition().layerType() == QgsProcessing.TypeFile:
                return self.widget.selectedoptions
            else:
                if self.parameterDefinition().layerType() == QgsProcessing.TypeRaster:
                    options = QgsProcessingUtils.compatibleRasterLayers(QgsProject.instance(), False)
                elif self.parameterDefinition().layerType() == QgsProcessing.TypeMesh:
                    options = QgsProcessingUtils.compatibleMeshLayers(QgsProject.instance(), False)
                elif self.parameterDefinition().layerType() in (QgsProcessing.TypeVectorAnyGeometry, QgsProcessing.TypeVector):
                    options = QgsProcessingUtils.compatibleVectorLayers(QgsProject.instance(), [], False)
                elif self.parameterDefinition().layerType() == QgsProcessing.TypeMapLayer:
                    options = QgsProcessingUtils.compatibleVectorLayers(QgsProject.instance(), [], False)
                    options.extend(QgsProcessingUtils.compatibleRasterLayers(QgsProject.instance(), False))
                    options.extend(QgsProcessingUtils.compatibleMeshLayers(QgsProject.instance(), False))
                else:
                    options = QgsProcessingUtils.compatibleVectorLayers(QgsProject.instance(), [self.parameterDefinition().layerType()],
                                                                        False)
                return [options[i] if isinstance(i, int) else i for i in self.widget.selectedoptions]
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.value()
        else:
            options = self._getOptions()
            values = [options[i] if isinstance(i, int) else QgsProcessingModelChildParameterSource.fromStaticValue(i)
                      for i in self.widget.selectedoptions]
            if len(values) == 0 and not self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional:
                raise InvalidParameterValue()
            return values


class NumberWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            widget = NumberInputPanel(self.parameterDefinition())
            widget.hasChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
            return widget
        else:
            return ModelerNumberInputPanel(self.parameterDefinition(), self.dialog)

    def setValue(self, value):
        if value is None or value == NULL:
            return

        self.widget.setValue(value)

    def value(self):
        return self.widget.getValue()

    def postInitialize(self, wrappers):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH) and self.parameterDefinition().isDynamic():
            for wrapper in wrappers:
                if wrapper.parameterDefinition().name() == self.parameterDefinition().dynamicLayerParameterName():
                    self.widget.setDynamicLayer(wrapper.parameterValue())
                    wrapper.widgetValueHasChanged.connect(self.parentLayerChanged)
                    break

    def parentLayerChanged(self, wrapper):
        self.widget.setDynamicLayer(wrapper.parameterValue())


class DistanceWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            widget = DistanceInputPanel(self.parameterDefinition())
            widget.hasChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
            return widget
        else:
            return ModelerNumberInputPanel(self.parameterDefinition(), self.dialog)

    def setValue(self, value):
        if value is None or value == NULL:
            return

        self.widget.setValue(value)

    def value(self):
        return self.widget.getValue()

    def postInitialize(self, wrappers):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            for wrapper in wrappers:
                if wrapper.parameterDefinition().name() == self.parameterDefinition().dynamicLayerParameterName():
                    self.widget.setDynamicLayer(wrapper.parameterValue())
                    wrapper.widgetValueHasChanged.connect(self.dynamicLayerChanged)
                if wrapper.parameterDefinition().name() == self.parameterDefinition().parentParameterName():
                    self.widget.setUnitParameterValue(wrapper.parameterValue())
                    wrapper.widgetValueHasChanged.connect(self.parentParameterChanged)

    def dynamicLayerChanged(self, wrapper):
        self.widget.setDynamicLayer(wrapper.parameterValue())

    def parentParameterChanged(self, wrapper):
        self.widget.setUnitParameterValue(wrapper.parameterValue())


class RangeWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        widget = RangePanel(self.parameterDefinition())
        widget.hasChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
        return widget

    def setValue(self, value):
        if value is None or value == NULL:
            return

        self.widget.setValue(value)

    def value(self):
        return self.widget.getValue()


class MapLayerWidgetWrapper(WidgetWrapper):
    NOT_SELECTED = '[Not selected]'

    def createWidget(self):
        if self.dialogType == DIALOG_STANDARD:
            widget = QWidget()
            layout = QHBoxLayout()
            layout.setMargin(0)
            layout.setContentsMargins(0, 0, 0, 0)
            layout.setSpacing(6)
            self.combo = QgsMapLayerComboBox()
            layout.addWidget(self.combo)
            btn = QToolButton()
            btn.setText('…')
            btn.setToolTip(self.tr("Select file"))
            btn.clicked.connect(self.selectFile)
            layout.addWidget(btn)

            widget.setLayout(layout)
            if ProcessingConfig.getSetting(ProcessingConfig.SHOW_CRS_DEF):
                self.combo.setShowCrs(True)

            self.setComboBoxFilters(self.combo)

            try:
                self.combo.setLayer(iface.activeLayer())
            except:
                pass

            if self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional:
                self.combo.setAllowEmptyLayer(True)
                self.combo.setLayer(None)

            self.combo.currentIndexChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
            self.combo.currentTextChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
            return widget
        elif self.dialogType == DIALOG_BATCH:
            widget = BatchInputSelectionPanel(self.parameterDefinition(), self.row, self.col, self.dialog)
            widget.valueChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
            return widget
        else:
            self.combo = QComboBox()
            layers = self.getAvailableLayers()
            self.combo.setEditable(True)
            if self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional:
                self.combo.addItem(self.NOT_SELECTED, self.NOT_SET_OPTION)
            for layer in layers:
                self.combo.addItem(self.dialog.resolveValueDescription(layer), layer)

            widget = QWidget()
            layout = QHBoxLayout()
            layout.setMargin(0)
            layout.setContentsMargins(0, 0, 0, 0)
            layout.setSpacing(6)
            layout.addWidget(self.combo)
            btn = QToolButton()
            btn.setText('…')
            btn.setToolTip(self.tr("Select file"))
            btn.clicked.connect(self.selectFile)
            layout.addWidget(btn)
            widget.setLayout(layout)
            return widget

    def setComboBoxFilters(self, combo):
        pass

    def getAvailableLayers(self):
        return self.dialog.getAvailableValuesOfType(
            [QgsProcessingParameterRasterLayer, QgsProcessingParameterMeshLayer, QgsProcessingParameterVectorLayer, QgsProcessingParameterMapLayer, QgsProcessingParameterString],
            [QgsProcessingOutputRasterLayer, QgsProcessingOutputVectorLayer, QgsProcessingOutputMapLayer, QgsProcessingOutputString, QgsProcessingOutputFile])

    def selectFile(self):
        filename, selected_filter = self.getFileName(self.combo.currentText())
        if filename:
            if isinstance(self.combo, QgsMapLayerComboBox):
                items = self.combo.additionalItems()
                items.append(filename)
                self.combo.setAdditionalItems(items)
                self.combo.setCurrentIndex(self.combo.findText(filename))
            else:
                self.combo.setEditText(filename)
            self.widgetValueHasChanged.emit(self)

    def setValue(self, value):
        if value is None or value == NULL:
            return

        if self.dialogType == DIALOG_STANDARD:
            if isinstance(value, str):
                layer = QgsProject.instance().mapLayer(value)
                if layer is not None:
                    value = layer

            found = False
            if isinstance(value, QgsMapLayer):
                self.combo.setLayer(value)
                found = self.combo.currentIndex() != -1

            if not found:
                if self.combo.findText(value) >= 0:
                    self.combo.setCurrentIndex(self.combo.findText(value))
                else:
                    items = self.combo.additionalItems()
                    items.append(value)
                    self.combo.setAdditionalItems(items)
                    self.combo.setCurrentIndex(self.combo.findText(value))
        elif self.dialogType == DIALOG_BATCH:
            self.widget.setValue(value)
        else:
            self.setComboValue(value, combobox=self.combo)
        self.widgetValueHasChanged.emit(self)

    def value(self):
        if self.dialogType == DIALOG_STANDARD:
            try:
                layer = self.combo.currentLayer()
                if layer is not None:
                    return layer
                else:
                    return self.combo.currentText() or None
            except:
                return self.combo.currentText()
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.value()
        else:
            def validator(v):
                if not bool(v):
                    return self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional
                else:
                    return os.path.exists(v)

            return self.comboValue(validator, combobox=self.combo)


class RasterWidgetWrapper(MapLayerWidgetWrapper):

    def getAvailableLayers(self):
        return self.dialog.getAvailableValuesOfType((QgsProcessingParameterRasterLayer, QgsProcessingParameterString),
                                                    (QgsProcessingOutputRasterLayer, QgsProcessingOutputFile, QgsProcessingOutputString))

    def setComboBoxFilters(self, combo):
        combo.setFilters(QgsMapLayerProxyModel.RasterLayer)
        combo.setExcludedProviders(['grass'])

    def selectFile(self):
        filename, selected_filter = self.getFileName(self.combo.currentText())
        if filename:
            filename = dataobjects.getRasterSublayer(filename, self.parameterDefinition())
            if isinstance(self.combo, QgsMapLayerComboBox):
                items = self.combo.additionalItems()
                items.append(filename)
                self.combo.setAdditionalItems(items)
                self.combo.setCurrentIndex(self.combo.findText(filename))
            else:
                self.combo.setEditText(filename)
            self.widgetValueHasChanged.emit(self)


class MeshWidgetWrapper(MapLayerWidgetWrapper):

    def getAvailableLayers(self):
        return self.dialog.getAvailableValuesOfType((QgsProcessingParameterMeshLayer, QgsProcessingParameterString),
                                                    ())

    def setComboBoxFilters(self, combo):
        combo.setFilters(QgsMapLayerProxyModel.MeshLayer)

    def selectFile(self):
        filename, selected_filter = self.getFileName(self.combo.currentText())
        if filename:
            if isinstance(self.combo, QgsMapLayerComboBox):
                items = self.combo.additionalItems()
                items.append(filename)
                self.combo.setAdditionalItems(items)
                self.combo.setCurrentIndex(self.combo.findText(filename))
            else:
                self.combo.setEditText(filename)
            self.widgetValueHasChanged.emit(self)


class EnumWidgetWrapper(WidgetWrapper):
    NOT_SELECTED = '[Not selected]'

    def createWidget(self, useCheckBoxes=False, columns=1):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            self._useCheckBoxes = useCheckBoxes
            if self._useCheckBoxes and not self.dialogType == DIALOG_BATCH:
                return CheckboxesPanel(options=self.parameterDefinition().options(),
                                       multiple=self.parameterDefinition().allowMultiple(),
                                       columns=columns)
            if self.parameterDefinition().allowMultiple():
                return MultipleInputPanel(options=self.parameterDefinition().options())
            else:
                widget = QComboBox()
                for i, option in enumerate(self.parameterDefinition().options()):
                    widget.addItem(option, i)
                if self.parameterDefinition().defaultValue():
                    widget.setCurrentIndex(widget.findData(self.parameterDefinition().defaultValue()))
                return widget
        else:
            self.combobox = QComboBox()
            if self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional:
                self.combobox.addItem(self.NOT_SELECTED, self.NOT_SET_OPTION)
            for i, option in enumerate(self.parameterDefinition().options()):
                self.combobox.addItem(option, i)
            values = self.dialog.getAvailableValuesOfType(QgsProcessingParameterEnum)
            for v in values:
                self.combobox.addItem(self.dialog.resolveValueDescription(v), v)
            return self.combobox

    def setValue(self, value):
        if value is None or value == NULL:
            return

        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            if self._useCheckBoxes and not self.dialogType == DIALOG_BATCH:
                self.widget.setValue(value)
                return
            if self.parameterDefinition().allowMultiple():
                self.widget.setSelectedItems(value)
            else:
                self.widget.setCurrentIndex(self.widget.findData(value))
        else:
            self.setComboValue(value, combobox=self.combobox)

    def value(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            if self._useCheckBoxes and not self.dialogType == DIALOG_BATCH:
                return self.widget.value()
            if self.parameterDefinition().allowMultiple():
                return self.widget.selectedoptions
            else:
                return self.widget.currentData()
        else:
            return self.comboValue(combobox=self.combobox)


class FeatureSourceWidgetWrapper(WidgetWrapper):
    NOT_SELECTED = '[Not selected]'

    def createWidget(self):
        self.fileBasedLayers = {}
        if self.dialogType == DIALOG_STANDARD:
            widget = QWidget()
            layout = QHBoxLayout()
            layout.setMargin(0)
            layout.setContentsMargins(0, 0, 0, 0)
            layout.setSpacing(6)
            self.combo = QgsMapLayerComboBox()
            layout.addWidget(self.combo)
            layout.setAlignment(self.combo, Qt.AlignTop)
            btn = QToolButton()
            btn.setText('…')
            btn.setToolTip(self.tr("Select file"))
            btn.clicked.connect(self.selectFile)
            layout.addWidget(btn)
            layout.setAlignment(btn, Qt.AlignTop)

            vl = QVBoxLayout()
            vl.setMargin(0)
            vl.setContentsMargins(0, 0, 0, 0)
            vl.setSpacing(6)
            vl.addLayout(layout)

            self.use_selection_checkbox = QCheckBox(self.tr('Selected features only'))
            self.use_selection_checkbox.setChecked(False)
            self.use_selection_checkbox.setEnabled(False)
            vl.addWidget(self.use_selection_checkbox)

            widget.setLayout(vl)

            filters = QgsMapLayerProxyModel.Filters()
            if QgsProcessing.TypeVectorAnyGeometry in self.parameterDefinition().dataTypes() or len(self.parameterDefinition().dataTypes()) == 0:
                filters = QgsMapLayerProxyModel.HasGeometry
            if QgsProcessing.TypeVectorPoint in self.parameterDefinition().dataTypes():
                filters |= QgsMapLayerProxyModel.PointLayer
            if QgsProcessing.TypeVectorLine in self.parameterDefinition().dataTypes():
                filters |= QgsMapLayerProxyModel.LineLayer
            if QgsProcessing.TypeVectorPolygon in self.parameterDefinition().dataTypes():
                filters |= QgsMapLayerProxyModel.PolygonLayer
            if not filters:
                filters = QgsMapLayerProxyModel.VectorLayer

            try:
                if iface.activeLayer().type() == QgsMapLayer.VectorLayer:
                    self.combo.setLayer(iface.activeLayer())
                    self.use_selection_checkbox.setEnabled(iface.activeLayer().selectedFeatureCount() > 0)

            except:
                pass

            if ProcessingConfig.getSetting(ProcessingConfig.SHOW_CRS_DEF):
                self.combo.setShowCrs(True)

            if filters:
                self.combo.setFilters(filters)
            self.combo.setExcludedProviders(['grass'])

            if self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional:
                self.combo.setAllowEmptyLayer(True)
                self.combo.setLayer(None)

            self.combo.layerChanged.connect(self.layerChanged)
            return widget

        elif self.dialogType == DIALOG_BATCH:
            widget = BatchInputSelectionPanel(self.parameterDefinition(), self.row, self.col, self.dialog)
            widget.valueChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
            return widget
        else:
            self.combo = QComboBox()
            layers = self.dialog.getAvailableValuesOfType(
                (QgsProcessingParameterFeatureSource, QgsProcessingParameterVectorLayer),
                (QgsProcessingOutputVectorLayer, QgsProcessingOutputMapLayer, QgsProcessingOutputString, QgsProcessingOutputFile), self.parameterDefinition().dataTypes())
            self.combo.setEditable(True)
            for layer in layers:
                self.combo.addItem(self.dialog.resolveValueDescription(layer), layer)
            if self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional:
                self.combo.setEditText("")

            widget = QWidget()
            layout = QHBoxLayout()
            layout.setMargin(0)
            layout.setContentsMargins(0, 0, 0, 0)
            layout.setSpacing(2)
            layout.addWidget(self.combo)
            btn = QToolButton()
            btn.setText('…')
            btn.setToolTip(self.tr("Select file"))
            btn.clicked.connect(self.selectFile)
            layout.addWidget(btn)
            widget.setLayout(layout)
            return widget

    def layerChanged(self, layer):
        if layer is None or layer.type() != QgsMapLayer.VectorLayer or layer.selectedFeatureCount() == 0:
            self.use_selection_checkbox.setChecked(False)
            self.use_selection_checkbox.setEnabled(False)
        else:
            self.use_selection_checkbox.setEnabled(True)
        self.widgetValueHasChanged.emit(self)

    def selectFile(self):
        filename, selected_filter = self.getFileName(self.combo.currentText())
        if filename:
            filename = dataobjects.getRasterSublayer(filename, self.parameterDefinition())
            if isinstance(self.combo, QgsMapLayerComboBox):
                items = self.combo.additionalItems()
                items.append(filename)
                self.combo.setAdditionalItems(items)
                self.combo.setCurrentIndex(self.combo.findText(filename))
            else:
                self.combo.setEditText(filename)
            self.widgetValueHasChanged.emit(self)

    def setValue(self, value):
        if value is None or value == NULL:
            return

        if self.dialogType == DIALOG_STANDARD:
            if isinstance(value, str):
                layer = QgsProject.instance().mapLayer(value)
                if layer is not None:
                    value = layer

            found = False
            if isinstance(value, QgsMapLayer):
                self.combo.setLayer(value)
                found = self.combo.currentIndex() != -1

            if not found:
                if self.combo.findText(value) >= 0:
                    self.combo.setCurrentIndex(self.combo.findText(value))
                else:
                    items = self.combo.additionalItems()
                    items.append(value)
                    self.combo.setAdditionalItems(items)
                    self.combo.setCurrentIndex(self.combo.findText(value))
        elif self.dialogType == DIALOG_BATCH:
            self.widget.setValue(value)
        else:
            self.setComboValue(value, combobox=self.combo)
        self.widgetValueHasChanged.emit(self)

    def value(self):
        if self.dialogType == DIALOG_STANDARD:
            use_selected_features = self.use_selection_checkbox.isChecked()
            try:
                layer = self.combo.currentLayer()
                if layer is not None:
                    if use_selected_features:
                        return QgsProcessingFeatureSourceDefinition(layer.id(), True)
                    else:
                        return layer.id()
                else:
                    if self.combo.currentText():
                        if use_selected_features:
                            return QgsProcessingFeatureSourceDefinition(self.combo.currentText(), True)
                        else:
                            return self.combo.currentText()
                    else:
                        return None
            except:
                return QgsProcessingFeatureSourceDefinition(self.combo.currentText(), use_selected_features)
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.value()
        else:
            def validator(v):
                if not bool(v):
                    return self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional
                else:
                    return os.path.exists(v)

            if self.combo.currentText():
                return self.comboValue(validator, combobox=self.combo)
            else:
                return None


class StringWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        if self.dialogType == DIALOG_STANDARD:
            if self.parameterDefinition().multiLine():
                widget = QPlainTextEdit()
            else:
                self._lineedit = QLineEdit()
                widget = self._lineedit

        elif self.dialogType == DIALOG_BATCH:
            widget = QLineEdit()

        else:
            # strings, numbers, files and table fields are all allowed input types
            strings = self.dialog.getAvailableValuesOfType(
                [QgsProcessingParameterString, QgsProcessingParameterNumber, QgsProcessingParameterDistance, QgsProcessingParameterFile,
                 QgsProcessingParameterField, QgsProcessingParameterExpression],
                [QgsProcessingOutputString, QgsProcessingOutputFile])
            options = [(self.dialog.resolveValueDescription(s), s) for s in strings]
            if self.parameterDefinition().multiLine():
                widget = MultilineTextPanel(options)
            else:
                widget = QComboBox()
                widget.setEditable(True)
                for desc, val in options:
                    widget.addItem(desc, val)
        return widget

    def showExpressionsBuilder(self):
        context = dataobjects.createExpressionContext()
        value = self.value()
        if not isinstance(value, str):
            value = ''
        dlg = QgsExpressionBuilderDialog(None, value, self.widget, 'generic', context)
        dlg.setWindowTitle(self.tr('Expression based input'))
        if dlg.exec_() == QDialog.Accepted:
            exp = QgsExpression(dlg.expressionText())
            if not exp.hasParserError():
                if self.dialogType == DIALOG_STANDARD:
                    self.setValue(str(exp.evaluate(context)))
                else:
                    self.setValue(dlg.expressionText())

    def setValue(self, value):
        if value is None or value == NULL:
            return

        if self.dialogType == DIALOG_STANDARD:
            if self.parameterDefinition().multiLine():
                self.widget.setPlainText(value)
            else:
                self._lineedit.setText(value)

        elif self.dialogType == DIALOG_BATCH:
            self.widget.setText(value)

        else:
            if self.parameterDefinition().multiLine():
                self.widget.setValue(value)
            else:
                self.setComboValue(value)

    def value(self):
        if self.dialogType == DIALOG_STANDARD:
            if self.parameterDefinition().multiLine():
                text = self.widget.toPlainText()
            else:
                text = self._lineedit.text()
            return text

        elif self.dialogType == DIALOG_BATCH:
            return self.widget.text()

        else:
            if self.parameterDefinition().multiLine():
                value = self.widget.getValue()
                option = self.widget.getOption()
                if option == MultilineTextPanel.USE_TEXT:
                    if value == '':
                        if self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional:
                            return None
                        else:
                            raise InvalidParameterValue()
                    else:
                        return value
                else:
                    return value
            else:
                def validator(v):
                    return bool(v) or self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional

                return self.comboValue(validator)


class ExpressionWidgetWrapper(WidgetWrapper):

    def __init__(self, param, dialog, row=0, col=0, **kwargs):
        super().__init__(param, dialog, row, col, **kwargs)
        self.context = dataobjects.createContext()

    def createWidget(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            if self.parameterDefinition().parentLayerParameterName():
                widget = QgsFieldExpressionWidget()
            else:
                widget = QgsExpressionLineEdit()
            if self.parameterDefinition().defaultValue():
                widget.setExpression(self.parameterDefinition().defaultValue())
        else:
            strings = self.dialog.getAvailableValuesOfType(
                [QgsProcessingParameterExpression, QgsProcessingParameterString, QgsProcessingParameterNumber, QgsProcessingParameterDistance],
                (QgsProcessingOutputString, QgsProcessingOutputNumber))
            options = [(self.dialog.resolveValueDescription(s), s) for s in strings]
            widget = QComboBox()
            widget.setEditable(True)
            for desc, val in options:
                widget.addItem(desc, val)
            widget.setEditText(self.parameterDefinition().defaultValue() or "")
        return widget

    def postInitialize(self, wrappers):
        for wrapper in wrappers:
            if wrapper.parameterDefinition().name() == self.parameterDefinition().parentLayerParameterName():
                if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
                    self.setLayer(wrapper.parameterValue())
                    wrapper.widgetValueHasChanged.connect(self.parentLayerChanged)
                break

    def parentLayerChanged(self, wrapper):
        self.setLayer(wrapper.parameterValue())

    def setLayer(self, layer):
        if isinstance(layer, QgsProcessingFeatureSourceDefinition):
            layer, ok = layer.source.valueAsString(self.context.expressionContext())
        if isinstance(layer, str):
            layer = QgsProcessingUtils.mapLayerFromString(layer, self.context)
        self.widget.setLayer(layer)

    def setValue(self, value):
        if value is None or value == NULL:
            return

        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            self.widget.setExpression(value)
        else:
            self.setComboValue(value)

    def value(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            try:
                return self.widget.asExpression()
            except:
                return self.widget.expression()
        else:
            def validator(v):
                return bool(v) or self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional

            return self.comboValue(validator)


class VectorLayerWidgetWrapper(WidgetWrapper):
    NOT_SELECTED = '[Not selected]'

    def createWidget(self):
        self.fileBasedLayers = {}
        if self.dialogType == DIALOG_STANDARD:
            widget = QWidget()
            layout = QHBoxLayout()
            layout.setMargin(0)
            layout.setContentsMargins(0, 0, 0, 0)
            layout.setSpacing(6)
            self.combo = QgsMapLayerComboBox()
            layout.addWidget(self.combo)
            btn = QToolButton()
            btn.setText('…')
            btn.setToolTip(self.tr("Select file"))
            btn.clicked.connect(self.selectFile)
            layout.addWidget(btn)

            widget.setLayout(layout)

            if ProcessingConfig.getSetting(ProcessingConfig.SHOW_CRS_DEF):
                self.combo.setShowCrs(True)

            filters = QgsMapLayerProxyModel.Filters()
            if QgsProcessing.TypeVectorAnyGeometry in self.parameterDefinition().dataTypes() or len(self.parameterDefinition().dataTypes()) == 0:
                filters = QgsMapLayerProxyModel.HasGeometry
            if QgsProcessing.TypeVectorPoint in self.parameterDefinition().dataTypes():
                filters |= QgsMapLayerProxyModel.PointLayer
            if QgsProcessing.TypeVectorLine in self.parameterDefinition().dataTypes():
                filters |= QgsMapLayerProxyModel.LineLayer
            if QgsProcessing.TypeVectorPolygon in self.parameterDefinition().dataTypes():
                filters |= QgsMapLayerProxyModel.PolygonLayer
            if not filters:
                filters = QgsMapLayerProxyModel.VectorLayer

            if filters:
                self.combo.setFilters(filters)

            self.combo.setExcludedProviders(['grass'])
            try:
                if iface.activeLayer().type() == QgsMapLayer.VectorLayer:
                    self.combo.setLayer(iface.activeLayer())
            except:
                pass

            if self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional:
                self.combo.setAllowEmptyLayer(True)
                self.combo.setLayer(None)

            self.combo.currentIndexChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
            self.combo.currentTextChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
            return widget

        elif self.dialogType == DIALOG_BATCH:
            widget = BatchInputSelectionPanel(self.parameterDefinition(), self.row, self.col, self.dialog)
            widget.valueChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
            return widget
        else:
            self.combo = QComboBox()
            self.combo.setEditable(True)
            tables = self.dialog.getAvailableValuesOfType((QgsProcessingParameterVectorLayer, QgsProcessingParameterString),
                                                          (QgsProcessingOutputVectorLayer, QgsProcessingOutputMapLayer, QgsProcessingOutputFile, QgsProcessingOutputString))
            if self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional:
                self.combo.addItem(self.NOT_SELECTED, self.NOT_SET_OPTION)
            for table in tables:
                self.combo.addItem(self.dialog.resolveValueDescription(table), table)

            widget = QWidget()
            layout = QHBoxLayout()
            layout.setMargin(0)
            layout.setContentsMargins(0, 0, 0, 0)
            layout.setSpacing(6)
            layout.addWidget(self.combo)
            btn = QToolButton()
            btn.setText('…')
            btn.setToolTip(self.tr("Select file"))
            btn.clicked.connect(self.selectFile)
            layout.addWidget(btn)
            widget.setLayout(layout)
            return widget

    def selectFile(self):
        filename, selected_filter = self.getFileName(self.combo.currentText())
        if filename:
            filename = dataobjects.getRasterSublayer(filename, self.parameterDefinition())
            if isinstance(self.combo, QgsMapLayerComboBox):
                items = self.combo.additionalItems()
                items.append(filename)
                self.combo.setAdditionalItems(items)
                self.combo.setCurrentIndex(self.combo.findText(filename))
            else:
                self.combo.setEditText(filename)
            self.widgetValueHasChanged.emit(self)

    def setValue(self, value):
        if value is None or value == NULL:
            return

        if self.dialogType == DIALOG_STANDARD:
            if isinstance(value, str):
                layer = QgsProject.instance().mapLayer(value)
                if layer is not None:
                    value = layer

            found = False
            if isinstance(value, QgsMapLayer):
                self.combo.setLayer(value)
                found = self.combo.currentIndex() != -1

            if not found:
                if self.combo.findText(value) >= 0:
                    self.combo.setCurrentIndex(self.combo.findText(value))
                else:
                    items = self.combo.additionalItems()
                    items.append(value)
                    self.combo.setAdditionalItems(items)
                    self.combo.setCurrentIndex(self.combo.findText(value))
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.setValue(value)
        else:
            self.setComboValue(value, combobox=self.combo)
        self.widgetValueHasChanged.emit(self)

    def value(self):
        if self.dialogType == DIALOG_STANDARD:
            try:
                layer = self.combo.currentLayer()
                if layer is not None:
                    return layer
                else:
                    return self.combo.currentText()
            except:
                return self.combo.currentText()
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.value()
        else:
            def validator(v):
                return bool(v) or self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional

            return self.comboValue(validator, combobox=self.combo)


class TableFieldWidgetWrapper(WidgetWrapper):
    NOT_SET = '[Not set]'

    def __init__(self, param, dialog, row=0, col=0, **kwargs):
        super().__init__(param, dialog, row, col, **kwargs)
        self.context = dataobjects.createContext()

    def createWidget(self):
        self._layer = None

        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            if self.parameterDefinition().allowMultiple():
                return MultipleInputPanel(options=[])
            else:
                widget = QgsFieldComboBox()
                widget.setAllowEmptyFieldName(self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional)
                widget.fieldChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
                if self.parameterDefinition().dataType() == QgsProcessingParameterField.Numeric:
                    widget.setFilters(QgsFieldProxyModel.Numeric)
                elif self.parameterDefinition().dataType() == QgsProcessingParameterField.String:
                    widget.setFilters(QgsFieldProxyModel.String)
                elif self.parameterDefinition().dataType() == QgsProcessingParameterField.DateTime:
                    widget.setFilters(QgsFieldProxyModel.Date | QgsFieldProxyModel.Time)
                return widget
        else:
            widget = QComboBox()
            widget.setEditable(True)
            fields = self.dialog.getAvailableValuesOfType([QgsProcessingParameterField, QgsProcessingParameterString],
                                                          [QgsProcessingOutputString])
            if self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional:
                widget.addItem(self.NOT_SET, self.NOT_SET_OPTION)
            for f in fields:
                widget.addItem(self.dialog.resolveValueDescription(f), f)
            widget.setToolTip(
                self.tr(
                    'Input parameter, or name of field (separate field names with ; for multiple field parameters)'))
            return widget

    def postInitialize(self, wrappers):
        for wrapper in wrappers:
            if wrapper.parameterDefinition().name() == self.parameterDefinition().parentLayerParameterName():
                if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
                    self.setLayer(wrapper.parameterValue())
                    wrapper.widgetValueHasChanged.connect(self.parentValueChanged)
                break

    def parentValueChanged(self, wrapper):
        value = wrapper.parameterValue()
        if value in wrapper.fileBasedLayers:
            self.setLayer(wrapper.fileBasedLayers[value])
        else:
            self.setLayer(value)
            wrapper.fileBasedLayers[value] = self._layer

    def setLayer(self, layer):
        if isinstance(layer, QgsProcessingFeatureSourceDefinition):
            layer, ok = layer.source.valueAsString(self.context.expressionContext())
        if isinstance(layer, str):
            layer = QgsProcessingUtils.mapLayerFromString(layer, self.context)
            if not isinstance(layer, QgsVectorLayer) or not layer.isValid():
                self.dialog.messageBar().clearWidgets()
                self.dialog.messageBar().pushMessage("", self.tr("Could not load selected layer/table. Dependent field could not be populated"),
                                                     level=Qgis.Warning, duration=5)
                return

        self._layer = layer

        self.refreshItems()

    def refreshItems(self):
        if self.parameterDefinition().allowMultiple():
            self.widget.updateForOptions(self.getFields())
        else:
            self.widget.setLayer(self._layer)
            self.widget.setCurrentIndex(0)
        if self.parameterDefinition().defaultValue() is not None:
            self.setValue(self.parameterDefinition().defaultValue())

    def getFields(self):
        if self._layer is None:
            return []
        fieldTypes = []
        if self.parameterDefinition().dataType() == QgsProcessingParameterField.String:
            fieldTypes = [QVariant.String]
        elif self.parameterDefinition().dataType() == QgsProcessingParameterField.Numeric:
            fieldTypes = [QVariant.Int, QVariant.Double, QVariant.LongLong,
                          QVariant.UInt, QVariant.ULongLong]
        elif self.parameterDefinition().dataType() == QgsProcessingParameterField.DateTime:
            fieldTypes = [QVariant.Date, QVariant.Time, QVariant.DateTime]

        fieldNames = []
        for field in self._layer.fields():
            if not fieldTypes or field.type() in fieldTypes:
                fieldNames.append(str(field.name()))
        return fieldNames

    def setValue(self, value):
        if value is None or value == NULL:
            return

        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            if self.parameterDefinition().allowMultiple():
                options = self.widget.options
                selected = []
                if isinstance(value, str):
                    value = value.split(';')

                for v in value:
                    for i, opt in enumerate(options):
                        if opt == v:
                            selected.append(i)
                        # case insensitive check - only do if matching case value is not present
                        elif v not in options and opt.lower() == v.lower():
                            selected.append(i)

                self.widget.setSelectedItems(selected)
            else:
                self.widget.setField(value)
        else:
            self.setComboValue(value)

    def value(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            if self.parameterDefinition().allowMultiple():
                return [self.widget.options[i] for i in self.widget.selectedoptions]
            else:
                f = self.widget.currentField()
                if self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional and not f:
                    return None
                return f
        else:
            def validator(v):
                return bool(v) or self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional

            return self.comboValue(validator)


class BandWidgetWrapper(WidgetWrapper):
    NOT_SET = '[Not set]'

    def __init__(self, param, dialog, row=0, col=0, **kwargs):
        super().__init__(param, dialog, row, col, **kwargs)
        self.context = dataobjects.createContext()

    def createWidget(self):
        self._layer = None

        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            if self.parameterDefinition().allowMultiple():
                return MultipleInputPanel(options=[])
            widget = QgsRasterBandComboBox()
            widget.setShowNotSetOption(self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional)
            widget.bandChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
            return widget
        else:
            widget = QComboBox()
            widget.setEditable(True)
            fields = self.dialog.getAvailableValuesOfType([QgsProcessingParameterBand, QgsProcessingParameterDistance, QgsProcessingParameterNumber],
                                                          [QgsProcessingOutputNumber])
            if self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional:
                widget.addItem(self.NOT_SET, self.NOT_SET_OPTION)
            for f in fields:
                widget.addItem(self.dialog.resolveValueDescription(f), f)
            return widget

    def postInitialize(self, wrappers):
        for wrapper in wrappers:
            if wrapper.parameterDefinition().name() == self.parameterDefinition().parentLayerParameterName():
                if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
                    self.setLayer(wrapper.parameterValue())
                    wrapper.widgetValueHasChanged.connect(self.parentValueChanged)
                break

    def parentValueChanged(self, wrapper):
        self.setLayer(wrapper.parameterValue())

    def setLayer(self, layer):
        if isinstance(layer, QgsProcessingParameterRasterLayer):
            layer, ok = layer.source.valueAsString(self.context.expressionContext())
        if isinstance(layer, str):
            layer = QgsProcessingUtils.mapLayerFromString(layer, self.context)
        self._layer = layer
        self.refreshItems()

    def getBands(self):
        bands = []

        if self._layer is not None:
            provider = self._layer.dataProvider()
            for band in range(1, provider.bandCount() + 1):
                name = provider.generateBandName(band)
                interpretation = provider.colorInterpretationName(band)
                if interpretation != "Undefined":
                    name = name + ' ({})'.format(interpretation)
                bands.append(name)
        return bands

    def refreshItems(self):
        if self.param.allowMultiple():
            self.widget.setSelectedItems([])
            self.widget.updateForOptions(self.getBands())
        else:
            self.widget.setLayer(self._layer)
            self.widget.setCurrentIndex(0)

    def setValue(self, value):
        if value is None or value == NULL:
            return

        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            if self.parameterDefinition().allowMultiple():
                options = self.widget.options
                selected = []
                if isinstance(value, str):
                    value = value.split(';')

                for v in value:
                    for i, opt in enumerate(options):
                        match = re.search('(?:\A|[^0-9]){}(?:\Z|[^0-9]|)'.format(v), opt)
                        if match:
                            selected.append(i)

                self.widget.setSelectedItems(selected)
            else:
                self.widget.setBand(value)
        else:
            self.setComboValue(value)

    def value(self):
        if self.dialogType in (DIALOG_STANDARD, DIALOG_BATCH):
            if self.parameterDefinition().allowMultiple():
                bands = []
                for i in self.widget.selectedoptions:
                    match = re.search('(?:\A|[^0-9])([0-9]+)(?:\Z|[^0-9]|)', self.widget.options[i])
                    if match:
                        bands.append(match.group(1))
                return bands
            else:
                f = self.widget.currentBand()
                if self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional and not f:
                    return None
            return f
        else:
            def validator(v):
                return bool(v) or self.parameterDefinition().flags() & QgsProcessingParameterDefinition.FlagOptional

            return self.comboValue(validator)


class WidgetWrapperFactory:

    """
    Factory for parameter widget wrappers
    """

    @staticmethod
    def create_wrapper(param, dialog, row=0, col=0):
        wrapper_metadata = param.metadata().get('widget_wrapper', None)
        # VERY messy logic here to avoid breaking 3.0 API which allowed metadata "widget_wrapper" value to be either
        # a string name of a class OR a dict.
        # TODO QGIS 4.0 -- require widget_wrapper to be a dict.
        if wrapper_metadata and (not isinstance(wrapper_metadata, dict) or wrapper_metadata.get('class', None) is not None):
            return WidgetWrapperFactory.create_wrapper_from_metadata(param, dialog, row, col)
        else:
            # try from c++ registry first
            class_type = dialog.__class__.__name__
            if class_type == 'ModelerParametersDialog':
                wrapper = QgsGui.processingGuiRegistry().createModelerParameterWidget(dialog.model,
                                                                                      dialog.childId,
                                                                                      param,
                                                                                      dialog.context)
            else:
                dialog_type = dialogTypes.get(class_type,
                                              QgsProcessingGui.Standard)
                wrapper = QgsGui.processingGuiRegistry().createParameterWidgetWrapper(param, dialog_type)
            if wrapper is not None:
                wrapper.setDialog(dialog)
                return wrapper

            # fallback to Python registry
            return WidgetWrapperFactory.create_wrapper_from_class(param, dialog, row, col)

    @staticmethod
    def create_wrapper_from_metadata(param, dialog, row=0, col=0):
        wrapper = param.metadata().get('widget_wrapper', None)
        params = {}
        # wrapper metadata should be a dict with class key
        if isinstance(wrapper, dict):
            params = deepcopy(wrapper)
            wrapper = params.pop('class')
        # wrapper metadata should be a class path
        if isinstance(wrapper, str):
            tokens = wrapper.split('.')
            mod = __import__('.'.join(tokens[:-1]), fromlist=[tokens[-1]])
            wrapper = getattr(mod, tokens[-1])
        # or directly a class object
        if isclass(wrapper):
            wrapper = wrapper(param, dialog, row, col, **params)
        # or a wrapper instance
        return wrapper

    @staticmethod
    def create_wrapper_from_class(param, dialog, row=0, col=0):
        wrapper = None
        if param.type() == 'boolean':
            wrapper = BooleanWidgetWrapper
        elif param.type() == 'crs':
            wrapper = CrsWidgetWrapper
        elif param.type() == 'extent':
            wrapper = ExtentWidgetWrapper
        elif param.type() == 'point':
            wrapper = PointWidgetWrapper
        elif param.type() == 'file':
            wrapper = FileWidgetWrapper
        elif param.type() == 'multilayer':
            wrapper = MultipleLayerWidgetWrapper
        elif param.type() == 'number':
            wrapper = NumberWidgetWrapper
        elif param.type() == 'distance':
            wrapper = DistanceWidgetWrapper
        elif param.type() == 'raster':
            wrapper = RasterWidgetWrapper
        elif param.type() == 'enum':
            wrapper = EnumWidgetWrapper
        elif param.type() == 'string':
            wrapper = StringWidgetWrapper
        elif param.type() == 'expression':
            wrapper = ExpressionWidgetWrapper
        elif param.type() == 'vector':
            wrapper = VectorLayerWidgetWrapper
        elif param.type() == 'field':
            wrapper = TableFieldWidgetWrapper
        elif param.type() == 'source':
            wrapper = FeatureSourceWidgetWrapper
        elif param.type() == 'band':
            wrapper = BandWidgetWrapper
        elif param.type() == 'layer':
            wrapper = MapLayerWidgetWrapper
        elif param.type() == 'range':
            wrapper = RangeWidgetWrapper
        elif param.type() == 'matrix':
            wrapper = FixedTableWidgetWrapper
        elif param.type() == 'mesh':
            wrapper = MeshWidgetWrapper
        else:
            assert False, param.type()
        return wrapper(param, dialog, row, col)
