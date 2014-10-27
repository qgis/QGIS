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

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from processing.core.parameters import Parameter
from processing.core.parameters import ParameterBoolean
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterTable
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterMultipleInput
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterTableField
from processing.core.parameters import ParameterExtent
from processing.core.parameters import ParameterFile


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
        PARAMETER_VECTOR,
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

        self.horizontalLayout = QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.label = QLabel(self.tr('Parameter name'))
        self.horizontalLayout.addWidget(self.label)
        self.nameTextBox = QLineEdit()
        self.horizontalLayout.addWidget(self.nameTextBox)
        self.verticalLayout.addLayout(self.horizontalLayout)

        self.horizontalLayout2 = QHBoxLayout(self)
        self.horizontalLayout2.setSpacing(2)
        self.horizontalLayout2.setMargin(0)
        self.horizontalLayout3 = QHBoxLayout(self)
        self.horizontalLayout3.setSpacing(2)
        self.horizontalLayout3.setMargin(0)

        if isinstance(self.param, Parameter):
            self.nameTextBox.setText(self.param.description)

        if self.paramType \
                == ModelerParameterDefinitionDialog.PARAMETER_BOOLEAN \
                or isinstance(self.param, ParameterBoolean):
            self.state = QCheckBox()
            self.state.setText(self.tr('Checked'))
            self.state.setChecked(False)
            if self.param is not None:
                self.state.setChecked(True if self.param.value else False)
            self.horizontalLayout2.addWidget(self.state)
            self.verticalLayout.addLayout(self.horizontalLayout2)
        elif self.paramType \
                == ModelerParameterDefinitionDialog.PARAMETER_TABLE_FIELD \
                or isinstance(self.param, ParameterTableField):
            self.horizontalLayout2.addWidget(QLabel(self.tr('Parent layer')))
            self.parentCombo = QComboBox()
            idx = 0
            for param in self.alg.inputs.values():
                if isinstance(param.param, (ParameterVector, ParameterTable)):
                    self.parentCombo.addItem(param.param.description, param.param.name)
                    if self.param is not None:
                        if self.param.parent == param.param.name:
                            self.parentCombo.setCurrentIndex(idx)
                    idx += 1
            self.horizontalLayout2.addWidget(self.parentCombo)
            self.verticalLayout.addLayout(self.horizontalLayout2)
        elif self.paramType \
                == ModelerParameterDefinitionDialog.PARAMETER_RASTER \
                or isinstance(self.param, ParameterRaster):
            self.horizontalLayout2.addWidget(QLabel(self.tr('Required')))
            self.yesNoCombo = QComboBox()
            self.yesNoCombo.addItem(self.tr('Yes'))
            self.yesNoCombo.addItem(self.tr('No'))
            if self.param is not None:
                self.yesNoCombo.setCurrentIndex(
                        (1 if self.param.optional else 0))
            self.horizontalLayout2.addWidget(self.yesNoCombo)
            self.verticalLayout.addLayout(self.horizontalLayout2)
        elif self.paramType \
                == ModelerParameterDefinitionDialog.PARAMETER_TABLE \
                or isinstance(self.param, ParameterTable):
            self.horizontalLayout2.addWidget(QLabel(self.tr('Required')))
            self.yesNoCombo = QComboBox()
            self.yesNoCombo.addItem(self.tr('Yes'))
            self.yesNoCombo.addItem(self.tr('No'))
            if self.param is not None:
                self.yesNoCombo.setCurrentIndex(
                        (1 if self.param.optional else 0))
            self.horizontalLayout2.addWidget(self.yesNoCombo)
            self.verticalLayout.addLayout(self.horizontalLayout2)
        elif self.paramType \
                == ModelerParameterDefinitionDialog.PARAMETER_VECTOR \
                or isinstance(self.param, ParameterVector):
            self.horizontalLayout2.addWidget(QLabel(self.tr('Required')))
            self.yesNoCombo = QComboBox()
            self.yesNoCombo.addItem(self.tr('Yes'))
            self.yesNoCombo.addItem(self.tr('No'))
            self.horizontalLayout2.addWidget(self.yesNoCombo)
            self.horizontalLayout3.addWidget(QLabel(self.tr('Shape type')))
            self.shapetypeCombo = QComboBox()
            self.shapetypeCombo.addItem(self.tr('Any'))
            self.shapetypeCombo.addItem(self.tr('Point'))
            self.shapetypeCombo.addItem(self.tr('Line'))
            self.shapetypeCombo.addItem(self.tr('Polygon'))
            if self.param is not None:
                self.yesNoCombo.setCurrentIndex(
                        (1 if self.param.optional else 0))
                self.shapetypeCombo.setCurrentIndex(self.param.shapetype[0]
                        + 1)
            self.horizontalLayout3.addWidget(self.shapetypeCombo)
            self.verticalLayout.addLayout(self.horizontalLayout3)
            self.verticalLayout.addLayout(self.horizontalLayout2)
        elif self.paramType \
                == ModelerParameterDefinitionDialog.PARAMETER_MULTIPLE \
                or isinstance(self.param, ParameterMultipleInput):
            self.horizontalLayout2.addWidget(QLabel(self.tr('Mandatory')))
            self.yesNoCombo = QComboBox()
            self.yesNoCombo.addItem(self.tr('Yes'))
            self.yesNoCombo.addItem(self.tr('No'))
            self.horizontalLayout2.addWidget(self.yesNoCombo)
            self.horizontalLayout3.addWidget(QLabel(self.tr('Data type')))
            self.datatypeCombo = QComboBox()
            self.datatypeCombo.addItem(self.tr('Vector (any)'))
            self.datatypeCombo.addItem(self.tr('Vector (point)'))
            self.datatypeCombo.addItem(self.tr('Vector (line)'))
            self.datatypeCombo.addItem(self.tr('Vector (polygon)'))
            self.datatypeCombo.addItem(self.tr('Raster'))
            self.datatypeCombo.addItem(self.tr('Table'))
            if self.param is not None:
                self.yesNoCombo.setCurrentIndex(
                        (1 if self.param.optional else 0))
                self.datatypeCombo.setCurrentIndex(self.param.datatype + 1)
            self.horizontalLayout3.addWidget(self.datatypeCombo)
            self.verticalLayout.addLayout(self.horizontalLayout3)
            self.verticalLayout.addLayout(self.horizontalLayout2)
        elif self.paramType \
                == ModelerParameterDefinitionDialog.PARAMETER_NUMBER \
                or isinstance(self.param, ParameterNumber):
            self.horizontalLayout2.addWidget(QLabel(self.tr('Min/Max values')))
            self.minTextBox = QLineEdit()
            self.maxTextBox = QLineEdit()
            if self.param is not None:
                self.minTextBox.setText(str(self.param.min))
                self.maxTextBox.setText(str(self.param.max))
            self.horizontalLayout2.addWidget(self.minTextBox)
            self.horizontalLayout2.addWidget(self.maxTextBox)
            self.verticalLayout.addLayout(self.horizontalLayout2)
            self.horizontalLayout3.addWidget(QLabel(self.tr('Default value')))
            self.defaultTextBox = QLineEdit()
            self.defaultTextBox.setText(self.tr('0'))
            if self.param is not None:
                default = self.param.default
                if self.param.isInteger:
                    default = int(math.floor(default))
                self.defaultTextBox.setText(str(default))
            self.horizontalLayout3.addWidget(self.defaultTextBox)
            self.verticalLayout.addLayout(self.horizontalLayout3)
        elif self.paramType \
                == ModelerParameterDefinitionDialog.PARAMETER_STRING \
                or isinstance(self.param, ParameterString):
            self.horizontalLayout2.addWidget(QLabel(self.tr('Default value')))
            self.defaultTextBox = QLineEdit()
            if self.param is not None:
                self.defaultTextBox.setText(self.param.default)
            self.horizontalLayout2.addWidget(self.defaultTextBox)
            self.verticalLayout.addLayout(self.horizontalLayout2)
        elif self.paramType == \
                ModelerParameterDefinitionDialog.PARAMETER_FILE \
                or isinstance(self.param, ParameterFile):
            self.horizontalLayout2.addWidget(QLabel(self.tr('Type')))
            self.fileFolderCombo = QComboBox()
            self.fileFolderCombo.addItem(self.tr('File'))
            self.fileFolderCombo.addItem(self.tr('Folder'))
            if self.param is not None:
                self.fileFolderCombo.setCurrentIndex(
                        (1 if self.param.isFolder else 0))
            self.horizontalLayout2.addWidget(self.fileFolderCombo)
            self.verticalLayout.addLayout(self.horizontalLayout2)

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
            name = self.paramType.upper().replace(' ', '') + '_' \
                + safeName.upper()
        else:
            name = self.param.name
        if self.paramType \
                == ModelerParameterDefinitionDialog.PARAMETER_BOOLEAN \
                or isinstance(self.param, ParameterBoolean):
            self.param = ParameterBoolean(name, description,
                    self.state.isChecked())
        elif self.paramType \
                == ModelerParameterDefinitionDialog.PARAMETER_TABLE_FIELD \
                or isinstance(self.param, ParameterTableField):
            if self.parentCombo.currentIndex() < 0:
                QMessageBox.warning(self, self.tr('Unable to define parameter'),
                                    self.tr('Wrong or missing parameter values'))
                return
            parent = self.parentCombo.itemData(self.parentCombo.currentIndex())
            self.param = ParameterTableField(name, description, parent)
        elif self.paramType \
                == ModelerParameterDefinitionDialog.PARAMETER_RASTER \
                or isinstance(self.param, ParameterRaster):
            self.param = ParameterRaster(name, description,
                    self.yesNoCombo.currentIndex() == 1)
        elif self.paramType \
                == ModelerParameterDefinitionDialog.PARAMETER_TABLE \
                or isinstance(self.param, ParameterTable):
            self.param = ParameterTable(name, description,
                    self.yesNoCombo.currentIndex() == 1)
        elif self.paramType \
                == ModelerParameterDefinitionDialog.PARAMETER_VECTOR \
                or isinstance(self.param, ParameterVector):
            self.param = ParameterVector(name, description,
                    [self.shapetypeCombo.currentIndex() - 1],
                    self.yesNoCombo.currentIndex() == 1)
        elif self.paramType \
                == ModelerParameterDefinitionDialog.PARAMETER_MULTIPLE \
                or isinstance(self.param, ParameterMultipleInput):
            self.param = ParameterMultipleInput(name, description,
                    self.datatypeCombo.currentIndex() - 1,
                    self.yesNoCombo.currentIndex() == 1)
        elif self.paramType \
                == ModelerParameterDefinitionDialog.PARAMETER_NUMBER \
                or isinstance(self.param, ParameterNumber):
            try:
                vmin = str(self.minTextBox.text()).strip()
                if vmin == '':
                    vmin = None
                else:
                    vmin = float(vmin)
                vmax = str(self.maxTextBox.text()).strip()
                if vmax == '':
                    vmax = None
                else:
                    vmax = float(vmax)
                self.param = ParameterNumber(name, description, vmin, vmax,
                        str(self.defaultTextBox.text()))
            except:
                QMessageBox.warning(self, self.tr('Unable to define parameter'),
                                    self.tr('Wrong or missing parameter values'))
                return
        elif self.paramType \
                == ModelerParameterDefinitionDialog.PARAMETER_STRING \
                or isinstance(self.param, ParameterString):
            self.param = ParameterString(name, description,
                    unicode(self.defaultTextBox.text()))
        elif self.paramType \
                == ModelerParameterDefinitionDialog.PARAMETER_EXTENT \
                or isinstance(self.param, ParameterExtent):
            self.param = ParameterExtent(name, description)
        elif self.paramType == \
                ModelerParameterDefinitionDialog.PARAMETER_FILE \
                or isinstance(self.param, ParameterFile):
            isFolder = self.fileFolderCombo.currentIndex() == 1
            self.param = ParameterFile(name, description, isFolder=isFolder)
        self.close()

    def cancelPressed(self):
        self.param = None
        self.close()
