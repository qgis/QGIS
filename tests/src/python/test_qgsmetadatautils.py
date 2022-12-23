# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMetadataUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2021-04-29'
__copyright__ = 'Copyright 2021, The QGIS Project'

from qgis.PyQt.QtCore import QDateTime
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsMetadataUtils,
    Qgis
)
from qgis.testing import (start_app,
                          unittest,
                          )

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsMetadataUtils(unittest.TestCase):

    def testConvertEsri(self):
        """
        Test ESRI metadata conversion
        """
        src = TEST_DATA_DIR + '/esri_metadata.xml'
        doc = QDomDocument()
        with open(src, 'rt') as f:
            doc.setContent('\n'.join(f.readlines()))

        metadata = QgsMetadataUtils.convertFromEsri(doc)
        self.assertEqual(metadata.title(), 'Baseline roads and tracks Queensland')
        self.assertEqual(metadata.identifier(), 'Baseline_roads_and_tracks')
        self.assertEqual(metadata.abstract(),
                         'This dataset represents street centrelines of Queensland. \n\nTo provide the digital road network of Queensland. \n\nThis is supplementary info')
        self.assertEqual(metadata.language(), 'ENG')
        self.assertEqual(metadata.keywords(), {'Search keys': ['road'], 'gmd:topicCategory': ['TRANSPORTATION Land']})
        self.assertEqual(metadata.categories(), ['TRANSPORTATION Land'])
        self.assertEqual(metadata.extent().temporalExtents()[0].begin(), QDateTime(2016, 6, 28, 0, 0))
        self.assertEqual(metadata.crs().authid(), 'EPSG:4283')
        self.assertEqual(metadata.extent().spatialExtents()[0].bounds.xMinimum(), 137.921721)
        self.assertEqual(metadata.extent().spatialExtents()[0].bounds.xMaximum(), 153.551682)
        self.assertEqual(metadata.extent().spatialExtents()[0].bounds.yMinimum(), -29.177948)
        self.assertEqual(metadata.extent().spatialExtents()[0].bounds.yMaximum(), -9.373145)
        self.assertEqual(metadata.extent().spatialExtents()[0].extentCrs.authid(), 'EPSG:4283')

        self.assertEqual(metadata.dateTime(Qgis.MetadataDateType.Created), QDateTime(2022, 11, 1, 0, 0))
        self.assertEqual(metadata.dateTime(Qgis.MetadataDateType.Published), QDateTime(2016, 6, 28, 0, 0))
        self.assertEqual(metadata.dateTime(Qgis.MetadataDateType.Revised), QDateTime(2022, 11, 5, 0, 0))
        self.assertEqual(metadata.dateTime(Qgis.MetadataDateType.Superseded), QDateTime(2022, 11, 12, 0, 0))

        self.assertEqual(metadata.licenses(), ['This material is licensed under a CC4'])
        self.assertEqual(metadata.rights(), ['The State of Queensland (Department of Natural Resources and Mines)',
                                             '© State of Queensland (Department of Natural Resources and Mines) 2016'])
        self.assertIn('Unrestricted to all levels of government and community.', metadata.constraints()[0].constraint)
        self.assertEqual(metadata.constraints()[0].type, 'Security constraints')
        self.assertIn('Dataset is wholly created and owned by Natural Resources and Mines for the State of Queensland',
                      metadata.constraints()[1].constraint)
        self.assertEqual(metadata.constraints()[1].type, 'Limitations of use')

        self.assertEqual(metadata.links()[0].type, 'Download Service')
        self.assertEqual(metadata.links()[0].name, 'Queensland Spatial Catalog')
        self.assertEqual(metadata.links()[0].url, 'http://qldspatial.information.qld.gov.au/catalog/custom/')

        self.assertEqual(metadata.history(), [
            'The street records of the State Digital Road Network (SDRN) baseline dataset have been generated from road casement boundaries of the QLD Digital Cadastre Database (DCDB)and updated where more accurate source data has been available. Other sources used in the maintenance of the streets dataset include aerial imagery, QLD Department of Transport and Main Roads State Controlled Roads dataset, supplied datasets from other State Government Departments, Local Government data, field work using GPS, and user feedback. ',
            'Data source: Land parcel boundaries Queensland',
            'Data source: QLD Department of Transport and Main Roads State Controlled Roads',
            'Data source: Local Government data', 'Data source: field work',
            'Data source: other State Government Departments'])

        self.assertEqual(metadata.contacts()[0].name, 'Name')
        self.assertEqual(metadata.contacts()[0].email, 'someone@gov.au')
        self.assertEqual(metadata.contacts()[0].voice, '77777777')
        self.assertEqual(metadata.contacts()[0].role, 'Point of contact')
        self.assertEqual(metadata.contacts()[0].organization, 'Department of Natural Resources and Mines')

    def testConvertEsriOld(self):
        """
        Test ESRI metadata conversion of older xml format
        """
        src = TEST_DATA_DIR + '/esri_metadata2.xml'
        doc = QDomDocument()
        with open(src, 'rt') as f:
            doc.setContent('\n'.join(f.readlines()))

        metadata = QgsMetadataUtils.convertFromEsri(doc)
        self.assertEqual(metadata.title(), 'QLD_STRUCTURAL_FRAMEWORK_OUTLINE')
        self.assertEqual(metadata.identifier(), 'QLD_STRUCTURAL_FRAMEWORK_OUTLINE')
        self.assertEqual(metadata.abstract(),
                         'abstract pt 1 \n\npurpose pt 1\n\nsupp info pt 1')
        self.assertEqual(metadata.language(), 'EN')
        self.assertEqual(metadata.keywords(), {'gmd:topicCategory': ['GEOSCIENCES Geology']})
        self.assertEqual(metadata.categories(), ['GEOSCIENCES Geology'])
        self.assertEqual(metadata.extent().temporalExtents()[0].begin(), QDateTime(2012, 7, 1, 0, 0))
        self.assertEqual(metadata.extent().spatialExtents()[0].bounds.xMinimum(), 137.9947)
        self.assertEqual(metadata.extent().spatialExtents()[0].bounds.xMaximum(), 153.55183)
        self.assertEqual(metadata.extent().spatialExtents()[0].bounds.yMinimum(), -29.17849)
        self.assertEqual(metadata.extent().spatialExtents()[0].bounds.yMaximum(), -9.2296)

        self.assertEqual(metadata.rights(), ['Creative Commons Attribution 3.0 Australia (CC BY)'])
        self.assertIn('Unrestricted to all levels of government and community.',
                      metadata.constraints()[0].constraint)
        self.assertEqual(metadata.constraints()[0].type, 'Access')

        self.assertEqual(metadata.links()[0].type, 'Local Area Network')
        self.assertEqual(metadata.links()[0].name, 'Shapefile')
        self.assertEqual(metadata.links()[0].url, 'file://some.shp')

        self.assertEqual(metadata.contacts()[0].name, 'org')
        self.assertEqual(metadata.contacts()[0].email, 'someone@gov.au')
        self.assertEqual(metadata.contacts()[0].voice, '777')
        self.assertEqual(metadata.contacts()[0].role, 'Point of contact')
        self.assertEqual(metadata.contacts()[0].organization, 'org')
        self.assertEqual(metadata.contacts()[0].addresses[0].type, 'mailing address')
        self.assertEqual(metadata.contacts()[0].addresses[0].city, 'BRISBANE CITY EAST')
        self.assertEqual(metadata.contacts()[0].addresses[1].type, 'physical address')
        self.assertEqual(metadata.contacts()[0].addresses[1].city, 'BRISBANE CITY EAST')


if __name__ == '__main__':
    unittest.main()
