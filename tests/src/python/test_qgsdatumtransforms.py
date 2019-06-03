
# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsDatumTransforms.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2019-05-25'
__copyright__ = 'Copyright 2019, The QGIS Project'

from qgis.core import (
    QgsProjUtils,
    QgsCoordinateReferenceSystem,
    QgsDatumTransform
)
from qgis.testing import (start_app,
                          unittest,
                          )
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsDatumTransform(unittest.TestCase):

    @unittest.skipIf(QgsProjUtils.projVersionMajor() < 6, 'Not a proj6 build')
    def testOperations(self):
        ops = QgsDatumTransform.operations(QgsCoordinateReferenceSystem(),
                                           QgsCoordinateReferenceSystem())
        self.assertEqual(ops, [])
        ops = QgsDatumTransform.operations(QgsCoordinateReferenceSystem('EPSG:3111'),
                                           QgsCoordinateReferenceSystem())
        self.assertEqual(ops, [])
        ops = QgsDatumTransform.operations(QgsCoordinateReferenceSystem(),
                                           QgsCoordinateReferenceSystem('EPSG:3111'))
        self.assertEqual(ops, [])

        ops = QgsDatumTransform.operations(QgsCoordinateReferenceSystem('EPSG:3111'),
                                           QgsCoordinateReferenceSystem('EPSG:3111'))
        self.assertEqual(len(ops), 1)
        self.assertTrue(ops[0].name)
        self.assertEqual(ops[0].proj, '+proj=noop')
        self.assertEqual(ops[0].accuracy, 0.0)
        self.assertTrue(ops[0].isAvailable)

        ops = QgsDatumTransform.operations(QgsCoordinateReferenceSystem('EPSG:3111'),
                                           QgsCoordinateReferenceSystem('EPSG:4283'))
        self.assertEqual(len(ops), 1)
        self.assertTrue(ops[0].name)
        self.assertEqual(ops[0].proj, '+proj=pipeline +step +inv +proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80 +step +proj=unitconvert +xy_in=rad +xy_out=deg')
        self.assertEqual(ops[0].accuracy, -1.0)
        self.assertTrue(ops[0].isAvailable)

        ops = QgsDatumTransform.operations(QgsCoordinateReferenceSystem('EPSG:3111'),
                                           QgsCoordinateReferenceSystem('EPSG:28355'))
        self.assertEqual(len(ops), 1)
        self.assertTrue(ops[0].name)
        self.assertEqual(ops[0].proj, '+proj=pipeline +step +inv +proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80 +step +proj=utm +zone=55 +south +ellps=GRS80')
        self.assertEqual(ops[0].accuracy, 0.0)
        self.assertTrue(ops[0].isAvailable)

        # uses a grid file
        ops = QgsDatumTransform.operations(QgsCoordinateReferenceSystem('EPSG:4283'),
                                           QgsCoordinateReferenceSystem('EPSG:7844'))
        self.assertGreaterEqual(len(ops), 5)

        op1_index = [i for i in range(len(ops)) if ops[i].proj == '+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=push +v_3 +step +proj=cart +ellps=GRS80 +step +proj=helmert +x=0.06155 +y=-0.01087 +z=-0.04019 +rx=-0.0394924 +ry=-0.0327221 +rz=-0.0328979 +s=-0.009994 +convention=coordinate_frame +step +inv +proj=cart +ellps=GRS80 +step +proj=pop +v_3 +step +proj=unitconvert +xy_in=rad +xy_out=deg'][0]
        self.assertTrue(ops[op1_index].name)
        self.assertEqual(ops[op1_index].proj, '+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=push +v_3 +step +proj=cart +ellps=GRS80 +step +proj=helmert +x=0.06155 +y=-0.01087 +z=-0.04019 +rx=-0.0394924 +ry=-0.0327221 +rz=-0.0328979 +s=-0.009994 +convention=coordinate_frame +step +inv +proj=cart +ellps=GRS80 +step +proj=pop +v_3 +step +proj=unitconvert +xy_in=rad +xy_out=deg')
        self.assertTrue(ops[op1_index].isAvailable)
        self.assertEqual(ops[op1_index].accuracy, 0.01)
        self.assertEqual(len(ops[op1_index].grids), 0)

        op2_index = [i for i in range(len(ops)) if ops[i].proj == '+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=hgridshift +grids=GDA94_GDA2020_conformal_and_distortion.gsb +step +proj=unitconvert +xy_in=rad +xy_out=deg'][0]
        self.assertTrue(ops[op2_index].name)
        self.assertEqual(ops[op2_index].proj, '+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=hgridshift +grids=GDA94_GDA2020_conformal_and_distortion.gsb +step +proj=unitconvert +xy_in=rad +xy_out=deg')
        self.assertEqual(ops[op2_index].accuracy, 0.05)
        self.assertEqual(len(ops[op2_index].grids), 1)
        self.assertEqual(ops[op2_index].grids[0].shortName, 'GDA94_GDA2020_conformal_and_distortion.gsb')
        self.assertEqual(ops[op2_index].grids[0].fullName, '')
        self.assertEqual(ops[op2_index].grids[0].packageName, 'proj-datumgrid-oceania')
        self.assertEqual(ops[op2_index].grids[0].url, 'https://download.osgeo.org/proj/proj-datumgrid-oceania-latest.zip')
        self.assertTrue(ops[op2_index].grids[0].directDownload)
        self.assertTrue(ops[op2_index].grids[0].openLicense)

        op3_index = [i for i in range(len(ops)) if ops[i].proj == '+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=hgridshift +grids=GDA94_GDA2020_conformal.gsb +step +proj=unitconvert +xy_in=rad +xy_out=deg'][0]
        self.assertTrue(ops[op3_index].name)
        self.assertEqual(ops[op3_index].proj, '+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=hgridshift +grids=GDA94_GDA2020_conformal.gsb +step +proj=unitconvert +xy_in=rad +xy_out=deg')
        self.assertEqual(ops[op3_index].accuracy, 0.05)
        self.assertEqual(len(ops[op3_index].grids), 1)
        self.assertEqual(ops[op3_index].grids[0].shortName, 'GDA94_GDA2020_conformal.gsb')
        self.assertEqual(ops[op3_index].grids[0].fullName, '')
        self.assertEqual(ops[op3_index].grids[0].packageName, 'proj-datumgrid-oceania')
        self.assertEqual(ops[op3_index].grids[0].url, 'https://download.osgeo.org/proj/proj-datumgrid-oceania-latest.zip')
        self.assertTrue(ops[op3_index].grids[0].directDownload)
        self.assertTrue(ops[op3_index].grids[0].openLicense)

        op4_index = [i for i in range(len(ops)) if ops[i].proj == '+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=hgridshift +grids=GDA94_GDA2020_conformal_cocos_island.gsb +step +proj=unitconvert +xy_in=rad +xy_out=deg'][0]
        self.assertTrue(ops[op4_index].name)
        self.assertEqual(ops[op4_index].proj, '+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=hgridshift +grids=GDA94_GDA2020_conformal_cocos_island.gsb +step +proj=unitconvert +xy_in=rad +xy_out=deg')
        self.assertEqual(ops[op4_index].accuracy, 0.05)
        self.assertEqual(len(ops[op4_index].grids), 1)
        self.assertEqual(ops[op4_index].grids[0].shortName, 'GDA94_GDA2020_conformal_cocos_island.gsb')
        self.assertEqual(ops[op4_index].grids[0].fullName, '')

        op5_index = [i for i in range(len(ops)) if ops[i].proj == '+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=hgridshift +grids=GDA94_GDA2020_conformal_christmas_island.gsb +step +proj=unitconvert +xy_in=rad +xy_out=deg'][0]
        self.assertTrue(ops[op5_index].name)
        self.assertEqual(ops[op5_index].proj, '+proj=pipeline +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +proj=hgridshift +grids=GDA94_GDA2020_conformal_christmas_island.gsb +step +proj=unitconvert +xy_in=rad +xy_out=deg')
        self.assertEqual(ops[op5_index].accuracy, 0.05)
        self.assertEqual(len(ops[op5_index].grids), 1)
        self.assertEqual(ops[op5_index].grids[0].shortName, 'GDA94_GDA2020_conformal_christmas_island.gsb')
        self.assertEqual(ops[op5_index].grids[0].fullName, '')

        # uses a pivot datum (technically a proj test, but this will help me sleep at night ;)
        ops = QgsDatumTransform.operations(QgsCoordinateReferenceSystem('EPSG:3111'),
                                           QgsCoordinateReferenceSystem('EPSG:7899'))

        self.assertGreaterEqual(len(ops), 3)

        op1_index = [i for i in range(len(ops)) if ops[i].proj == '+proj=pipeline +step +inv +proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80 +step +proj=push +v_3 +step +proj=cart +ellps=GRS80 +step +proj=helmert +x=0.06155 +y=-0.01087 +z=-0.04019 +rx=-0.0394924 +ry=-0.0327221 +rz=-0.0328979 +s=-0.009994 +convention=coordinate_frame +step +inv +proj=cart +ellps=GRS80 +step +proj=pop +v_3 +step +proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80'][0]
        self.assertTrue(ops[op1_index].name)
        self.assertEqual(ops[op1_index].proj, '+proj=pipeline +step +inv +proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80 +step +proj=push +v_3 +step +proj=cart +ellps=GRS80 +step +proj=helmert +x=0.06155 +y=-0.01087 +z=-0.04019 +rx=-0.0394924 +ry=-0.0327221 +rz=-0.0328979 +s=-0.009994 +convention=coordinate_frame +step +inv +proj=cart +ellps=GRS80 +step +proj=pop +v_3 +step +proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80')
        self.assertTrue(ops[op1_index].isAvailable)
        self.assertEqual(ops[op1_index].accuracy, 0.01)
        self.assertEqual(len(ops[op1_index].grids), 0)

        op2_index = [i for i in range(len(ops)) if ops[i].proj == '+proj=pipeline +step +inv +proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80 +step +proj=hgridshift +grids=GDA94_GDA2020_conformal_and_distortion.gsb +step +proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80'][0]
        self.assertTrue(ops[op2_index].name)
        self.assertEqual(ops[op2_index].proj, '+proj=pipeline +step +inv +proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80 +step +proj=hgridshift +grids=GDA94_GDA2020_conformal_and_distortion.gsb +step +proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80')
        self.assertEqual(ops[op2_index].accuracy, 0.05)
        self.assertEqual(len(ops[op2_index].grids), 1)
        self.assertEqual(ops[op2_index].grids[0].shortName, 'GDA94_GDA2020_conformal_and_distortion.gsb')
        self.assertEqual(ops[op2_index].grids[0].fullName, '')
        self.assertEqual(ops[op2_index].grids[0].packageName, 'proj-datumgrid-oceania')
        self.assertEqual(ops[op2_index].grids[0].url, 'https://download.osgeo.org/proj/proj-datumgrid-oceania-latest.zip')
        self.assertTrue(ops[op2_index].grids[0].directDownload)
        self.assertTrue(ops[op2_index].grids[0].openLicense)

        op3_index = [i for i in range(len(ops)) if ops[i].proj == '+proj=pipeline +step +inv +proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80 +step +proj=hgridshift +grids=GDA94_GDA2020_conformal.gsb +step +proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80'][0]
        self.assertTrue(ops[op3_index].name)
        self.assertEqual(ops[op3_index].proj, '+proj=pipeline +step +inv +proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80 +step +proj=hgridshift +grids=GDA94_GDA2020_conformal.gsb +step +proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80')
        self.assertEqual(ops[op3_index].accuracy, 0.05)
        self.assertEqual(len(ops[op3_index].grids), 1)
        self.assertEqual(ops[op3_index].grids[0].shortName, 'GDA94_GDA2020_conformal.gsb')
        self.assertEqual(ops[op3_index].grids[0].fullName, '')
        self.assertEqual(ops[op3_index].grids[0].packageName, 'proj-datumgrid-oceania')
        self.assertEqual(ops[op3_index].grids[0].url, 'https://download.osgeo.org/proj/proj-datumgrid-oceania-latest.zip')
        self.assertTrue(ops[op3_index].grids[0].directDownload)
        self.assertTrue(ops[op3_index].grids[0].openLicense)


if __name__ == '__main__':
    unittest.main()
