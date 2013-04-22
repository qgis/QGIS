##[Example scripts]=group
##input=vector
##output=output vector

from PyQt4.QtCore import *
from qgis.core import *

from sextante.core.SextanteVectorWriter import SextanteVectorWriter

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
