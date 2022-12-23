# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMetadataBase.

Run with: ctest -V -R QgsMetadataBase

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

from qgis.core import (QgsAbstractMetadataBase,
                       QgsCoordinateReferenceSystem,
                       QgsVectorLayer,
                       QgsNativeMetadataBaseValidator,
                       QgsBox3d,
                       QgsDateTimeRange,
                       Qgis)
from qgis.PyQt.QtCore import (QDate,
                              QTime,
                              QDateTime)
from qgis.testing import start_app, unittest

start_app()


class TestMetadata(QgsAbstractMetadataBase):
    pass


class TestQgsMetadataBase(unittest.TestCase):

    def testGettersSetters(self):
        m = TestMetadata()

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

        m.setDateTime(Qgis.MetadataDateType.Published, QDateTime(QDate(2022, 1, 2), QTime(12, 13, 14)))
        self.assertEqual(m.dateTime(Qgis.MetadataDateType.Published), QDateTime(QDate(2022, 1, 2), QTime(12, 13, 14)))
        self.assertEqual(m.dateTime(Qgis.MetadataDateType.Created),
                         QDateTime())
        m.setDateTime(Qgis.MetadataDateType.Created,
                      QDateTime(QDate(2020, 1, 2), QTime(12, 13, 14)))
        self.assertEqual(m.dateTime(Qgis.MetadataDateType.Published), QDateTime(QDate(2022, 1, 2), QTime(12, 13, 14)))
        self.assertEqual(m.dateTime(Qgis.MetadataDateType.Created),
                         QDateTime(QDate(2020, 1, 2), QTime(12, 13, 14)))

    def testEquality(self):
        a = QgsAbstractMetadataBase.Address()
        a.type = 'postal'
        a.address = '13 north rd'
        a.city = 'huxleys haven'
        a.administrativeArea = 'land of the queens'
        a.postalCode = '4123'
        a.country = 'straya!'
        a2 = QgsAbstractMetadataBase.Address(a)
        self.assertEqual(a, a2)
        a2.type = 'postal2'
        self.assertNotEqual(a, a2)
        a2 = QgsAbstractMetadataBase.Address(a)
        a2.address = 'address2'
        self.assertNotEqual(a, a2)
        a2 = QgsAbstractMetadataBase.Address(a)
        a2.city = 'city'
        self.assertNotEqual(a, a2)
        a2 = QgsAbstractMetadataBase.Address(a)
        a2.administrativeArea = 'area2'
        self.assertNotEqual(a, a2)
        a2 = QgsAbstractMetadataBase.Address(a)
        a2.postalCode = 'postal2'
        self.assertNotEqual(a, a2)
        a2 = QgsAbstractMetadataBase.Address(a)
        a2.country = 'country2'
        self.assertNotEqual(a, a2)

        c = QgsAbstractMetadataBase.Contact()
        c.name = 'name'
        c.organization = 'org'
        c.position = 'pos'
        c.voice = '1500 515 555'
        c.fax = 'fax'
        c.email = 'email'
        c.role = 'role'
        a = QgsAbstractMetadataBase.Address()
        a.type = 'postal'
        a2 = QgsAbstractMetadataBase.Address()
        a2.type = 'street'
        c.addresses = [a, a2]
        c2 = QgsAbstractMetadataBase.Contact(c)
        self.assertEqual(c, c2)
        c2.name = 'name2'
        self.assertNotEqual(c, c2)
        c2 = QgsAbstractMetadataBase.Contact(c)
        c2.organization = 'org2'
        self.assertNotEqual(c, c2)
        c2 = QgsAbstractMetadataBase.Contact(c)
        c2.position = 'pos2'
        self.assertNotEqual(c, c2)
        c2 = QgsAbstractMetadataBase.Contact(c)
        c2.voice = 'voice2'
        self.assertNotEqual(c, c2)
        c2 = QgsAbstractMetadataBase.Contact(c)
        c2.fax = 'fax2'
        self.assertNotEqual(c, c2)
        c2 = QgsAbstractMetadataBase.Contact(c)
        c2.email = 'email2'
        self.assertNotEqual(c, c2)
        c2 = QgsAbstractMetadataBase.Contact(c)
        c2.role = 'role2'
        self.assertNotEqual(c, c2)
        c2 = QgsAbstractMetadataBase.Contact(c)
        c2.addresses = [a2]
        self.assertNotEqual(c, c2)

        # link
        l = QgsAbstractMetadataBase.Link()
        l.name = 'name'
        l.type = 'type'
        l.description = 'desc'
        l.url = 'url'
        l.format = 'format'
        l.mimeType = 'mime'
        l.size = '112'
        l2 = QgsAbstractMetadataBase.Link(l)
        self.assertEqual(l, l2)
        l2 = QgsAbstractMetadataBase.Link(l)
        l2.name = 'name2'
        self.assertNotEqual(l, l2)
        l2 = QgsAbstractMetadataBase.Link(l)
        l2.type = 'type2'
        self.assertNotEqual(l, l2)
        l2 = QgsAbstractMetadataBase.Link(l)
        l2.description = 'desc2'
        self.assertNotEqual(l, l2)
        l2 = QgsAbstractMetadataBase.Link(l)
        l2.url = 'url2'
        self.assertNotEqual(l, l2)
        l2 = QgsAbstractMetadataBase.Link(l)
        l2.format = 'format2'
        self.assertNotEqual(l, l2)
        l2 = QgsAbstractMetadataBase.Link(l)
        l2.mimeType = 'mime2'
        self.assertNotEqual(l, l2)
        l2 = QgsAbstractMetadataBase.Link(l)
        l2.size = '113'
        self.assertNotEqual(l, l2)

    def testKeywords(self):
        m = TestMetadata()

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
        a = QgsAbstractMetadataBase.Address()
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
        c = QgsAbstractMetadataBase.Contact()
        c.name = 'Prince Gristle'
        c.organization = 'Bergen co'
        c.position = 'prince'
        c.voice = '1500 515 555'
        c.fax = 'who the f*** still uses fax?'
        c.email = 'limpbiskitrulez69@hotmail.com'
        c.role = 'person to blame when all goes wrong'
        a = QgsAbstractMetadataBase.Address()
        a.type = 'postal'
        a2 = QgsAbstractMetadataBase.Address()
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

        m = TestMetadata()
        c2 = QgsAbstractMetadataBase.Contact(c)
        c2.name = 'Bridgette'

        m.setContacts([c, c2])
        self.assertEqual(m.contacts()[0].name, 'Prince Gristle')
        self.assertEqual(m.contacts()[1].name, 'Bridgette')

        # add contact
        c3 = QgsAbstractMetadataBase.Contact(c)
        c3.name = 'Princess Poppy'
        m.addContact(c3)
        self.assertEqual(len(m.contacts()), 3)
        self.assertEqual(m.contacts()[2].name, 'Princess Poppy')

    def testLinks(self):
        l = QgsAbstractMetadataBase.Link()
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

        m = TestMetadata()
        l2 = QgsAbstractMetadataBase.Link(l)
        l2.name = 'Trashbat2'

        m.setLinks([l, l2])
        self.assertEqual(m.links()[0].name, 'Trashbat')
        self.assertEqual(m.links()[1].name, 'Trashbat2')

        # add link
        l3 = QgsAbstractMetadataBase.Link(l)
        l3.name = 'Trashbat3'
        m.addLink(l3)
        self.assertEqual(len(m.links()), 3)
        self.assertEqual(m.links()[2].name, 'Trashbat3')

    def createTestMetadata(self):
        """
        Returns a standard metadata which can be tested with checkExpectedMetadata
        """
        m = TestMetadata()
        m.setIdentifier('1234')
        m.setParentIdentifier('xyz')
        m.setLanguage('en-CA')
        m.setType('dataset')
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

        m.setDateTime(Qgis.MetadataDateType.Created, QDateTime(QDate(2020, 1, 2), QTime(11, 12, 13)))
        m.setDateTime(Qgis.MetadataDateType.Published,
                      QDateTime(QDate(2020, 1, 3), QTime(11, 12, 13)))
        m.setDateTime(Qgis.MetadataDateType.Revised,
                      QDateTime(QDate(2020, 1, 4), QTime(11, 12, 13)))
        m.setDateTime(Qgis.MetadataDateType.Superseded,
                      QDateTime(QDate(2020, 1, 5), QTime(11, 12, 13)))

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

        self.assertEqual(m.dateTime(Qgis.MetadataDateType.Created), QDateTime(QDate(2020, 1, 2), QTime(11, 12, 13)))
        self.assertEqual(m.dateTime(Qgis.MetadataDateType.Published),
                         QDateTime(QDate(2020, 1, 3), QTime(11, 12, 13)))
        self.assertEqual(m.dateTime(Qgis.MetadataDateType.Revised),
                         QDateTime(QDate(2020, 1, 4), QTime(11, 12, 13)))
        self.assertEqual(m.dateTime(Qgis.MetadataDateType.Superseded),
                         QDateTime(QDate(2020, 1, 5), QTime(11, 12, 13)))

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
        m2 = TestMetadata()
        m2.readMetadataXml(elem)
        self.checkExpectedMetadata(m2)

    def testValidateNative(self):  # spellok
        """
        Test validating metadata against QGIS native schema
        """
        m = self.createTestMetadata()
        v = QgsNativeMetadataBaseValidator()

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

    def testCombine(self):
        m1 = TestMetadata()
        m2 = TestMetadata()

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

        m1.setContacts([QgsAbstractMetadataBase.Contact('c1'), QgsAbstractMetadataBase.Contact('cc1')])
        m2.setContacts([])
        m1.combine(m2)
        self.assertEqual(m1.contacts(), [QgsAbstractMetadataBase.Contact('c1'), QgsAbstractMetadataBase.Contact('cc1')])

        m1.setContacts([])
        m2.setContacts([QgsAbstractMetadataBase.Contact('c2'), QgsAbstractMetadataBase.Contact('cc2')])
        m1.combine(m2)
        self.assertEqual(m1.contacts(), [QgsAbstractMetadataBase.Contact('c2'), QgsAbstractMetadataBase.Contact('cc2')])

        m1.setLinks([QgsAbstractMetadataBase.Link('l1'), QgsAbstractMetadataBase.Link('ll1')])
        m2.setLinks([])
        m1.combine(m2)
        self.assertEqual(m1.links(), [QgsAbstractMetadataBase.Link('l1'), QgsAbstractMetadataBase.Link('ll1')])

        m1.setLinks([])
        m2.setLinks([QgsAbstractMetadataBase.Link('l2'), QgsAbstractMetadataBase.Link('ll2')])
        m1.combine(m2)
        self.assertEqual(m1.links(), [QgsAbstractMetadataBase.Link('l2'), QgsAbstractMetadataBase.Link('ll2')])

        m1.setDateTime(Qgis.MetadataDateType.Created, QDateTime(QDate(2020, 1, 2), QTime(1, 2, 3)))
        m1.setDateTime(Qgis.MetadataDateType.Revised, QDateTime(QDate(2020, 1, 3), QTime(1, 2, 3)))

        m2.setDateTime(Qgis.MetadataDateType.Revised, QDateTime(QDate(2020, 1, 4), QTime(1, 2, 3)))
        m2.setDateTime(Qgis.MetadataDateType.Superseded, QDateTime(QDate(2020, 1, 5), QTime(1, 2, 3)))
        m1.combine(m2)

        self.assertEqual(m1.dateTime(Qgis.MetadataDateType.Created), QDateTime(QDate(2020, 1, 2), QTime(1, 2, 3)))
        self.assertEqual(m1.dateTime(Qgis.MetadataDateType.Revised),
                         QDateTime(QDate(2020, 1, 4), QTime(1, 2, 3)))
        self.assertEqual(m1.dateTime(Qgis.MetadataDateType.Superseded),
                         QDateTime(QDate(2020, 1, 5), QTime(1, 2, 3)))


if __name__ == '__main__':
    unittest.main()
