# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsProjectMetadata.

Run with: ctest -V -R PyQgsProjectMetadata

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '19/03/2018'
__copyright__ = 'Copyright 2018, The QGIS Project'

import qgis  # NOQA

from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (QgsProjectMetadata,
                       QgsAbstractMetadataBase,
                       QgsProject,
                       QgsNativeProjectMetadataValidator)
from qgis.PyQt.QtCore import (QDate,
                              QTime,
                              QDateTime)
from qgis.testing import start_app, unittest
from qgis.PyQt.QtTest import QSignalSpy

start_app()


class TestQgsProjectMetadata(unittest.TestCase):

    def testGettersSetters(self):
        m = QgsProjectMetadata()

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

        m.setHistory(['loaded into QGIS'])
        self.assertEqual(m.history(), ['loaded into QGIS'])
        m.setHistory(['accidentally deleted some features'])
        self.assertEqual(m.history(), ['accidentally deleted some features'])
        m.addHistoryItem('panicked and deleted more')
        self.assertEqual(m.history(), ['accidentally deleted some features', 'panicked and deleted more'])

        m.setAuthor('my author')
        self.assertEqual(m.author(), 'my author')

        m.setCreationDateTime(QDateTime(QDate(2001, 12, 17), QTime(9, 30, 47)))
        self.assertEqual(m.creationDateTime(), QDateTime(QDate(2001, 12, 17), QTime(9, 30, 47)))

    def createTestMetadata(self):
        """
        Returns a standard metadata which can be tested with checkExpectedMetadata
        """
        m = QgsProjectMetadata()
        m.setIdentifier('1234')
        m.setParentIdentifier('xyz')
        m.setLanguage('en-CA')
        m.setType('project')
        m.setTitle('roads')
        m.setAbstract('my roads')
        m.setHistory(['history a', 'history b'])
        m.setKeywords({
            'GEMET': ['kw1', 'kw2'],
            'gmd:topicCategory': ['natural'],
        })

        c = QgsAbstractMetadataBase.Contact()
        c.name = 'John Smith'
        c.organization = 'ACME'
        c.position = 'staff'
        c.voice = '1500 515 555'
        c.fax = 'xx.xxx.xxx.xxxx'
        c.email = 'foo@example.org'
        c.role = 'pointOfContact'
        address = QgsAbstractMetadataBase.Address()
        address.type = 'postal'
        address.address = '123 Main Street'
        address.city = 'anycity'
        address.administrativeArea = 'anyprovince'
        address.postalCode = '90210'
        address.country = 'Canada'
        c.addresses = [address]
        m.setContacts([c])

        l = QgsAbstractMetadataBase.Link()
        l.name = 'geonode:roads'
        l.type = 'OGC:WMS'
        l.description = 'my GeoNode road layer'
        l.url = 'http://example.org/wms'

        l2 = QgsAbstractMetadataBase.Link()
        l2.name = 'geonode:roads'
        l2.type = 'OGC:WFS'
        l2.description = 'my GeoNode road layer'
        l2.url = 'http://example.org/wfs'

        l3 = QgsAbstractMetadataBase.Link()
        l3.name = 'roads'
        l3.type = 'WWW:LINK'
        l3.description = 'full dataset download'
        l3.url = 'http://example.org/roads.tgz'
        l3.format = 'ESRI Shapefile'
        l3.mimeType = 'application/gzip'
        l3.size = '283676'

        m.setLinks([l, l2, l3])

        m.setAuthor('my author')
        m.setCreationDateTime(QDateTime(QDate(2001, 12, 17), QTime(9, 30, 47)))

        return m

    def testEquality(self):
        md = self.createTestMetadata()
        md2 = self.createTestMetadata()

        self.assertEqual(md, md2)
        md2.setAuthor('xx')
        self.assertNotEqual(md, md2)

        md2 = self.createTestMetadata()
        md2.setCreationDateTime(QDateTime(QDate(2003, 12, 17), QTime(9, 30, 47)))
        self.assertNotEqual(md, md2)

    def checkExpectedMetadata(self, m):
        """
        Checks that a metadata object matches that returned by createTestMetadata
        """
        self.assertEqual(m.identifier(), '1234')
        self.assertEqual(m.parentIdentifier(), 'xyz')
        self.assertEqual(m.language(), 'en-CA')
        self.assertEqual(m.type(), 'project')
        self.assertEqual(m.title(), 'roads')
        self.assertEqual(m.abstract(), 'my roads')
        self.assertEqual(m.history(), ['history a', 'history b'])
        self.assertEqual(
            m.keywords(),
            {'GEMET': ['kw1', 'kw2'], 'gmd:topicCategory': ['natural']})

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

        self.assertEqual(m.author(), 'my author')
        self.assertEqual(m.creationDateTime(), QDateTime(QDate(2001, 12, 17), QTime(9, 30, 47)))

    def testStandard(self):
        m = self.createTestMetadata()
        self.checkExpectedMetadata(m)

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
        m2 = QgsProjectMetadata()
        m2.readMetadataXml(elem)
        self.checkExpectedMetadata(m2)

    def testValidateNative(self):  # spellok
        """
        Test validating metadata against QGIS native schema
        """
        m = self.createTestMetadata()
        v = QgsNativeProjectMetadataValidator()

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

        m = self.createTestMetadata()
        m.setAuthor('')
        res, list = v.validate(m)
        self.assertFalse(res)
        self.assertEqual(list[0].section, 'author')

        m = self.createTestMetadata()
        m.setCreationDateTime(QDateTime())
        res, list = v.validate(m)
        self.assertFalse(res)
        self.assertEqual(list[0].section, 'creation')

    def testProject(self):
        p = QgsProject()
        m = self.createTestMetadata()

        metadata_changed_spy = QSignalSpy(p.metadataChanged)
        p.setMetadata(m)
        self.assertEqual(len(metadata_changed_spy), 1)
        self.checkExpectedMetadata(p.metadata())

        p.clear()
        self.assertEqual(len(metadata_changed_spy), 2)
        self.assertEqual(p.metadata().title(), '')

        # test that the project title is just a shortcut to the metadata title field
        p.setTitle('my title')
        self.assertEqual(p.metadata().title(), 'my title')
        m.setTitle('my title 2')
        p.setMetadata(m)
        self.assertEqual(p.title(), 'my title 2')


if __name__ == '__main__':
    unittest.main()
