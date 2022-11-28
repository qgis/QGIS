# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMetadataWidget.

Run with: ctest -V -R QgsMetadataWidget

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '20/03/2018'
__copyright__ = 'Copyright 2018, The QGIS Project'

import qgis  # NOQA

from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (Qgis,
                       QgsCoordinateReferenceSystem,
                       QgsAbstractMetadataBase,
                       QgsLayerMetadata,
                       QgsProjectMetadata,
                       QgsBox3d,
                       QgsDateTimeRange)
from qgis.gui import (QgsMetadataWidget)
from qgis.PyQt.QtCore import (QDate,
                              QTime,
                              QDateTime)
from qgis.testing import start_app, unittest

start_app()


class TestQgsMetadataWidget(unittest.TestCase):

    def testLayerMode(self):
        """
        Create a fully populated QgsLayerMetadata object, then set it to the widget and re-read back
        the generated metadata to ensure that no content is lost.
        """
        w = QgsMetadataWidget()

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
        # m.setEncoding('utf-8')
        m.setCrs(QgsCoordinateReferenceSystem.fromOgcWmsCrs('EPSG:4326'))

        e = QgsLayerMetadata.Extent()
        se = QgsLayerMetadata.SpatialExtent()
        se.extentCrs = QgsCoordinateReferenceSystem.fromOgcWmsCrs('EPSG:4326')
        se.bounds = QgsBox3d(-180, -90, 0, 180, 90, 0)
        e.setSpatialExtents([se])
        dates = [
            QgsDateTimeRange(
                QDateTime(QDate(2001, 12, 17), QTime(9, 30, 47)),
                QDateTime(QDate(2001, 12, 17), QTime(9, 30, 47)))
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

        m.setDateTime(Qgis.MetadataDateType.Published, QDateTime(QDate(2020, 1, 2), QTime(3, 4, 5)))
        m.setDateTime(Qgis.MetadataDateType.Revised,
                      QDateTime(QDate(2020, 1, 3), QTime(3, 4, 5)))
        m.setDateTime(Qgis.MetadataDateType.Superseded,
                      QDateTime(QDate(2020, 1, 4), QTime(3, 4, 5)))

        # set widget metadata
        w.setMetadata(m)
        self.assertEqual(w.mode(), QgsMetadataWidget.LayerMetadata)

        m = w.metadata()
        self.assertIsInstance(m, QgsLayerMetadata)

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
        # self.assertEqual(m.encoding(), 'utf-8')
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

        self.assertEqual(m.dateTime(Qgis.MetadataDateType.Published), QDateTime(QDate(2020, 1, 2), QTime(3, 4, 5)))
        self.assertEqual(m.dateTime(Qgis.MetadataDateType.Revised),
                         QDateTime(QDate(2020, 1, 3), QTime(3, 4, 5)))
        self.assertEqual(m.dateTime(Qgis.MetadataDateType.Superseded),
                         QDateTime(QDate(2020, 1, 4), QTime(3, 4, 5)))

    def testDates(self):
        """
        Test date handling
        """
        w = QgsMetadataWidget()

        m = QgsLayerMetadata()

        m.setDateTime(Qgis.MetadataDateType.Created,
                      QDateTime(QDate(2020, 1, 2), QTime(3, 4, 5)))
        m.setDateTime(Qgis.MetadataDateType.Superseded,
                      QDateTime(QDate(2020, 1, 4), QTime(3, 4, 5)))

        # set widget metadata
        w.setMetadata(m)

        m = w.metadata()

        self.assertEqual(m.dateTime(Qgis.MetadataDateType.Created),
                         QDateTime(QDate(2020, 1, 2), QTime(3, 4, 5)))
        self.assertEqual(m.dateTime(Qgis.MetadataDateType.Published),
                         QDateTime())
        self.assertEqual(m.dateTime(Qgis.MetadataDateType.Revised),
                         QDateTime())
        self.assertEqual(m.dateTime(Qgis.MetadataDateType.Superseded),
                         QDateTime(QDate(2020, 1, 4), QTime(3, 4, 5)))

        # with project metadata
        w = QgsMetadataWidget()

        m = QgsProjectMetadata()

        m.setDateTime(Qgis.MetadataDateType.Created,
                      QDateTime(QDate(2020, 1, 2), QTime(3, 4, 5)))
        m.setDateTime(Qgis.MetadataDateType.Superseded,
                      QDateTime(QDate(2020, 1, 4), QTime(3, 4, 5)))

        # set widget metadata
        w.setMetadata(m)

        m = w.metadata()

        self.assertEqual(m.dateTime(Qgis.MetadataDateType.Created),
                         QDateTime(QDate(2020, 1, 2), QTime(3, 4, 5)))
        self.assertEqual(m.dateTime(Qgis.MetadataDateType.Published),
                         QDateTime())
        self.assertEqual(m.dateTime(Qgis.MetadataDateType.Revised),
                         QDateTime())
        self.assertEqual(m.dateTime(Qgis.MetadataDateType.Superseded),
                         QDateTime(QDate(2020, 1, 4), QTime(3, 4, 5)))

        w = QgsMetadataWidget()

        m = QgsProjectMetadata()

        m.setDateTime(Qgis.MetadataDateType.Created,
                      QDateTime())
        m.setDateTime(Qgis.MetadataDateType.Superseded,
                      QDateTime(QDate(2020, 1, 4), QTime(3, 4, 5)))

        # set widget metadata
        w.setMetadata(m)

        m = w.metadata()

        self.assertEqual(m.dateTime(Qgis.MetadataDateType.Created),
                         QDateTime())
        self.assertEqual(m.dateTime(Qgis.MetadataDateType.Published),
                         QDateTime())
        self.assertEqual(m.dateTime(Qgis.MetadataDateType.Revised),
                         QDateTime())
        self.assertEqual(m.dateTime(Qgis.MetadataDateType.Superseded),
                         QDateTime(QDate(2020, 1, 4), QTime(3, 4, 5)))

    def testProjectMode(self):
        """
        Create a fully populated QgsProjectMetadata object, then set it to the widget and re-read back
        the generated metadata to ensure that no content is lost.
        """
        w = QgsMetadataWidget()

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

        # set widget metadata
        w.setMetadata(m)
        self.assertEqual(w.mode(), QgsMetadataWidget.ProjectMetadata)

        m = w.metadata()
        self.assertIsInstance(m, QgsProjectMetadata)

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


if __name__ == '__main__':
    unittest.main()
