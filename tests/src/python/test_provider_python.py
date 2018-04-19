# -*- coding: utf-8 -*-
"""QGIS Unit tests for the python layer provider.

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

from qgis.testing import (
    start_app,
    unittest
)

from utilities import (
    unitTestDataPath,
    compareWkt
)

from providertestbase import ProviderTestCase
from qgis.PyQt.QtCore import QVariant

start_app()
TEST_DATA_DIR = unitTestDataPath()


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

    @classmethod
    def providerKey(cls):
        """Returns the memory provider key"""
        #static QString providerKey();
        return 'py'

    @classmethod
    def description(cls):
        """Returns the memory provider description"""
        #static QString providerDescription();
        return 'py provider'

    @classmethod
    def createProvider(cls, uri):
        #static QgsMemoryProvider *createProvider( const QString &uri );
        return PyProvider(uri)

    # Implementation of functions from QgsVectorDataProvider

    def __init__(self, uri=''):
        super(PyProvider, self).__init__(uri)
        self._fields = QgsFields()
        self._fields.append(QgsField('id', QVariant.Int, 'id', 10, 2, 'an id'))
        self._features = []

    def featureSource(self):
        return PyFeatureSource(self)

    def dataSourceUri(self, expandAuthConfig=True):
        #QString dataSourceUri( bool expandAuthConfig = true ) const override;
        # TODO: add some params
        return 'py'

    def storageType(self):
        #QString storageType() const override;
        return "Py storage"

    def getFeatures(self, request):
        #QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) const override;
        return PyFeatureIterator(PyFeatureSource(self), request)

    def wkbType(self):
        #QgsWkbTypes::Type wkbType() const override;
        return QgsWkbTypes.Point

    def featureCount(self):
        # TODO: get from source
        # long featureCount() const override;
        return len(self._features)

    def fields(self):
        #QgsFields fields() const override;
        return self._fields

    def addFeatures(self, flist, flags=None):
        # bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = nullptr ) override;
        self._features += flist

    def deleteFeatures(self, ids):
        #bool deleteFeatures( const QgsFeatureIds &id ) override;
        removed = False
        for f in self._features:
            if f.id() in ids:
                self._features.remove(f)
                removed = True
        return removed

    def addAttributes(self, attrs):
        #bool addAttributes( const QList<QgsField> &attributes ) override;
        # TODO
        return True

    def renameAttributes(self, renameAttributes):
        #bool renameAttributes( const QgsFieldNameMap &renamedAttributes ) override;
        # TODO:
        return True

    def deleteAttributes(self, attributes):
        #bool deleteAttributes( const QgsAttributeIds &attributes ) override;
        # TODO
        return True

    def changeAttributeValues(self, attr_map):
        #bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;
        # TODO
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
        return QgsVectorDataProvider.AddFeatures | QgsVectorDataProvider.DeleteFeatures | QgsVectorDataProvider.ChangeGeometries | QgsVectorDataProvider.ChangeAttributeValues | QgsVectorDataProvider.AddAttributes | QgsVectorDataProvider.DeleteAttributes | QgsVectorDataProvider.RenameAttributes | QgsVectorDataProvider.CreateSpatialIndex | QgsVectorDataProvider.SelectAtId | QgsVectorDataProvider. CircularGeometries

    #/* Implementation of functions from QgsDataProvider */

    def name(self):
        #QString name() const override;
        return self.providerKey()

    def extent(self):
        #QgsRectangle extent() const override;
        return QgsRectangle(1, 1, 2, 2)

    def updateExtents(self):
        #void updateExtents() override;
        pass

    def isValid(self):
        #bool isValid() const override;
        return True

    def crs(self):
        #QgsCoordinateReferenceSystem crs() const override;
        return QgsCoordinateReferenceSystem(4326)


class TestPyQgsPythonProvider(unittest.TestCase, ProviderTestCase):

    @classmethod
    def populateLayer(cls, provider):
        f1 = QgsFeature(provider.fields(), 1)
        f1.setAttributes([1])
        f1.setGeometry(QgsGeometry.fromWkt('Point (1 1)'))
        f2 = QgsFeature(provider.fields(), 2)
        f2.setAttributes([2])
        f2.setGeometry(QgsGeometry.fromWkt('Point (2 2)'))
        provider.addFeatures([f1, f2])

    @classmethod
    def createLayer(cls):
        vl = QgsVectorLayer(
            'my_data_source_specification',
            'test', 'py')
        assert (vl.isValid())

        return vl

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Register provider

        r = QgsProviderRegistry.instance()
        metadata = QgsProviderMetadata(PyProvider.providerKey(), PyProvider.description(), PyProvider.createProvider)
        r.registerProvider(metadata)

        assert r.providerMetadata(PyProvider.providerKey()) == metadata

        # Create test layer
        cls.vl = cls.createLayer()
        cls.populateLayer(cls.vl.dataProvider())
        assert (cls.vl.isValid())
        cls.source = cls.vl.dataProvider()

    def test_Provider(self):
        p = PyProvider('my_uri')
        self.populateLayer(p)
        features = [f for f in p.getFeatures(QgsFeatureRequest())]
        self.assertTrue(features[0].isValid())
        self.assertTrue(features[1].isValid())
        self.assertEqual(features[0].attributes(), [1])
        self.assertEqual(features[1].attributes(), [2])
        self.assertEqual(features[0].geometry().asWkt(), 'Point (1 1)')
        self.assertEqual(features[1].geometry().asWkt(), 'Point (2 2)')

        self.assertEqual(p.featureCount(), 2)

        f3 = QgsFeature(p.fields())
        f3.setAttributes([3])
        f3.setGeometry(QgsGeometry.fromWkt('Point (3 3)'))
        p.addFeatures(f3)
        self.assertEqual(p.featureCount(), 3)

    def test_featureSource(self):
        p = PyProvider('my_uri')
        self.populateLayer(p)
        s = p.featureSource()
        it = s.getFeatures(QgsFeatureRequest())
        f = QgsFeature()
        it.nextFeature(f)
        self.assertTrue(f.isValid())
        self.assertEqual(f.attributes(), [1])
        self.assertEqual(f.geometry().asWkt(), 'Point (1 1)')


if __name__ == '__main__':
    unittest.main()
