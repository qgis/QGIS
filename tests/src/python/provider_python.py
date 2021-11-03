# -*- coding: utf-8 -*-
"""QGIS python layer provider test.

This module is a Python implementation of (a clone of) the core memory
vector layer provider, to be used for test_provider_python.py

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Alessandro Pasotti'
__date__ = '2018-03-18'
__copyright__ = 'Copyright 2018, The QGIS Project'

from qgis.core import (
    Qgis,
    QgsField,
    QgsFields,
    QgsLayerDefinition,
    QgsPointXY,
    QgsReadWriteContext,
    QgsVectorLayer,
    QgsFeatureRequest,
    QgsFeature,
    QgsGeometry,
    QgsProject,
    QgsWkbTypes,
    QgsExpression,
    QgsExpressionContext,
    QgsExpressionContextUtils,
    NULL,
    QgsCoordinateTransform,
    QgsMemoryProviderUtils,
    QgsCoordinateReferenceSystem,
    QgsRectangle,
    QgsTestUtils,
    QgsVectorDataProvider,
    QgsAbstractFeatureSource,
    QgsAbstractFeatureIterator,
    QgsFeatureIterator,
    QgsApplication,
    QgsProviderRegistry,
    QgsProviderMetadata,
    QgsGeometryEngine,
    QgsSpatialIndex,
    QgsDataProvider,
    QgsCsException,
)

from qgis.PyQt.QtCore import QVariant


class PyFeatureIterator(QgsAbstractFeatureIterator):

    def __init__(self, source, request):
        super().__init__(request)
        self._request = request if request is not None else QgsFeatureRequest()
        self._source = source
        self._index = 0
        self._transform = QgsCoordinateTransform()
        if self._request.destinationCrs().isValid() and self._request.destinationCrs() != self._source._provider.crs():
            self._transform = QgsCoordinateTransform(self._source._provider.crs(), self._request.destinationCrs(), self._request.transformContext())
        try:
            self._filter_rect = self.filterRectToSourceCrs(self._transform)
        except QgsCsException as e:
            self.close()
            return
        self._filter_rect = self.filterRectToSourceCrs(self._transform)
        if not self._filter_rect.isNull():
            self._select_rect_geom = QgsGeometry.fromRect(self._filter_rect)
            self._select_rect_engine = QgsGeometry.createGeometryEngine(self._select_rect_geom.constGet())
            self._select_rect_engine.prepareGeometry()
        else:
            self._select_rect_engine = None
            self._select_rect_geom = None

        if self._request.spatialFilterType() == Qgis.SpatialFilterType.DistanceWithin and not self._request.referenceGeometry().isEmpty():
            self._select_distance_within_geom = self._request.referenceGeometry()
            self._select_distance_within_engine = QgsGeometry.createGeometryEngine(self._select_distance_within_geom.constGet())
            self._select_distance_within_engine.prepareGeometry()
        else:
            self._select_distance_within_geom = None
            self._select_distance_within_engine = None

        self._feature_id_list = None
        if self._filter_rect is not None and self._source._provider._spatialindex is not None:
            self._feature_id_list = self._source._provider._spatialindex.intersects(self._filter_rect)

        if self._request.filterType() == QgsFeatureRequest.FilterFid or self._request.filterType() == QgsFeatureRequest.FilterFids:
            fids = [self._request.filterFid()] if self._request.filterType() == QgsFeatureRequest.FilterFid else self._request.filterFids()
            self._feature_id_list = list(set(self._feature_id_list).intersection(set(fids))) if self._feature_id_list else fids

    def fetchFeature(self, f):
        """fetch next feature, return true on success"""
        # virtual bool nextFeature( QgsFeature &f );
        if self._index < 0:
            f.setValid(False)
            return False
        try:
            found = False
            while not found:
                _f = self._source._features[list(self._source._features.keys())[self._index]]
                self._index += 1

                if self._feature_id_list is not None and _f.id() not in self._feature_id_list:
                    continue

                if not self._filter_rect.isNull():
                    if not _f.hasGeometry():
                        continue
                    if self._request.flags() & QgsFeatureRequest.ExactIntersect:
                        # do exact check in case we're doing intersection
                        if not self._select_rect_engine.intersects(_f.geometry().constGet()):
                            continue
                    else:
                        if not _f.geometry().boundingBox().intersects(self._filter_rect):
                            continue

                self._source._expression_context.setFeature(_f)
                if self._request.filterType() == QgsFeatureRequest.FilterExpression:
                    if not self._request.filterExpression().evaluate(self._source._expression_context):
                        continue
                if self._source._subset_expression:
                    if not self._source._subset_expression.evaluate(self._source._expression_context):
                        continue
                elif self._request.filterType() == QgsFeatureRequest.FilterFids:
                    if not _f.id() in self._request.filterFids():
                        continue
                elif self._request.filterType() == QgsFeatureRequest.FilterFid:
                    if _f.id() != self._request.filterFid():
                        continue
                f.setGeometry(_f.geometry())
                self.geometryToDestinationCrs(f, self._transform)

                if self._select_distance_within_engine and self._select_distance_within_engine.distance(f.geometry().constGet()) > self._request.distanceWithin():
                    continue

                f.setFields(_f.fields())
                f.setAttributes(_f.attributes())
                f.setValid(_f.isValid())
                f.setId(_f.id())
                return True
        except IndexError as e:
            f.setValid(False)
            return False

    def __iter__(self):
        """Returns self as an iterator object"""
        self._index = 0
        return self

    def __next__(self):
        """Returns the next value till current is lower than high"""
        f = QgsFeature()
        if not self.nextFeature(f):
            raise StopIteration
        else:
            return f

    def rewind(self):
        """reset the iterator to the starting position"""
        # virtual bool rewind() = 0;
        if self._index < 0:
            return False
        self._index = 0
        return True

    def close(self):
        """end of iterating: free the resources / lock"""
        # virtual bool close() = 0;
        self._index = -1
        return True


class PyFeatureSource(QgsAbstractFeatureSource):

    def __init__(self, provider):
        super(PyFeatureSource, self).__init__()
        self._provider = provider
        self._features = provider._features

        self._expression_context = QgsExpressionContext()
        self._expression_context.appendScope(QgsExpressionContextUtils.globalScope())
        self._expression_context.appendScope(QgsExpressionContextUtils.projectScope(QgsProject.instance()))
        self._expression_context.setFields(self._provider.fields())
        if self._provider.subsetString():
            self._subset_expression = QgsExpression(self._provider.subsetString())
            self._subset_expression.prepare(self._expression_context)
        else:
            self._subset_expression = None

    def getFeatures(self, request):
        return QgsFeatureIterator(PyFeatureIterator(self, request))


class PyProvider(QgsVectorDataProvider):

    next_feature_id = 1

    @classmethod
    def providerKey(cls):
        """Returns the memory provider key"""
        return 'pythonprovider'

    @classmethod
    def description(cls):
        """Returns the memory provider description"""
        return 'Python Test Provider'

    @classmethod
    def createProvider(cls, uri, providerOptions, flags=QgsDataProvider.ReadFlags()):
        return PyProvider(uri, providerOptions, flags)

    # Implementation of functions from QgsVectorDataProvider

    def __init__(self, uri='', providerOptions=QgsDataProvider.ProviderOptions(), flags=QgsDataProvider.ReadFlags()):
        super().__init__(uri)
        # Use the memory layer to parse the uri
        mlayer = QgsVectorLayer(uri, 'ml', 'memory')
        self.setNativeTypes(mlayer.dataProvider().nativeTypes())
        self._uri = uri
        self._fields = mlayer.fields()
        self._wkbType = mlayer.wkbType()
        self._features = {}
        self._extent = QgsRectangle()
        self._extent.setMinimal()
        self._subset_string = ''
        self._crs = mlayer.crs()
        self._spatialindex = None
        self._provider_options = providerOptions
        self._flags = flags
        if 'index=yes' in self._uri:
            self.createSpatialIndex()

    def featureSource(self):
        return PyFeatureSource(self)

    def dataSourceUri(self, expandAuthConfig=True):
        return self._uri

    def storageType(self):
        return "Python test memory storage"

    def getFeatures(self, request=QgsFeatureRequest()):
        return QgsFeatureIterator(PyFeatureIterator(PyFeatureSource(self), request))

    def uniqueValues(self, fieldIndex, limit=1):
        results = set()
        if fieldIndex >= 0 and fieldIndex < self.fields().count():
            req = QgsFeatureRequest()
            req.setFlags(QgsFeatureRequest.NoGeometry)
            req.setSubsetOfAttributes([fieldIndex])
            for f in self.getFeatures(req):
                results.add(f.attributes()[fieldIndex])
        return results

    def wkbType(self):
        return self._wkbType

    def featureCount(self):
        if not self.subsetString():
            return len(self._features)
        else:
            req = QgsFeatureRequest()
            req.setFlags(QgsFeatureRequest.NoGeometry)
            req.setSubsetOfAttributes([])
            return len([f for f in self.getFeatures(req)])

    def fields(self):
        return self._fields

    def addFeatures(self, flist, flags=None):
        added = False
        f_added = []
        for f in flist:
            if f.hasGeometry() and (f.geometry().wkbType() != self.wkbType()):
                return added, f_added

        for f in flist:
            _f = QgsFeature(self.fields())
            _f.setGeometry(f.geometry())
            attrs = [None for i in range(_f.fields().count())]
            for i in range(min(len(attrs), len(f.attributes()))):
                attrs[i] = f.attributes()[i]
            _f.setAttributes(attrs)
            _f.setId(self.next_feature_id)
            self._features[self.next_feature_id] = _f
            self.next_feature_id += 1
            added = True
            f_added.append(_f)

            if self._spatialindex is not None:
                self._spatialindex.addFeature(_f)

        if len(f_added):
            self.clearMinMaxCache()
            self.updateExtents()

        return added, f_added

    def deleteFeatures(self, ids):
        if not ids:
            return True
        removed = False
        for id in ids:
            if id in self._features:
                if self._spatialindex is not None:
                    self._spatialindex.deleteFeature(self._features[id])
                del self._features[id]
                removed = True
        if removed:
            self.clearMinMaxCache()
            self.updateExtents()
        return removed

    def addAttributes(self, attrs):
        try:
            for new_f in attrs:
                if new_f.type() not in (QVariant.Int, QVariant.Double, QVariant.String, QVariant.Date, QVariant.Time, QVariant.DateTime, QVariant.LongLong, QVariant.StringList, QVariant.List):
                    continue
                self._fields.append(new_f)
                for f in self._features.values():
                    old_attrs = f.attributes()
                    old_attrs.append(None)
                    f.setAttributes(old_attrs)
            self.clearMinMaxCache()
            return True
        except Exception:
            return False

    def renameAttributes(self, renamedAttributes):
        result = True
        # We need to replace all fields because python bindings return a copy from [] and at()
        new_fields = [self._fields.at(i) for i in range(self._fields.count())]
        for fieldIndex, new_name in renamedAttributes.items():
            if fieldIndex < 0 or fieldIndex >= self._fields.count():
                result = False
                continue
            if self._fields.indexFromName(new_name) >= 0:
                # field name already in use
                result = False
                continue
            new_fields[fieldIndex].setName(new_name)
        if result:
            self._fields = QgsFields()
            for i in range(len(new_fields)):
                self._fields.append(new_fields[i])
        return result

    def deleteAttributes(self, attributes):
        attrIdx = sorted(attributes, reverse=True)

        # delete attributes one-by-one with decreasing index
        for idx in attrIdx:
            self._fields.remove(idx)
            for f in self._features.values():
                attr = f.attributes()
                del (attr[idx])
                f.setAttributes(attr)
        self.clearMinMaxCache()
        return True

    def changeAttributeValues(self, attr_map):
        for feature_id, attrs in attr_map.items():
            try:
                f = self._features[feature_id]
            except KeyError:
                continue
            for k, v in attrs.items():
                f.setAttribute(k, v)
        self.clearMinMaxCache()
        return True

    def changeGeometryValues(self, geometry_map):
        for feature_id, geometry in geometry_map.items():
            try:
                f = self._features[feature_id]
                f.setGeometry(geometry)
            except KeyError:
                continue
        self.updateExtents()
        return True

    def allFeatureIds(self):
        return list(self._features.keys())

    def subsetString(self):
        return self._subset_string

    def setSubsetString(self, subsetString):
        if subsetString == self._subset_string:
            return True
        self._subset_string = subsetString
        self.updateExtents()
        self.clearMinMaxCache()
        self.dataChanged.emit()
        return True

    def supportsSubsetString(self):
        return True

    def createSpatialIndex(self):
        if self._spatialindex is None:
            self._spatialindex = QgsSpatialIndex()
            for f in self._features.values():
                self._spatialindex.insertFeature(f)
        return True

    def capabilities(self):
        return QgsVectorDataProvider.AddFeatures | QgsVectorDataProvider.DeleteFeatures | QgsVectorDataProvider.CreateSpatialIndex | QgsVectorDataProvider.ChangeGeometries | QgsVectorDataProvider.ChangeAttributeValues | QgsVectorDataProvider.AddAttributes | QgsVectorDataProvider.DeleteAttributes | QgsVectorDataProvider.RenameAttributes | QgsVectorDataProvider.SelectAtId | QgsVectorDataProvider. CircularGeometries

    # /* Implementation of functions from QgsDataProvider */

    def name(self):
        return self.providerKey()

    def extent(self):
        if self._extent.isEmpty() and self._features:
            self._extent.setMinimal()
            if not self._subset_string:
                # fast way - iterate through all features
                for feat in self._features.values():
                    if feat.hasGeometry():
                        self._extent.combineExtentWith(feat.geometry().boundingBox())
            else:
                for f in self.getFeatures(QgsFeatureRequest().setSubsetOfAttributes([])):
                    if f.hasGeometry():
                        self._extent.combineExtentWith(f.geometry().boundingBox())

        elif not self._features:
            self._extent.setMinimal()
        return QgsRectangle(self._extent)

    def updateExtents(self):
        self._extent.setMinimal()

    def isValid(self):
        return True

    def crs(self):
        return self._crs

    def handlePostCloneOperations(self, source):
        self._features = source._features
