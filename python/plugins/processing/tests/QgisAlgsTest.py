# -*- coding: utf-8 -*-

"""
***************************************************************************
    QgisAlgsTest.py
    ---------------------
    Date                 : March 2013
    Copyright            : (C) 2013 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'March 2013'
__copyright__ = '(C) 2013, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import processing
import unittest
from processing.tests.TestData import points, points2, polygons, polygons2, lines, union,\
    table
from processing.tools import dataobjects

class QgisAlgsTest(unittest.TestCase):

    def test_qgiscountpointsinpolygon(self):
        outputs=processing.runalg("qgis:countpointsinpolygon",polygons(),points(),"NUMPOINTS",None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','POLY_NUM_A','POLY_ST_A','NUMPOINTS']
        expectedtypes=['Integer','Real','String','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(2, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","string a","6.0"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)

    def test_qgiscountpointsinpolygonweighted(self):
        outputs=processing.runalg("qgis:countpointsinpolygonweighted",polygons(),points(),"PT_NUM_A","NUMPOINTS",None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','POLY_NUM_A','POLY_ST_A','NUMPOINTS']
        expectedtypes=['Integer','Real','String','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(2, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","string a","48.4"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)

    def test_qgiscountuniquepointsinpolygon(self):
        outputs=processing.runalg("qgis:countuniquepointsinpolygon", polygons(),points(),"PT_ST_A","NUMPOINTS",None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','POLY_NUM_A','POLY_ST_A','NUMPOINTS']
        expectedtypes=['Integer','Real','String','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(2, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","string a","3.0"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)

    def test_qgisdistancetonearesthub(self):
        outputs=processing.runalg("qgis:distancetonearesthub",points(),points2(),"ID",1,0,None)
        output=outputs['SAVENAME']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','PT_NUM_A','PT_ST_A','HubName','HubDist']
        expectedtypes=['Integer','Real','String','String','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(12, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","a","8","16.4497544108"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)

    def test_qgismeancoordinates(self):
        outputs=processing.runalg("qgis:meancoordinates",union(),"POLY_NUM_A","ID_2",None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['MEAN_X','MEAN_Y','UID']
        expectedtypes=['Real','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(3, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["270814.229197286","4458944.20935905","0"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)

    def test_qgissumlinelengths(self):
        outputs=processing.runalg("qgis:sumlinelengths",lines(),polygons(),"LENGTH","COUNT",None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','POLY_NUM_A','POLY_ST_A','LENGTH','COUNT']
        expectedtypes=['Integer','Real','String','Real','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(2, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","string a","56.4157223602428","1"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)

    def test_qgiscreategrid(self):
        outputs=processing.runalg("qgis:creategrid",10,10,360,180,0,0,0,None)
        output=outputs['SAVENAME']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['longitude','latitude']
        expectedtypes=['Real','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(56, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["-180.0","0.0"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='LINESTRING(-180.0 -90.0, -180.0 -80.0, -180.0 -70.0, -180.0 -60.0, -180.0 -50.0, -180.0 -40.0, -180.0 -30.0, -180.0 -20.0, -180.0 -10.0, -180.0 0.0, -180.0 10.0, -180.0 20.0, -180.0 30.0, -180.0 40.0, -180.0 50.0, -180.0 60.0, -180.0 70.0, -180.0 80.0, -180.0 90.0)'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgiscreategridnointeger(self):
        outputs=processing.runalg("qgis:creategrid",0.1,0.1,1,1,0,0,0,None)
        output=outputs['SAVENAME']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['longitude','latitude']
        expectedtypes=['Real','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(22, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["-0.5","0.0"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='LINESTRING(-0.5 -0.5, -0.5 -0.4, -0.5 -0.3, -0.5 -0.2, -0.5 -0.1, -0.5 -0.0, -0.5 0.1, -0.5 0.2, -0.5 0.3, -0.5 0.4, -0.5 0.5)'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgiscreategridhex(self):
        outputs=processing.runalg("qgis:creategrid",10,10,360,180,0,0,3,None)
        output=outputs['SAVENAME']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['longitude','latitude']
        expectedtypes=['Real','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(718, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["-174.226497308","-85.0"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POLYGON((-168.45299462 -85.0,-171.33974596 -90.0,-177.11324865 -90.0,-180.0 -85.0,-177.11324865 -80.0,-171.33974596 -80.0,-168.45299462 -85.0))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgispointslayerfromtable(self):
        outputs=processing.runalg("qgis:pointslayerfromtable",table(),"NUM_A","NUM_A",None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','NUM_A','ST_A']
        expectedtypes=['Integer','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(8, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","string a"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POINT(1.1 1.1)'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgisreprojectlayer(self):
        outputs=processing.runalg("qgis:reprojectlayer",polygons(),"EPSG:4326",None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','POLY_NUM_A','POLY_ST_A']
        expectedtypes=['Integer','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(2, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","string a"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POLYGON((-5.69622817 40.24813465,-5.69599713 40.24815018,-5.69590662 40.24814891,-5.69590662 40.24814891,-5.69571264 40.24809764,-5.69567054 40.24798198,-5.69588577 40.2474425,-5.69609238 40.24743402,-5.69634148 40.24749916,-5.69628781 40.24802507,-5.69622817 40.24813465))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgispolygonfromlayerextent(self):
        outputs=processing.runalg("qgis:polygonfromlayerextent",polygons(),False,None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['MINX','MINY','MAXX','MAXY','CNTX','CNTY','AREA','PERIM','HEIGHT','WIDTH']
        expectedtypes=['Real','Real','Real','Real','Real','Real','Real','Real','Real','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(1, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["270759.848331309","4458902.27146471","270870.435817579","4458995.73740534","270815.142074444","4458949.00443503","10336.1634267741","408.106853811652","93.4659406356514","110.587486270175"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POLYGON((270759.84833131 4458902.27146471,270759.84833131 4458995.73740534,270870.43581758 4458995.73740534,270870.43581758 4458902.27146471,270759.84833131 4458902.27146471))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgispolygonfromlayerextentindividual(self):
        outputs=processing.runalg("qgis:polygonfromlayerextent",polygons(),True,None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['MINX','MINY','MAXX','MAXY','CNTX','CNTY','AREA','PERIM','HEIGHT','WIDTH']
        expectedtypes=['Real','Real','Real','Real','Real','Real','Real','Real','Real','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(2, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["270759.848331309","4458914.21983449","270818.553954039","4458993.47958869","270789.201142674","4458953.84971159","4652.99322754798","275.930753853521","79.2597541967407","58.70562273002"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POLYGON((270759.84833131 4458914.21983449,270759.84833131 4458993.47958869,270818.55395404 4458993.47958869,270818.55395404 4458914.21983449,270759.84833131 4458914.21983449))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgisconvexhull(self):
        outputs=processing.runalg("qgis:convexhull",points(),"ID",0,None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['id','value','area','perim']
        expectedtypes=['Integer','String','Real','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(1, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["0","all","3592.818848","230.989919"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POLYGON((270839.46818665 4458921.97813894,270778.60197966 4458935.96883677,270786.54279065 4458980.04784113,270803.15756434 4458983.84880322,270839.65586926 4458983.16267036,270855.74530134 4458940.79948673,270839.46818665 4458921.97813894))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgisconvexhullindividual(self):
        outputs=processing.runalg("qgis:convexhull",points(),"PT_ST_A",1,None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['id','value','area','perim']
        expectedtypes=['Integer','String','Real','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(2, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["0","a","1800.305054","178.087389"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POLYGON((270855.74530134 4458940.79948673,270824.16637636 4458946.78425,270793.142411 4458952.93170025,270803.15756434 4458983.84880322,270839.65586926 4458983.16267036,270855.74530134 4458940.79948673))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgisdelaunaytriangulation(self):
        outputs=processing.runalg("qgis:delaunaytriangulation",points(),None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['POINTA','POINTB','POINTC']
        expectedtypes=['Real','Real','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(16, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["7.0","1.0","2.0"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POLYGON((270778.60197966 4458935.96883677,270799.11642513 4458934.55287392,270839.46818665 4458921.97813894,270778.60197966 4458935.96883677))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgisconvertgeometrytypepolyg(self):
        outputs=processing.runalg("qgis:convertgeometrytype",lines(),4,None)
        output=outputs['SAVENAME']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','LINE_NUM_A','LINE_ST_A']
        expectedtypes=['Integer','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(0, len(features))

    def test_qgisconvertgeometrytypemultiline(self):
        outputs=processing.runalg("qgis:convertgeometrytype",polygons(),3,None)
        output=outputs['SAVENAME']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','POLY_NUM_A','POLY_ST_A']
        expectedtypes=['Integer','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(2, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","string a"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='LINESTRING(270771.63330111 4458992.35349302, 270791.33997534 4458993.47958869, 270799.03496242 4458993.10422346, 270799.03496242 4458993.10422346, 270815.36334964 4458986.91069727, 270818.55395404 4458973.96059707, 270798.42294527 4458914.62661676, 270780.81854858 4458914.21983449, 270759.84833131 4458922.09614031, 270766.19050537 4458980.34180587, 270771.63330111 4458992.35349302)'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgisconvertgeometrytypenodes(self):
        outputs=processing.runalg("qgis:convertgeometrytype",polygons(),1,None)
        output=outputs['SAVENAME']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','POLY_NUM_A','POLY_ST_A']
        expectedtypes=['Integer','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(20, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","string a"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POINT(270771.63330111 4458992.35349302)'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgissnappointstogrid(self):
        outputs=processing.runalg("qgis:snappointstogrid",points(),5,5,None)
        output=outputs['SAVENAME']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','PT_NUM_A','PT_ST_A']
        expectedtypes=['Integer','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(12, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","a"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POINT(270840.0 4458985.0)'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))


    def test_qgisdensifygeometriesgivenaninterval(self):
        outputs=processing.runalg("qgis:densifygeometriesgivenaninterval",lines(),25,None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','LINE_NUM_A','LINE_ST_A']
        expectedtypes=['Integer','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(3, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","11.1","string a"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='LINESTRING(270818.44773413 4458997.23886624, 270833.27466046 4458983.16267036, 270830.83478651 4458975.28000067, 270822.38906898 4458967.20964836, 270823.32748204 4458959.70234389, 270822.7644342 4458958.01320039)'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgisdensifygeometries(self):
        outputs=processing.runalg("qgis:densifygeometries",lines(),5,None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','LINE_NUM_A','LINE_ST_A']
        expectedtypes=['Integer','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(3, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","11.1","string a"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='LINESTRING(270818.44773413 4458997.23886624, 270820.91888852 4458994.89283359, 270823.39004291 4458992.54680094, 270825.8611973 4458990.2007683, 270828.33235168 4458987.85473565, 270830.80350607 4458985.508703, 270833.27466046 4458983.16267036, 270832.8680148 4458981.84889208, 270832.46136914 4458980.53511379, 270832.05472348 4458979.22133551, 270831.64807782 4458977.90755723, 270831.24143217 4458976.59377895, 270830.83478651 4458975.28000067, 270829.42716692 4458973.93494195, 270828.01954733 4458972.58988323, 270826.61192774 4458971.24482451, 270825.20430816 4458969.8997658, 270823.79668857 4458968.55470708, 270822.38906898 4458967.20964836, 270822.54547116 4458965.95843095, 270822.70187333 4458964.70721354, 270822.85827551 4458963.45599613, 270823.01467769 4458962.20477872, 270823.17107986 4458960.95356131, 270823.32748204 4458959.70234389, 270823.23364073 4458959.42081998, 270823.13979943 4458959.13929606, 270823.04595812 4458958.85777214, 270822.95211681 4458958.57624822, 270822.85827551 4458958.29472431, 270822.7644342 4458958.01320039)'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgisexplodelines(self):
        outputs=processing.runalg("qgis:explodelines",lines(),None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','LINE_NUM_A','LINE_ST_A']
        expectedtypes=['Integer','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(25, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","11.1","string a"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='LINESTRING(270818.44773413 4458997.23886624, 270833.27466046 4458983.16267036)'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgisextractnodes(self):
        outputs=processing.runalg("qgis:extractnodes",lines(),None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','LINE_NUM_A','LINE_ST_A']
        expectedtypes=['Integer','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(28, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","11.1","string a"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POINT(270818.44773413 4458997.23886624)'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgisfixeddistancebuffer(self):
        outputs=processing.runalg("qgis:fixeddistancebuffer",points(),10,5,False,None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','PT_NUM_A','PT_ST_A']
        expectedtypes=['Integer','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(12, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","a"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POLYGON((270849.65586926 4458983.16267036,270849.16643442 4458980.07250041,270847.7460392 4458977.28481783,270845.53372178 4458975.07250041,270842.7460392 4458973.65210519,270839.65586926 4458973.16267036,270836.56569931 4458973.65210519,270833.77801673 4458975.07250041,270831.56569931 4458977.28481783,270830.14530409 4458980.07250041,270829.65586926 4458983.16267036,270830.14530409 4458986.2528403,270831.56569931 4458989.04052288,270833.77801673 4458991.2528403,270836.56569931 4458992.67323552,270839.65586926 4458993.16267036,270842.7460392 4458992.67323552,270845.53372178 4458991.2528403,270847.7460392 4458989.04052288,270849.16643442 4458986.2528403,270849.65586926 4458983.16267036))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgisfixeddistancebufferdissolve(self):
        outputs=processing.runalg("qgis:fixeddistancebuffer",points(),10,5,True,None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','PT_NUM_A','PT_ST_A']
        expectedtypes=['Integer','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(1, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["12","13.2","b"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='MULTIPOLYGON(((270849.65586926 4458983.16267036,270849.16643442 4458980.07250041,270847.7460392 4458977.28481783,270847.45014495 4458976.98892358,270848.50492607 4458976.82186266,270851.29260865 4458975.40146744,270853.50492607 4458973.18915002,270854.92532129 4458970.40146744,270855.41475612 4458967.3112975,270854.92532129 4458964.22112756,270853.50492607 4458961.43344498,270851.29260865 4458959.22112756,270848.50492607 4458957.80073234,270845.41475612 4458957.3112975,270842.32458618 4458957.80073234,270839.5369036 4458959.22112756,270837.32458618 4458961.43344498,270835.90419096 4458964.22112756,270835.41475612 4458967.3112975,270835.90419096 4458970.40146744,270837.32458618 4458973.18915002,270837.62048043 4458973.48504428,270836.56569931 4458973.65210519,270833.77801673 4458975.07250041,270831.56569931 4458977.28481783,270831.03129321 4458978.33364887,270830.67495367 4458976.08380958,270829.25455845 4458973.296127,270827.04224103 4458971.08380958,270824.25455845 4458969.66341436,270821.16438851 4458969.17397952,270818.07421856 4458969.66341436,270817.38801377 4458970.01305316,270818.54620806 4458967.73996887,270819.0356429 4458964.64979893,270818.54620806 4458961.55962899,270817.12581284 4458958.77194641,270814.91349542 4458956.55962899,270812.12581284 4458955.13923377,270809.0356429 4458954.64979893,270805.94547296 4458955.13923377,270803.15779038 4458956.55962899,270801.56990778 4458958.14751159,270802.65297616 4458956.0218702,270803.142411 4458952.93170025,270802.65297616 4458949.84153031,270801.23258094 4458947.05384773,270799.02026352 4458944.84153031,270798.15483659 4458944.40057326,270799.11642513 4458944.55287392,270802.20659507 4458944.06343909,270804.99427765 4458942.64304387,270807.20659507 4458940.43072645,270808.62699029 4458937.64304387,270809.11642513 4458934.55287392,270808.62699029 4458931.46270398,270807.20659507 4458928.6750214,270804.99427765 4458926.46270398,270802.20659507 4458925.04230876,270799.11642513 4458924.55287392,270796.02625518 4458925.04230876,270793.2385726 4458926.46270398,270791.02625518 4458928.6750214,270789.60585996 4458931.46270398,270789.11642513 4458934.55287392,270789.60585996 4458937.64304387,270791.02625518 4458940.43072645,270793.2385726 4458942.64304387,270794.10399954 4458943.08400091,270793.142411 4458942.93170025,270790.05224106 4458943.42113509,270787.26455848 4458944.84153031,270785.05224106 4458947.05384773,270783.63184584 4458949.84153031,270783.142411 4458952.93170025,270783.63184584 4458956.0218702,270785.05224106 4458958.80955277,270787.26455848 4458961.0218702,270790.05224106 4458962.44226541,270793.142411 4458962.93170025,270796.23258094 4458962.44226541,270799.02026352 4458961.0218702,270800.60814612 4458959.4339876,270799.52507774 4458961.55962899,270799.0356429 4458964.64979893,270799.52507774 4458967.73996887,270800.94547296 4458970.52765145,270803.15779038 4458972.73996887,270805.94547296 4458974.16036409,270809.0356429 4458974.64979893,270812.12581284 4458974.16036409,270812.81201764 4458973.81072529,270811.65382334 4458976.08380958,270811.32950986 4458978.13144431,270811.24773428 4458977.9709507,270809.03541686 4458975.75863327,270806.24773428 4458974.33823806,270803.15756434 4458973.84880322,270800.06739439 4458974.33823806,270797.27971182 4458975.75863327,270796.05709098 4458976.98125411,270796.05335581 4458976.95767119,270794.63296059 4458974.16998861,270792.42064317 4458971.95767119,270789.63296059 4458970.53727597,270786.54279065 4458970.04784113,270783.4526207 4458970.53727597,270780.66493812 4458971.95767119,270778.4526207 4458974.16998861,270777.03222548 4458976.95767119,270776.54279065 4458980.04784113,270777.03222548 4458983.13801107,270778.4526207 4458985.92569365,270780.66493812 4458988.13801107,270783.4526207 4458989.55840629,270786.54279065 4458990.04784113,270789.63296059 4458989.55840629,270792.42064317 4458988.13801107,270793.64326401 4458986.91539024,270793.64699918 4458986.93897316,270795.06739439 4458989.72665574,270797.27971182 4458991.93897316,270800.06739439 4458993.35936838,270803.15756434 4458993.84880322,270806.24773428 4458993.35936838,270809.03541686 4458991.93897316,270811.24773428 4458989.72665574,270812.6681295 4458986.93897316,270812.99244298 4458984.89133843,270813.07421856 4458985.05183204,270815.28653598 4458987.26414946,270818.07421856 4458988.68454468,270821.16438851 4458989.17397952,270824.25455845 4458988.68454468,270827.04224103 4458987.26414946,270829.25455845 4458985.05183204,270829.78896456 4458984.003001,270830.14530409 4458986.2528403,270831.56569931 4458989.04052288,270833.77801673 4458991.2528403,270836.56569931 4458992.67323552,270839.65586926 4458993.16267036,270842.7460392 4458992.67323552,270845.53372178 4458991.2528403,270847.7460392 4458989.04052288,270849.16643442 4458986.2528403,270849.65586926 4458983.16267036)),((270849.46818665 4458921.97813894,270848.97875181 4458918.88796899,270847.55835659 4458916.10028641,270845.34603917 4458913.88796899,270842.55835659 4458912.46757377,270839.46818665 4458911.97813894,270836.3780167 4458912.46757377,270833.59033412 4458913.88796899,270831.3780167 4458916.10028641,270829.95762148 4458918.88796899,270829.46818665 4458921.97813894,270829.95762148 4458925.06830888,270831.3780167 4458927.85599146,270833.59033412 4458930.06830888,270836.3780167 4458931.4887041,270839.46818665 4458931.97813894,270842.55835659 4458931.4887041,270845.34603917 4458930.06830888,270847.55835659 4458927.85599146,270848.97875181 4458925.06830888,270849.46818665 4458921.97813894)),((270865.74530134 4458940.79948673,270865.2558665 4458937.70931679,270863.83547128 4458934.92163421,270861.62315386 4458932.70931679,270858.83547128 4458931.28892157,270855.74530134 4458930.79948673,270852.6551314 4458931.28892157,270849.86744882 4458932.70931679,270847.6551314 4458934.92163421,270846.23473618 4458937.70931679,270845.74530134 4458940.79948673,270846.23473618 4458943.88965668,270847.6551314 4458946.67733926,270849.86744882 4458948.88965668,270852.6551314 4458950.3100519,270855.74530134 4458950.79948673,270858.83547128 4458950.3100519,270861.62315386 4458948.88965668,270863.83547128 4458946.67733926,270865.2558665 4458943.88965668,270865.74530134 4458940.79948673)),((270788.60197966 4458935.96883677,270788.11254482 4458932.87866683,270786.6921496 4458930.09098425,270784.47983218 4458927.87866683,270781.6921496 4458926.45827161,270778.60197966 4458925.96883677,270775.51180971 4458926.45827161,270772.72412713 4458927.87866683,270770.51180971 4458930.09098425,270769.09141449 4458932.87866683,270768.60197966 4458935.96883677,270769.09141449 4458939.05900671,270770.51180971 4458941.84668929,270772.72412713 4458944.05900671,270775.51180971 4458945.47940193,270778.60197966 4458945.96883677,270781.6921496 4458945.47940193,270784.47983218 4458944.05900671,270786.6921496 4458941.84668929,270788.11254482 4458939.05900671,270788.60197966 4458935.96883677)),((270834.16637636 4458946.78425,270833.67694153 4458943.69408006,270832.25654631 4458940.90639748,270830.04422889 4458938.69408006,270827.25654631 4458937.27368484,270824.16637636 4458936.78425,270821.07620642 4458937.27368484,270818.28852384 4458938.69408006,270816.07620642 4458940.90639748,270814.6558112 4458943.69408006,270814.16637636 4458946.78425,270814.6558112 4458949.87441995,270816.07620642 4458952.66210252,270818.28852384 4458954.87441995,270821.07620642 4458956.29481516,270824.16637636 4458956.78425,270827.25654631 4458956.29481516,270830.04422889 4458954.87441995,270832.25654631 4458952.66210252,270833.67694153 4458949.87441995,270834.16637636 4458946.78425)))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgisvoronoipolygons(self):
        outputs=processing.runalg("qgis:voronoipolygons",points(),None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','PT_NUM_A','PT_ST_A']
        expectedtypes=['Integer','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(12, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["3","3.3","c"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POLYGON((270817.33298671 4458921.97813894,270818.67102388 4458926.27184031,270839.013642 4458938.82031605,270855.74530134 4458924.3504098,270855.74530134 4458921.97813894,270817.33298671 4458921.97813894))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgisvariabledistancebufferdissolve(self):
        outputs=processing.runalg("qgis:variabledistancebuffer",lines(),"LINE_NUM_A",5,True,None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','LINE_NUM_A','LINE_ST_A']
        expectedtypes=['Integer','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(1, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["3","33.33","string c"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POLYGON((270814.21413752 4458985.95258457,270810.80529356 4458989.18882884,270808.69174287 4458991.94446972,270807.53317657 4458995.21836303,270807.44300321 4458998.69003727,270808.43004959 4459002.01966078,270810.39769673 4459004.88130681,270813.15333762 4459006.9948575,270816.42723093 4459008.1534238,270819.89890517 4459008.24359716,270823.22852868 4459007.25655078,270826.0901747 4459005.28890364,270833.35926918 4458998.38786457,270833.88132614 4458998.58087742,270837.13058272 4458998.7108223,270840.20989607 4459003.90320656,270845.42552819 4459008.49012215,270851.80332397 4459011.24081915,270858.71898032 4459011.88604018,270865.49554463 4459010.3626265,270871.46967955 4459006.81970046,270876.05659514 4459001.60406834,270885.44072573 4458986.77714202,270888.00298315 4458981.0903228,270888.88216283 4458974.91520184,270888.00886181 4458968.73924675,270885.45201899 4458963.04999103,270878.69544497 4458952.35208217,270874.54643839 4458947.50131335,270869.82909771 4458944.44634772,270870.7469857 4458939.45533181,270871.27388916 4458932.19317961,270871.08620655 4458927.12574909,270868.96819999 4458916.60667165,270863.61762521 4458907.30568827,270855.35959029 4458897.17082724,270848.45236135 4458890.79288506,270840.0388579 4458886.59754031,270830.78850095 4458884.9185959,270821.43729584 4458885.88963717,270807.17341735 4458889.4556068,270805.51748594 4458889.91525846,270798.76091192 4458891.97976719,270792.90585799 4458894.3983138,270786.52464919 4458897.77660081,270784.7394181 4458898.79336855,270777.98284408 4458902.92238601,270774.11264098 4458905.6850268,270768.66984524 4458910.18940948,270763.47897731 4458915.5747221,270757.28545113 4458923.6450744,270751.9136333 4458933.99549899,270746.28315495 4458952.01302971,270744.77017061 4458961.42555425,270744.582488 4458973.24955879,270746.05010473 4458983.57368813,270750.63622261 4458992.9390001,270757.89192046 4459000.42875272,270767.10696002 4459005.30979681,270777.37930902 4459007.10434178,270787.70343836 4459005.63672504,270797.06875033 4459001.05060716,270804.55850295 4458993.79490932,270809.43954704 4458984.57986975,270810.24028767 4458979.99626368,270814.21413752 4458985.95258457))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgisvariabledistancebuffer(self):
        outputs=processing.runalg("qgis:variabledistancebuffer",lines(),"LINE_NUM_A",5,False,None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','LINE_NUM_A','LINE_ST_A']
        expectedtypes=['Integer','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(3, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","11.1","string a"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POLYGON((270812.05109654 4458960.42236689,270811.37478455 4458965.83286281,270811.48149089 4458969.26749682,270812.6335486 4458972.50491248,270814.72054772 4458975.23484503,270820.09450366 4458980.36995849,270810.80529356 4458989.18882884,270808.69174287 4458991.94446972,270807.53317657 4458995.21836303,270807.44300321 4458998.69003727,270808.43004959 4459002.01966078,270810.39769673 4459004.88130681,270813.15333762 4459006.9948575,270816.42723093 4459008.1534238,270819.89890517 4459008.24359716,270823.22852868 4459007.25655078,270826.0901747 4459005.28890364,270840.91710103 4458991.21270776,270842.83222213 4458988.80740374,270844.01404792 4458985.96901264,270844.37190388 4458982.91530746,270843.87833385 4458979.88058098,270841.43845989 4458971.99791128,270840.27367103 4458969.43898543,270838.50330777 4458967.25480399,270834.09621044 4458963.04357766,270834.34176647 4458961.07912945,270833.85786665 4458956.19221569,270833.29481881 4458954.50307219,270831.69473584 4458951.42080229,270829.22049272 4458948.98384225,270826.11428561 4458947.43073869,270822.6801717 4458946.91352022,270819.254306 4458947.48281578,270816.1720361 4458949.08289875,270813.73507606 4458951.55714187,270812.18197251 4458954.66334898,270811.66475403 4458958.09746289,270812.05109654 4458960.42236689))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgissimplifygeometries(self):
        outputs=processing.runalg("qgis:simplifygeometries",polygons(),5,None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','POLY_NUM_A','POLY_ST_A']
        expectedtypes=['Integer','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(2, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","string a"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POLYGON((270771.63330111 4458992.35349302,270815.36334964 4458986.91069727,270818.55395404 4458973.96059707,270798.42294527 4458914.62661676,270759.84833131 4458922.09614031,270771.63330111 4458992.35349302))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgispolygonstolines(self):
        outputs=processing.runalg("qgis:polygonstolines",polygons(),None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','POLY_NUM_A','POLY_ST_A']
        expectedtypes=['Integer','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(2, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","string a"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='LINESTRING(270771.63330111 4458992.35349302, 270791.33997534 4458993.47958869, 270799.03496242 4458993.10422346, 270799.03496242 4458993.10422346, 270815.36334964 4458986.91069727, 270818.55395404 4458973.96059707, 270798.42294527 4458914.62661676, 270780.81854858 4458914.21983449, 270759.84833131 4458922.09614031, 270766.19050537 4458980.34180587, 270771.63330111 4458992.35349302)'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgispolygoncentroids(self):
        outputs=processing.runalg("qgis:polygoncentroids",polygons(),None)
        output=outputs['OUTPUT_LAYER']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','POLY_NUM_A','POLY_ST_A']
        expectedtypes=['Integer','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(2, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","string a"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POINT(270787.49991451 4458955.46775295)'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgislinestopolygons(self):
        outputs=processing.runalg("qgis:linestopolygons",lines(),None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','LINE_NUM_A','LINE_ST_A']
        expectedtypes=['Integer','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(3, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","11.1","string a"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POLYGON((270818.44773413 4458997.23886624,270833.27466046 4458983.16267036,270830.83478651 4458975.28000067,270822.38906898 4458967.20964836,270823.32748204 4458959.70234389,270822.7644342 4458958.01320039,270818.44773413 4458997.23886624))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgisunion(self):
        outputs=processing.runalg("qgis:union",polygons(),polygons2(),None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','POLY_NUM_A','POLY_ST_A','id_2','POLY_NUM_B','POLY_ST_B']
        expectedtypes=['Integer','Real','String','Integer','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(8, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","string a","2","1","string a"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POLYGON((270807.08580285 4458940.1594565,270798.42294527 4458914.62661676,270780.81854858 4458914.21983449,270763.52289518 4458920.715993,270760.3449542 4458926.6570575,270763.78234766 4458958.22561242,270794.30290024 4458942.16424502,270807.08580285 4458940.1594565))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgisclip(self):
        outputs=processing.runalg("qgis:clip",polygons(),polygons2(),None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','POLY_NUM_A','POLY_ST_A']
        expectedtypes=['Integer','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(2, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","string a"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='MULTIPOLYGON(((270771.63330111 4458992.35349302,270791.33997534 4458993.47958869,270799.03496242 4458993.10422346,270815.36334964 4458986.91069727,270818.55395404 4458973.96059707,270807.55271096 4458941.5356179,270794.85883582 4458943.97723852,270764.42437908 4458964.12194181,270766.19050537 4458980.34180587,270771.63330111 4458992.35349302)),((270807.08580285 4458940.1594565,270798.42294527 4458914.62661676,270780.81854858 4458914.21983449,270763.52289518 4458920.715993,270760.3449542 4458926.6570575,270763.78234766 4458958.22561242,270794.30290024 4458942.16424502,270807.08580285 4458940.1594565)))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgisdifference(self):
        outputs=processing.runalg("qgis:difference",polygons(),polygons2(),None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','POLY_NUM_A','POLY_ST_A']
        expectedtypes=['Integer','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(2, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","string a"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='MULTIPOLYGON(((270807.55271096 4458941.5356179,270807.08580285 4458940.1594565,270794.30290024 4458942.16424502,270763.78234766 4458958.22561242,270764.42437908 4458964.12194181,270794.85883582 4458943.97723852,270807.55271096 4458941.5356179)),((270763.52289518 4458920.715993,270759.84833131 4458922.09614031,270760.3449542 4458926.6570575,270763.52289518 4458920.715993)))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))


    def test_qgisintersection(self):
        outputs=processing.runalg("qgis:intersection",polygons(),polygons2(),None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','POLY_NUM_A','POLY_ST_A','id_2','POLY_NUM_B','POLY_ST_B']
        expectedtypes=['Integer','Real','String','Integer','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(4, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","string a","2","1.0","string a"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POLYGON((270807.08580285 4458940.1594565,270798.42294527 4458914.62661676,270780.81854858 4458914.21983449,270763.52289518 4458920.715993,270760.3449542 4458926.6570575,270763.78234766 4458958.22561242,270794.30290024 4458942.16424502,270807.08580285 4458940.1594565))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgisaddautoincrementalfield(self):
        outputs=processing.runalg("qgis:addautoincrementalfield",points(),None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','PT_NUM_A','PT_ST_A','AUTO']
        expectedtypes=['Integer','Real','String','Integer']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(12, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","a","1"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POINT(270839.65586926 4458983.16267036)'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgisaddfieldtoattributestable(self):
        outputs=processing.runalg("qgis:addfieldtoattributestable",lines(),"field",0,10,0,None)
        output=outputs['OUTPUT_LAYER']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','LINE_NUM_A','LINE_ST_A','field']
        expectedtypes=['Integer','Real','String','Integer']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(3, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","11.1","string a",""]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='LINESTRING(270818.44773413 4458997.23886624, 270833.27466046 4458983.16267036, 270830.83478651 4458975.28000067, 270822.38906898 4458967.20964836, 270823.32748204 4458959.70234389, 270822.7644342 4458958.01320039)'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgiscreateequivalentnumericalfield(self):
        outputs=processing.runalg("qgis:createequivalentnumericalfield",points2(),"POLY_ST_A",None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','POLY_NUM_A','POLY_ST_A','ID_2','POLY_NUM_B','POLY_ST_B','NUM_FIELD']
        expectedtypes=['Integer','Real','String','Integer','Real','String','Integer']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(8, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","string a","2","1.0","string a","0"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POINT(270781.07973944 4458932.92171896)'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgisdeletecolumn(self):
        outputs=processing.runalg("qgis:deletecolumn",points(),"PT_NUM_A",None)
        output=outputs['SAVENAME']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','PT_ST_A']
        expectedtypes=['Integer','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(12, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","a"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POINT(270839.65586926 4458983.16267036)'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgisexportaddgeometrycolumnspoints(self):
        outputs=processing.runalg("qgis:exportaddgeometrycolumns",points(),0,None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','PT_NUM_A','PT_ST_A','xcoord','ycoord']
        expectedtypes=['Integer','Real','String','Real','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(12, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","a","270839.655869","4458983.16267"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POINT(270839.65586926 4458983.16267036)'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))


    def test_qgisexportaddgeometrycolumnslines(self):
        outputs=processing.runalg("qgis:exportaddgeometrycolumns",lines(),0,None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','LINE_NUM_A','LINE_ST_A','length']
        expectedtypes=['Integer','Real','String','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(3, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","11.1","string a","49.724003"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='LINESTRING(270818.44773413 4458997.23886624, 270833.27466046 4458983.16267036, 270830.83478651 4458975.28000067, 270822.38906898 4458967.20964836, 270823.32748204 4458959.70234389, 270822.7644342 4458958.01320039)'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgisexportaddgeometrycolumnspolygons(self):
        outputs=processing.runalg("qgis:exportaddgeometrycolumns",polygons(),0,None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','POLY_NUM_A','POLY_ST_A','area','perimeter']
        expectedtypes=['Integer','Real','String','Real','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(2, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","string a","3543.718994","232.686821"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POLYGON((270771.63330111 4458992.35349302,270791.33997534 4458993.47958869,270799.03496242 4458993.10422346,270799.03496242 4458993.10422346,270815.36334964 4458986.91069727,270818.55395404 4458973.96059707,270798.42294527 4458914.62661676,270780.81854858 4458914.21983449,270759.84833131 4458922.09614031,270766.19050537 4458980.34180587,270771.63330111 4458992.35349302))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgisstatisticsbycategories(self):
        outputs=processing.runalg("qgis:statisticsbycategories",points2(),"POLY_NUM_A","POLY_ST_B", None)
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['category','min','max','mean','stddev']
        names=[str(f.name()) for f in fields]
        self.assertEqual(expectednames, names)
        features=processing.features(layer)
        self.assertEqual(3, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["","1.1","2.2","1.925","0.55"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)

def suite():
    suite = unittest.makeSuite(QgisAlgsTest, 'test')
    return suite

def runtests():
    result = unittest.TestResult()
    testsuite = suite()
    testsuite.run(result)
    return result
