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
from sextante.parameters.ParameterFile import ParameterFile

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4 import QtCore, QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from processing.parameters.Parameter import Parameter
from processing.parameters.ParameterBoolean import ParameterBoolean
from processing.parameters.ParameterRaster import ParameterRaster
from processing.parameters.ParameterTable import ParameterTable
from processing.parameters.ParameterVector import ParameterVector
from processing.parameters.ParameterMultipleInput import ParameterMultipleInput
from processing.parameters.ParameterNumber import ParameterNumber
from processing.parameters.ParameterString import ParameterString
from processing.parameters.ParameterTableField import ParameterTableField
from processing.parameters.ParameterExtent import ParameterExtent


class ModelerParameterDefinitionDialog(QtGui.QDialog):

    PARAMETER_NUMBER="Number"
    PARAMETER_RASTER="Raster Layer"
    PARAMETER_TABLE="Table"
    PARAMETER_VECTOR="Vector layer"
    PARAMETER_STRING="String"
    PARAMETER_BOOLEAN="Boolean"
    PARAMETER_TABLE_FIELD="Table field"
    PARAMETER_EXTENT="Extent"
    PARAMETER_FILE="File"
    #TO ADD
    PARAMETER_MULTIPLE="Multiple input"
    PARAMETER_FIXED_TABLE="Fixed table"

    paramTypes = [PARAMETER_BOOLEAN, PARAMETER_EXTENT, PARAMETER_FILE, PARAMETER_NUMBER, PARAMETER_RASTER,
                  PARAMETER_STRING, PARAMETER_TABLE, PARAMETER_TABLE_FIELD, PARAMETER_VECTOR]

    def __init__(self, alg, paramType = None, param = None):
        self.alg = alg;
        self.paramType = paramType
        self.param = param
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.setupUi()

    def setupUi(self):
        self.setWindowTitle("Parameter definition")

        self.verticalLayout = QtGui.QVBoxLayout(self)
        self.verticalLayout.setSpacing(40)
        self.verticalLayout.setMargin(20)

        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.label = QtGui.QLabel("Parameter name")
        self.horizontalLayout.addWidget(self.label)
        self.nameTextBox = QtGui.QLineEdit()
        self.horizontalLayout.addWidget(self.nameTextBox)
        self.verticalLayout.addLayout(self.horizontalLayout)

        self.horizontalLayout2 = QtGui.QHBoxLayout(self)
        self.horizontalLayout2.setSpacing(2)
        self.horizontalLayout2.setMargin(0)
        self.horizontalLayout3 = QtGui.QHBoxLayout(self)
        self.horizontalLayout3.setSpacing(2)
        self.horizontalLayout3.setMargin(0)

        if isinstance(self.param, Parameter):
            self.nameTextBox.setText(self.param.description)

        if self.paramType == ModelerParameterDefinitionDialog.PARAMETER_BOOLEAN or \
                    isinstance(self.param, ParameterBoolean):
            self.horizontalLayout2.addWidget(QtGui.QLabel("Default value"))
            self.yesNoCombo = QtGui.QComboBox()
            self.yesNoCombo.addItem("Yes")
            self.yesNoCombo.addItem("No")
            if self.param is not None:
                self.yesNoCombo.setCurrentIndex(0 if self.param.value else 1)
            self.horizontalLayout2.addWidget(self.yesNoCombo)
            self.verticalLayout.addLayout(self.horizontalLayout2)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_TABLE_FIELD \
                    or isinstance(self.param, ParameterTableField):
            self.horizontalLayout2.addWidget(QtGui.QLabel("Parent layer"))
            self.parentCombo = QtGui.QComboBox()
            idx = 0;
            for param in self.alg.parameters:
                if isinstance(param, (ParameterVector, ParameterTable)):
                    self.parentCombo.addItem(param.description, param.name)
                    if self.param is not None:
                        if self.param.parent == param.name:
                            self.parentCombo.setCurrentIndex(idx)
                    idx += 1
            self.horizontalLayout2.addWidget(self.parentCombo)
            self.verticalLayout.addLayout(self.horizontalLayout2)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_RASTER \
                    or isinstance(self.param, ParameterRaster):
            self.horizontalLayout2.addWidget(QtGui.QLabel("Required"))
            self.yesNoCombo = QtGui.QComboBox()
            self.yesNoCombo.addItem("Yes")
            self.yesNoCombo.addItem("No")
            if self.param is not None:
                self.yesNoCombo.setCurrentIndex(1 if self.param.optional else 0)
            self.horizontalLayout2.addWidget(self.yesNoCombo)
            self.verticalLayout.addLayout(self.horizontalLayout2)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_TABLE \
                    or isinstance(self.param, ParameterTable):
            self.horizontalLayout2.addWidget(QtGui.QLabel("Required"))
            self.yesNoCombo = QtGui.QComboBox()
            self.yesNoCombo.addItem("Yes")
            self.yesNoCombo.addItem("No")
            if self.param is not None:
                self.yesNoCombo.setCurrentIndex(1 if self.param.optional else 0)
            self.horizontalLayout2.addWidget(self.yesNoCombo)
            self.verticalLayout.addLayout(self.horizontalLayout2)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_VECTOR \
                    or isinstance(self.param, ParameterVector):
            self.horizontalLayout2.addWidget(QtGui.QLabel("Required"))
            self.yesNoCombo = QtGui.QComboBox()
            self.yesNoCombo.addItem("Yes")
            self.yesNoCombo.addItem("No")
            self.horizontalLayout2.addWidget(self.yesNoCombo)
            self.horizontalLayout3.addWidget(QtGui.QLabel("Shape type"))
            self.shapetypeCombo = QtGui.QComboBox()
            self.shapetypeCombo.addItem("Any")
            self.shapetypeCombo.addItem("Point")
            self.shapetypeCombo.addItem("Line")
            self.shapetypeCombo.addItem("Polygon")
            if self.param is not None:
                self.yesNoCombo.setCurrentIndex(1 if self.param.optional else 0)
                self.shapetypeCombo.setCurrentIndex(self.param.shapetype[0] + 1)
            self.horizontalLayout3.addWidget(self.shapetypeCombo)
            self.verticalLayout.addLayout(self.horizontalLayout3)
            self.verticalLayout.addLayout(self.horizontalLayout2)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_MULTIPLE \
                    or isinstance(self.param, ParameterMultipleInput):
            self.horizontalLayout2.addWidget(QtGui.QLabel("Mandatory"))
            self.yesNoCombo = QtGui.QComboBox()
            self.yesNoCombo.addItem("Yes")
            self.yesNoCombo.addItem("No")
            self.horizontalLayout2.addWidget(self.yesNoCombo)
            self.horizontalLayout3.addWidget(QtGui.QLabel("Data type"))
            self.datatypeCombo = QtGui.QComboBox()
            self.datatypeCombo.addItem("Vector (any)")
            self.datatypeCombo.addItem("Vector (point)")
            self.datatypeCombo.addItem("Vector (line)")
            self.datatypeCombo.addItem("Vector (polygon)")
            self.datatypeCombo.addItem("Raster")
            self.datatypeCombo.addItem("Table")
            if self.param is not None:
                self.yesNoCombo.setCurrentIndex(1 if self.param.optional else 0)
                self.datatypeCombo.setCurrentIndex(self.param.datatype + 1)
            self.horizontalLayout3.addWidget(self.datatypeCombo)
            self.verticalLayout.addLayout(self.horizontalLayout3)
            self.verticalLayout.addLayout(self.horizontalLayout2)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_NUMBER \
                    or isinstance(self.param, ParameterNumber):
            self.horizontalLayout2.addWidget(QtGui.QLabel("Min/Max values"))
            self.minTextBox = QtGui.QLineEdit()
            self.maxTextBox = QtGui.QLineEdit()
            self.horizontalLayout2.addWidget(self.minTextBox)
            self.horizontalLayout2.addWidget(self.maxTextBox)
            self.verticalLayout.addLayout(self.horizontalLayout2)
            self.horizontalLayout3.addWidget(QtGui.QLabel("Default value"))
            self.defaultTextBox = QtGui.QLineEdit()
            self.defaultTextBox.setText("0")
            if self.param is not None:
                self.defaultTextBox.setText(str(self.param.default))
            self.horizontalLayout3.addWidget(self.defaultTextBox)
            self.verticalLayout.addLayout(self.horizontalLayout3)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_STRING \
                    or isinstance(self.param, ParameterString):
            self.horizontalLayout2.addWidget(QtGui.QLabel("Default value"))
            self.defaultTextBox = QtGui.QLineEdit()
            if self.param is not None:
                self.defaultTextBox.setText(self.param.default)
            self.horizontalLayout2.addWidget(self.defaultTextBox)
            self.verticalLayout.addLayout(self.horizontalLayout2)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_FILE \
                    or isinstance(self.param, ParameterFile):
            self.horizontalLayout2.addWidget(QtGui.QLabel("Type"))
            self.fileFolderCombo = QtGui.QComboBox()
            self.fileFolderCombo.addItem("File")
            self.fileFolderCombo.addItem("Folder")
            if self.param is not None:
                self.fileFolderCombo.setCurrentIndex(1 if self.param.isFolder else 0)
            self.horizontalLayout2.addWidget(self.fileFolderCombo)
            self.verticalLayout.addLayout(self.horizontalLayout2)

        self.buttonBox = QtGui.QDialogButtonBox(self)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName("buttonBox")
        QObject.connect(self.buttonBox, QtCore.SIGNAL("accepted()"), self.okPressed)
        QObject.connect(self.buttonBox, QtCore.SIGNAL("rejected()"), self.cancelPressed)

        self.verticalLayout.addWidget(self.buttonBox)

        self.setLayout(self.verticalLayout)


    def okPressed(self):
        description = unicode(self.nameTextBox.text())
        if description.strip() == "":
            QMessageBox.critical(self, "Unable to define parameter", "Invalid parameter name")
            return
        if self.param is None:
            validChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
            safeName = ''.join(c for c in description if c in validChars)
            name = self.paramType.upper().replace(" ","") + "_" + safeName.upper()
        else:
            name = self.param.name
        if self.paramType == ModelerParameterDefinitionDialog.PARAMETER_BOOLEAN or isinstance(self.param, ParameterBoolean):
            self.param = ParameterBoolean(name, description, self.yesNoCombo.currentIndex() == 0)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_TABLE_FIELD or isinstance(self.param, ParameterTableField):
            if self.parentCombo.currentIndex() < 0:
                QMessageBox.critical(self, "Unable to define parameter", "Wrong or missing parameter values")
                return
            parent = self.parentCombo.itemData(self.parentCombo.currentIndex())
            self.param = ParameterTableField(name, description, parent)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_RASTER or isinstance(self.param, ParameterRaster):
            self.param = ParameterRaster(name, description, self.yesNoCombo.currentIndex() == 1)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_TABLE or isinstance(self.param, ParameterTable):
            self.param = ParameterTable(name, description, self.yesNoCombo.currentIndex() == 1)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_VECTOR or isinstance(self.param, ParameterVector):
            self.param = ParameterVector(name, description, [self.shapetypeCombo.currentIndex() - 1], self.yesNoCombo.currentIndex() == 1)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_MULTIPLE or isinstance(self.param, ParameterMultipleInput):
            self.param = ParameterMultipleInput(name, description, self.datatypeCombo.currentIndex()-1, self.yesNoCombo.currentIndex() == 1)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_NUMBER or isinstance(self.param, ParameterNumber):
            try:
                vmin = str(self.minTextBox.text()).strip()
                if vmin == "":
                    vmin = None
                else:
                    vmin = float(vmin)
                vmax = str(self.maxTextBox.text()).strip()
                if vmax == "":
                    vmax = None
                else:
                    vmax = float(vmax)
                self.param = ParameterNumber(name, description, vmin, vmax, float(str(self.defaultTextBox.text())))
            except:
                QMessageBox.critical(self, "Unable to define parameter", "Wrong or missing parameter values")
                return
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_STRING or isinstance(self.param, ParameterString):
            self.param = ParameterString(name, description, str(self.defaultTextBox.text()))
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_EXTENT or isinstance(self.param, ParameterExtent):
            self.param = ParameterExtent(name, description)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_FILE or isinstance(self.param, ParameterFile):
            isFolder = self.fileFolderCombo.currentIndex() == 1
            self.param = ParameterFile(name, description, isFolder=isFolder)
        self.close()

    def cancelPressed(self):
        self.param = None
        self.close()

