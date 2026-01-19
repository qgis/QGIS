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

__author__ = "Victor Olaya"
__date__ = "August 2012"
__copyright__ = "(C) 2012, Victor Olaya"

import math

from qgis.PyQt.QtCore import Qt, QByteArray, QCoreApplication
from qgis.PyQt.QtWidgets import (
    QDialog,
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
    QHBoxLayout,
)
from qgis.PyQt.QtGui import QColor

from qgis.gui import (
    QgsProcessingLayerOutputDestinationWidget,
    QgsColorButton,
    QgsProcessingModelConfigWidget,
    QgsGui,
)
from qgis.core import (
    QgsApplication,
    QgsSettings,
    QgsProcessing,
    QgsProcessingParameterDefinition,
    QgsProcessingDestinationParameter,
    QgsProcessingParameterFeatureSink,
    QgsProcessingParameterFileDestination,
    QgsProcessingParameterFolderDestination,
    QgsProcessingParameterRasterDestination,
    QgsProcessingParameterVectorDestination,
    QgsProcessingModelAlgorithm,
)

from processing.core import parameters
from processing.modeler.exceptions import UndefinedParameterException


class ModelerParameterDefinitionWidget(QgsProcessingModelConfigWidget):

    def __init__(self, alg, paramType=None, param=None):
        self.alg = alg
        self.paramType = paramType
        self.param = param.clone() if param else None
        QgsProcessingModelConfigWidget.__init__(self)
        self.setupUi(self.param, self.paramType)

    def switchToCommentTab(self):
        self.tab.setCurrentIndex(1)
        self.commentEdit.setFocus()
        self.commentEdit.selectAll()

    def setupUi(self, existing_param, paramType):
        type_metadata = QgsApplication.processingRegistry().parameterType(
            existing_param.type() if existing_param else paramType
        )
        self.setPanelTitle(
            self.tr("{} Parameter Definition").format(type_metadata.name())
        )

        self.mainLayout = QVBoxLayout()
        self.mainLayout.setContentsMargins(0, 0, 0, 0)
        self.tab = QTabWidget()
        self.mainLayout.addWidget(self.tab)

        self.verticalLayout = QVBoxLayout()

        self.label = QLabel(self.tr("Parameter name"))
        self.verticalLayout.addWidget(self.label)
        self.nameTextBox = QLineEdit()
        self.verticalLayout.addWidget(self.nameTextBox)

        if isinstance(existing_param, QgsProcessingParameterDefinition):
            self.nameTextBox.setText(existing_param.description())

        self.nameTextBox.textChanged.connect(self.widgetChanged)

        if isinstance(existing_param, QgsProcessingDestinationParameter):
            self.verticalLayout.addWidget(QLabel(self.tr("Default value")))
            self.defaultWidget = QgsProcessingLayerOutputDestinationWidget(
                existing_param, defaultSelection=True
            )
            self.verticalLayout.addWidget(self.defaultWidget)
            # TODO check
            self.defaultWidget.destinationChanged.connect(self.widgetChanged)

        self.verticalLayout.addSpacing(20)
        self.requiredCheck = QCheckBox()
        self.requiredCheck.setText(self.tr("Mandatory"))
        self.requiredCheck.setChecked(True)
        if existing_param is not None:
            self.requiredCheck.setChecked(
                not existing_param.flags()
                & QgsProcessingParameterDefinition.Flag.FlagOptional
            )
        self.verticalLayout.addWidget(self.requiredCheck)
        self.requiredCheck.toggled.connect(self.widgetChanged)

        self.advancedCheck = QCheckBox()
        self.advancedCheck.setText(self.tr("Advanced"))
        self.advancedCheck.setChecked(False)
        if existing_param is not None:
            self.advancedCheck.setChecked(
                existing_param.flags()
                & QgsProcessingParameterDefinition.Flag.FlagAdvanced
            )
        self.verticalLayout.addWidget(self.advancedCheck)
        self.advancedCheck.toggled.connect(self.widgetChanged)

        # If child algorithm output is mandatory, disable checkbox
        if isinstance(existing_param, QgsProcessingDestinationParameter):
            child = self.alg.childAlgorithms()[
                existing_param.metadata()["_modelChildId"]
            ]
            model_output = child.modelOutput(
                existing_param.metadata()["_modelChildOutputName"]
            )
            param_def = child.algorithm().parameterDefinition(
                model_output.childOutputName()
            )
            if not (
                param_def.flags() & QgsProcessingParameterDefinition.Flag.FlagOptional
            ):
                self.requiredCheck.setEnabled(False)
                self.requiredCheck.setChecked(True)

            self.advancedCheck.setEnabled(False)
            self.advancedCheck.setChecked(False)

        self.verticalLayout.addStretch()

        w = QWidget()
        w.setLayout(self.verticalLayout)
        self.tab.addTab(w, self.tr("Properties"))

        self.commentLayout = QVBoxLayout()
        self.commentEdit = QTextEdit()
        self.commentEdit.setAcceptRichText(False)
        self.commentLayout.addWidget(self.commentEdit, 1)
        self.commentEdit.textChanged.connect(self.widgetChanged)

        hl = QHBoxLayout()
        hl.setContentsMargins(0, 0, 0, 0)
        hl.addWidget(QLabel(self.tr("Color")))
        self.comment_color_button = QgsColorButton()
        self.comment_color_button.setAllowOpacity(True)
        self.comment_color_button.setWindowTitle(self.tr("Comment Color"))
        self.comment_color_button.setShowNull(True, self.tr("Default"))
        hl.addWidget(self.comment_color_button)
        self.commentLayout.addLayout(hl)
        self.comment_color_button.colorChanged.connect(self.widgetChanged)

        w2 = QWidget()
        w2.setLayout(self.commentLayout)
        self.tab.addTab(w2, self.tr("Comments"))

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
        return (
            self.comment_color_button.color()
            if not self.comment_color_button.isNull()
            else QColor()
        )

    def create_parameter(self):
        description = self.nameTextBox.text()

        safeName = QgsProcessingModelAlgorithm.safeName(description)
        name = safeName.lower()

        # Destination parameter
        new_param = None
        if isinstance(self.param, QgsProcessingParameterFeatureSink):
            new_param = QgsProcessingParameterFeatureSink(
                name=name,
                description=description,
                type=self.param.dataType(),
                defaultValue=self.defaultWidget.value(),
            )
        elif isinstance(self.param, QgsProcessingParameterFileDestination):
            new_param = QgsProcessingParameterFileDestination(
                name=name,
                description=description,
                fileFilter=self.param.fileFilter(),
                defaultValue=self.defaultWidget.value(),
            )
        elif isinstance(self.param, QgsProcessingParameterFolderDestination):
            new_param = QgsProcessingParameterFolderDestination(
                name=name,
                description=description,
                defaultValue=self.defaultWidget.value(),
            )
        elif isinstance(self.param, QgsProcessingParameterRasterDestination):
            new_param = QgsProcessingParameterRasterDestination(
                name=name,
                description=description,
                defaultValue=self.defaultWidget.value(),
            )
        elif isinstance(self.param, QgsProcessingParameterVectorDestination):
            new_param = QgsProcessingParameterVectorDestination(
                name=name,
                description=description,
                type=self.param.dataType(),
                defaultValue=self.defaultWidget.value(),
            )

        else:
            if self.paramType:
                typeId = self.paramType
            else:
                typeId = self.param.type()

            paramTypeDef = (
                QgsApplication.instance().processingRegistry().parameterType(typeId)
            )
            if not paramTypeDef:
                msg = self.tr(
                    "The parameter `{}` is not registered, are you missing a required plugin?"
                ).format(typeId)
                raise UndefinedParameterException(msg)
            new_param = paramTypeDef.create(name)
            new_param.setDescription(description)
            new_param.setMetadata(paramTypeDef.metadata())

        if not self.requiredCheck.isChecked():
            new_param.setFlags(
                new_param.flags() | QgsProcessingParameterDefinition.Flag.FlagOptional
            )
        else:
            new_param.setFlags(
                new_param.flags() & ~QgsProcessingParameterDefinition.Flag.FlagOptional
            )

        if self.advancedCheck.isChecked():
            new_param.setFlags(
                new_param.flags() | QgsProcessingParameterDefinition.Flag.FlagAdvanced
            )
        else:
            new_param.setFlags(
                new_param.flags() & ~QgsProcessingParameterDefinition.Flag.FlagAdvanced
            )

        return new_param


class ModelerParameterDefinitionDialog(QDialog):

    @staticmethod
    def use_legacy_dialog(param=None, paramType=None):
        if isinstance(param, QgsProcessingDestinationParameter):
            return True

        # yay, use new API!
        return False

    def __init__(self, alg, paramType=None, param=None):
        QDialog.__init__(self)

        self.widget = ModelerParameterDefinitionWidget(alg, paramType, param)
        self.setObjectName("ModelerParameterDefinitionDialog")
        self.setModal(True)
        self.setupUi()

        QgsGui.enableAutoGeometryRestore(self)

    def switchToCommentTab(self):
        self.widget.switchToCommentTab()

    def setupUi(self):
        self.mainLayout = QVBoxLayout()
        self.mainLayout.addWidget(self.widget, 1)

        self.setMinimumWidth(300)

        self.buttonBox = QDialogButtonBox(self)
        self.buttonBox.setOrientation(Qt.Orientation.Horizontal)
        self.buttonBox.setStandardButtons(
            QDialogButtonBox.StandardButton.Cancel | QDialogButtonBox.StandardButton.Ok
        )
        self.buttonBox.setObjectName("buttonBox")
        self.buttonBox.accepted.connect(self.accept)
        self.buttonBox.rejected.connect(self.reject)

        self.mainLayout.addWidget(self.buttonBox)

        self.setLayout(self.mainLayout)

        self.widget.panelAccepted.connect(self.reject)

    def setComments(self, text):
        self.widget.setComments(text)

    def comments(self):
        return self.widget.comments()

    def setCommentColor(self, color):
        self.widget.setCommentColor(color)

    def commentColor(self):
        return self.widget.commentColor()

    def create_parameter(self):
        return self.widget.create_parameter()

    def accept(self):
        description = self.widget.nameTextBox.text()
        if description.strip() == "":
            QMessageBox.warning(
                self,
                self.tr("Unable to define parameter"),
                self.tr("Invalid parameter name"),
            )
            return

        super().accept()
