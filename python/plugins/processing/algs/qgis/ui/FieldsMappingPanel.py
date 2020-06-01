# -*- coding: utf-8 -*-

"""
***************************************************************************
    FieldsMappingWidget.py
    ---------------------
    Date                 : October 2014
    Copyright            : (C) 2014 by Arnaud Morvan
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
__date__ = 'October 2014'
__copyright__ = '(C) 2014, Arnaud Morvan'

import os

from qgis.PyQt import uic
from qgis.PyQt.QtCore import (
    QItemSelectionModel,
    QModelIndex,
    pyqtSlot,
    QCoreApplication,
    QVariant,
)

from qgis.PyQt.QtWidgets import (
    QComboBox,
    QSpacerItem,
    QMessageBox,
    QWidget,
    QVBoxLayout
)

from qgis.core import (
    QgsApplication,
    QgsMapLayerProxyModel,
    QgsProcessingFeatureSourceDefinition,
    QgsProcessingUtils,
    QgsVectorLayer,
    QgsField,
    QgsFields,
)

from processing.gui.wrappers import WidgetWrapper, DIALOG_STANDARD, DIALOG_MODELER
from processing.tools import dataobjects


class FieldsMappingWidgetWrapper(WidgetWrapper):

    def __init__(self, *args, **kwargs):
        super(FieldsMappingWidgetWrapper, self).__init__(*args, **kwargs)
        self._layer = None
