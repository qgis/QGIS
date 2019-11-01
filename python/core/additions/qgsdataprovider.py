# -*- coding: utf-8 -*-

"""
***************************************************************************
    qgsdataprovider.py
    ---------------------
    Date                 : November 2019
    Copyright            : (C) 2019 by Matthias Kuhn
    Email                : matthias@opengis.ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

from qgis.core import QgsDataProvider

# Monkey patch static const "QgsDataProvider.SUBLAYER_SEPARATOR" which was removed for QGIS 3.12
QgsDataProvider.SUBLAYER_SEPARATOR = QgsDataProvider.sublayerSeparator()
