# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayerMetadata.

Run with: ctest -V -R PyQgsLayerMetadata

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '11/04/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA

from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (QgsLayerMetadata,
                       QgsCoordinateReferenceSystem,
                       QgsVectorLayer,
                       QgsNativeMetadataValidator,
                       QgsBox3d,
                       QgsDateTimeRange)
from qgis.PyQt.QtCore import (QDate,
                              QTime,
                              QDateTime,
                              QRegularExpression,)
from qgis.testing import start_app, unittest

start_app()


class TestQgsLayerMetadata(unittest.TestCase):

    def testGettersSetters(self):
        m = QgsLayerMetadata()

        m.setIdentifier('identifier')
        self.assertEqual(m.identifier(), 'identifier')

        m.setParentIdentifier('parent identifier')
        self.assertEqual(m.parentIdentifier(), 'parent identifier')

        m.setLanguage('en-us')
        self.assertEqual(m.language(), 'en-us')

        m.setType('type')
        self.assertEqual(m.type(), 'type')

        m.setTitle('title')
        self.assertEqual(m.title(), 'title')

        m.setCategories(['category'])
        self.assertEqual(m.categories(), ['category'])

        m.setAbstract('abstract')
        self.assertEqual(m.abstract(), 'abstract')

        m.setFees('fees')
        self.assertEqual(m.fees(), 'fees')

        m.setConstraints([QgsLayerMetadata.Constraint('constraint a'), QgsLayerMetadata.Constraint('constraint b')])
        m.addConstraint(QgsLayerMetadata.Constraint('constraint c'))
        self.assertEqual(m.constraints()[0].constraint, 'constraint a')
        self.assertEqual(m.constraints()[1].constraint, 'constraint b')
        self.assertEqual(m.constraints()[2].constraint, 'constraint c')

        m.setRights(['right a', 'right b'])
        self.assertEqual(m.rights(), ['right a', 'right b'])

        m.setLicenses(['l a', 'l b'])
        self.assertEqual(m.licenses(), ['l a', 'l b'])

        m.setHistory(['loaded into QGIS'])
        self.assertEqual(m.history(), ['loaded into QGIS'])
        m.setHistory(['accidentally deleted some features'])
        self.assertEqual(m.history(), ['accidentally deleted some features'])
        m.addHistoryItem('panicked and deleted more')
        self.assertEqual(m.history(), ['accidentally deleted some features', 'panicked and deleted more'])

        m.setEncoding('encoding')
        self.assertEqual(m.encoding(), 'encoding')

        m.setCrs(QgsCoordinateReferenceSystem.fromEpsgId(3111))
        self.assertEqual(m.crs().authid(), 'EPSG:3111')

    def testEquality(self):
        # spatial extent
        extent = QgsLayerMetadata.SpatialExtent()
        extent.extentCrs = QgsCoordinateReferenceSystem.fromEpsgId(3111)
        extent.bounds = QgsBox3d(5.0, 6.0, 7.0, 11.0, 13.0, 15.0)
        extent2 = QgsLayerMetadata.SpatialExtent()
        extent2.extentCrs = QgsCoordinateReferenceSystem.fromEpsgId(3111)
        extent2.bounds = QgsBox3d(5.0, 6.0, 7.0, 11.0, 13.0, 15.0)
        self.assertEqual(extent, extent2)
        extent2.extentCrs = QgsCoordinateReferenceSystem.fromEpsgId(3113)
        self.assertNotEqual(extent, extent2)
        extent2.extentCrs = QgsCoordinateReferenceSystem.fromEpsgId(3111)
        extent2.bounds = QgsBox3d(5.0, 6.0, 7.0, 11.0, 13.0, 16.0)
        self.assertNotEqual(extent, extent2)

        # extent
        extent = QgsLayerMetadata.Extent()
        extent1 = QgsLayerMetadata.SpatialExtent()
        extent1.extentCrs = QgsCoordinateReferenceSystem.fromEpsgId(3111)
        extent1.bounds = QgsBox3d(5.0, 6.0, 7.0, 11.0, 13.0, 15.0)
        extent2 = QgsLayerMetadata.SpatialExtent()
        extent2.extentCrs = QgsCoordinateReferenceSystem.fromEpsgId(3113)
        extent2.bounds = QgsBox3d(5.0, 6.0, 7.0, 11.0, 13.0, 16.0)
        extent.setSpatialExtents([extent1, extent2])
        dates = [
            QgsDateTimeRange(
                QDateTime(QDate(2001, 12, 17), QTime(9, 30, 47)),
                QDateTime(QDate(2001, 12, 17), QTime(9, 30, 47))),
            QgsDateTimeRange(
                QDateTime(QDate(2010, 12, 17), QTime(9, 30, 47)),
                QDateTime(QDate(2020, 12, 17), QTime(9, 30, 47)))
        ]
        extent.setTemporalExtents(dates)
        extent_copy = QgsLayerMetadata.Extent(extent)
        self.assertEqual(extent, extent_copy)
        extent_copy.setTemporalExtents([
            QgsDateTimeRange(
                QDateTime(QDate(2001, 12, 17), QTime(9, 30, 47)),
                QDateTime(QDate(2001, 12, 17), QTime(9, 30, 47))),
            QgsDateTimeRange(
                QDateTime(QDate(2010, 12, 17), QTime(9, 30, 48)),
                QDateTime(QDate(2020, 12, 17), QTime(9, 30, 49)))
        ])
        self.assertNotEqual(extent, extent_copy)
        extent_copy = QgsLayerMetadata.Extent(extent)
        extent3 = QgsLayerMetadata.SpatialExtent()
        extent3.extentCrs = QgsCoordinateReferenceSystem.fromEpsgId(3113)
        extent3.bounds = QgsBox3d(5.0, 6.0, 7.0, 11.0, 13.0, 19.0)
        extent_copy.setSpatialExtents([extent1, extent3])
        self.assertNotEqual(extent, extent_copy)

        constraint = QgsLayerMetadata.Constraint('c', 'type1')
        self.assertEqual(constraint, QgsLayerMetadata.Constraint('c', 'type1'))
        self.assertNotEqual(constraint, QgsLayerMetadata.Constraint('c2', 'type1'))
        self.assertNotEqual(constraint, QgsLayerMetadata.Constraint('c', 'type2'))

    def testExtent(self):
        e = QgsLayerMetadata.Extent()
        se = QgsLayerMetadata.SpatialExtent()
        se.extentCrs = QgsCoordinateReferenceSystem.fromEpsgId(3111)
        se.bounds = QgsBox3d(1, 2, 3, 4, 5, 6)
        e.setSpatialExtents([se])
        e.setTemporalExtents([QgsDateTimeRange(QDateTime(QDate(2017, 1, 3), QTime(11, 34, 56)), QDateTime(QDate(2018, 1, 3), QTime(12, 35, 57)))])

        m = QgsLayerMetadata()
        m.setExtent(e)

        extents = m.extent().spatialExtents()
        self.assertEqual(extents[0].extentCrs.authid(), 'EPSG:3111')
        self.assertEqual(extents[0].bounds.xMinimum(), 1.0)
        self.assertEqual(extents[0].bounds.yMinimum(), 2.0)
        self.assertEqual(extents[0].bounds.zMinimum(), 3.0)
        self.assertEqual(extents[0].bounds.xMaximum(), 4.0)
        self.assertEqual(extents[0].bounds.yMaximum(), 5.0)
        self.assertEqual(extents[0].bounds.zMaximum(), 6.0)
        self.assertEqual(m.extent().temporalExtents()[0].begin(), QDateTime(QDate(2017, 1, 3), QTime(11, 34, 56)))
        self.assertEqual(m.extent().temporalExtents()[0].end(), QDateTime(QDate(2018, 1, 3), QTime(12, 35, 57)))

    def createTestMetadata(self):
        """
        Returns a standard metadata which can be tested with checkExpectedMetadata
        """
        m = QgsLayerMetadata()
        m.setIdentifier('1234')
        m.setParentIdentifier('xyz')
        m.setLanguage('en-CA')
        m.setType('dataset')
        m.setTitle('roads')
        m.setAbstract('my roads')
        m.setFees('None')
        m.setConstraints([QgsLayerMetadata.Constraint('None', 'access')])
        m.setRights(['Copyright foo 2017'])
        m.setLicenses(['WTFPL'])
        m.setHistory(['history a', 'history b'])
        m.setKeywords({
            'GEMET': ['kw1', 'kw2'],
            'gmd:topicCategory': ['natural'],
        })
        m.setEncoding('utf-8')
        m.setCrs(QgsCoordinateReferenceSystem.fromOgcWmsCrs('EPSG:4326'))

        e = QgsLayerMetadata.Extent()
        se = QgsLayerMetadata.SpatialExtent()
        se.extentCrs = QgsCoordinateReferenceSystem.fromOgcWmsCrs('EPSG:4326')
        se.bounds = QgsBox3d(-180, -90, 0, 180, 90, 0)
        e.setSpatialExtents([se])
        dates = [
            QgsDateTimeRange(
                QDateTime(QDate(2001, 12, 17), QTime(9, 30, 47)),
                QDateTime(QDate(2001, 12, 17), QTime(9, 30, 47))),
            QgsDateTimeRange(
                QDateTime(QDate(2010, 12, 17), QTime(9, 30, 47)),
                QDateTime(QDate(2020, 12, 17), QTime(9, 30, 47)))
        ]
        e.setTemporalExtents(dates)
        m.setExtent(e)

        c = QgsLayerMetadata.Contact()
        c.name = 'John Smith'
        c.organization = 'ACME'
        c.position = 'staff'
        c.voice = '1500 515 555'
        c.fax = 'xx.xxx.xxx.xxxx'
        c.email = 'foo@example.org'
        c.role = 'pointOfContact'
        address = QgsLayerMetadata.Address()
        address.type = 'postal'
        address.address = '123 Main Street'
        address.city = 'anycity'
        address.administrativeArea = 'anyprovince'
        address.postalCode = '90210'
        address.country = 'Canada'
        c.addresses = [address]
        m.setContacts([c])

        l = QgsLayerMetadata.Link()
        l.name = 'geonode:roads'
        l.type = 'OGC:WMS'
        l.description = 'my GeoNode road layer'
        l.url = 'http://example.org/wms'

        l2 = QgsLayerMetadata.Link()
        l2.name = 'geonode:roads'
        l2.type = 'OGC:WFS'
        l2.description = 'my GeoNode road layer'
        l2.url = 'http://example.org/wfs'

        l3 = QgsLayerMetadata.Link()
        l3.name = 'roads'
        l3.type = 'WWW:LINK'
        l3.description = 'full dataset download'
        l3.url = 'http://example.org/roads.tgz'
        l3.format = 'ESRI Shapefile'
        l3.mimeType = 'application/gzip'
        l3.size = '283676'

        m.setLinks([l, l2, l3])

        return m

    def checkExpectedMetadata(self, m):
        """
        Checks that a metadata object matches that returned by createTestMetadata
        """
        self.assertEqual(m.identifier(), '1234')
        self.assertEqual(m.parentIdentifier(), 'xyz')
        self.assertEqual(m.language(), 'en-CA')
        self.assertEqual(m.type(), 'dataset')
        self.assertEqual(m.title(), 'roads')
        self.assertEqual(m.abstract(), 'my roads')
        self.assertEqual(m.fees(), 'None')
        self.assertEqual(m.constraints()[0].constraint, 'None')
        self.assertEqual(m.constraints()[0].type, 'access')
        self.assertEqual(m.rights(), ['Copyright foo 2017'])
        self.assertEqual(m.licenses(), ['WTFPL'])
        self.assertEqual(m.history(), ['history a', 'history b'])
        self.assertEqual(m.encoding(), 'utf-8')
        self.assertEqual(
            m.keywords(),
            {'GEMET': ['kw1', 'kw2'], 'gmd:topicCategory': ['natural']})
        self.assertEqual(m.crs().authid(), 'EPSG:4326')

        extent = m.extent().spatialExtents()[0]
        self.assertEqual(extent.extentCrs.authid(), 'EPSG:4326')
        self.assertEqual(extent.bounds.xMinimum(), -180.0)
        self.assertEqual(extent.bounds.yMinimum(), -90.0)
        self.assertEqual(extent.bounds.xMaximum(), 180.0)
        self.assertEqual(extent.bounds.yMaximum(), 90.0)
        self.assertEqual(m.extent().temporalExtents()[0].begin(), QDateTime(QDate(2001, 12, 17), QTime(9, 30, 47)))
        self.assertTrue(m.extent().temporalExtents()[0].isInstant())
        self.assertFalse(m.extent().temporalExtents()[1].isInstant())
        self.assertEqual(m.extent().temporalExtents()[1].end(), QDateTime(QDate(2020, 12, 17), QTime(9, 30, 47)))

        self.assertEqual(m.contacts()[0].name, 'John Smith')
        self.assertEqual(m.contacts()[0].organization, 'ACME')
        self.assertEqual(m.contacts()[0].position, 'staff')
        self.assertEqual(m.contacts()[0].voice, '1500 515 555')
        self.assertEqual(m.contacts()[0].fax, 'xx.xxx.xxx.xxxx')
        self.assertEqual(m.contacts()[0].email, 'foo@example.org')
        self.assertEqual(m.contacts()[0].role, 'pointOfContact')
        self.assertEqual(m.contacts()[0].addresses[0].type, 'postal')
        self.assertEqual(m.contacts()[0].addresses[0].address, '123 Main Street')
        self.assertEqual(m.contacts()[0].addresses[0].city, 'anycity')
        self.assertEqual(m.contacts()[0].addresses[0].administrativeArea, 'anyprovince')
        self.assertEqual(m.contacts()[0].addresses[0].postalCode, '90210')
        self.assertEqual(m.contacts()[0].addresses[0].country, 'Canada')
        self.assertEqual(m.links()[0].name, 'geonode:roads')
        self.assertEqual(m.links()[0].type, 'OGC:WMS')
        self.assertEqual(m.links()[0].description, 'my GeoNode road layer')
        self.assertEqual(m.links()[0].url, 'http://example.org/wms')
        self.assertEqual(m.links()[1].name, 'geonode:roads')
        self.assertEqual(m.links()[1].type, 'OGC:WFS')
        self.assertEqual(m.links()[1].description, 'my GeoNode road layer')
        self.assertEqual(m.links()[1].url, 'http://example.org/wfs')
        self.assertEqual(m.links()[2].name, 'roads')
        self.assertEqual(m.links()[2].type, 'WWW:LINK')
        self.assertEqual(m.links()[2].description, 'full dataset download')
        self.assertEqual(m.links()[2].url, 'http://example.org/roads.tgz')
        self.assertEqual(m.links()[2].format, 'ESRI Shapefile')
        self.assertEqual(m.links()[2].mimeType, 'application/gzip')
        self.assertEqual(m.links()[2].size, '283676')

    def testStandard(self):
        m = self.createTestMetadata()
        self.checkExpectedMetadata(m)

    def testSaveReadFromLayer(self):
        """
        Test saving and reading metadata from a layer
        """
        vl = QgsVectorLayer('Point', 'test', 'memory')
        self.assertTrue(vl.isValid())

        # save metadata to layer
        m = self.createTestMetadata()
        m.saveToLayer(vl)

        # read back from layer and check result
        m2 = QgsLayerMetadata()
        m2.readFromLayer(vl)
        self.checkExpectedMetadata(m2)

    def testSaveReadFromXml(self):
        """
        Test saving and reading metadata from a XML.
        """

        # save metadata to XML
        m = self.createTestMetadata()

        doc = QDomDocument("testdoc")
        elem = doc.createElement("metadata")
        self.assertTrue(m.writeMetadataXml(elem, doc))

        # read back from XML and check result
        m2 = QgsLayerMetadata()
        m2.readMetadataXml(elem)
        self.checkExpectedMetadata(m2)

    def testValidateNative(self):  # spellok
        """
        Test validating metadata against QGIS native schema
        """
        m = self.createTestMetadata()
        v = QgsNativeMetadataValidator()

        res, list = v.validate(m)
        self.assertTrue(res)
        self.assertFalse(list)

        # corrupt metadata piece by piece...
        m = self.createTestMetadata()
        m.setIdentifier('')
        res, list = v.validate(m)
        self.assertFalse(res)
        self.assertEqual(list[0].section, 'identifier')

        m = self.createTestMetadata()
        m.setLanguage('')
        res, list = v.validate(m)
        self.assertFalse(res)
        self.assertEqual(list[0].section, 'language')

        m = self.createTestMetadata()
        m.setType('')
        res, list = v.validate(m)
        self.assertFalse(res)
        self.assertEqual(list[0].section, 'type')

        m = self.createTestMetadata()
        m.setTitle('')
        res, list = v.validate(m)
        self.assertFalse(res)
        self.assertEqual(list[0].section, 'title')

        m = self.createTestMetadata()
        m.setAbstract('')
        res, list = v.validate(m)
        self.assertFalse(res)
        self.assertEqual(list[0].section, 'abstract')

        m = self.createTestMetadata()
        m.setLicenses([])
        res, list = v.validate(m)
        self.assertFalse(res)
        self.assertEqual(list[0].section, 'license')

        m = self.createTestMetadata()
        m.setCrs(QgsCoordinateReferenceSystem())
        res, list = v.validate(m)
        self.assertFalse(res)
        self.assertEqual(list[0].section, 'crs')

        m = self.createTestMetadata()
        m.setContacts([])
        res, list = v.validate(m)
        self.assertFalse(res)
        self.assertEqual(list[0].section, 'contacts')

        m = self.createTestMetadata()
        m.setLinks([])
        res, list = v.validate(m)
        self.assertFalse(res)
        self.assertEqual(list[0].section, 'links')

        m = self.createTestMetadata()
        m.setKeywords({'': ['kw1', 'kw2']})
        res, list = v.validate(m)
        self.assertFalse(res)
        self.assertEqual(list[0].section, 'keywords')
        self.assertEqual(list[0].identifier, 0)

        m = self.createTestMetadata()
        m.setKeywords({'AA': []})
        res, list = v.validate(m)
        self.assertFalse(res)
        self.assertEqual(list[0].section, 'keywords')
        self.assertEqual(list[0].identifier, 0)

        m = self.createTestMetadata()
        e = m.extent()
        se = e.spatialExtents()[0]
        se.extentCrs = QgsCoordinateReferenceSystem()
        e.setSpatialExtents([se])
        m.setExtent(e)
        res, list = v.validate(m)
        self.assertFalse(res)
        self.assertEqual(list[0].section, 'extent')
        self.assertEqual(list[0].identifier, 0)

        m = self.createTestMetadata()
        e = m.extent()
        se = e.spatialExtents()[0]
        se.bounds = QgsBox3d(1, 1, 0, 1, 2)
        e.setSpatialExtents([se])
        m.setExtent(e)
        res, list = v.validate(m)
        self.assertFalse(res)
        self.assertEqual(list[0].section, 'extent')
        self.assertEqual(list[0].identifier, 0)

        m = self.createTestMetadata()
        c = m.contacts()[0]
        c.name = ''
        m.setContacts([c])
        res, list = v.validate(m)
        self.assertFalse(res)
        self.assertEqual(list[0].section, 'contacts')
        self.assertEqual(list[0].identifier, 0)

        m = self.createTestMetadata()
        l = m.links()[0]
        l.name = ''
        m.setLinks([l])
        res, list = v.validate(m)
        self.assertFalse(res)
        self.assertEqual(list[0].section, 'links')
        self.assertEqual(list[0].identifier, 0)

        m = self.createTestMetadata()
        l = m.links()[0]
        l.type = ''
        m.setLinks([l])
        res, list = v.validate(m)
        self.assertFalse(res)
        self.assertEqual(list[0].section, 'links')
        self.assertEqual(list[0].identifier, 0)

        m = self.createTestMetadata()
        l = m.links()[0]
        l.url = ''
        m.setLinks([l])
        res, list = v.validate(m)
        self.assertFalse(res)
        self.assertEqual(list[0].section, 'links')
        self.assertEqual(list[0].identifier, 0)

    def testCombine(self):
        m1 = QgsLayerMetadata()
        m2 = QgsLayerMetadata()

        # should be retained
        m1.setIdentifier('i1')
        m1.combine(m2)
        self.assertEqual(m1.identifier(), 'i1')

        # should be overwritten
        m1.setIdentifier(None)
        m2.setIdentifier('i2')
        m1.combine(m2)
        self.assertEqual(m1.identifier(), 'i2')

        # should be overwritten
        m1.setIdentifier('i1')
        m2.setIdentifier('i2')
        m1.combine(m2)
        self.assertEqual(m1.identifier(), 'i2')

        m1.setParentIdentifier('pi1')
        m2.setParentIdentifier(None)
        m1.combine(m2)
        self.assertEqual(m1.parentIdentifier(), 'pi1')

        m1.setParentIdentifier(None)
        m2.setParentIdentifier('pi2')
        m1.combine(m2)
        self.assertEqual(m1.parentIdentifier(), 'pi2')

        m1.setLanguage('l1')
        m2.setLanguage(None)
        m1.combine(m2)
        self.assertEqual(m1.language(), 'l1')

        m1.setLanguage(None)
        m2.setLanguage('l2')
        m1.combine(m2)
        self.assertEqual(m1.language(), 'l2')

        m1.setType('ty1')
        m2.setType(None)
        m1.combine(m2)
        self.assertEqual(m1.type(), 'ty1')

        m1.setType(None)
        m2.setType('ty2')
        m1.combine(m2)
        self.assertEqual(m1.type(), 'ty2')

        m1.setTitle('t1')
        m2.setTitle(None)
        m1.combine(m2)
        self.assertEqual(m1.title(), 't1')

        m1.setTitle(None)
        m2.setTitle('t2')
        m1.combine(m2)
        self.assertEqual(m1.title(), 't2')

        m1.setAbstract('a1')
        m2.setAbstract(None)
        m1.combine(m2)
        self.assertEqual(m1.abstract(), 'a1')

        m1.setAbstract(None)
        m2.setAbstract('a2')
        m1.combine(m2)
        self.assertEqual(m1.abstract(), 'a2')

        m1.setHistory(['h1', 'hh1'])
        m2.setHistory([])
        m1.combine(m2)
        self.assertEqual(m1.history(), ['h1', 'hh1'])

        m1.setHistory([])
        m2.setHistory(['h2', 'hh2'])
        m1.combine(m2)
        self.assertEqual(m1.history(), ['h2', 'hh2'])

        m1.setKeywords({'words': ['k1', 'kk1']})
        m2.setKeywords({})
        m1.combine(m2)
        self.assertEqual(m1.keywords(), {'words': ['k1', 'kk1']})

        m1.setKeywords({})
        m2.setKeywords({'words': ['k2', 'kk2']})
        m1.combine(m2)
        self.assertEqual(m1.keywords(), {'words': ['k2', 'kk2']})

        m1.setContacts([QgsLayerMetadata.Contact('c1'), QgsLayerMetadata.Contact('cc1')])
        m2.setContacts([])
        m1.combine(m2)
        self.assertEqual(m1.contacts(), [QgsLayerMetadata.Contact('c1'), QgsLayerMetadata.Contact('cc1')])

        m1.setContacts([])
        m2.setContacts([QgsLayerMetadata.Contact('c2'), QgsLayerMetadata.Contact('cc2')])
        m1.combine(m2)
        self.assertEqual(m1.contacts(), [QgsLayerMetadata.Contact('c2'), QgsLayerMetadata.Contact('cc2')])

        m1.setLinks([QgsLayerMetadata.Link('l1'), QgsLayerMetadata.Link('ll1')])
        m2.setLinks([])
        m1.combine(m2)
        self.assertEqual(m1.links(), [QgsLayerMetadata.Link('l1'), QgsLayerMetadata.Link('ll1')])

        m1.setLinks([])
        m2.setLinks([QgsLayerMetadata.Link('l2'), QgsLayerMetadata.Link('ll2')])
        m1.combine(m2)
        self.assertEqual(m1.links(), [QgsLayerMetadata.Link('l2'), QgsLayerMetadata.Link('ll2')])

        m1.setFees('f1')
        m2.setFees(None)
        m1.combine(m2)
        self.assertEqual(m1.fees(), 'f1')

        m1.setFees(None)
        m2.setFees('f2')
        m1.combine(m2)
        self.assertEqual(m1.fees(), 'f2')

        m1.setConstraints([QgsLayerMetadata.Constraint('c1'), QgsLayerMetadata.Constraint('cc1')])
        m2.setConstraints([])
        m1.combine(m2)
        self.assertEqual(m1.constraints(), [QgsLayerMetadata.Constraint('c1'), QgsLayerMetadata.Constraint('cc1')])

        m1.setConstraints([])
        m2.setConstraints([QgsLayerMetadata.Constraint('c2'), QgsLayerMetadata.Constraint('cc2')])
        m1.combine(m2)
        self.assertEqual(m1.constraints(), [QgsLayerMetadata.Constraint('c2'), QgsLayerMetadata.Constraint('cc2')])

        m1.setRights(['r1', 'rr1'])
        m2.setRights([])
        m1.combine(m2)
        self.assertEqual(m1.rights(), ['r1', 'rr1'])

        m1.setRights([])
        m2.setRights(['r2', 'rr2'])
        m1.combine(m2)
        self.assertEqual(m1.rights(), ['r2', 'rr2'])

        m1.setLicenses(['li1', 'lli1'])
        m2.setLicenses([])
        m1.combine(m2)
        self.assertEqual(m1.licenses(), ['li1', 'lli1'])

        m1.setLicenses([])
        m2.setLicenses(['li2', 'lli2'])
        m1.combine(m2)
        self.assertEqual(m1.licenses(), ['li2', 'lli2'])

        m1.setEncoding('e1')
        m2.setEncoding(None)
        m1.combine(m2)
        self.assertEqual(m1.encoding(), 'e1')

        m1.setEncoding(None)
        m2.setEncoding('e2')
        m1.combine(m2)
        self.assertEqual(m1.encoding(), 'e2')

        m1.setCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        m2.setCrs(QgsCoordinateReferenceSystem())
        m1.combine(m2)
        self.assertEqual(m1.crs().authid(), 'EPSG:3111')

        m1.setCrs(QgsCoordinateReferenceSystem())
        m2.setCrs(QgsCoordinateReferenceSystem('EPSG:3113'))
        m1.combine(m2)
        self.assertEqual(m1.crs().authid(), 'EPSG:3113')

        s = QgsLayerMetadata.SpatialExtent()
        s.bounds = QgsBox3d(1, 2, 3, 4, 5, 6)
        m1.extent().setSpatialExtents([s])
        m2.extent().setSpatialExtents([])
        m1.combine(m2)
        self.assertEqual(m1.extent().spatialExtents()[0].bounds, QgsBox3d(1, 2, 3, 4, 5, 6))

        s.bounds = QgsBox3d(11, 12, 13, 14, 15, 16)
        m1.extent().setSpatialExtents([])
        m2.extent().setSpatialExtents([s])
        m1.combine(m2)
        self.assertEqual(m1.extent().spatialExtents()[0].bounds, QgsBox3d(11, 12, 13, 14, 15, 16))

        s = QgsDateTimeRange(QDateTime(2020, 1, 1, 0, 0, 0), QDateTime(2020, 2, 1, 0, 0, 0))
        m1.extent().setTemporalExtents([s])
        m2.extent().setTemporalExtents([])
        m1.combine(m2)
        self.assertEqual(m1.extent().temporalExtents()[0], s)

        s = QgsDateTimeRange(QDateTime(2021, 1, 1, 0, 0, 0), QDateTime(2021, 2, 1, 0, 0, 0))
        m1.extent().setTemporalExtents([])
        m2.extent().setTemporalExtents([s])
        m1.combine(m2)
        self.assertEqual(m1.extent().temporalExtents()[0], s)

    def testContainsAndMatches(self):
        """Test case-insensitive contains"""

        m = self.createTestMetadata()

        self.assertFalse(m.contains('XXXX'))
        self.assertTrue(m.contains('23'))
        relist = [QRegularExpression('XXXX')]
        self.assertFalse(m.matches(relist))
        relist = [QRegularExpression('23')]
        self.assertTrue(m.matches(relist))

        self.assertTrue(m.contains('W1'))
        relist = [QRegularExpression('w1'), QRegularExpression('XXXX')]
        self.assertTrue(m.matches(relist))

        self.assertTrue(m.contains('uRal'))
        relist = [QRegularExpression('ural'), QRegularExpression('XXXX')]
        self.assertTrue(m.matches(relist))

        self.assertTrue(m.contains('My Ro'))
        relist = [QRegularExpression('my ro'), QRegularExpression('XXXX')]
        self.assertTrue(m.matches(relist))

        self.assertFalse(m.contains(''))
        self.assertFalse(m.contains(' '))


if __name__ == '__main__':
    unittest.main()
