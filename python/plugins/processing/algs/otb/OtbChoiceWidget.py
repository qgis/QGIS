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
                       QgsProcessingOutputString,
                       QgsProcessingParameterDefinition)
from qgis.gui import QgsProcessingModelerParameterWidget
from processing.gui.wrappers import (WidgetWrapper,
                                     DIALOG_STANDARD,
                                     DIALOG_BATCH,
                                     DIALOG_MODELER)

# TODO: QGIS 3.8 move this class to processing/gui/
# OtbChoiceWidget is a crucial parameter type in otb provider
# It is the one that can handles required parameters are run-time depending on user input!.
# This idea is indeed different from static list of required/optional parameters from a descriptor file.
# So this class (if treated as first class citizen in processing/gui) have potential
# to be reused in existing or future processing providers.


class OtbChoiceWidgetWrapper(WidgetWrapper):

    def __init__(self, param, dialog, row=0, col=0, **kwargs):
        self.flagsModified = {}
        super().__init__(param, dialog, row=0, col=0, **kwargs)
        self.updateAllParameters(None)

    def createWidget(self):
        widget = QComboBox()
        widget.addItems(self.param.options)
        if self.dialogType in(DIALOG_MODELER, DIALOG_STANDARD):
            widget.currentIndexChanged.connect(self.updateAllParameters)
        return widget

    def get_algorithm(self):
        if self.dialogType == DIALOG_MODELER:
            return self.dialog._alg
        else:
            return self.dialog.algorithm()

    def __updateWrapper(self, name, visible):
        if self.dialogType == DIALOG_BATCH:
            return
        elif self.dialogType == DIALOG_STANDARD:
            if self.dialog.mainWidget() is None:
                return
            if name in self.dialog.mainWidget().wrappers:
                self.__setWrapperVisibility(self.dialog.mainWidget().wrappers[name], visible)

        #Fur Qgis modeler
        else:
            if name in self.dialog.wrappers:
                self.__setWrapperVisibility(self.dialog.wrappers[name], visible)
            if name in self.dialog.widget_labels:
                self.dialog.widget_labels[name].setVisible(visible)

    def __setWrapperVisibility(self, wrapper, v):
        # For compatibility with 3.x API, we need to check whether the wrapper is
        # the deprecated WidgetWrapper class. If not, it's the newer
        # QgsAbstractProcessingParameterWidgetWrapper class
        # TODO QGIS 4.0 - remove is_python_wrapper logic
        if issubclass(wrapper.__class__, WidgetWrapper):
            wrapper.widget.setVisible(v)
            if wrapper.label:
                wrapper.label.setVisible(v)
        elif issubclass(wrapper.__class__, QgsProcessingModelerParameterWidget):
            wrapper.setVisible(v)
        else:
            wrapper.wrappedWidget().setVisible(v)
            if wrapper.wrappedLabel():
                wrapper.wrappedLabel().setVisible(v)

    def updateAllParameters(self, current_value):
        for parameter in self.get_algorithm().parameterDefinitions():
            if not 'group_key' in parameter.metadata() or parameter.metadata()['group_key'] != self.param.name():
                continue
            name = parameter.name()
            choice_key = parameter.metadata()['group_key']
            if choice_key and choice_key + "." in name:
                choice_param = self.get_algorithm().parameterDefinition(choice_key)
                if current_value is None:
                    current_value = choice_param.defaultValue()
                pattern = "{}.{}.".format(choice_key, choice_param.getValueAsText(current_value))
                if not pattern in name:
                    flags = self.get_algorithm().parameterDefinition(name).flags()
                    if not flags & QgsProcessingParameterDefinition.FlagOptional:
                        self.flagsModified[name] = True
                    self.get_algorithm().parameterDefinition(name).setFlags(QgsProcessingParameterDefinition.FlagOptional)
                    self.__updateWrapper(name, False)
                else:
                    if name in self.flagsModified.keys():
                        self.get_algorithm().parameterDefinition(name).setFlags(QgsProcessingParameterDefinition.FlagAdvanced)
                    self.__updateWrapper(name, True)

    def setValue(self, value):
        if value in self.param.options:
            index = self.param.options.index(value)
        else:
            index = int(value)
        self.widget.setCurrentIndex(index)

    def value(self):
        return self.widget.currentText()

    def postInitialize(self, wrappers):
        # if self.dialogType == DIALOG_BATCH:
        #     return
        self.updateAllParameters(current_value=None)
        for parameter in self.get_algorithm().parameterDefinitions():
            if not 'group_key' in parameter.metadata() or parameter.metadata()['group_key'] != self.param.name():
                continue
            for wrapper in wrappers:
                if wrapper.param.name() == parameter.name():
                    v = self.value() == parameter.metadata()['group_value']
                    self.__setWrapperVisibility(wrapper, v)


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

    def getValueAsText(self, value):
        if not value in self.options:
            value = self.options[int(value)]
        return value

    def setValue(self, value):
        if value is None:
            self.value = 0
        else:
            self.value = int(value)
            return True

    def type(self):
        #This value is written by otbQgisDescriptor.
        return 'OTBParameterChoice'
