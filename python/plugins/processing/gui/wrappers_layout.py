# -*- coding: utf-8 -*-

"""
***************************************************************************
    wrappers_layout.py - Layout widget wrappers
    ---------------------
    Date                 : February 2019
    Copyright            : (C) 2019 by fpsampayo
    Email                : fpsampayo@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""


from qgis.core import QgsProject

from qgis.PyQt.QtWidgets import QComboBox

from processing.gui.wrappers import (
    BasicWidgetWrapper
)


class LayoutWrapper(BasicWidgetWrapper):
    """
    WidgetWrapper for ParameterString that create a combobox widget
    with the existing layouts.
    """

    def createWidget(self):
        self._combo = QComboBox()
        self._combo.addItem('', '')
        for item in self.items():
            self._combo.addItem(item, item)
        self._combo.currentIndexChanged.connect(lambda:
                                                self.widgetValueHasChanged.emit(self))
        return self._combo

    def items(self):
        layouts = QgsProject().instance().layoutManager().layouts()
        return [layout.name() for layout in layouts if layout.referenceMap() is not None]

    def setValue(self, value):
        self.setComboValue(value, self._combo)

    def value(self):
        return self.comboValue(combobox=self._combo)
