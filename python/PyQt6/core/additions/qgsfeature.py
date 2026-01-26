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


def _mapping_feature(feature):
    geom = feature.geometry()
    fields = [field.name() for field in feature.fields()]
    properties = dict(list(zip(fields, feature.attributes())))
    return {
        "type": "Feature",
        "properties": properties,
        "geometry": geom.__geo_interface__,
    }
