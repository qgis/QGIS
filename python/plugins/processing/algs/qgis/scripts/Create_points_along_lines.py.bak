##Vector geometry tools=group
##Lines=vector
##Distance=number 1
##Startpoint=number 0
##Endpoint=number 0
##output=output vector

from PyQt4.QtCore import QVariant
from qgis.core import QGis, QgsFeature, QgsField
from processing.tools.vector import VectorWriter


def create_points(feat):
    geom = feat.geometry()
    length = geom.length()
    currentdistance = 0

    if Endpoint > 0:
        length = Endpoint

    out = QgsFeature()

    while Startpoint + currentdistance <= length:
        point = geom.interpolate(Startpoint + currentdistance)
        currentdistance = currentdistance + Distance
        out.setGeometry(point)
        attrs = feat.attributes()
        attrs.append(currentdistance)
        out.setAttributes(attrs)
        writer.addFeature(out)


layer = processing.getObject(Lines)
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
