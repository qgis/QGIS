# -*- coding: utf-8 -*-

"""
***************************************************************************
    ParametersPanel.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
                           (C) 2013 by CS Systemes d'information (CS SI)
    Email                : volayaf at gmail dot com
                           otb at c-s dot fr (CS SI)
    Contributors         : Victor Olaya

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
import locale

from qgis.core import QgsMapLayerRegistry, QgsMapLayer

from qgis.PyQt import uic
from qgis.PyQt.QtCore import QCoreApplication, QVariant
from qgis.PyQt.QtWidgets import QWidget, QLayout, QVBoxLayout, QHBoxLayout, QToolButton, QLabel, QCheckBox, QComboBox, QLineEdit, QPlainTextEdit
from qgis.PyQt.QtGui import QIcon

from processing.core.ProcessingConfig import ProcessingConfig

from processing.gui.OutputSelectionPanel import OutputSelectionPanel
from processing.gui.InputLayerSelectorPanel import InputLayerSelectorPanel
from processing.gui.FixedTablePanel import FixedTablePanel
from processing.gui.RangePanel import RangePanel
from processing.gui.MultipleInputPanel import MultipleInputPanel
from processing.gui.NumberInputPanel import NumberInputPanel
from processing.gui.ExtentSelectionPanel import ExtentSelectionPanel
from processing.gui.FileSelectionPanel import FileSelectionPanel
from processing.gui.CrsSelectionPanel import CrsSelectionPanel
from processing.gui.PointSelectionPanel import PointSelectionPanel
from processing.gui.GeometryPredicateSelectionPanel import \
    GeometryPredicateSelectionPanel
from processing.gui.ListMultiselectWidget import ListMultiSelectWidget

from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTable
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterTableField
from processing.core.parameters import ParameterTableMultipleField
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterFixedTable
from processing.core.parameters import ParameterRange
from processing.core.parameters import ParameterMultipleInput
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterExtent
from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterCrs
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterPoint
from processing.core.parameters import ParameterGeometryPredicate

from processing.core.outputs import OutputRaster
from processing.core.outputs import OutputTable
from processing.core.outputs import OutputVector

from processing.tools import dataobjects

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'widgetParametersPanel.ui'))


class ParametersPanel(BASE, WIDGET):

    NOT_SELECTED = QCoreApplication.translate('ParametersPanel', '[Not selected]')

    def __init__(self, parent, alg):
        super(ParametersPanel, self).__init__(None)
        self.setupUi(self)

        self.grpAdvanced.hide()

        self.layoutMain = self.scrollAreaWidgetContents.layout()
        self.layoutAdvanced = self.grpAdvanced.layout()

        self.parent = parent
        self.alg = alg
        self.valueItems = {}
        self.labels = {}
        self.widgets = {}
        self.checkBoxes = {}
        self.dependentItems = {}
        self.iterateButtons = {}

        self.initWidgets()

    def layerAdded(self, layer):
        if layer.type() == QgsMapLayer.VectorLayer:
            for param in self.alg.parameters:
                if param.hidden:
                    continue
                if isinstance(param, ParameterVector):
                    if dataobjects.canUseVectorLayer(layer, param.shapetype):
                        widget = self.valueItems[param.name]
                        if isinstance(widget, InputLayerSelectorPanel):
                            widget = widget.cmbText
                        widget.addItem(self.getExtendedLayerName(layer), layer)
        elif layer.type() == QgsMapLayer.RasterLayer and dataobjects.canUseRasterLayer(layer):
            for param in self.alg.parameters:
                if param.hidden:
                    continue
                if isinstance(param, ParameterRaster):
                    widget = self.valueItems[param.name].cmbText
                    widget.addItem(self.getExtendedLayerName(layer), layer)

        self.updateMultipleInputs()

    def layersWillBeRemoved(self, layers):
        for layer in layers:
            self.layerRemoved(layer)

    def layerRemoved(self, layer):
        layer = QgsMapLayerRegistry.instance().mapLayer(layer)
        widget = None
        if layer.type() == QgsMapLayer.VectorLayer:
            for param in self.alg.parameters:
                if param.hidden:
                    continue
                if isinstance(param, ParameterVector):
                    widget = self.valueItems[param.name]
                    if isinstance(widget, InputLayerSelectorPanel):
                        widget = widget.cmbText

        elif layer.type() == QgsMapLayer.RasterLayer:
            for param in self.alg.parameters:
                if param.hidden:
                    continue
                if isinstance(param, ParameterRaster):
                    widget = self.valueItems[param.name].cmbText

        if widget is not None:
            idx = widget.findData(layer)
            if idx != -1:
                widget.removeItem(idx)

        self.updateMultipleInputs()

    def updateMultipleInputs(self):
        for param in self.alg.parameters:
            if isinstance(param, ParameterMultipleInput) and param.datatype != ParameterMultipleInput.TYPE_FILE:
                if param.datatype == ParameterMultipleInput.TYPE_RASTER:
                    options = dataobjects.getRasterLayers(sorting=False)
                elif param.datatype == ParameterMultipleInput.TYPE_VECTOR_ANY:
                    options = dataobjects.getVectorLayers(sorting=False)
                else:
                    options = dataobjects.getVectorLayers([param.datatype], sorting=False)
                opts = [self.getExtendedLayerName(opt) for opt in options]
                widget = self.valueItems[param.name]
                widget.updateForOptions(opts)

    def initWidgets(self):
        # If there are advanced parameters — show corresponding groupbox
        for param in self.alg.parameters:
            if param.isAdvanced:
                self.grpAdvanced.show()
                break
        # Create widgets and put them in layouts
        for param in self.alg.parameters:
            if param.hidden:
                continue

            desc = param.description
            if isinstance(param, ParameterExtent):
                desc += self.tr(' (xmin, xmax, ymin, ymax)')
            if isinstance(param, ParameterPoint):
                desc += self.tr(' (x, y)')
            try:
                if param.optional:
                    desc += self.tr(' [optional]')
            except:
                pass

            widget = self.getWidgetFromParameter(param)
            self.valueItems[param.name] = widget

            if isinstance(param, ParameterVector) and \
                    not self.alg.allowOnlyOpenedLayers:
                layout = QHBoxLayout()
                layout.setSpacing(2)
                layout.setMargin(0)
                layout.addWidget(widget)
                button = QToolButton()
                icon = QIcon(os.path.join(pluginPath, 'images', 'iterate.png'))
                button.setIcon(icon)
                button.setToolTip(self.tr('Iterate over this layer'))
                button.setCheckable(True)
                layout.addWidget(button)
                self.iterateButtons[param.name] = button
                button.toggled.connect(self.buttonToggled)
                widget = QWidget()
                widget.setLayout(layout)

            tooltips = self.alg.getParameterDescriptions()
            widget.setToolTip(tooltips.get(param.name, param.description))

            if isinstance(param, ParameterBoolean):
                widget.setText(desc)
                if param.isAdvanced:
                    self.layoutAdvanced.addWidget(widget)
                else:
                    self.layoutMain.insertWidget(
                        self.layoutMain.count() - 2, widget)
            else:
                label = QLabel(desc)
                #label.setToolTip(tooltip)
                self.labels[param.name] = label
                if param.isAdvanced:
                    self.layoutAdvanced.addWidget(label)
                    self.layoutAdvanced.addWidget(widget)
                else:
                    self.layoutMain.insertWidget(
                        self.layoutMain.count() - 2, label)
                    self.layoutMain.insertWidget(
                        self.layoutMain.count() - 2, widget)

            self.widgets[param.name] = widget

        for output in self.alg.outputs:
            if output.hidden:
                continue

            label = QLabel(output.description)
            widget = OutputSelectionPanel(output, self.alg)
            self.layoutMain.insertWidget(self.layoutMain.count() - 1, label)
            self.layoutMain.insertWidget(self.layoutMain.count() - 1, widget)
            if isinstance(output, (OutputRaster, OutputVector, OutputTable)):
                check = QCheckBox()
                check.setText(self.tr('Open output file after running algorithm'))
                check.setChecked(True)
                self.layoutMain.insertWidget(self.layoutMain.count() - 1, check)
                self.checkBoxes[output.name] = check
            self.valueItems[output.name] = widget

            if isinstance(output, OutputVector):
                if output.base_input in self.dependentItems:
                    items = self.dependentItems[output.base_input]
                else:
                    items = []
                    self.dependentItems[output.base_input] = items
                items.append(output)

                base_input = self.alg.getParameterFromName(output.base_input)
                if isinstance(base_input, ParameterVector):
                    layers = dataobjects.getVectorLayers(base_input.shapetype)
                else:
                    layers = dataobjects.getTables()
                if len(layers) > 0:
                    output.base_layer = layers[0]

    def buttonToggled(self, value):
        if value:
            sender = self.sender()
            for button in self.iterateButtons.values():
                if button is not sender:
                    button.setChecked(False)

    def getExtendedLayerName(self, layer):
        authid = layer.crs().authid()
        if ProcessingConfig.getSetting(ProcessingConfig.SHOW_CRS_DEF) \
                and authid is not None:
            return u'{} [{}]'.format(layer.name(), authid)
        else:
            return layer.name()

    def getWidgetFromParameter(self, param):
        # TODO Create Parameter widget class that holds the logic
        # for creating a widget that belongs to the parameter.
        if isinstance(param, ParameterRaster):
            layers = dataobjects.getRasterLayers()
            items = []
            if param.optional:
                items.append((self.NOT_SELECTED, None))
            for layer in layers:
                items.append((self.getExtendedLayerName(layer), layer))
            item = InputLayerSelectorPanel(items, param)
        elif isinstance(param, ParameterVector):
            if self.somethingDependsOnThisParameter(param) or self.alg.allowOnlyOpenedLayers:
                item = QComboBox()
                layers = dataobjects.getVectorLayers(param.shapetype)
                layers.sort(key=lambda lay: lay.name())
                if param.optional:
                    item.addItem(self.NOT_SELECTED, None)
                for layer in layers:
                    item.addItem(self.getExtendedLayerName(layer), layer)
                item.currentIndexChanged.connect(self.updateDependentFields)
                item.name = param.name
            else:
                layers = dataobjects.getVectorLayers(param.shapetype)
                items = []
                if param.optional:
                    items.append((self.NOT_SELECTED, None))
                for layer in layers:
                    items.append((self.getExtendedLayerName(layer), layer))
                # if already set, put first in list
                for i, (name, layer) in enumerate(items):
                    if layer and layer.source() == param.value:
                        items.insert(0, items.pop(i))
                item = InputLayerSelectorPanel(items, param)
        elif isinstance(param, ParameterTable):
            if self.somethingDependsOnThisParameter(param):
                item = QComboBox()
                layers = dataobjects.getTables()
                if param.optional:
                    item.addItem(self.NOT_SELECTED, None)
                for layer in layers:
                    item.addItem(layer.name(), layer)
                item.currentIndexChanged.connect(self.updateDependentFields)
                item.name = param.name
            else:
                layers = dataobjects.getTables()
                items = []
                if param.optional:
                    items.append((self.NOT_SELECTED, None))
                for layer in layers:
                    items.append((layer.name(), layer))
                # if already set, put first in list
                for i, (name, layer) in enumerate(items):
                    if layer and layer.source() == param.value:
                        items.insert(0, items.pop(i))
                item = InputLayerSelectorPanel(items, param)
        elif isinstance(param, ParameterBoolean):
            item = QCheckBox()
            if param.default:
                item.setChecked(True)
            else:
                item.setChecked(False)
        elif isinstance(param, ParameterTableField) or isinstance(param, ParameterTableMultipleField):
            if isinstance(param, ParameterTableMultipleField):
                item = ListMultiSelectWidget()
            else:
                item = QComboBox()
            if param.parent in self.dependentItems:
                items = self.dependentItems[param.parent]
            else:
                items = []
                self.dependentItems[param.parent] = items
            items.append(param)
            parent = self.alg.getParameterFromName(param.parent)
            if isinstance(parent, ParameterVector):
                layers = dataobjects.getVectorLayers(parent.shapetype)
            else:
                layers = dataobjects.getTables()
            if len(layers) > 0:
                if param.optional and isinstance(param, ParameterTableField):
                    item.addItem(self.tr('[not set]'))
                item.addItems(self.getFields(layers[0], param.datatype))
        elif isinstance(param, ParameterSelection):
            item = QComboBox()
            item.addItems(param.options)
            if param.default:
                item.setCurrentIndex(param.default)
        elif isinstance(param, ParameterFixedTable):
            item = FixedTablePanel(param)
        elif isinstance(param, ParameterRange):
            item = RangePanel(param)
        elif isinstance(param, ParameterFile):
            item = FileSelectionPanel(param.isFolder, param.ext)
        elif isinstance(param, ParameterMultipleInput):
            if param.datatype == ParameterMultipleInput.TYPE_FILE:
                item = MultipleInputPanel(datatype=ParameterMultipleInput.TYPE_FILE)
            else:
                if param.datatype == ParameterMultipleInput.TYPE_RASTER:
                    options = dataobjects.getRasterLayers(sorting=False)
                elif param.datatype == ParameterMultipleInput.TYPE_VECTOR_ANY:
                    options = dataobjects.getVectorLayers(sorting=False)
                else:
                    options = dataobjects.getVectorLayers([param.datatype], sorting=False)
                opts = [self.getExtendedLayerName(opt) for opt in options]
                item = MultipleInputPanel(opts)
        elif isinstance(param, ParameterNumber):
            item = NumberInputPanel(param.default, param.min, param.max,
                                    param.isInteger)
        elif isinstance(param, ParameterExtent):
            item = ExtentSelectionPanel(self.parent, self.alg, param.default)
        elif isinstance(param, ParameterPoint):
            item = PointSelectionPanel(self.parent, param.default)
        elif isinstance(param, ParameterCrs):
            item = CrsSelectionPanel(param.default)
        elif isinstance(param, ParameterString):
            if param.multiline:
                verticalLayout = QVBoxLayout()
                verticalLayout.setSizeConstraint(
                    QLayout.SetDefaultConstraint)
                textEdit = QPlainTextEdit()
                if param.default:
                    textEdit.setPlainText(param.default)
                verticalLayout.addWidget(textEdit)
                item = textEdit
            else:
                item = QLineEdit()
                if param.default:
                    item.setText(unicode(param.default))
        elif isinstance(param, ParameterGeometryPredicate):
            item = GeometryPredicateSelectionPanel(param.enabledPredicates)
            if param.left:
                widget = self.valueItems[param.left]
                if isinstance(widget, InputLayerSelectorPanel):
                    widget = widget.cmbText
                widget.currentIndexChanged.connect(item.onLeftLayerChange)
                item.leftLayer = widget.itemData(widget.currentIndex())
            if param.right:
                widget = self.valueItems[param.right]
                if isinstance(widget, InputLayerSelectorPanel):
                    widget = widget.cmbText
                widget.currentIndexChanged.connect(item.onRightLayerChange)
                item.rightLayer = widget.itemData(widget.currentIndex())
            item.updatePredicates()
            if param.default:
                item.setValue(param.default)
        else:
            item = QLineEdit()
            if param.default:
                item.setText(unicode(param.default))

        return item

    def updateDependentFields(self):
        sender = self.sender()
        if not isinstance(sender, QComboBox):
            return
        if sender.name not in self.dependentItems:
            return
        layer = sender.itemData(sender.currentIndex())
        if not layer:
            return
        children = self.dependentItems[sender.name]
        for child in children:
            if (isinstance(child, ParameterTableField) or isinstance(
                    child, ParameterTableMultipleField)):
                widget = self.valueItems[child.name]
                widget.clear()
                if (self.alg.getParameterFromName(child.name).optional and
                        not isinstance(child, ParameterTableMultipleField)):
                    widget.addItem(self.tr('[not set]'))
                widget.addItems(
                    self.getFields(layer, self.alg.getParameterFromName(
                                   child.name).datatype))
            if isinstance(child, OutputVector):
                child.base_layer = layer

    def getFields(self, layer, datatype):
        fieldTypes = []
        if datatype == ParameterTableField.DATA_TYPE_STRING:
            fieldTypes = [QVariant.String]
        elif datatype == ParameterTableField.DATA_TYPE_NUMBER:
            fieldTypes = [QVariant.Int, QVariant.Double, QVariant.LongLong,
                          QVariant.UInt, QVariant.ULongLong]

        fieldNames = set()
        for field in layer.pendingFields():
            if not fieldTypes or field.type() in fieldTypes:
                fieldNames.add(unicode(field.name()))
        return sorted(list(fieldNames), cmp=locale.strcoll)

    def somethingDependsOnThisParameter(self, parent):
        for param in self.alg.parameters:
            if isinstance(param, (ParameterTableField,
                                  ParameterTableMultipleField)):
                if param.parent == parent.name:
                    return True
        for output in self.alg.outputs:
            if isinstance(output, OutputVector):
                if output.base_layer == parent.name:
                    return True
        return False
