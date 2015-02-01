# -*- coding: utf-8 -*-

"""
***************************************************************************
    Buffer.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
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
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import QgsFeature, QgsGeometry
from processing.tools import vector


def buffering(progress, writer, distance, field, useField, layer, dissolve,
              segments):

    if useField:
        field = layer.fieldNameIndex(field)

    outFeat = QgsFeature()
    inFeat = QgsFeature()
    inGeom = QgsGeometry()
    outGeom = QgsGeometry()

    current = 0
    features = vector.features(layer)
    total = 100.0 / float(len(features))

    # With dissolve
    if dissolve:
        first = True
        for inFeat in features:
            attrs = inFeat.attributes()
            if useField:
                value = attrs[field]
            else:
                value = distance

            inGeom = QgsGeometry(inFeat.geometry())
            outGeom = inGeom.buffer(float(value), segments)
            if first:
                tempGeom = QgsGeometry(outGeom)
                first = False
            else:
                tempGeom = tempGeom.combine(outGeom)

            current += 1
            progress.setPercentage(int(current * total))

        outFeat.setGeometry(tempGeom)
        outFeat.setAttributes(attrs)
        writer.addFeature(outFeat)
    else:
        # Without dissolve
        for inFeat in features:
            attrs = inFeat.attributes()
            if useField:
                value = attrs[field]
            else:
                value = distance
            inGeom = QgsGeometry(inFeat.geometry())
            outGeom = inGeom.buffer(float(value), segments)
            outFeat.setGeometry(outGeom)
            outFeat.setAttributes(attrs)
            writer.addFeature(outFeat)
            current += 1
            progress.setPercentage(int(current * total))

    del writer
