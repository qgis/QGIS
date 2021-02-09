# -*- coding: utf-8 -*-

"""
***************************************************************************
    RasterOptionsWidget.py
    ---------------------
    Date                 : December 2016
    Copyright            : (C) 2016 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'December 2016'
__copyright__ = '(C) 2016, Alexander Bruy'

from qgis.PyQt.QtWidgets import QLineEdit, QComboBox
from qgis.gui import QgsRasterFormatSaveOptionsWidget

from qgis.core import (QgsProcessingParameterString,
                       QgsProcessingOutputString)
from processing.gui.wrappers import WidgetWrapper, DIALOG_MODELER, DIALOG_BATCH


class RasterOptionsWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        if self.dialogType == DIALOG_MODELER:
            widget = QComboBox()
            widget.setEditable(True)
            strings = self.dialog.getAvailableValuesOfType(QgsProcessingParameterString, QgsProcessingOutputString)
            options = [(self.dialog.resolveValueDescription(s), s) for s in strings]
            for desc, val in options:
                widget.addItem(desc, val)
            widget.setEditText(self.parameterDefinition().defaultValue() or '')
            return widget
        elif self.dialogType == DIALOG_BATCH:
            widget = QLineEdit()
            if self.parameterDefinition().defaultValue():
                widget.setText(self.parameterDefinition().defaultValue())
            return widget
        else:
            return QgsRasterFormatSaveOptionsWidget()

    def setValue(self, value):
        if value is None:
            value = ''

        if self.dialogType == DIALOG_MODELER:
            self.setComboValue(value)
        elif self.dialogType == DIALOG_BATCH:
            self.widget.setText(value)
        else:
            # The QgsRasterFormatSaveOptionsWidget requires space delimited options, but this wrapper uses | delimiter
            self.widget.setOptions(value.replace('|', ' '))

    def value(self):
        if self.dialogType == DIALOG_MODELER:
            return self.comboValue()
        elif self.dialogType == DIALOG_BATCH:
            return self.widget.text()
        else:
            return '|'.join(self.widget.options())
