# -*- coding: utf-8 -*-

"""
***************************************************************************
    __init__.py
    ---------------------
    Date                 : May 2014
    Copyright            : (C) 2014 by Nathan Woodrow
    Email                : woodrow dot nathan at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nathan Woodrow'
__date__ = 'May 2014'
__copyright__ = '(C) 2014, Nathan Woodrow'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.PyQt import QtCore
from qgis._gui import *

# -----------------
# DO NOT EDIT BELOW
# These are automatically added by calling sipify.pl script
QgsAuthSettingsWidget.WarningType.parentClass = lambda: QgsAuthSettingsWidget
QgsColorButton.Behavior.parentClass = lambda: QgsColorButton
QgsColorTextWidget.ColorTextFormat.parentClass = lambda: QgsColorTextWidget
QgsFilterLineEdit.ClearMode.parentClass = lambda: QgsFilterLineEdit
QgsFloatingWidget.AnchorPoint.parentClass = lambda: QgsFloatingWidget
QgsFontButton.Mode.parentClass = lambda: QgsFontButton
QgsMapToolIdentify.IdentifyMode.parentClass = lambda: QgsMapToolIdentify
QgsAttributeTableFilterModel.FilterMode.parentClass = lambda: QgsAttributeTableFilterModel
QgsAttributeTableFilterModel.ColumnType.parentClass = lambda: QgsAttributeTableFilterModel
QgsDualView.ViewMode.parentClass = lambda: QgsDualView
