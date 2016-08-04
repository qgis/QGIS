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
from qgis.PyQt.QtWidgets import QDialog, QVBoxLayout, QHBoxLayout, QLabel, QLineEdit, QComboBox, QCheckBox, QDialogButtonBox, QMessageBox

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
                                        ParameterTableMultipleField)


class ModelerParameterDefinitionDialog(QDialog):

    PARAMETER_NUMBER = 'Number'
    PARAMETER_RASTER = 'Raster layer'
    PARAMETER_TABLE = 'Table'
    PARAMETER_VECTOR = 'Vector layer'
    PARAMETER_STRING = 'String'
    PARAMETER_BOOLEAN = 'Boolean'
    PARAMETER_TABLE_FIELD = 'Table field'
    PARAMETER_TABLE_MULTIPLE_FIELD = 'Table multiple field'
    PARAMETER_EXTENT = 'Extent'
    PARAMETER_FILE = 'File'
    PARAMETER_POINT = 'Point'

    # To add
    PARAMETER_MULTIPLE = 'Multiple input'
    PARAMETER_FIXED_TABLE = 'Fixed table'

    paramTypes = [
        PARAMETER_BOOLEAN,
        PARAMETER_EXTENT,
        PARAMETER_FILE,
        PARAMETER_NUMBER,
        PARAMETER_RASTER,
        PARAMETER_STRING,
        PARAMETER_TABLE,
        PARAMETER_TABLE_FIELD,
        PARAMETER_TABLE_MULTIPLE_FIELD,
        PARAMETER_VECTOR,
        PARAMETER_POINT
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

        self.verticalLayout = QVBoxLayout(self)
        self.verticalLayout.setSpacing(40)
        self.verticalLayout.setMargin(20)

        self.horizontalLayoutName = QHBoxLayout(self)
        self.horizontalLayoutName.setSpacing(2)
        self.horizontalLayoutName.setMargin(0)
        self.label = QLabel(self.tr('Parameter name'))
        self.horizontalLayoutName.addWidget(self.label)
        self.nameTextBox = QLineEdit()
        self.horizontalLayoutName.addWidget(self.nameTextBox)
        self.verticalLayout.addLayout(self.horizontalLayoutName)

        self.horizontalLayoutRequired = QHBoxLayout(self)
        self.horizontalLayoutRequired.setSpacing(2)
        self.horizontalLayoutRequired.setMargin(0)
        self.horizontalLayoutParent = QHBoxLayout(self)
        self.horizontalLayoutParent.setSpacing(2)
        self.horizontalLayoutParent.setMargin(0)
        self.horizontalLayoutDefault = QHBoxLayout(self)
        self.horizontalLayoutDefault.setSpacing(2)
        self.horizontalLayoutDefault.setMargin(0)
        self.horizontalLayoutDatatype = QHBoxLayout(self)
        self.horizontalLayoutDatatype.setSpacing(2)
        self.horizontalLayoutDatatype.setMargin(0)

        if isinstance(self.param, Parameter):
            self.nameTextBox.setText(self.param.description)

        if self.paramType == ModelerParameterDefinitionDialog.PARAMETER_BOOLEAN or \
           isinstance(self.param, ParameterBoolean):
            self.state = QCheckBox()
            self.state.setText(self.tr('Checked'))
            self.state.setChecked(False)
            if self.param is not None:
                self.state.setChecked(True if self.param.value else False)
            self.horizontalLayoutParent.addWidget(self.state)
            self.verticalLayout.addLayout(self.horizontalLayoutParent)
        elif self.paramType in (
            ModelerParameterDefinitionDialog.PARAMETER_TABLE_FIELD,
            ModelerParameterDefinitionDialog.PARAMETER_TABLE_MULTIPLE_FIELD)\
            or isinstance(self.param, (ParameterTableField,
                                       ParameterTableMultipleField)):
            self.horizontalLayoutParent.addWidget(QLabel(self.tr('Parent layer')))
            self.parentCombo = QComboBox()
            idx = 0
            for param in self.alg.inputs.values():
                if isinstance(param.param, (ParameterVector, ParameterTable)):
                    self.parentCombo.addItem(param.param.description, param.param.name)
                    if self.param is not None:
                        if self.param.parent == param.param.name:
                            self.parentCombo.setCurrentIndex(idx)
                    idx += 1
            self.horizontalLayoutParent.addWidget(self.parentCombo)
            self.verticalLayout.addLayout(self.horizontalLayoutParent)

            # add the datatype selector
            self.horizontalLayoutDatatype.addWidget(QLabel(self.tr('Allowed '
                                                                   'data type')))
            self.datatypeCombo = QComboBox()
            self.datatypeCombo.addItem(self.tr('Any'), -1)
            self.datatypeCombo.addItem(self.tr('Number'), 0)
            self.datatypeCombo.addItem(self.tr('String'), 1)
            self.horizontalLayoutDatatype.addWidget(self.datatypeCombo)

            if self.param is not None and self.param.datatype is not None:
                # QComboBoxes indexes start at 0,
                # self.param.datatype start with -1 that is why I need to do +1
                datatype_index = self.param.datatype + 1
                self.datatypeCombo.setCurrentIndex(datatype_index)
            self.verticalLayout.addLayout(self.horizontalLayoutDatatype)

        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_VECTOR or \
                isinstance(self.param, ParameterVector):
            self.horizontalLayoutParent.addWidget(QLabel(self.tr('Shape type')))
            self.shapetypeCombo = QComboBox()
            self.shapetypeCombo.addItem(self.tr('Any'))
            self.shapetypeCombo.addItem(self.tr('Point'))
            self.shapetypeCombo.addItem(self.tr('Line'))
            self.shapetypeCombo.addItem(self.tr('Polygon'))
            if self.param is not None:
                self.shapetypeCombo.setCurrentIndex(self.param.shapetype[0] + 1)
            self.horizontalLayoutParent.addWidget(self.shapetypeCombo)
            self.verticalLayout.addLayout(self.horizontalLayoutParent)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_MULTIPLE or \
                isinstance(self.param, ParameterMultipleInput):
            self.horizontalLayoutParent.addWidget(QLabel(self.tr('Data type')))
            self.datatypeCombo = QComboBox()
            self.datatypeCombo.addItem(self.tr('Vector (any)'))
            self.datatypeCombo.addItem(self.tr('Vector (point)'))
            self.datatypeCombo.addItem(self.tr('Vector (line)'))
            self.datatypeCombo.addItem(self.tr('Vector (polygon)'))
            self.datatypeCombo.addItem(self.tr('Raster'))
            self.datatypeCombo.addItem(self.tr('Table'))
            if self.param is not None:
                self.datatypeCombo.setCurrentIndex(self.param.datatype + 1)
            self.horizontalLayoutParent.addWidget(self.datatypeCombo)
            self.verticalLayout.addLayout(self.horizontalLayoutParent)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_NUMBER or \
                isinstance(self.param, ParameterNumber):
            self.horizontalLayoutParent.addWidget(QLabel(self.tr('Min/Max values')))
            self.minTextBox = QLineEdit()
            self.maxTextBox = QLineEdit()
            if self.param is not None:
                self.minTextBox.setText(unicode(self.param.min))
                self.maxTextBox.setText(unicode(self.param.max))
            self.horizontalLayoutParent.addWidget(self.minTextBox)
            self.horizontalLayoutParent.addWidget(self.maxTextBox)
            self.verticalLayout.addLayout(self.horizontalLayoutParent)
            self.horizontalLayoutDefault.addWidget(QLabel(self.tr('Default value')))
            self.defaultTextBox = QLineEdit()
            self.defaultTextBox.setText(self.tr('0'))
            if self.param is not None:
                default = self.param.default
                if self.param.isInteger:
                    default = int(math.floor(default))
                self.defaultTextBox.setText(unicode(default))
            self.horizontalLayoutDefault.addWidget(self.defaultTextBox)
            self.verticalLayout.addLayout(self.horizontalLayoutDefault)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_STRING or \
                isinstance(self.param, ParameterString):
            self.horizontalLayoutParent.addWidget(QLabel(self.tr('Default value')))
            self.defaultTextBox = QLineEdit()
            if self.param is not None:
                self.defaultTextBox.setText(self.param.default)
            self.horizontalLayoutParent.addWidget(self.defaultTextBox)
            self.verticalLayout.addLayout(self.horizontalLayoutParent)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_FILE or \
                isinstance(self.param, ParameterFile):
            self.horizontalLayoutParent.addWidget(QLabel(self.tr('Type')))
            self.fileFolderCombo = QComboBox()
            self.fileFolderCombo.addItem(self.tr('File'))
            self.fileFolderCombo.addItem(self.tr('Folder'))
            if self.param is not None:
                self.fileFolderCombo.setCurrentIndex(
                    1 if self.param.isFolder else 0)
            self.horizontalLayoutParent.addWidget(self.fileFolderCombo)
            self.verticalLayout.addLayout(self.horizontalLayoutParent)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_POINT or \
                isinstance(self.param, ParameterPoint):
            self.horizontalLayoutParent.addWidget(QLabel(self.tr('Default value')))
            self.defaultTextBox = QLineEdit()
            if self.param is not None:
                self.defaultTextBox.setText(self.param.default)
            self.horizontalLayoutParent.addWidget(self.defaultTextBox)
            self.verticalLayout.addLayout(self.horizontalLayoutParent)

        self.horizontalLayoutRequired.addWidget(QLabel(self.tr('Required')))
        self.yesNoCombo = QComboBox()
        self.yesNoCombo.addItem(self.tr('Yes'))
        self.yesNoCombo.addItem(self.tr('No'))
        self.horizontalLayoutRequired.addWidget(self.yesNoCombo)
        if self.param is not None:
            self.yesNoCombo.setCurrentIndex(
                1 if self.param.optional else 0)
        self.verticalLayout.addLayout(self.horizontalLayoutRequired)

        self.buttonBox = QDialogButtonBox(self)
        self.buttonBox.setOrientation(Qt.Horizontal)
        self.buttonBox.setStandardButtons(QDialogButtonBox.Cancel
                                          | QDialogButtonBox.Ok)
        self.buttonBox.setObjectName('buttonBox')
        self.buttonBox.accepted.connect(self.okPressed)
        self.buttonBox.rejected.connect(self.cancelPressed)

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
        if self.paramType \
                == ModelerParameterDefinitionDialog.PARAMETER_BOOLEAN \
                or isinstance(self.param, ParameterBoolean):
            self.param = ParameterBoolean(name, description,
                                          self.state.isChecked())
        elif self.paramType in (
                ModelerParameterDefinitionDialog.PARAMETER_TABLE_FIELD,
                ModelerParameterDefinitionDialog.PARAMETER_TABLE_MULTIPLE_FIELD)\
            or isinstance(self.param, (ParameterTableField,
                                       ParameterTableMultipleField)):
            if self.parentCombo.currentIndex() < 0:
                QMessageBox.warning(self, self.tr('Unable to define parameter'),
                                    self.tr('Wrong or missing parameter values'))
                return
            parent = self.parentCombo.itemData(self.parentCombo.currentIndex())
            datatype = self.datatypeCombo.itemData(
                self.datatypeCombo.currentIndex())

            if (self.paramType ==
                    ModelerParameterDefinitionDialog.PARAMETER_TABLE_FIELD or
                    isinstance(self.param, ParameterTableField)):
                self.param = ParameterTableField(
                    name, description, parent, datatype)
            else:
                self.param = ParameterTableMultipleField(
                    name, description, parent, datatype)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_RASTER or \
                isinstance(self.param, ParameterRaster):
            self.param = ParameterRaster(
                name, description,
                self.yesNoCombo.currentIndex() == 1)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_TABLE or \
                isinstance(self.param, ParameterTable):
            self.param = ParameterTable(
                name, description,
                self.yesNoCombo.currentIndex() == 1)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_VECTOR or \
                isinstance(self.param, ParameterVector):
            self.param = ParameterVector(
                name, description,
                [self.shapetypeCombo.currentIndex() - 1],
                self.yesNoCombo.currentIndex() == 1)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_MULTIPLE or \
                isinstance(self.param, ParameterMultipleInput):
            self.param = ParameterMultipleInput(
                name, description,
                self.datatypeCombo.currentIndex() - 1,
                self.yesNoCombo.currentIndex() == 1)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_NUMBER or \
                isinstance(self.param, ParameterNumber):
            try:
                vmin = unicode(self.minTextBox.text()).strip()
                if vmin == '':
                    vmin = None
                else:
                    vmin = float(vmin)
                vmax = unicode(self.maxTextBox.text()).strip()
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
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_STRING or \
                isinstance(self.param, ParameterString):
            self.param = ParameterString(name, description,
                                         unicode(self.defaultTextBox.text()))
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_EXTENT or \
                isinstance(self.param, ParameterExtent):
            self.param = ParameterExtent(name, description)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_FILE or \
                isinstance(self.param, ParameterFile):
            isFolder = self.fileFolderCombo.currentIndex() == 1
            self.param = ParameterFile(name, description, isFolder=isFolder)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_POINT or \
                isinstance(self.param, ParameterPoint):
            self.param = ParameterPoint(name, description,
                                        unicode(self.defaultTextBox.text()))
        self.param.optional = self.yesNoCombo.currentIndex() == 1
        self.close()

    def cancelPressed(self):
        self.param = None
        self.close()
