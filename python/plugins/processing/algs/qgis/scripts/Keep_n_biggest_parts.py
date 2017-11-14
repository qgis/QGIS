from builtins import range
##Vector geometry=group

#inputs

##Polygons=source polygon
##To_keep=number 1
##Biggest parts=sink polygon


from qgis.core import QgsGeometry, QgsWkbTypes, QgsProcessingUtils
from operator import itemgetter

To_keep = int(To_keep)
if To_keep < 1:
    feedback.pushInfo("'To keep' value has been modified to be at least 1.")
    To_keep = 1

count = Polygons.featureCount()
(sink, Biggest_parts) = self.parameterAsSink(parameters, 'Biggest parts', context,
                                             Polygons.fields(), QgsWkbTypes.MultiPolygon, Polygons.sourceCrs())


for n, feat in enumerate(Polygons.getFeatures()):
    if feedback.isCanceled():
        break
    feedback.setProgress(int(100 * n / count))

    geom = feat.geometry()
    if geom.isMultipart():
        features = feat
        geoms = geom.asGeometryCollection()
        geomarea = [(i, geoms[i].area()) for i in range(len(geoms))]
        geomarea.sort(key=itemgetter(1))
        if To_keep == 1:
            features.setGeometry(geoms[geomarea[-1][0]])
        elif To_keep > len(geoms):
            features.setGeometry(geom)
        else:
            features.setGeometry(geom)
            geomres = [geoms[i].asPolygon() for i, a in geomarea[-1 * To_keep:]]
            features.setGeometry(QgsGeometry.fromMultiPolygonXY(geomres))
        sink.addFeature(features)
    else:
        sink.addFeature(feat)
