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

import math

from qgis.PyQt.QtCore import (Qt,
                              QByteArray,
                              QCoreApplication)
from qgis.PyQt.QtWidgets import (QDialog,
                                 QVBoxLayout,
                                 QLabel,
                                 QLineEdit,
                                 QComboBox,
                                 QCheckBox,
                                 QDialogButtonBox,
                                 QMessageBox,
                                 QTabWidget,
                                 QWidget,
                                 QTextEdit,
                                 QHBoxLayout)
from qgis.PyQt.QtGui import QColor

from qgis.gui import (QgsProcessingLayerOutputDestinationWidget,
                      QgsColorButton)
from qgis.core import (QgsApplication,
                       QgsSettings,
                       QgsProcessing,
                       QgsProcessingParameterDefinition,
                       QgsProcessingDestinationParameter,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterFileDestination,
                       QgsProcessingParameterFolderDestination,
                       QgsProcessingParameterRasterDestination,
                       QgsProcessingParameterVectorDestination)

from processing.core import parameters
from processing.modeler.exceptions import UndefinedParameterException


class ModelerParameterDefinitionDialog(QDialog):

    @staticmethod
    def use_legacy_dialog(param=None, paramType=None):
        if isinstance(param, QgsProcessingDestinationParameter):
            return True

        # yay, use new API!
        return False

    def __init__(self, alg, paramType=None, param=None):
        self.alg = alg
        self.paramType = paramType
        self.param = param
        QDialog.__init__(self)
        self.setModal(True)
        self.setupUi()
        settings = QgsSettings()
        self.restoreGeometry(settings.value("/Processing/modelParametersDefinitionDialogGeometry", QByteArray()))

    def closeEvent(self, event):
        settings = QgsSettings()
        settings.setValue("/Processing/modelParametersDefinitionDialogGeometry", self.saveGeometry())
        super(ModelerParameterDefinitionDialog, self).closeEvent(event)

    def switchToCommentTab(self):
        self.tab.setCurrentIndex(1)
        self.commentEdit.setFocus()
        self.commentEdit.selectAll()

    def setupUi(self):
        type_metadata = QgsApplication.processingRegistry().parameterType(self.param.type() if self.param else self.paramType)
        self.setWindowTitle(self.tr('{} Parameter Definition').format(type_metadata.name()))

        self.mainLayout = QVBoxLayout()
        self.tab = QTabWidget()
        self.mainLayout.addWidget(self.tab)

        self.setMinimumWidth(300)

        self.verticalLayout = QVBoxLayout()

        self.label = QLabel(self.tr('Parameter name'))
        self.verticalLayout.addWidget(self.label)
        self.nameTextBox = QLineEdit()
        self.verticalLayout.addWidget(self.nameTextBox)

        if isinstance(self.param, QgsProcessingParameterDefinition):
            self.nameTextBox.setText(self.param.description())

        if isinstance(self.param, QgsProcessingDestinationParameter):
            self.verticalLayout.addWidget(QLabel(self.tr('Default value')))
            self.defaultWidget = QgsProcessingLayerOutputDestinationWidget(self.param, defaultSelection=True)
            self.verticalLayout.addWidget(self.defaultWidget)

        self.verticalLayout.addSpacing(20)
        self.requiredCheck = QCheckBox()
        self.requiredCheck.setText(self.tr('Mandatory'))
        self.requiredCheck.setChecked(True)
        if self.param is not None:
            self.requiredCheck.setChecked(not self.param.flags() & QgsProcessingParameterDefinition.FlagOptional)
        self.verticalLayout.addWidget(self.requiredCheck)

        self.advancedCheck = QCheckBox()
        self.advancedCheck.setText(self.tr('Advanced'))
        self.advancedCheck.setChecked(False)
        if self.param is not None:
            self.advancedCheck.setChecked(self.param.flags() & QgsProcessingParameterDefinition.FlagAdvanced)
        self.verticalLayout.addWidget(self.advancedCheck)

        # If child algorithm output is mandatory, disable checkbox
        if isinstance(self.param, QgsProcessingDestinationParameter):
            child = self.alg.childAlgorithms()[self.param.metadata()['_modelChildId']]
            model_output = child.modelOutput(self.param.metadata()['_modelChildOutputName'])
            param_def = child.algorithm().parameterDefinition(model_output.childOutputName())
            if not (param_def.flags() & QgsProcessingParameterDefinition.FlagOptional):
                self.requiredCheck.setEnabled(False)
                self.requiredCheck.setChecked(True)

            self.advancedCheck.setEnabled(False)
            self.advancedCheck.setChecked(False)

        self.verticalLayout.addStretch()

        w = QWidget()
        w.setLayout(self.verticalLayout)
        self.tab.addTab(w, self.tr('Properties'))

        self.commentLayout = QVBoxLayout()
        self.commentEdit = QTextEdit()
        self.commentEdit.setAcceptRichText(False)
        self.commentLayout.addWidget(self.commentEdit, 1)

        hl = QHBoxLayout()
        hl.setContentsMargins(0, 0, 0, 0)
        hl.addWidget(QLabel(self.tr('Color')))
        self.comment_color_button = QgsColorButton()
        self.comment_color_button.setAllowOpacity(True)
        self.comment_color_button.setWindowTitle(self.tr('Comment Color'))
        self.comment_color_button.setShowNull(True, self.tr('Default'))
        hl.addWidget(self.comment_color_button)
        self.commentLayout.addLayout(hl)

        w2 = QWidget()
        w2.setLayout(self.commentLayout)
        self.tab.addTab(w2, self.tr('Comments'))

        self.buttonBox = QDialogButtonBox(self)
        self.buttonBox.setOrientation(Qt.Horizontal)
        self.buttonBox.setStandardButtons(QDialogButtonBox.Cancel |
                                          QDialogButtonBox.Ok)
        self.buttonBox.setObjectName('buttonBox')
        self.buttonBox.accepted.connect(self.accept)
        self.buttonBox.rejected.connect(self.reject)

        self.mainLayout.addWidget(self.buttonBox)

        self.setLayout(self.mainLayout)

    def setComments(self, text):
        self.commentEdit.setPlainText(text)

    def comments(self):
        return self.commentEdit.toPlainText()

    def setCommentColor(self, color):
        if color.isValid():
            self.comment_color_button.setColor(color)
        else:
            self.comment_color_button.setToNull()

    def commentColor(self):
        return self.comment_color_button.color() if not self.comment_color_button.isNull() else QColor()

    def accept(self):
        description = self.nameTextBox.text()
        if description.strip() == '':
            QMessageBox.warning(self, self.tr('Unable to define parameter'),
                                self.tr('Invalid parameter name'))
            return

        validChars = \
            'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'
        safeName = ''.join(c for c in description if c in validChars)
        name = safeName.lower()

        # Destination parameter
        if (isinstance(self.param, QgsProcessingParameterFeatureSink)):
            self.param = QgsProcessingParameterFeatureSink(
                name=name,
                description=description,
                type=self.param.dataType(),
                defaultValue=self.defaultWidget.value())
        elif (isinstance(self.param, QgsProcessingParameterFileDestination)):
            self.param = QgsProcessingParameterFileDestination(
                name=name,
                description=description,
                fileFilter=self.param.fileFilter(),
                defaultValue=self.defaultWidget.value())
        elif (isinstance(self.param, QgsProcessingParameterFolderDestination)):
            self.param = QgsProcessingParameterFolderDestination(
                name=name,
                description=description,
                defaultValue=self.defaultWidget.value())
        elif (isinstance(self.param, QgsProcessingParameterRasterDestination)):
            self.param = QgsProcessingParameterRasterDestination(
                name=name,
                description=description,
                defaultValue=self.defaultWidget.value())
        elif (isinstance(self.param, QgsProcessingParameterVectorDestination)):
            self.param = QgsProcessingParameterVectorDestination(
                name=name,
                description=description,
                type=self.param.dataType(),
                defaultValue=self.defaultWidget.value())

        else:
            if self.paramType:
                typeId = self.paramType
            else:
                typeId = self.param.type()

            paramTypeDef = QgsApplication.instance().processingRegistry().parameterType(typeId)
            if not paramTypeDef:
                msg = self.tr('The parameter `{}` is not registered, are you missing a required plugin?'.format(typeId))
                raise UndefinedParameterException(msg)
            self.param = paramTypeDef.create(name)
            self.param.setDescription(description)
            self.param.setMetadata(paramTypeDef.metadata())

        if not self.requiredCheck.isChecked():
            self.param.setFlags(self.param.flags() | QgsProcessingParameterDefinition.FlagOptional)
        else:
            self.param.setFlags(self.param.flags() & ~QgsProcessingParameterDefinition.FlagOptional)

        if self.advancedCheck.isChecked():
            self.param.setFlags(self.param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        else:
            self.param.setFlags(self.param.flags() & ~QgsProcessingParameterDefinition.FlagAdvanced)

        settings = QgsSettings()
        settings.setValue("/Processing/modelParametersDefinitionDialogGeometry", self.saveGeometry())

        QDialog.accept(self)

    def reject(self):
        self.param = None

        settings = QgsSettings()
        settings.setValue("/Processing/modelParametersDefinitionDialogGeometry", self.saveGeometry())

        QDialog.reject(self)
