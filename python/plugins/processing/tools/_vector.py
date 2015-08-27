import os
from PyQt4.QtCore import *
from qgis.core import *
from collections import Iterator

TYPE_MAP = {
    str: QVariant.String,
    float: QVariant.Double,
    int: QVariant.Int,
    bool: QVariant.Bool
}

GEOM_TYPE_MAP = {
    QGis.WKBPoint: 'Point',
    QGis.WKBLineString: 'LineString',
    QGis.WKBPolygon: 'Polygon',
    QGis.WKBMultiPoint: 'MultiPoint',
    QGis.WKBMultiLineString: 'MultiLineString',
    QGis.WKBMultiPolygon: 'MultiPolygon',
}

QGSGEOM = "qgs_geom"


def _toQgsGeometry(geom, geomtype):
    def toQgsPoint(p):
        if isinstance(p, tuple):
            return QgsPoint(p[0], p[1])
        elif isinstance(p, QgsPoint):
            return p
        else:
            return [toQgsPoint(item) for item in p]
    if isinstance(geom, QgsGeometry):
        return geom
    if geomtype == QGis.WKBPoint:
        return QgsGeometry.fromPoint(toQgsPoint(geom))
    elif geomtype == QGis.WKBLineString:
        return QgsGeometry.fromPolyline(toQgsPoint(geom))
    elif geomtype == QGis.WKBPolygon:
        return QgsGeometry.fromPolygon(toQgsPoint(geom))
    else:
        pass
        #TODO


class VectorLayer(object):

    def __init__(self, layer):
        self.layer = layer

    def __getattr__(self, name):
        return getattr(self.__dict__['layer'], name)

    def addFeature(self, *args):
        if len(args) == 2:
            feature = QgsFeature()
            feature.setGeometry(_toQgsGeometry(args[0], self.wkbType()))
            feature.setAttributes(args[1])
        if len(args) == 1:
            if isinstance(args[0], dict):
                feature = QgsFeature()
                attrs = [args[0].get(f.name(), None) for f in self.layer.dataProvider().fields()]
                feature.setAttributes(attrs)
                if QGSGEOM in args[0]:
                    feature.setGeometry[args[0][QGSGEOM]]
            elif isinstance(args[0], QgsFeature):
                feature = args[0]
        self.dataProvider().addFeatures([feature])

    def addField(self, name, fieldtype, defaultValue=None):
        fields = [f.name() for f in self.dataProvider().fields()]
        self.dataProvider().addAttributes([_toQgsField((name, fieldtype))])
        self.updateFields()
        if defaultValue is not None:
            idx = self.dataProvider().fieldNameIndex(name)
            for featureIdx, f in enumerate(self.getFeatures()):
                if callable(defaultValue):
                    attrs = {QGSGEOM: Geometry(f.geometry())}
                    for i, field in enumerate(fields):
                        attrs[field] = f.attributes()[i]
                        v = defaultValue(featureIdx, attrs)
                else:
                    v = defaultValue
                self.dataProvider().changeAttributeValues({f.id(): {idx: v}})

    def features(self):
        class todict(Iterator):

            def __init__(self, featureiter, fields):
                self.featureiter = featureiter
                self.fields = fields

            def next(self):
                feature = self.featureiter.next()

                class geomdict(dict):

                    def geom(self):
                        return self[QGSGEOM]
                d = geomdict()
                d[QGSGEOM] = Geometry(feature.geometry())
                for i, field in enumerate(self.fields):
                    d[field] = feature.attributes()[i]
                return d
        fields = [f.name() for f in self.dataProvider().fields()]
        return todict(self.getFeatures(), fields)

TYPE_MAP = {
    str: QVariant.String,
    float: QVariant.Double,
    int: QVariant.Int,
    bool: QVariant.Bool
}

GEOM_TYPE_MAP = {
    QGis.WKBPoint: 'Point',
    QGis.WKBLineString: 'LineString',
    QGis.WKBPolygon: 'Polygon',
    QGis.WKBMultiPoint: 'MultiPoint',
    QGis.WKBMultiLineString: 'MultiLineString',
    QGis.WKBMultiPolygon: 'MultiPolygon',
}


def _toQgsField(f):
    if isinstance(f, QgsField):
        return f
    return QgsField(f[0], TYPE_MAP.get(f[1], QVariant.String))


def _fieldName(f):
    if isinstance(f, basestring):
        return f
    return f.name()


def newPointsLayer(filename, fields, crs, encoding="utf-8"):
    return newVectorLayer(filename, fields, QGis.WKBPoint, crs, encoding)


def newLinesLayer(filename, fields, crs, encoding="utf-8"):
    return newVectorLayer(filename, fields, QGis.WKBLine, crs, encoding)


def newPolygonsLayer(filename, fields, crs, encoding="utf-8"):
    return newVectorLayer(filename, fields, QGis.WKBPolygon, crs, encoding)


def newVectorLayer(filename, fields, geometryType, crs, encoding="utf-8"):
    if isinstance(crs, basestring):
        crs = QgsCoordinateReferenceSystem(crs)
    if filename is None:
        uri = self.GEOM_TYPE_MAP[geometryType]
        if crs.isValid():
            uri += '?crs=' + crs.authid() + '&'
        fieldsdesc = ['field=' + f for f in fields]

        fieldsstring = '&'.join(fieldsdesc)
        uri += fieldsstring
        layer = QgsVectorLayer(uri, "mem_layer", 'memory')
    else:
        formats = QgsVectorFileWriter.supportedFiltersAndFormats()
        OGRCodes = {}
        for (key, value) in formats.items():
            extension = unicode(key)
            extension = extension[extension.find('*.') + 2:]
            extension = extension[:extension.find(' ')]
            OGRCodes[extension] = value

        extension = os.path.splitext(filename)[1][1:]
        if extension not in OGRCodes:
            extension = 'shp'
            filename = filename + '.shp'

        qgsfields = QgsFields()
        for field in fields:
            qgsfields.append(_toQgsField(field))

        QgsVectorFileWriter(filename, encoding, qgsfields,
                            geometryType, crs, OGRCodes[extension])

        layer = QgsVectorLayer(filename, os.path.basename(filename), 'ogr')

    return VectorLayer(layer)


class Geometry(QgsGeometry):

    def __init__(self, geom):
        self.geom = geom

    def __getattr__(self, name):
        return getattr(self.__dict__['geom'], name)

    def __iter__(self):
        geomType = self.wkbType()
        if geomType in [QGis.WKBMultiPolygon, QGis.WKBMultiPolygon25D]:
            return self.asMultiPolygon()
        elif geomType in [QGis.WKBPolygon, QGis.WKBPolygon25D]:
            return self.asPolygon()
        elif geomType in [QGis.WKBMultiLineString, QGis.WKBMultiLineString25D]:
            return self.asMultiPolyline()
        elif geomType in [QGis.WKBLineString, QGis.WKBLineString25D]:
            return self.asPolyline()
        elif geomType in [QGis.WKBMultiPoint, QGis.WKBMultiPoint25D]:
            return self.asMultiPoint()
        elif geomType in [QGis.WKBPoint, QGis.WKBPoint25D]:
            return self.asPoint()

    def next(self):
        return


def layerFromName(name):
    layers = QgsMapLayerRegistry.instance().mapLayers().values()
    for layer in layers:
        if layer.name() == name:
            return VectorLayer(layer)


def loadLayer(filename):
    name = os.path.split(filename)[1]
    qgslayer = QgsVectorLayer(filename, name, 'ogr')
    if not qgslayer.isValid():
        qgslayer = QgsRasterLayer(filename, name)
        if not qgslayer.isValid():
            raise RuntimeError('Could not load layer: ' + unicode(filename))

    return VectorLayer(qgslayer)
