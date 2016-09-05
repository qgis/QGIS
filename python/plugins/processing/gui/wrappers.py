# -*- coding: utf-8 -*-

"""
***************************************************************************
    BooleanWidget.py
    ---------------------
    Date                 : May 2016
    Copyright            : (C) 2016 by Arnaud Morvan
    Email                : arnaud dot morvan at camptocamp dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Arnaud Morvan'
__date__ = 'May 2016'
__copyright__ = '(C) 2016, Arnaud Morvan'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


from inspect import isclass

from qgis.core import QgsCoordinateReferenceSystem
from qgis.PyQt.QtWidgets import (
    QCheckBox,
    QComboBox,
    )

from processing.gui.CrsSelectionPanel import CrsSelectionPanel

DIALOG_STANDARD = 'standard'
DIALOG_BATCH = 'batch'
DIALOG_MODELER = 'modeler'


def wrapper_from_param(param, dialog=DIALOG_STANDARD):
    wrapper = param.metadata.get('widget_wrapper', None)
    # wrapper metadata should be a class path
    if isinstance(wrapper, basestring):
        tokens = wrapper.split('.')
        mod = __import__('.'.join(tokens[:-1]), fromlist=[tokens[-1]])
        wrapper = getattr(mod, tokens[-1])
    # or directly a class object
    if isclass(wrapper):
        wrapper = wrapper(param=param, dialog=dialog)
    # or a wrapper instance
    return wrapper


class NotYetImplementedWidgetWrapper():
    """Temporary substitute class for not yet implemented wrappers"""

    implemented = False

    def __init__(self, param, widget):
        self.param = param
        self.widget = widget


class WidgetWrapper():

    implemented = True  # TODO: Should be removed at the end

    def __init__(self, param, dialog=DIALOG_STANDARD):
        self.param = param
        self.dialog = dialog
        self.widget = self.createWidget()
        self.setValue(param.default)

    def comboValue(self):
        return self.widget.itemData(self.widget.currentIndex())

    def createWidget(self):
        pass

    def setValue(self, value):
        pass

    def setComboValue(self, value):
        values = [self.widget.itemData(i) for i in range(self.widget.count())]
        try:
            idx = values.index(value)
            self.widget.setCurrentIndex(idx)
        except ValueError:
            pass

    def value(self):
        pass


class BooleanWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        if self.dialog == DIALOG_STANDARD:
            return QCheckBox()

        if self.dialog in (DIALOG_BATCH, DIALOG_MODELER):
            widget = QComboBox()
            widget.addItem(widget.tr('Yes'), True)
            widget.addItem(widget.tr('No'), False)
            return widget

    def setValue(self, value):
        if self.dialog == DIALOG_STANDARD:
            self.widget.setChecked(value)

        if self.dialog in (DIALOG_BATCH, DIALOG_MODELER):
            self.setComboValue(value)

    def value(self):
        if self.dialog == DIALOG_STANDARD:
            return self.widget.isChecked()

        if self.dialog in (DIALOG_BATCH, DIALOG_MODELER):
            return self.comboValue()


class CrsWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        return CrsSelectionPanel()

    def setValue(self, value):
        if isinstance(value, basestring):  # authId
            self.widget.crs = value
        else:
            self.widget.crs = QgsCoordinateReferenceSystem(value).authid()
        self.widget.updateText()

    def value(self):
        return self.widget.getValue()
