"""
/***************************************************************************
Name                 : GdalTools
Description          : Integrate gdal tools into qgis
Date                 : 17/Sep/09
copyright            : (C) 2009 by Lorenzo Masini and Giuseppe Sucameli (Faunalia)
email                : lorenxo86@gmail.com - brush.tyler@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 This script initializes the plugin, making it known to QGIS.
"""


def classFactory(iface):
    # load GdalTools class from file GdalTools
    from .GdalTools import GdalTools
    return GdalTools(iface)
