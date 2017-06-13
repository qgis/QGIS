# -*- coding: utf-8 -*-

"""
***************************************************************************
    ModelerParameterDefinitionDialog.py
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
from builtins import str

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import math

from qgis.gui import QgsExpressionLineEdit, QgsProjectionSelectionWidget
from qgis.core import (QgsCoordinateReferenceSystem,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterMapLayer,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterPoint,
                       QgsProcessingParameterFile,
                       QgsProcessingParameterMatrix,
                       QgsProcessingParameterMultipleLayers,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterRange,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterString,
                       QgsProcessingParameterExpression,
                       QgsProcessingParameterTable,
                       QgsProcessingParameterTableField,
                       QgsProcessingParameterFeatureSource)
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtWidgets import (QDialog,
                                 QVBoxLayout,
                                 QLabel,
                                 QLineEdit,
                                 QComboBox,
                                 QCheckBox,
                                 QDialogButtonBox,
                                 QMessageBox)


class ModelerParameterDefinitionDialog(QDialog):

    PARAMETER_NUMBER = 'Number'
    PARAMETER_RASTER = 'Raster layer'
    PARAMETER_TABLE = 'Table'
    PARAMETER_VECTOR = 'Vector layer'
    PARAMETER_STRING = 'String'
    PARAMETER_EXPRESSION = 'Expression'
    PARAMETER_BOOLEAN = 'Boolean'
    PARAMETER_TABLE_FIELD = 'Table field'
    PARAMETER_EXTENT = 'Extent'
    PARAMETER_FILE = 'File'
    PARAMETER_POINT = 'Point'
    PARAMETER_CRS = 'CRS'
    PARAMETER_MULTIPLE = 'Multiple input'

    paramTypes = [
        PARAMETER_BOOLEAN,
        PARAMETER_EXTENT,
        PARAMETER_FILE,
        PARAMETER_NUMBER,
        PARAMETER_RASTER,
        PARAMETER_STRING,
        PARAMETER_EXPRESSION,
        PARAMETER_TABLE,
        PARAMETER_TABLE_FIELD,
        PARAMETER_VECTOR,
        PARAMETER_POINT,
        PARAMETER_CRS,
        PARAMETER_MULTIPLE
    ]

    def __init__(self, alg, paramType=None, param=None):
        self.alg = alg
        self.paramType = paramType
        self.param = param
        QDialog.__init__(self)
        self.setModal(True)
        self.setupUi()

    def setupUi(self):
        self.setWindowTitle(self.tr('Parameter definition'))
        self.setMinimumWidth(300)

        self.verticalLayout = QVBoxLayout(self)
        self.verticalLayout.setMargin(20)

        self.label = QLabel(self.tr('Parameter name'))
        self.verticalLayout.addWidget(self.label)
        self.nameTextBox = QLineEdit()
        self.verticalLayout.addWidget(self.nameTextBox)

        if isinstance(self.param, QgsProcessingParameterDefinition):
            self.nameTextBox.setText(self.param.description())

        if self.paramType == ModelerParameterDefinitionDialog.PARAMETER_BOOLEAN or \
                isinstance(self.param, QgsProcessingParameterBoolean):
            self.state = QCheckBox()
            self.state.setText(self.tr('Checked'))
            self.state.setChecked(False)
            if self.param is not None:
                self.state.setChecked(bool(self.param.defaultValue()))
            self.verticalLayout.addWidget(self.state)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_TABLE_FIELD or \
                isinstance(self.param, QgsProcessingParameterTableField):
            self.verticalLayout.addWidget(QLabel(self.tr('Parent layer')))
            self.parentCombo = QComboBox()
            idx = 0
            for param in list(self.alg.inputs.values()):
                if isinstance(param.param, (QgsProcessingParameterFeatureSource, QgsProcessingParameterTable)):
                    self.parentCombo.addItem(param.param.description(), param.param.name())
                    if self.param is not None:
                        if self.param.parentLayerParameter() == param.param.name():
                            self.parentCombo.setCurrentIndex(idx)
                    idx += 1
            self.verticalLayout.addWidget(self.parentCombo)

            # add the datatype selector
            self.verticalLayout.addWidget(QLabel(self.tr('Allowed data type')))
            self.datatypeCombo = QComboBox()
            self.datatypeCombo.addItem(self.tr('Any'), -1)
            self.datatypeCombo.addItem(self.tr('Number'), 0)
            self.datatypeCombo.addItem(self.tr('String'), 1)
            self.datatypeCombo.addItem(self.tr('Date/time'), 2)
            self.verticalLayout.addWidget(self.datatypeCombo)

            if self.param is not None and self.param.dataType() is not None:
                # QComboBoxes indexes start at 0,
                # self.param.datatype start with -1 that is why I need to do +1
                datatypeIndex = self.param.dataType() + 1
                self.datatypeCombo.setCurrentIndex(datatypeIndex)

            self.multipleCheck = QCheckBox()
            self.multipleCheck.setText(self.tr('Accept multiple fields'))
            self.multipleCheck.setChecked(False)
            if self.param is not None:
                self.multipleCheck.setChecked(self.param.allowMultiple())
            self.verticalLayout.addWidget(self.multipleCheck)

        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_VECTOR or
                isinstance(self.param, QgsProcessingParameterFeatureSource)):
            self.verticalLayout.addWidget(QLabel(self.tr('Shape type')))
            self.shapetypeCombo = QComboBox()
            self.shapetypeCombo.addItem(self.tr('Any'))
            self.shapetypeCombo.addItem(self.tr('Point'))
            self.shapetypeCombo.addItem(self.tr('Line'))
            self.shapetypeCombo.addItem(self.tr('Polygon'))
            if self.param is not None:
                self.shapetypeCombo.setCurrentIndex(self.param.dataTypes()[0] + 1)
            self.verticalLayout.addWidget(self.shapetypeCombo)
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_MULTIPLE or
                isinstance(self.param, QgsProcessingParameterMultipleLayers)):
            self.verticalLayout.addWidget(QLabel(self.tr('Data type')))
            self.datatypeCombo = QComboBox()
            self.datatypeCombo.addItem(self.tr('Vector (any)'))
            self.datatypeCombo.addItem(self.tr('Vector (point)'))
            self.datatypeCombo.addItem(self.tr('Vector (line)'))
            self.datatypeCombo.addItem(self.tr('Vector (polygon)'))
            self.datatypeCombo.addItem(self.tr('Raster'))
            self.datatypeCombo.addItem(self.tr('File'))
            if self.param is not None:
                self.datatypeCombo.setCurrentIndex(self.param.layerType() + 1)
            self.verticalLayout.addWidget(self.datatypeCombo)
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_NUMBER or
                isinstance(self.param, QgsProcessingParameterNumber)):
            self.verticalLayout.addWidget(QLabel(self.tr('Min value')))
            self.minTextBox = QLineEdit()
            self.verticalLayout.addWidget(self.minTextBox)
            self.verticalLayout.addWidget(QLabel(self.tr('Max value')))
            self.maxTextBox = QLineEdit()
            self.verticalLayout.addWidget(self.maxTextBox)
            if self.param is not None:
                self.minTextBox.setText(str(self.param.minimum()))
                self.maxTextBox.setText(str(self.param.maximum()))
            self.verticalLayout.addWidget(QLabel(self.tr('Default value')))
            self.defaultTextBox = QLineEdit()
            self.defaultTextBox.setText(self.tr('0'))
            if self.param is not None:
                default = self.param.defaultValue()
                if self.param.dataType() == QgsProcessingParameterNumber.Integer:
                    default = int(math.floor(default))
                if default:
                    self.defaultTextBox.setText(str(default))
            self.verticalLayout.addWidget(self.defaultTextBox)
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_EXPRESSION or
              isinstance(self.param, QgsProcessingParameterExpression)):
            self.verticalLayout.addWidget(QLabel(self.tr('Default value')))
            self.defaultEdit = QgsExpressionLineEdit()
            if self.param is not None:
                self.defaultEdit.setExpression(self.param.defaultValue())
            self.verticalLayout.addWidget(self.defaultEdit)

            self.verticalLayout.addWidget(QLabel(self.tr('Parent layer')))
            self.parentCombo = QComboBox()
            self.parentCombo.addItem(self.tr("None"), None)
            idx = 1
            for param in list(self.alg.inputs.values()):
                if isinstance(param.param, (QgsProcessingParameterFeatureSource, QgsProcessingParameterTable)):
                    self.parentCombo.addItem(param.param.description(), param.param.name())
                    if self.param is not None:
                        if self.param.parentLayerParameter() == param.param.name():
                            self.parentCombo.setCurrentIndex(idx)
                    idx += 1
            self.verticalLayout.addWidget(self.parentCombo)
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_STRING or
                isinstance(self.param, QgsProcessingParameterString)):
            self.verticalLayout.addWidget(QLabel(self.tr('Default value')))
            self.defaultTextBox = QLineEdit()
            if self.param is not None:
                self.defaultTextBox.setText(self.param.defaultValue())
            self.verticalLayout.addWidget(self.defaultTextBox)
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_FILE or
                isinstance(self.param, QgsProcessingParameterFile)):
            self.verticalLayout.addWidget(QLabel(self.tr('Type')))
            self.fileFolderCombo = QComboBox()
            self.fileFolderCombo.addItem(self.tr('File'))
            self.fileFolderCombo.addItem(self.tr('Folder'))
            if self.param is not None:
                self.fileFolderCombo.setCurrentIndex(
                    1 if self.param.behavior() == QgsProcessingParameterFile.Folder else 0)
            self.verticalLayout.addWidget(self.fileFolderCombo)
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_POINT or
                isinstance(self.param, QgsProcessingParameterPoint)):
            self.verticalLayout.addWidget(QLabel(self.tr('Default value')))
            self.defaultTextBox = QLineEdit()
            if self.param is not None:
                self.defaultTextBox.setText(self.param.defaultValue())
            self.verticalLayout.addWidget(self.defaultTextBox)
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_CRS or
                isinstance(self.param, QgsProcessingParameterCrs)):
            self.verticalLayout.addWidget(QLabel(self.tr('Default value')))
            self.selector = QgsProjectionSelectionWidget()
            if self.param is not None:
                self.selector.setCrs(QgsCoordinateReferenceSystem(self.param.defaultValue()))
            else:
                self.selector.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
            self.verticalLayout.addWidget(self.selector)

        self.verticalLayout.addSpacing(20)
        self.requiredCheck = QCheckBox()
        self.requiredCheck.setText(self.tr('Mandatory'))
        self.requiredCheck.setChecked(True)
        if self.param is not None:
            self.requiredCheck.setChecked(not self.param.flags() & QgsProcessingParameterDefinition.FlagOptional)
        self.verticalLayout.addWidget(self.requiredCheck)

        self.buttonBox = QDialogButtonBox(self)
        self.buttonBox.setOrientation(Qt.Horizontal)
        self.buttonBox.setStandardButtons(QDialogButtonBox.Cancel |
                                          QDialogButtonBox.Ok)
        self.buttonBox.setObjectName('buttonBox')
        self.buttonBox.accepted.connect(self.okPressed)
        self.buttonBox.rejected.connect(self.cancelPressed)

        self.verticalLayout.addStretch()
        self.verticalLayout.addWidget(self.buttonBox)

        self.setLayout(self.verticalLayout)

    def okPressed(self):
        description = str(self.nameTextBox.text())
        if description.strip() == '':
            QMessageBox.warning(self, self.tr('Unable to define parameter'),
                                self.tr('Invalid parameter name'))
            return
        if self.param is None:
            validChars = \
                'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'
            safeName = ''.join(c for c in description if c in validChars)
            name = safeName.lower()
            i = 2
            while name in self.alg.inputs:
                name = safeName.lower() + str(i)
        else:
            name = self.param.name()
        if (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_BOOLEAN or
                isinstance(self.param, QgsProcessingParameterBoolean)):
            self.param = QgsProcessingParameterBoolean(name, description, self.state.isChecked())
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_TABLE_FIELD or
              isinstance(self.param, QgsProcessingParameterTableField)):
            if self.parentCombo.currentIndex() < 0:
                QMessageBox.warning(self, self.tr('Unable to define parameter'),
                                    self.tr('Wrong or missing parameter values'))
                return
            parent = self.parentCombo.currentData()
            datatype = self.datatypeCombo.currentData()
            self.param = QgsProcessingParameterTableField(name, description, None, parent, datatype, self.multipleCheck.isChecked())
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_RASTER or
                isinstance(self.param, QgsProcessingParameterRasterLayer)):
            self.param = QgsProcessingParameterRasterLayer(
                name, description)
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_TABLE or
                isinstance(self.param, QgsProcessingParameterTable)):
            self.param = QgsProcessingParameterTable(
                name, description)
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_VECTOR or
                isinstance(self.param, QgsProcessingParameterFeatureSource)):
            self.param = QgsProcessingParameterFeatureSource(
                name, description,
                [self.shapetypeCombo.currentIndex() - 1])
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_MULTIPLE or
                isinstance(self.param, QgsProcessingParameterMultipleLayers)):
            self.param = QgsProcessingParameterMultipleLayers(
                name, description,
                self.datatypeCombo.currentIndex() - 1)
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_NUMBER or
                isinstance(self.param, QgsProcessingParameterNumber)):
            try:
                self.param = QgsProcessingParameterNumber(name, description, QgsProcessingParameterNumber.Double,
                                                          self.defaultTextBox.text())
                vmin = self.minTextBox.text().strip()
                if not vmin == '':
                    self.param.setMinimum(float(vmin))
                vmax = self.maxTextBox.text().strip()
                if not vmax == '':
                    self.param.setMaximum(float(vmax))
            except:
                QMessageBox.warning(self, self.tr('Unable to define parameter'),
                                    self.tr('Wrong or missing parameter values'))
                return
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_EXPRESSION or
                isinstance(self.param, QgsProcessingParameterExpression)):
            parent = self.parentCombo.currentData()
            self.param = QgsProcessingParameterExpression(name, description,
                                                          str(self.defaultEdit.expression()),
                                                          parent)
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_STRING or
                isinstance(self.param, QgsProcessingParameterString)):
            self.param = QgsProcessingParameterString(name, description,
                                                      str(self.defaultTextBox.text()))
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_EXTENT or
                isinstance(self.param, QgsProcessingParameterExtent)):
            self.param = QgsProcessingParameterExtent(name, description)
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_FILE or
                isinstance(self.param, QgsProcessingParameterFile)):
            isFolder = self.fileFolderCombo.currentIndex() == 1
            self.param = QgsProcessingParameterFile(name, description, QgsProcessingParameterFile.Folder if isFolder else QgsProcessingParameterFile.File)
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_POINT or
                isinstance(self.param, QgsProcessingParameterPoint)):
            self.param = QgsProcessingParameterPoint(name, description,
                                                     str(self.defaultTextBox.text()))
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_CRS or
                isinstance(self.param, QgsProcessingParameterCrs)):
            self.param = QgsProcessingParameterCrs(name, description, self.selector.crs().authid())
        if not self.requiredCheck.isChecked():
            self.param.setFlags(self.param.flags() | QgsProcessingParameterDefinition.FlagOptional)
        self.close()

    def cancelPressed(self):
        self.param = None
        self.close()
