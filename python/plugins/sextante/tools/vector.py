# -*- coding: utf-8 -*-

"""
***************************************************************************
    vector.py
    ---------------------
    Date                 : February 2013
    Copyright            : (C) 2013 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'February 2013'
__copyright__ = '(C) 2013, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from sextante.core.QGisLayers import QGisLayers


def getAttributeValues(layer, *attributeNames):
    ret = {}
    for name in attributeNames:
        values = []
        features = QGisLayers.features(layer)
        index = layer.fieldNameIndex(name)
        if index == -1:
            raise ValueError('Wrong field name')
        for feature in features:
            try:
                v = float(feature.attributes()[index].toString())
                values.append(v)
            except:
                values.append(None)
        ret[name] = values;
    return ret

