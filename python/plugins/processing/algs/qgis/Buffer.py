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

from processing.core.ProcessingLog import ProcessingLog
from processing.tools import vector


def buffering(progress, writer, distance, field, useField, layer, dissolve,
              segments, endCapStyle=1, joinStyle=1, mitreLimit=2):

    if useField:
        field = layer.fieldNameIndex(field)

    outFeat = QgsFeature()

    current = 0
    features = vector.features(layer)
    total = 100.0 / float(len(features))

    # With dissolve
    if dissolve:
        first = True
        buffered_geometries = []
        for inFeat in features:
            attrs = inFeat.attributes()
            if useField:
                value = attrs[field]
            else:
                value = distance

            inGeom = inFeat.geometry()
            if not inGeom:
                ProcessingLog.addToLog(ProcessingLog.LOG_WARNING, 'Feature {} has empty geometry. Skipping...'.format(inFeat.id()))
                continue
            if not inGeom.isGeosValid():
                ProcessingLog.addToLog(ProcessingLog.LOG_WARNING, 'Feature {} has invalid geometry. Skipping...'.format(inFeat.id()))
                continue
            buffered_geometries.append(inGeom.buffer(float(value), segments, endCapStyle, joinStyle, mitreLimit))

            current += 1
            progress.setPercentage(int(current * total))

        final_geometry = QgsGeometry.unaryUnion(buffered_geometries)
        outFeat.setGeometry(final_geometry)
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
            inGeom = inFeat.geometry()
            outFeat = QgsFeature()
            if inGeom.isEmpty() or inGeom.isGeosEmpty():
                pass
            elif not inGeom.isGeosValid():
                ProcessingLog.addToLog(ProcessingLog.LOG_WARNING, 'Feature {} has invalid geometry. Skipping...'.format(inFeat.id()))
                continue
            else:
                outGeom = inGeom.buffer(float(value), segments, endCapStyle, joinStyle, mitreLimit)
                outFeat.setGeometry(outGeom)
            outFeat.setAttributes(attrs)
            writer.addFeature(outFeat)
            current += 1
            progress.setPercentage(int(current * total))

    del writer
