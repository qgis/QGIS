# -*- coding: utf-8 -*-
"""QGIS test python layer provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Alessandro Pasotti'
__date__ = '2018-03-18'
__copyright__ = 'Copyright 2018, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


from qgis.core import (
    QgsField,
    QgsFields,
    QgsLayerDefinition,
    QgsPointXY,
    QgsReadWriteContext,
    QgsVectorLayer,
    QgsFeatureRequest,
    QgsFeature,
    QgsGeometry,
    QgsWkbTypes,
    NULL,
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
)

from qgis.PyQt.QtCore import QVariant


class PyFeatureIterator(QgsFeatureIterator):

    def __init__(self, source, request):
        super(PyFeatureIterator, self).__init__()
        self._request = request
        self._index = 0
        self._source = source

    def nextFeature(self, f):
        """fetch next feature, return true on success"""
        #virtual bool nextFeature( QgsFeature &f );
        try:
            _f = self._source._features[self._index]
            self._index += 1
            f.setAttributes(_f.attributes())
            f.setGeometry(_f.geometry())
            f.setValid(_f.isValid())
            return True
        except Exception as e:
            f.setValid(False)
            return False

    def __iter__(self):
        'Returns itself as an iterator object'
        return self

    def __next__(self):
        'Returns the next value till current is lower than high'
        f = QgsFeature()
        if not self.nextFeature(f):
            raise StopIteration
        else:
            return f

    def rewind(self):
        """reset the iterator to the starting position"""
        #virtual bool rewind() = 0;
        self._index = 0

    def close(self):
        """end of iterating: free the resources / lock"""
        #virtual bool close() = 0;
        self._index = 0
        return True


class PyFeatureSource(QgsAbstractFeatureSource):

    def __init__(self, provider):
        super(PyFeatureSource, self).__init__()
        self._provider = provider
        self._features = provider._features

    def getFeatures(self, request):
        # QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;
        # NOTE: this is the same as PyProvider.getFeatures
        return PyFeatureIterator(self, request)


class PyProvider(QgsVectorDataProvider):

    next_feature_id = 1

    @classmethod
    def providerKey(cls):
        """Returns the memory provider key"""
        #static QString providerKey();
        return 'pythonprovider'

    @classmethod
    def description(cls):
        """Returns the memory provider description"""
        #static QString providerDescription();
        return 'Python Test Provider'

    @classmethod
    def createProvider(cls, uri):
        #static QgsMemoryProvider *createProvider( const QString &uri );
        return PyProvider(uri)

    # Implementation of functions from QgsVectorDataProvider

    def __init__(self, uri=''):
        super(PyProvider, self).__init__(uri)
        # Use the memory layer to parse the uri
        mlayer = QgsVectorLayer(uri, 'ml', 'memory')
        self._uri = uri
        self._fields = mlayer.fields()
        self._wkbType = mlayer.wkbType()
        self._features = {}
        self._extent = QgsRectangle()
        self._extent.setMinimal()
        self._subset_string = ''

    def featureSource(self):
        return PyFeatureSource(self)

    def dataSourceUri(self, expandAuthConfig=True):
        #QString dataSourceUri( bool expandAuthConfig = true ) const override;
        return self._uri

    def storageType(self):
        #QString storageType() const override;
        return "Python test memory storage"

    def getFeatures(self, request=None):
        #QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) const override;
        return PyFeatureIterator(PyFeatureSource(self), request)

    def wkbType(self):
        #QgsWkbTypes::Type wkbType() const override;
        return self._wkbType

    def featureCount(self):
        # TODO: get from source
        # long featureCount() const override;
        return len(self._features)

    def fields(self):
        #QgsFields fields() const override;
        return self._fields

    def addFeatures(self, flist, flags=None):
        # bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = nullptr ) override;
        added = False
        for f in flist:
            f.setId(self.next_feature_id)
            self._features[self.next_feature_id] = f
            self.next_feature_id += 1
            added = True
        if added:
            self.updateExtents()
        return added, flist

    def deleteFeatures(self, ids):
        #bool deleteFeatures( const QgsFeatureIds &id ) override;
        removed = False
        for id in ids:
            if id in self._features:
                del self._features[id]
                removed = True
        if removed:
            self.updateExtents()
        return removed

    def addAttributes(self, attrs):
        #bool addAttributes( const QList<QgsField> &attributes ) override;
        try:
            self._fields.append(attrs)
            for new_f in attrs:
                for f in self._features.values():
                    old_attrs = f.attributes()
                    old_attrs.app
                    old_attrs.append(None)
                    f.setAttributes(old_attrs)
            return True
        except:
            return False

    def renameAttributes(self, renameAttributes):
        #bool renameAttributes( const QgsFieldNameMap &renamedAttributes ) override;
        result = True
        for key, new_name in renamedAttributes:
            fieldIndex = key
            if fieldIndex < 0 or fieldIndex >= lself._fields.count():
                result = false
                continue
            if new_name in self._fields.indexFromName(new_name) >= 0:
                #field name already in use
                result = False
                continue
            self._fields[fieldIndex].setName(new_name)
        return True

    def deleteAttributes(self, attributes):
        #bool deleteAttributes( const QgsAttributeIds &attributes ) override;
        attrIdx = sorted(attributes.toList(), reverse=True)

        # delete attributes one-by-one with decreasing index
        for idx in attrIdx:
            self._fields.remove(idx)
            for f in self._features:
                attr = f.attributes()
                attr.remove(idx)
                f.setAttributes(attr)
        return True

    def changeAttributeValues(self, attr_map):
        #bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;
        for feature_id, attrs in attr_map.items():
            try:
                f = self._features[feature_id]
            except KeyError:
                continue
            for k, v in attrs:
                f.setAttribute(k, v)
        return True

    def changeGeometryValues(self, geometry_map):
        #bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override;
        #TODO
        return True

    def subsetString(self):
        #QString subsetString() const override;
        return self._subset_string

    def setSubsetString(self, subsetString):
        #bool setSubsetString( const QString &theSQL, bool updateFeatureCount = true ) override;
        self._subset_string = subsetString

    def supportsSubsetString(self):
        #bool supportsSubsetString() const override { return true; }
        return True

    def createSpatialIndex(self):
        #bool createSpatialIndex() override;
        return True

    def capabilities(self):
        #QgsVectorDataProvider::Capabilities capabilities() const override;
        return QgsVectorDataProvider.AddFeatures | QgsVectorDataProvider.DeleteFeatures | QgsVectorDataProvider.ChangeGeometries | QgsVectorDataProvider.ChangeAttributeValues | QgsVectorDataProvider.AddAttributes | QgsVectorDataProvider.DeleteAttributes | QgsVectorDataProvider.RenameAttributes | QgsVectorDataProvider.SelectAtId | QgsVectorDataProvider. CircularGeometries

    #/* Implementation of functions from QgsDataProvider */

    def name(self):
        #QString name() const override;
        return self.providerKey()

    def extent(self):
        #QgsRectangle extent() const override;
        if self._extent.isEmpty() and not self._features:
            self._extent.setMinimal()
            if self._subset_string.isEmpty():
                # fast way - iterate through all features
                for feat in self._features.values():
                    if feat.hasGeometry():
                        self._extent.combineExtentWith(feat.geometry().boundingBox())
            else:
                for f in self.getFeatures(QgsFeatureRequest().setSubsetOfAttributes([])):
                    if f.hasGeometry():
                        self._extent.combineExtentWith(f.geometry().boundingBox())

        elif self._features:
            self._extent.setMinimal()
        return self._extent

    def updateExtents(self):
        #void updateExtents() override;
        self._extent.setMinimal()

    def isValid(self):
        #bool isValid() const override;
        return True

    def crs(self):
        #QgsCoordinateReferenceSystem crs() const override;
        return QgsCoordinateReferenceSystem(4326)
