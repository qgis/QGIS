# -*- coding: utf-8 -*-
"""
***************************************************************************
    OtbChoiceWidget.py
    ------------------
    Date                 : June 2017
    Copyright            : (C) 2017 by CS Systemes d'Information (CS SI)
    Email                : rashad dot kanavath at c-s fr

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Rashad Kanavath'
__date__ = 'June 2017'
__copyright__ = "(C) 2017,2018 by CS Systemes d'information (CS SI)"

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.PyQt.QtWidgets import QComboBox
from qgis.core import (QgsProcessingParameterString,
                       QgsProcessingOutputString)
from processing.gui.wrappers import (WidgetWrapper,
                                     DIALOG_STANDARD,
                                     DIALOG_BATCH,
                                     DIALOG_MODELER)


class OtbChoiceWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        widget = QComboBox()
        widget.addItems(self.param.options)
        if self.dialogType in(DIALOG_MODELER, DIALOG_STANDARD):
            widget.currentIndexChanged.connect(self.valueChanged)
        return widget

    def get_algorithm(self):
        if self.dialogType == DIALOG_MODELER:
            return self.dialog._alg
        else:
            return self.dialog.algorithm()

    def setWrapperVisible(self, name, visible):
        if self.dialogType == DIALOG_STANDARD:
            if self.dialog.mainWidget() is None:
                return
            # For compatibility with 3.x API, we need to check whether the wrapper is
            # the deprecated WidgetWrapper class. If not, it's the newer
            # QgsAbstractProcessingParameterWidgetWrapper class
            # TODO QGIS 4.0 - remove is_python_wrapper logic
            is_python_wrapper = issubclass(self.dialog.mainWidget().wrappers[name].__class__, WidgetWrapper)
            if is_python_wrapper:
                self.dialog.mainWidget().wrappers[name].widget.setVisible(visible)
                if self.dialog.mainWidget().wrappers[name].label:
                    self.dialog.mainWidget().wrappers[name].label.setVisible(visible)
            else:
                self.dialog.mainWidget().wrappers[name].wrappedWidget().setVisible(visible)
                if self.dialog.mainWidget().wrappers[name].wrappedLabel():
                    self.dialog.mainWidget().wrappers[name].wrappedLabel().setVisible(visible)
        else:
            # For compatibility with 3.x API, we need to check whether the wrapper is
            # the deprecated WidgetWrapper class. If not, it's the newer
            # QgsAbstractProcessingParameterWidgetWrapper class
            # TODO QGIS 4.0 - remove is_python_wrapper logic
            if name in self.dialog.wrappers:
                is_python_wrapper = issubclass(self.dialog.wrappers[name].__class__, WidgetWrapper)
                if is_python_wrapper:
                    self.dialog.wrappers[name].widget.setVisible(visible)
                    if self.dialog.wrappers[name].label:
                        self.dialog.wrappers[name].label.setVisible(visible)
                else:
                    self.dialog.wrappers[name].setVisible(visible)
                    if name in self.dialog.widget_labels:
                        self.dialog.widget_labels[name].setVisible(visible)

    def valueChanged(self, value):
        for parameter in self.get_algorithm().parameterDefinitions():
            if not 'group_key' in parameter.metadata() or parameter.metadata()['group_key'] != self.param.name():
                continue
            name = parameter.name()
            v = self.value() == parameter.metadata()['group_value']
            self.setWrapperVisible(name, v)

    def setValue(self, value):
        if value in self.param.options:
            index = self.param.options.index(value)
        else:
            index = int(value)
        self.widget.setCurrentIndex(index)

    def value(self):
        return self.widget.currentText()

    def postInitialize(self, wrappers):
        if self.dialogType == DIALOG_BATCH:
            return

        for parameter in self.get_algorithm().parameterDefinitions():
            if not 'group_key' in parameter.metadata() or parameter.metadata()['group_key'] != self.param.name():
                continue
            name = parameter.name()
            v = self.value() == parameter.metadata()['group_value']
            for wrapper in wrappers:
                # For compatibility with 3.x API, we need to check whether the wrapper is
                # the deprecated WidgetWrapper class. If not, it's the newer
                # QgsAbstractProcessingParameterWidgetWrapper class
                # TODO QGIS 4.0 - remove is_python_wrapper logic
                is_python_wrapper = issubclass(wrapper.__class__, WidgetWrapper)
                if wrapper.param.name() == name:
                    if is_python_wrapper:
                        wrapper.widget.setVisible(v)
                        if wrapper.label:
                            wrapper.label.setVisible(v)
                    else:
                        wrapper.wrappedWidget().setVisible(v)
                        if wrapper.wrappedLabel():
                            wrapper.wrappedLabel().setVisible(v)


from qgis.core import QgsProcessingParameterDefinition


class OtbParameterChoice(QgsProcessingParameterDefinition):

    def __init__(self, name='', description='', options=[], default=None, isSource=False,
                 multiple=False, optional=False):

        QgsProcessingParameterDefinition.__init__(self, name, description, default, optional)

        self.setMetadata({
            'widget_wrapper': {
                'class': 'processing.algs.otb.OtbChoiceWidget.OtbChoiceWidgetWrapper'}})
        self.options = options

        if default is not None:
            try:
                self.default = int(default)
            except:
                self.default = 0
            self.value = self.default

    def setValue(self, value):
        if value is None:
            self.value = 0
        else:
            self.value = int(value)
            return True

    def type(self):
        #This value is written by otbQgisDescriptor.
        return 'OTBParameterChoice'
