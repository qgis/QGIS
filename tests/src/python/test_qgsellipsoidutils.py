# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsEllipsoidUtils

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '18/4/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA
from qgis.core import (QgsEllipsoidUtils,
                       QgsProjUtils)
from qgis.testing import start_app, unittest

app = start_app()


class TestQgsEllipsoidUtils(unittest.TestCase):

    def testParams(self):
        """
        Test fetching ellipsoid parameters
        """

        # run each test twice, so that ellipsoid is fetched from cache on the second time

        for i in range(2):
            params = QgsEllipsoidUtils.ellipsoidParameters("WGS84")
            self.assertTrue(params.valid)
            self.assertEqual(params.semiMajor, 6378137.0)
            self.assertAlmostEqual(params.semiMinor, 6356752.314245179, 5)
            self.assertAlmostEqual(params.inverseFlattening, 298.257223563, 5)
            self.assertFalse(params.useCustomParameters)
            if QgsProjUtils.projVersionMajor() < 6:
                self.assertEqual(params.crs.authid(), 'EPSG:4030')
            else:
                self.assertEqual(params.crs.toProj4(), '+proj=longlat +a=6378137 +rf=298.25722356300003 +no_defs')

        for i in range(2):
            params = QgsEllipsoidUtils.ellipsoidParameters("Ganymede2000")
            self.assertTrue(params.valid)
            self.assertEqual(params.semiMajor, 2632400.0 if QgsProjUtils.projVersionMajor() < 6 else 2632345.0)
            self.assertEqual(params.semiMinor, 2632350.0 if QgsProjUtils.projVersionMajor() < 6 else 2632345.0)
            self.assertEqual(params.inverseFlattening, 52648.0 if QgsProjUtils.projVersionMajor() < 6 else 0)
            self.assertFalse(params.useCustomParameters)
            if QgsProjUtils.projVersionMajor() < 6:
                self.assertEqual(params.crs.authid(), '')
            else:
                self.assertEqual(params.crs.toProj4(), '+proj=longlat +a=2632345 +no_defs')

            if QgsProjUtils.projVersionMajor() >= 6:
                params = QgsEllipsoidUtils.ellipsoidParameters("ESRI:107916")
                self.assertTrue(params.valid)
                self.assertEqual(params.semiMajor, 2632345.0)
                self.assertEqual(params.semiMinor, 2632345.0)
                self.assertEqual(params.inverseFlattening, 0)
                self.assertFalse(params.useCustomParameters)
                self.assertEqual(params.crs.toProj4(), '+proj=longlat +a=2632345 +no_defs')

                params = QgsEllipsoidUtils.ellipsoidParameters("EPSG:7001")
                self.assertTrue(params.valid)
                self.assertEqual(params.semiMajor, 6377563.396)
                self.assertEqual(params.semiMinor, 6356256.909237285)
                self.assertEqual(params.inverseFlattening, 299.3249646)
                self.assertFalse(params.useCustomParameters)
                self.assertEqual(params.crs.toProj4(),
                                 '+proj=longlat +a=6377563.3959999997 +rf=299.32496459999999 +no_defs')

                params = QgsEllipsoidUtils.ellipsoidParameters("EPSG:7008")
                self.assertTrue(params.valid)
                self.assertEqual(params.semiMajor, 6378206.4)
                self.assertEqual(params.semiMinor, 6356583.8)
                self.assertEqual(params.inverseFlattening, 294.9786982138982)
                self.assertFalse(params.useCustomParameters)
                self.assertEqual(params.crs.toProj4(),
                                 '+proj=longlat +a=6378206.4000000004 +b=6356583.7999999998 +no_defs')

        # using parameters
        for i in range(2):
            params = QgsEllipsoidUtils.ellipsoidParameters("PARAMETER:2631400:2341350")
            self.assertTrue(params.valid)
            self.assertEqual(params.semiMajor, 2631400.0)
            self.assertEqual(params.semiMinor, 2341350.0)
            self.assertAlmostEqual(params.inverseFlattening, 9.07223, 4)
            self.assertTrue(params.useCustomParameters)
            self.assertEqual(params.crs.authid(), '')

        # invalid
        for i in range(2):
            params = QgsEllipsoidUtils.ellipsoidParameters("Babies first ellipsoid!")
            self.assertFalse(params.valid)

    def testAcronyms(self):
        self.assertTrue('WGS84' if QgsProjUtils.projVersionMajor() < 6 else 'EPSG:7030' in QgsEllipsoidUtils.acronyms())
        self.assertTrue(
            'Ganymede2000' if QgsProjUtils.projVersionMajor() < 6 else 'ESRI:107916' in QgsEllipsoidUtils.acronyms())

    def testDefinitions(self):
        defs = QgsEllipsoidUtils.definitions()

        gany_id = 'Ganymede2000' if QgsProjUtils.projVersionMajor() < 6 else 'ESRI:107916'
        gany_defs = [d for d in defs if d.acronym == gany_id][0]
        self.assertEqual(gany_defs.acronym, gany_id)
        self.assertEqual(gany_defs.description,
                         'Ganymede2000' if QgsProjUtils.projVersionMajor() < 6 else 'Ganymede 2000 IAU IAG (ESRI:107916)')
        self.assertTrue(gany_defs.parameters.valid)
        self.assertEqual(gany_defs.parameters.semiMajor,
                         2632400.0 if QgsProjUtils.projVersionMajor() < 6 else 2632345.0)
        self.assertEqual(gany_defs.parameters.semiMinor,
                         2632350.0 if QgsProjUtils.projVersionMajor() < 6 else 2632345.0)
        self.assertEqual(gany_defs.parameters.inverseFlattening,
                         52648.0 if QgsProjUtils.projVersionMajor() < 6 else 0.0)
        self.assertFalse(gany_defs.parameters.useCustomParameters)
        self.assertEqual(gany_defs.parameters.crs.authid(), '')

    @unittest.skipIf(QgsProjUtils.projVersionMajor() < 6, 'Not a proj6 build')
    def testMappingEllipsoidsToProj6(self):
        old_qgis_ellipsoids = {'Adrastea2000': 'Adrastea2000', 'airy': 'Airy 1830', 'Amalthea2000': 'Amalthea2000',
                               'Ananke2000': 'Ananke2000',
                               'andrae': 'Andrae 1876 (Den., Iclnd.)',
                               'Ariel2000': 'Ariel2000',
                               'Atlas2000': 'Atlas2000', 'aust_SA': 'Australian Natl & S. Amer. 1969',
                               'Belinda2000': 'Belinda2000',
                               'bessel': 'Bessel 1841', 'bess_nam': 'Bessel 1841 (Namibia)', 'Bianca2000': 'Bianca2000',
                               'Callisto2000': 'Callisto2000', 'Calypso2000': 'Calypso2000', 'Carme2000': 'Carme2000',
                               'Charon2000': 'Charon2000', 'clrk66': 'Clarke 1866', 'IGNF:ELG004': 'Clarke 1866',
                               'IGNF:ELG003': 'Clarke 1880 Anglais', 'IGNF:ELG010': 'Clarke 1880 IGN',
                               'clrk80': 'Clarke 1880 mod.',
                               'cape': 'Clarke 1880 mod.', 'CPM': 'Comm. des Poids et Mesures 1799',  # spellok
                               'Cordelia2000': 'Cordelia2000',
                               'Cressida2000': 'Cressida2000', 'Deimos2000': 'Deimos2000',
                               'delmbr': 'Delambre 1810 (Belgium)',
                               'Desdemona2000': 'Desdemona2000', 'Despina2000': 'Despina2000', 'Dione2000': 'Dione2000',
                               'Earth2000': 'Earth2000', 'Elara2000': 'Elara2000', 'Enceladus2000': 'Enceladus2000',
                               'engelis': 'Engelis 1985',
                               'Epimetheus2000': 'Epimetheus2000', 'Europa2000': 'Europa2000',
                               'evrstSS': 'Everest (Sabah & Sarawak)',
                               'evrst30': 'Everest 1830', 'evrst48': 'Everest 1948', 'evrst56': 'Everest 1956',
                               'evrst69': 'Everest 1969',
                               'fschr60': 'Fischer (Mercury Datum) 1960', 'fschr68': 'Fischer 1968',
                               'GRS80': 'GRS 1980(IUGG, 1980)',
                               'GRS67': 'GRS 67(IUGG 1967)', 'Galatea2000': 'Galatea2000',
                               'Ganymede2000': 'Ganymede2000',
                               'Helene2000': 'Helene2000', 'helmert': 'Helmert 1906', 'Himalia2000': 'Himalia2000',
                               'hough': 'Hough',
                               'Hyperion2000': 'Hyperion2000', 'IGNF:ELG108': 'IAG GRS 1967',
                               'IGNF:ELG037': 'IAG GRS 1980',
                               'IAU76': 'IAU 1976', 'Iapetus2000': 'Iapetus2000',
                               'intl': 'International 1909 (Hayford)',
                               'IGNF:ELG001': 'International-Hayford 1909', 'Io2000': 'Io2000',
                               'Janus2000': 'Janus2000',
                               'Juliet2000': 'Juliet2000', 'Jupiter2000': 'Jupiter2000', 'kaula': 'Kaula 1961',
                               'krass': 'Krassovsky, 1942',
                               'Larissa2000': 'Larissa2000', 'Leda2000': 'Leda2000', 'lerch': 'Lerch 1979',
                               'Lysithea2000': 'Lysithea2000',
                               'MERIT': 'MERIT 1983', 'Mars2000': 'Mars2000', 'mprts': 'Maupertius 1738',
                               'Mercury2000': 'Mercury2000',
                               'Metis2000': 'Metis2000', 'Mimas2000': 'Mimas2000', 'Miranda2000': 'Miranda2000',
                               'mod_airy': 'Modified Airy',
                               'fschr60m': 'Modified Fischer 1960', 'Moon2000': 'Moon2000', 'Naiad2000': 'Naiad2000',
                               'NWL9D': 'Naval Weapons Lab., 1965', 'Neptune2000': 'Neptune2000',
                               'Nereid2000': 'Nereid2000',
                               'new_intl': 'New International 1967', 'sphere': 'Normal Sphere (r=6370997)',
                               'Oberon2000': 'Oberon2000',
                               'Ophelia2000': 'Ophelia2000', 'IGNF:ELG017': 'PLESSIS 1817', 'Pan2000': 'Pan2000',
                               'Pandora2000': 'Pandora2000',
                               'Pasiphae2000': 'Pasiphae2000', 'Phobos2000': 'Phobos2000', 'Phoebe2000': 'Phoebe2000',
                               'plessis': 'Plessis 1817 (France)', 'Pluto2000': 'Pluto2000', 'Portia2000': 'Portia2000',
                               'Prometheus2000': 'Prometheus2000', 'Proteus2000': 'Proteus2000', 'Puck2000': 'Puck2000',
                               'Rhea2000': 'Rhea2000',
                               'Rosalind2000': 'Rosalind2000', 'IGNF:ELG032': 'SPHERE PICARD',
                               'Saturn2000': 'Saturn2000',
                               'Sinope2000': 'Sinope2000', 'SEasia': 'Southeast Asia',
                               'SGS85': 'Soviet Geodetic System 85',
                               'Telesto2000': 'Telesto2000', 'Tethys2000': 'Tethys2000', 'Thalassa2000': 'Thalassa2000',
                               'Thebe2000': 'Thebe2000', 'Titan2000': 'Titan2000', 'Titania2000': 'Titania2000',
                               'Triton2000': 'Triton2000',
                               'Umbriel2000': 'Umbriel2000', 'Uranus2000': 'Uranus2000', 'Venus2000': 'Venus2000',
                               'WGS60': 'WGS 60',
                               'WGS66': 'WGS 66', 'WGS72': 'WGS 72', 'WGS84': 'WGS 84', 'IGNF:ELG052': 'WGS72',
                               'IGNF:ELG102': 'WGS72 (NWL-10F)', 'IGNF:ELG053': 'WGS84', 'walbeck': 'Walbeck'}

        # ensure that all old QGIS custom ellipsoid definitions map across to new PROJ6 ones
        for o in old_qgis_ellipsoids:
            self.assertTrue(QgsEllipsoidUtils.ellipsoidParameters(o).valid, 'no defs for {}'.format(o))


if __name__ == '__main__':
    unittest.main()
