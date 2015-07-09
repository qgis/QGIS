##Vector geometry tools=group
##Lines=vector
##Distance=number 1
##Startpoint=number 0
##Endpoint=number 0
##Points along lines=output vector

from PyQt4.QtCore import QVariant
from qgis.core import QGis, QgsFeature, QgsField
from processing.tools.vector import VectorWriter


def create_points(feat):
    geom = feat.geometry()
    length = geom.length()
    currentdistance = 0

    if endpoint > 0:
        length = endpoint

    out = QgsFeature()

    while startpoint + currentdistance <= length:
        point = geom.interpolate(startpoint + currentdistance)
        currentdistance = currentdistance + distance
        out.setGeometry(point)
        attrs = feat.attributes()
        attrs.append(currentdistance)
        out.setAttributes(attrs)
        writer.addFeature(out)


layer = processing.getObject(lines)
fields = layer.dataProvider().fields()
fields.append(QgsField('Distance', QVariant.Double))
writer = VectorWriter(output, None, fields, QGis.WKBPoint,
                      layer.crs())

feats = processing.features(layer)
nFeat = len(feats)
for i, feat in enumerate(feats):
    progress.setPercentage(int(100 * i / nFeat))
    create_points(feat)

del writer
