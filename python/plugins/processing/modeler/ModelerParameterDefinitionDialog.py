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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import math

from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtWidgets import (QDialog,
                                 QVBoxLayout,
                                 QHBoxLayout,
                                 QLabel,
                                 QLineEdit,
                                 QComboBox,
                                 QCheckBox,
                                 QDialogButtonBox,
                                 QMessageBox)

from processing.core.parameters import (Parameter,
                                        ParameterBoolean,
                                        ParameterRaster,
                                        ParameterTable,
                                        ParameterVector,
                                        ParameterMultipleInput,
                                        ParameterNumber,
                                        ParameterString,
                                        ParameterTableField,
                                        ParameterExtent,
                                        ParameterFile,
                                        ParameterPoint,
                                        ParameterCrs)
from processing.gui.CrsSelectionPanel import CrsSelectionPanel


class ModelerParameterDefinitionDialog(QDialog):

    PARAMETER_NUMBER = 'Number'
    PARAMETER_RASTER = 'Raster layer'
    PARAMETER_TABLE = 'Table'
    PARAMETER_VECTOR = 'Vector layer'
    PARAMETER_STRING = 'String'
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

        if isinstance(self.param, Parameter):
            self.nameTextBox.setText(self.param.description)

        if self.paramType == ModelerParameterDefinitionDialog.PARAMETER_BOOLEAN or \
                isinstance(self.param, ParameterBoolean):
            self.state = QCheckBox()
            self.state.setText(self.tr('Checked'))
            self.state.setChecked(False)
            if self.param is not None:
                self.state.setChecked(bool(self.param.value))
            self.verticalLayout.addWidget(self.state)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_TABLE_FIELD or \
                isinstance(self.param, ParameterTableField):
            self.verticalLayout.addWidget(QLabel(self.tr('Parent layer')))
            self.parentCombo = QComboBox()
            idx = 0
            for param in self.alg.inputs.values():
                if isinstance(param.param, (ParameterVector, ParameterTable)):
                    self.parentCombo.addItem(param.param.description, param.param.name)
                    if self.param is not None:
                        if self.param.parent == param.param.name:
                            self.parentCombo.setCurrentIndex(idx)
                    idx += 1
            self.verticalLayout.addWidget(self.parentCombo)

            # add the datatype selector
            self.verticalLayout.addWidget(QLabel(self.tr('Allowed data type')))
            self.datatypeCombo = QComboBox()
            self.datatypeCombo.addItem(self.tr('Any'), -1)
            self.datatypeCombo.addItem(self.tr('Number'), 0)
            self.datatypeCombo.addItem(self.tr('String'), 1)
            self.verticalLayout.addWidget(self.datatypeCombo)

            if self.param is not None and self.param.datatype is not None:
                # QComboBoxes indexes start at 0,
                # self.param.datatype start with -1 that is why I need to do +1
                datatypeIndex = self.param.datatype + 1
                self.datatypeCombo.setCurrentIndex(datatypeIndex)

        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_VECTOR or
                isinstance(self.param, ParameterVector)):
            self.verticalLayout.addWidget(QLabel(self.tr('Shape type')))
            self.shapetypeCombo = QComboBox()
            self.shapetypeCombo.addItem(self.tr('Any'))
            self.shapetypeCombo.addItem(self.tr('Point'))
            self.shapetypeCombo.addItem(self.tr('Line'))
            self.shapetypeCombo.addItem(self.tr('Polygon'))
            if self.param is not None:
                self.shapetypeCombo.setCurrentIndex(self.param.datatype[0] + 1)
            self.verticalLayout.addWidget(self.shapetypeCombo)
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_MULTIPLE or
                isinstance(self.param, ParameterMultipleInput)):
            self.verticalLayout.addWidget(QLabel(self.tr('Data type')))
            self.datatypeCombo = QComboBox()
            self.datatypeCombo.addItem(self.tr('Vector (any)'))
            self.datatypeCombo.addItem(self.tr('Vector (point)'))
            self.datatypeCombo.addItem(self.tr('Vector (line)'))
            self.datatypeCombo.addItem(self.tr('Vector (polygon)'))
            self.datatypeCombo.addItem(self.tr('Raster'))
            self.datatypeCombo.addItem(self.tr('File'))
            if self.param is not None:
                self.datatypeCombo.setCurrentIndex(self.param.datatype + 1)
            self.verticalLayout.addWidget(self.datatypeCombo)
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_NUMBER or
                isinstance(self.param, ParameterNumber)):
            self.verticalLayout.addWidget(QLabel(self.tr('Min value')))
            self.minTextBox = QLineEdit()
            self.verticalLayout.addWidget(self.minTextBox)
            self.verticalLayout.addWidget(QLabel(self.tr('Max value')))
            self.maxTextBox = QLineEdit()
            self.verticalLayout.addWidget(self.maxTextBox)
            if self.param is not None:
                self.minTextBox.setText(unicode(self.param.min))
                self.maxTextBox.setText(unicode(self.param.max))
            self.horizontalLayoutDefault.addWidget(QLabel(self.tr('Default value')))
            self.defaultTextBox = QLineEdit()
            self.defaultTextBox.setText(self.tr('0'))
            if self.param is not None:
                default = self.param.default
                if self.param.isInteger:
                    default = int(math.floor(default))
                if default:
                    self.defaultTextBox.setText(unicode(default))
            self.verticalLayout.addWidget(self.defaultTextBox)
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_STRING or
                isinstance(self.param, ParameterString)):
            self.verticalLayout.addWidget(QLabel(self.tr('Default value')))
            self.defaultTextBox = QLineEdit()
            if self.param is not None:
                self.defaultTextBox.setText(self.param.default)
            self.verticalLayout.addWidget(self.defaultTextBox)
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_FILE or
                isinstance(self.param, ParameterFile)):
            self.verticalLayout.addWidget(QLabel(self.tr('Type')))
            self.fileFolderCombo = QComboBox()
            self.fileFolderCombo.addItem(self.tr('File'))
            self.fileFolderCombo.addItem(self.tr('Folder'))
            if self.param is not None:
                self.fileFolderCombo.setCurrentIndex(
                    1 if self.param.isFolder else 0)
            self.verticalLayout.addWidget(self.fileFolderCombo)
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_POINT or
                isinstance(self.param, ParameterPoint)):
            self.verticalLayout.addWidget(QLabel(self.tr('Default value')))
            self.defaultTextBox = QLineEdit()
            if self.param is not None:
                self.defaultTextBox.setText(self.param.default)
            self.verticalLayout.addWidget(self.defaultTextBox)
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_CRS or
                isinstance(self.param, ParameterCrs)):
            self.verticalLayout.addWidget(QLabel(self.tr('Default value')))
            self.defaultTextBox = CrsSelectionPanel('EPSG:4326')
            if self.param is not None:
                self.defaultTextBox.setAuthId(self.param.default)
            self.verticalLayout.addWidget(self.defaultTextBox)

        self.verticalLayout.addSpacing(20)
        self.requiredCheck = QCheckBox()
        self.requiredCheck.setText(self.tr('Mandatory'))
        self.requiredCheck.setChecked(True)
        if self.param is not None:
            self.requiredCheck.setChecked(self.param.optional)
        self.verticalLayout.addWidget(self.requiredCheck)

        self.buttonBox = QDialogButtonBox(self)
        self.buttonBox.setOrientation(Qt.Horizontal)
        self.buttonBox.setStandardButtons(QDialogButtonBox.Cancel
                                          | QDialogButtonBox.Ok)
        self.buttonBox.setObjectName('buttonBox')
        self.buttonBox.accepted.connect(self.okPressed)
        self.buttonBox.rejected.connect(self.cancelPressed)

        self.verticalLayout.addStretch()
        self.verticalLayout.addWidget(self.buttonBox)

        self.setLayout(self.verticalLayout)

    def okPressed(self):
        description = unicode(self.nameTextBox.text())
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
            name = self.param.name
        if (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_BOOLEAN
                or isinstance(self.param, ParameterBoolean)):
            self.param = ParameterBoolean(name, description, self.state.isChecked())
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_TABLE_FIELD
                or isinstance(self.param, ParameterTableField)):
            if self.parentCombo.currentIndex() < 0:
                QMessageBox.warning(self, self.tr('Unable to define parameter'),
                                    self.tr('Wrong or missing parameter values'))
                return
            parent = self.parentCombo.itemData(self.parentCombo.currentIndex())
            datatype = self.datatypeCombo.itemData(self.datatypeCombo.currentIndex())
            self.param = ParameterTableField(name, description, parent, datatype)
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_RASTER or
                isinstance(self.param, ParameterRaster)):
            self.param = ParameterRaster(
                name, description)
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_TABLE or
                isinstance(self.param, ParameterTable)):
            self.param = ParameterTable(
                name, description)
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_VECTOR or
                isinstance(self.param, ParameterVector)):
            self.param = ParameterVector(
                name, description,
                [self.shapetypeCombo.currentIndex() - 1])
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_MULTIPLE or
                isinstance(self.param, ParameterMultipleInput)):
            self.param = ParameterMultipleInput(
                name, description,
                self.datatypeCombo.currentIndex() - 1)
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_NUMBER or
                isinstance(self.param, ParameterNumber)):
            try:
                vmin = self.minTextBox.text().strip()
                if vmin == '':
                    vmin = None
                else:
                    vmin = float(vmin)
                vmax = self.maxTextBox.text().strip()
                if vmax == '':
                    vmax = None
                else:
                    vmax = float(vmax)
                self.param = ParameterNumber(name, description, vmin, vmax,
                                             unicode(self.defaultTextBox.text()))
            except:
                QMessageBox.warning(self, self.tr('Unable to define parameter'),
                                    self.tr('Wrong or missing parameter values'))
                return
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_STRING or
                isinstance(self.param, ParameterString)):
            self.param = ParameterString(name, description,
                                         unicode(self.defaultTextBox.text()))
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_EXTENT or
                isinstance(self.param, ParameterExtent)):
            self.param = ParameterExtent(name, description)
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_FILE or
                isinstance(self.param, ParameterFile)):
            isFolder = self.fileFolderCombo.currentIndex() == 1
            self.param = ParameterFile(name, description, isFolder=isFolder)
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_POINT or
                isinstance(self.param, ParameterPoint)):
            self.param = ParameterPoint(name, description,
                                        unicode(self.defaultTextBox.text()))
        elif (self.paramType == ModelerParameterDefinitionDialog.PARAMETER_CRS or
                isinstance(self.param, ParameterCrs)):
            self.param = ParameterCrs(name, description, self.defaultTextBox.getValue())
        self.param.optional = self.requiredCheck.isChecked()
        self.close()

    def cancelPressed(self):
        self.param = None
        self.close()
