##Vector geometry tools=group
##Polygons=vector polygon
##To_keep=number 1
##Biggest parts=output vector

from qgis.core import Qgis, QgsGeometry, QgsWkbTypes
from operator import itemgetter

To_keep = int(To_keep)
if To_keep < 1:
    progress.setInfo("'To keep' value has been modified to be at least 1.")
    To_keep = 1


polyLayer = processing.getObject(Polygons)
polyPrder = polyLayer.dataProvider()
count = polyLayer.featureCount()
writer = processing.VectorWriter(Results, None, polyPrder.fields(),
                                 QgsWkbTypes.MultiPolygon, polyPrder.crs())


for n, feat in enumerate(processing.features(polyLayer)):
    progress.setPercentage(int(100 * n / count))
    geom = feat.geometry()
    if geom.isMultipart():
        featres = feat
        geoms = geom.asGeometryCollection()
        geomarea = [(i, geoms[i].area()) for i in range(len(geoms))]
        geomarea.sort(key=itemgetter(1))
        if To_keep == 1:
            featres.setGeometry(geoms[geomarea[-1][0]])
        elif To_keep > len(geoms):
            featres.setGeometry(geom)
        else:
            featres.setGeometry(geom)
            geomres = [geoms[i].asPolygon() for i, a in geomarea[-1 * To_keep:]]
            featres.setGeometry(QgsGeometry.fromMultiPolygon(geomres))
        writer.addFeature(featres)
    else:
        writer.addFeature(feat)

del writer
