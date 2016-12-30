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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.gui import QgsRasterFormatSaveOptionsWidget

from processing.gui.wrappers import WidgetWrapper


class RasterOptionsWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        return QgsRasterFormatSaveOptionsWidget()

    def setValue(self, value):
        if value is None:
            value = ''
        self.widget.setValue(value)

    def value(self):
        return ' '.join(self.widget.options())
