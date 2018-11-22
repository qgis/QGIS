# -*- coding: utf-8 -*-

"""
***************************************************************************
    wrappers_map_theme.py - Map theme widget wrappers
    ---------------------
    Date                 : August 2017
    Copyright            : (C) 2017 by OPENGIS.ch
    Email                : mario@opengis.ch
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


class MapThemeWrapper(BasicWidgetWrapper):
    """
    WidgetWrapper for ParameterString that createe a combobox widget
    with the existing map themes.
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
        return QgsProject.instance().mapThemeCollection().mapThemes()

    def setValue(self, value):
        self.setComboValue(value, self._combo)

    def value(self):
        return self.comboValue(combobox=self._combo)
