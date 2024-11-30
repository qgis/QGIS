"""
***************************************************************************
    qgsfeature.py
    ---------------------
    Date                 : May 2018
    Copyright            : (C) 2018 by Denis Rouzaud
    Email                : denis@opengis.ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

from PyQt5.QtCore import QVariant


def _mapping_feature(feature):
    geom = feature.geometry()
    properties = {
        k: None if (v is None or (isinstance(v, QVariant) and v.isNull())) else v
        for k, v in feature.attributeMap().items()
    }
    return {
        "type": "Feature",
        "properties": properties,
        "geometry": geom.__geo_interface__,
    }
