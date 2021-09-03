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

from qgis.core import (Qgis,
                       QgsFeature,
                       QgsGeometry,
                       QgsFeatureRequest,
                       QgsFeatureSink)


def buffering(feedback, context, sink, distance, field, useField, source, dissolve, segments, endCapStyle=1,
              joinStyle=1, miterLimit=2):
    if useField:
        field = source.fields().lookupField(field)

    outFeat = QgsFeature()

    current = 0
    total = 100.0 / source.featureCount() if source.featureCount() else 0

    # With dissolve
    if dissolve:
        attributes_to_fetch = []
        if useField:
            attributes_to_fetch.append(field)

        features = source.getFeatures(QgsFeatureRequest().setSubsetOfAttributes(attributes_to_fetch))
        buffered_geometries = []
        for inFeat in features:
            if feedback.isCanceled():
                break

            attrs = inFeat.attributes()
            if useField:
                value = attrs[field]
            else:
                value = distance

            inGeom = inFeat.geometry()

            buffered_geometries.append(inGeom.buffer(float(value), segments, Qgis.EndCapStyle(endCapStyle), Qgis.JoinStyle(joinStyle), miterLimit))

            current += 1
            feedback.setProgress(int(current * total))

        final_geometry = QgsGeometry.unaryUnion(buffered_geometries)
        outFeat.setGeometry(final_geometry)
        outFeat.setAttributes(attrs)
        sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
    else:

        features = source.getFeatures()

        # Without dissolve
        for inFeat in features:
            if feedback.isCanceled():
                break
            attrs = inFeat.attributes()
            if useField:
                value = attrs[field]
            else:
                value = distance
            inGeom = inFeat.geometry()
            outFeat = QgsFeature()
            outGeom = inGeom.buffer(float(value), segments, Qgis.EndCapStyle(endCapStyle), Qgis.JoinStyle(joinStyle), miterLimit)
            outFeat.setGeometry(outGeom)
            outFeat.setAttributes(attrs)
            sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
            current += 1
            feedback.setProgress(int(current * total))
