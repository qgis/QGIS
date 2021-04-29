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
    QgsMetadataUtils
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
        self.assertEqual(metadata.keywords(), {'gmd:topicCategory': ['TRANSPORTATION Land', 'road']})
        self.assertEqual(metadata.categories(), ['TRANSPORTATION Land', 'road'])
        self.assertEqual(metadata.extent().temporalExtents()[0].begin(), QDateTime(2016, 6, 28, 0, 0))
        self.assertEqual(metadata.crs().authid(), 'EPSG:4283')
        self.assertEqual(metadata.extent().spatialExtents()[0].bounds.xMinimum(), 137.921721)
        self.assertEqual(metadata.extent().spatialExtents()[0].bounds.xMaximum(), 153.551682)
        self.assertEqual(metadata.extent().spatialExtents()[0].bounds.yMinimum(), -29.177948)
        self.assertEqual(metadata.extent().spatialExtents()[0].bounds.yMaximum(), -9.373145)
        self.assertEqual(metadata.extent().spatialExtents()[0].extentCrs.authid(), 'EPSG:4283')

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


if __name__ == '__main__':
    unittest.main()
