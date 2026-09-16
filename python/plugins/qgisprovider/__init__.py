"""
***************************************************************************
    __init__.py
    ---------------------
    Date                 : August 2026
    Copyright            : (C) 2026 by Nyall Dawson
    Email                : nyall.dawson@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""


def classFactory(iface):
    from qgisprovider.qgis_plugin import QgisProviderPlugin

    return QgisProviderPlugin()
