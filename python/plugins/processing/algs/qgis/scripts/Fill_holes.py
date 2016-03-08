##Vector geometry tools=group
##Polygons=vector
##Max_area=number 100000
##Results=output vector

from qgis.core import QGis, QgsFeature, QgsGeometry
from shapely.geometry import Polygon, MultiPolygon
from shapely.wkb import loads
from shapely.wkt import dumps


polyLayer = processing.getObject(Polygons)
polyPrder = polyLayer.dataProvider()
n = polyLayer.featureCount()
l = 0

writer = processing.VectorWriter(Results, None, polyPrder.fields(),
                                 QGis.WKBMultiPolygon, polyPrder.crs())


resgeom = QgsGeometry()
resfeat = QgsFeature()

for feat in processing.features(polyLayer):
    progress.setPercentage(int(100 * l / n))
    l += 1

    g = loads(feat.geometry().asWkb())

    if g.geom_type == 'MultiPolygon':
        resg = [Polygon(p.exterior,
                        [r for r in p.interiors if Polygon(r).area > Max_area]) for p in g]

    else:
        resg = [Polygon(g.exterior,
                        [r for r in g.interiors if Polygon(r).area > Max_area])]

    resgeom = QgsGeometry().fromWkt(dumps(MultiPolygon(resg)))

    resfeat.setAttributes(feat.attributes())
    resfeat.setGeometry(resgeom)
    writer.addFeature(resfeat)

del writer
