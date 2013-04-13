# -*- coding: utf-8 -*-

"""
***************************************************************************
    Summarize.py
    ---------------------
    Date                 : April 2013
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
__date__ = 'April 2013'
__copyright__ = '(C) 2013, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

##input=vector
##output=output vector
from sextante.core.SextanteVectorWriter import SextanteVectorWriter
from qgis.core import *
from PyQt4.QtCore import *

inputLayer = sextante.getobject(input)
features = sextante.getfeatures(inputLayer)
fields = inputLayer.pendingFields().toList()
outputLayer = SextanteVectorWriter(output, None, fields, QGis.WKBPoint, inputLayer.crs())
count = 0
mean = [0 for field in fields]
x = 0
y = 0
for ft in features:
    c = ft.geometry().centroid().asPoint()
    x += c.x()
    y += c.y()
    attrs = ft.attributes()
    for f in range(len(fields)):
        try:
            mean[f] += float(attrs[f].toDouble()[0])
        except:
            pass
    count += 1
if count != 0:
    mean = [value / count for value in mean]
    x /= count
    y /= count
outFeat = QgsFeature()
meanPoint = QgsPoint(x, y)
outFeat.setGeometry(QgsGeometry.fromPoint(meanPoint))
outFeat.setAttributes([QVariant(v) for v in mean])
outputLayer.addFeature(outFeat)
