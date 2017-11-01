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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

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
                              QDateTime)
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

    def testKeywords(self):
        m = QgsLayerMetadata()

        m.setKeywords({'gmd:topicCategory': ['natural']})
        self.assertEqual(m.keywords(), {'gmd:topicCategory': ['natural']})
        self.assertEqual(m.categories(), ['natural'])
        self.assertTrue(m.removeKeywords('gmd:topicCategory'))

        m.setKeywords({'vocab a': ['keyword a', 'other a'],
                       'vocab b': ['keyword b', 'other b']})
        self.assertEqual(m.keywords(), {'vocab a': ['keyword a', 'other a'],
                                        'vocab b': ['keyword b', 'other b']})
        self.assertEqual(m.keywordVocabularies(), ['vocab a', 'vocab b'])
        self.assertEqual(m.keywords('vocab a'), ['keyword a', 'other a'])
        self.assertEqual(m.keywords('vocab b'), ['keyword b', 'other b'])
        self.assertEqual(m.keywords('not valid'), [])

        m.addKeywords('vocab c', ['keyword c'])
        self.assertEqual(m.keywords(), {'vocab a': ['keyword a', 'other a'],
                                        'vocab b': ['keyword b', 'other b'],
                                        'vocab c': ['keyword c']})
        # replace existing using addKeywords
        m.addKeywords('vocab c', ['c'])
        self.assertEqual(m.keywords(), {'vocab a': ['keyword a', 'other a'],
                                        'vocab b': ['keyword b', 'other b'],
                                        'vocab c': ['c']})
        # replace existing using setKeywords
        m.setKeywords({'x': ['x'], 'y': ['y']})
        self.assertEqual(m.keywords(), {'x': ['x'],
                                        'y': ['y']})

    def testAddress(self):
        a = QgsLayerMetadata.Address()
        a.type = 'postal'
        a.address = '13 north rd'
        a.city = 'huxleys haven'
        a.administrativeArea = 'land of the queens'
        a.postalCode = '4123'
        a.country = 'straya!'
        self.assertEqual(a.type, 'postal')
        self.assertEqual(a.address, '13 north rd')
        self.assertEqual(a.city, 'huxleys haven')
        self.assertEqual(a.administrativeArea, 'land of the queens')
        self.assertEqual(a.postalCode, '4123')
        self.assertEqual(a.country, 'straya!')

    def testContact(self):
        c = QgsLayerMetadata.Contact()
        c.name = 'Prince Gristle'
        c.organization = 'Bergen co'
        c.position = 'prince'
        c.voice = '1500 515 555'
        c.fax = 'who the f*** still uses fax?'
        c.email = 'limpbiskitrulez69@hotmail.com'
        c.role = 'person to blame when all goes wrong'
        a = QgsLayerMetadata.Address()
        a.type = 'postal'
        a2 = QgsLayerMetadata.Address()
        a2.type = 'street'
        c.addresses = [a, a2]
        self.assertEqual(c.name, 'Prince Gristle')
        self.assertEqual(c.organization, 'Bergen co')
        self.assertEqual(c.position, 'prince')
        self.assertEqual(c.voice, '1500 515 555')
        self.assertEqual(c.fax, 'who the f*** still uses fax?')
        self.assertEqual(c.email, 'limpbiskitrulez69@hotmail.com')
        self.assertEqual(c.role, 'person to blame when all goes wrong')
        self.assertEqual(c.addresses[0].type, 'postal')
        self.assertEqual(c.addresses[1].type, 'street')

        m = QgsLayerMetadata()
        c2 = QgsLayerMetadata.Contact(c)
        c2.name = 'Bridgette'

        m.setContacts([c, c2])
        self.assertEqual(m.contacts()[0].name, 'Prince Gristle')
        self.assertEqual(m.contacts()[1].name, 'Bridgette')

        # add contact
        c3 = QgsLayerMetadata.Contact(c)
        c3.name = 'Princess Poppy'
        m.addContact(c3)
        self.assertEqual(len(m.contacts()), 3)
        self.assertEqual(m.contacts()[2].name, 'Princess Poppy')

    def testLinks(self):
        l = QgsLayerMetadata.Link()
        l.name = 'Trashbat'
        l.type = 'fashion'
        l.description = 'registered in the cook islands!'
        l.url = 'http://trashbat.co.uk'
        l.format = 'whois'
        l.mimeType = 'text/string'
        l.size = '112'
        self.assertEqual(l.name, 'Trashbat')
        self.assertEqual(l.type, 'fashion')
        self.assertEqual(l.description, 'registered in the cook islands!')
        self.assertEqual(l.url, 'http://trashbat.co.uk')
        self.assertEqual(l.format, 'whois')
        self.assertEqual(l.mimeType, 'text/string')
        self.assertEqual(l.size, '112')

        m = QgsLayerMetadata()
        l2 = QgsLayerMetadata.Link(l)
        l2.name = 'Trashbat2'

        m.setLinks([l, l2])
        self.assertEqual(m.links()[0].name, 'Trashbat')
        self.assertEqual(m.links()[1].name, 'Trashbat2')

        # add link
        l3 = QgsLayerMetadata.Link(l)
        l3.name = 'Trashbat3'
        m.addLink(l3)
        self.assertEqual(len(m.links()), 3)
        self.assertEqual(m.links()[2].name, 'Trashbat3')

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
        vl = QgsVectorLayer('Point', 'test', 'memory')
        self.assertTrue(vl.isValid())

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


if __name__ == '__main__':
    unittest.main()
