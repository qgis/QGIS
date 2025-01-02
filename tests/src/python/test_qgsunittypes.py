"""QGIS Unit tests for QgsUnitTypes

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "03.02.2016"
__copyright__ = "Copyright 2016, The QGIS Project"

from qgis.PyQt.QtCore import QLocale
from qgis.core import Qgis, QgsUnitTypes
from qgis.testing import unittest


class TestQgsUnitTypes(unittest.TestCase):

    def setUp(self):
        super().setUp()
        # enforce C locale because the tests expect it
        # (decimal separators / thousand separators)
        QLocale.setDefault(QLocale.c())

    def testEncodeDecodeUnitType(self):
        """Test encoding and decoding unit type"""
        units = [
            QgsUnitTypes.UnitType.TypeDistance,
            QgsUnitTypes.UnitType.TypeArea,
            QgsUnitTypes.UnitType.TypeVolume,
            QgsUnitTypes.UnitType.TypeTemporal,
            QgsUnitTypes.UnitType.TypeUnknown,
        ]

        for u in units:
            res, ok = QgsUnitTypes.decodeUnitType(QgsUnitTypes.encodeUnitType(u))
            assert ok
            self.assertEqual(res, u)

        # Test decoding bad units
        res, ok = QgsUnitTypes.decodeUnitType("bad")
        self.assertFalse(ok)
        self.assertEqual(res, QgsUnitTypes.UnitType.TypeUnknown)

        # Test that string is cleaned before decoding
        res, ok = QgsUnitTypes.decodeUnitType(" volUme  ")
        assert ok
        self.assertEqual(res, QgsUnitTypes.UnitType.TypeVolume)

    def testDistanceUnitType(self):
        """Test QgsUnitTypes::unitType()"""
        expected = {
            QgsUnitTypes.DistanceUnit.DistanceMeters: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.DistanceUnit.DistanceKilometers: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.DistanceUnit.DistanceFeet: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.DistanceUnit.DistanceYards: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.DistanceUnit.DistanceMiles: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.DistanceUnit.DistanceDegrees: QgsUnitTypes.DistanceUnitType.Geographic,
            QgsUnitTypes.DistanceUnit.DistanceCentimeters: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.DistanceUnit.DistanceMillimeters: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.DistanceUnit.DistanceUnknownUnit: QgsUnitTypes.DistanceUnitType.UnknownType,
            QgsUnitTypes.DistanceUnit.DistanceNauticalMiles: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.Inches: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.ChainsInternational: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.ChainsBritishBenoit1895A: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.ChainsBritishBenoit1895B: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.ChainsBritishSears1922Truncated: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.ChainsBritishSears1922: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.ChainsClarkes: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.ChainsUSSurvey: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.FeetBritish1865: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.FeetBritish1936: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.FeetBritishBenoit1895A: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.FeetBritishBenoit1895B: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.FeetBritishSears1922Truncated: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.FeetBritishSears1922: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.FeetClarkes: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.FeetGoldCoast: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.FeetIndian: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.FeetIndian1937: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.FeetIndian1962: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.FeetIndian1975: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.FeetUSSurvey: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.LinksInternational: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.LinksBritishBenoit1895A: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.LinksBritishBenoit1895B: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.LinksBritishSears1922Truncated: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.LinksBritishSears1922: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.LinksClarkes: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.LinksUSSurvey: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.YardsBritishBenoit1895A: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.YardsBritishBenoit1895B: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.YardsBritishSears1922Truncated: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.YardsBritishSears1922: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.YardsClarkes: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.YardsIndian: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.YardsIndian1937: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.YardsIndian1962: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.YardsIndian1975: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.MilesUSSurvey: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.Fathoms: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.DistanceUnit.MetersGermanLegal: QgsUnitTypes.DistanceUnitType.Standard,
        }

        for t in list(expected.keys()):
            self.assertEqual(QgsUnitTypes.unitType(t), expected[t])

    def testEncodeDecodeDistanceUnits(self):
        """Test encoding and decoding distance units"""
        units = [
            QgsUnitTypes.DistanceUnit.DistanceMeters,
            QgsUnitTypes.DistanceUnit.DistanceKilometers,
            QgsUnitTypes.DistanceUnit.DistanceFeet,
            QgsUnitTypes.DistanceUnit.DistanceYards,
            QgsUnitTypes.DistanceUnit.DistanceMiles,
            QgsUnitTypes.DistanceUnit.DistanceDegrees,
            QgsUnitTypes.DistanceUnit.DistanceCentimeters,
            QgsUnitTypes.DistanceUnit.DistanceMillimeters,
            QgsUnitTypes.DistanceUnit.DistanceUnknownUnit,
            QgsUnitTypes.DistanceUnit.DistanceNauticalMiles,
            Qgis.DistanceUnit.Inches,
            Qgis.DistanceUnit.ChainsInternational,
            Qgis.DistanceUnit.ChainsBritishBenoit1895A,
            Qgis.DistanceUnit.ChainsBritishBenoit1895B,
            Qgis.DistanceUnit.ChainsBritishSears1922Truncated,
            Qgis.DistanceUnit.ChainsBritishSears1922,
            Qgis.DistanceUnit.ChainsClarkes,
            Qgis.DistanceUnit.ChainsUSSurvey,
            Qgis.DistanceUnit.FeetBritish1865,
            Qgis.DistanceUnit.FeetBritish1936,
            Qgis.DistanceUnit.FeetBritishBenoit1895A,
            Qgis.DistanceUnit.FeetBritishBenoit1895B,
            Qgis.DistanceUnit.FeetBritishSears1922Truncated,
            Qgis.DistanceUnit.FeetBritishSears1922,
            Qgis.DistanceUnit.FeetClarkes,
            Qgis.DistanceUnit.FeetGoldCoast,
            Qgis.DistanceUnit.FeetIndian,
            Qgis.DistanceUnit.FeetIndian1937,
            Qgis.DistanceUnit.FeetIndian1962,
            Qgis.DistanceUnit.FeetIndian1975,
            Qgis.DistanceUnit.FeetUSSurvey,
            Qgis.DistanceUnit.LinksInternational,
            Qgis.DistanceUnit.LinksBritishBenoit1895A,
            Qgis.DistanceUnit.LinksBritishBenoit1895B,
            Qgis.DistanceUnit.LinksBritishSears1922Truncated,
            Qgis.DistanceUnit.LinksBritishSears1922,
            Qgis.DistanceUnit.LinksClarkes,
            Qgis.DistanceUnit.LinksUSSurvey,
            Qgis.DistanceUnit.YardsBritishBenoit1895A,
            Qgis.DistanceUnit.YardsBritishBenoit1895B,
            Qgis.DistanceUnit.YardsBritishSears1922Truncated,
            Qgis.DistanceUnit.YardsBritishSears1922,
            Qgis.DistanceUnit.YardsClarkes,
            Qgis.DistanceUnit.YardsIndian,
            Qgis.DistanceUnit.YardsIndian1937,
            Qgis.DistanceUnit.YardsIndian1962,
            Qgis.DistanceUnit.YardsIndian1975,
            Qgis.DistanceUnit.MilesUSSurvey,
            Qgis.DistanceUnit.Fathoms,
            Qgis.DistanceUnit.MetersGermanLegal,
        ]

        for u in units:
            res, ok = QgsUnitTypes.decodeDistanceUnit(QgsUnitTypes.encodeUnit(u))
            assert ok
            self.assertEqual(res, u)

        # Test decoding bad units
        res, ok = QgsUnitTypes.decodeDistanceUnit("bad")
        self.assertFalse(ok)
        self.assertEqual(res, QgsUnitTypes.DistanceUnit.DistanceUnknownUnit)

        # Test that string is cleaned before decoding
        res, ok = QgsUnitTypes.decodeDistanceUnit(" FeEt  ")
        assert ok
        self.assertEqual(res, QgsUnitTypes.DistanceUnit.DistanceFeet)

    def testDistanceUnitsToFromString(self):
        """Test converting distance units to and from translated strings"""
        units = [
            QgsUnitTypes.DistanceUnit.DistanceMeters,
            QgsUnitTypes.DistanceUnit.DistanceKilometers,
            QgsUnitTypes.DistanceUnit.DistanceFeet,
            QgsUnitTypes.DistanceUnit.DistanceYards,
            QgsUnitTypes.DistanceUnit.DistanceMiles,
            QgsUnitTypes.DistanceUnit.DistanceDegrees,
            QgsUnitTypes.DistanceUnit.DistanceCentimeters,
            QgsUnitTypes.DistanceUnit.DistanceMillimeters,
            QgsUnitTypes.DistanceUnit.DistanceUnknownUnit,
            QgsUnitTypes.DistanceUnit.DistanceNauticalMiles,
            Qgis.DistanceUnit.Inches,
            Qgis.DistanceUnit.ChainsInternational,
            Qgis.DistanceUnit.ChainsBritishBenoit1895A,
            Qgis.DistanceUnit.ChainsBritishBenoit1895B,
            Qgis.DistanceUnit.ChainsBritishSears1922Truncated,
            Qgis.DistanceUnit.ChainsBritishSears1922,
            Qgis.DistanceUnit.ChainsClarkes,
            Qgis.DistanceUnit.ChainsUSSurvey,
            Qgis.DistanceUnit.FeetBritish1865,
            Qgis.DistanceUnit.FeetBritish1936,
            Qgis.DistanceUnit.FeetBritishBenoit1895A,
            Qgis.DistanceUnit.FeetBritishBenoit1895B,
            Qgis.DistanceUnit.FeetBritishSears1922Truncated,
            Qgis.DistanceUnit.FeetBritishSears1922,
            Qgis.DistanceUnit.FeetClarkes,
            Qgis.DistanceUnit.FeetGoldCoast,
            Qgis.DistanceUnit.FeetIndian,
            Qgis.DistanceUnit.FeetIndian1937,
            Qgis.DistanceUnit.FeetIndian1962,
            Qgis.DistanceUnit.FeetIndian1975,
            Qgis.DistanceUnit.FeetUSSurvey,
            Qgis.DistanceUnit.LinksInternational,
            Qgis.DistanceUnit.LinksBritishBenoit1895A,
            Qgis.DistanceUnit.LinksBritishBenoit1895B,
            Qgis.DistanceUnit.LinksBritishSears1922Truncated,
            Qgis.DistanceUnit.LinksBritishSears1922,
            Qgis.DistanceUnit.LinksClarkes,
            Qgis.DistanceUnit.LinksUSSurvey,
            Qgis.DistanceUnit.YardsBritishBenoit1895A,
            Qgis.DistanceUnit.YardsBritishBenoit1895B,
            Qgis.DistanceUnit.YardsBritishSears1922Truncated,
            Qgis.DistanceUnit.YardsBritishSears1922,
            Qgis.DistanceUnit.YardsClarkes,
            Qgis.DistanceUnit.YardsIndian,
            Qgis.DistanceUnit.YardsIndian1937,
            Qgis.DistanceUnit.YardsIndian1962,
            Qgis.DistanceUnit.YardsIndian1975,
            Qgis.DistanceUnit.MilesUSSurvey,
            Qgis.DistanceUnit.Fathoms,
            Qgis.DistanceUnit.MetersGermanLegal,
        ]

        for u in units:
            res, ok = QgsUnitTypes.stringToDistanceUnit(QgsUnitTypes.toString(u))
            self.assertTrue(
                ok, f"QgsUnitTypes.stringToDistanceUnit failed for {u.name}"
            )
            self.assertEqual(res, u)

        # Test converting bad strings
        res, ok = QgsUnitTypes.stringToDistanceUnit("bad")
        self.assertFalse(ok)
        self.assertEqual(res, QgsUnitTypes.DistanceUnit.DistanceUnknownUnit)

        # Test that string is cleaned before conversion
        res, ok = QgsUnitTypes.stringToDistanceUnit(
            f" {QgsUnitTypes.toString(QgsUnitTypes.DistanceUnit.DistanceFeet).upper()}  "
        )
        print(
            f" {QgsUnitTypes.toString(QgsUnitTypes.DistanceUnit.DistanceFeet).upper()}  "
        )
        assert ok
        self.assertEqual(res, QgsUnitTypes.DistanceUnit.DistanceFeet)

    def testAreaUnitType(self):
        """Test QgsUnitTypes::unitType() for area units"""
        expected = {
            QgsUnitTypes.AreaUnit.AreaSquareMeters: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.AreaUnit.AreaSquareKilometers: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.AreaUnit.AreaSquareFeet: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.AreaUnit.AreaSquareYards: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.AreaUnit.AreaSquareMiles: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.AreaUnit.AreaHectares: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.AreaUnit.AreaAcres: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.AreaUnit.AreaSquareNauticalMiles: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.AreaUnit.AreaSquareDegrees: QgsUnitTypes.DistanceUnitType.Geographic,
            QgsUnitTypes.AreaUnit.AreaSquareCentimeters: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.AreaUnit.AreaSquareMillimeters: QgsUnitTypes.DistanceUnitType.Standard,
            Qgis.AreaUnit.SquareInches: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.AreaUnit.AreaUnknownUnit: QgsUnitTypes.DistanceUnitType.UnknownType,
        }

        for t in list(expected.keys()):
            self.assertEqual(QgsUnitTypes.unitType(t), expected[t])

    def testEncodeDecodeAreaUnits(self):
        """Test encoding and decoding area units"""
        units = [
            QgsUnitTypes.AreaUnit.AreaSquareMeters,
            QgsUnitTypes.AreaUnit.AreaSquareKilometers,
            QgsUnitTypes.AreaUnit.AreaSquareFeet,
            QgsUnitTypes.AreaUnit.AreaSquareYards,
            QgsUnitTypes.AreaUnit.AreaSquareMiles,
            QgsUnitTypes.AreaUnit.AreaHectares,
            QgsUnitTypes.AreaUnit.AreaAcres,
            QgsUnitTypes.AreaUnit.AreaSquareNauticalMiles,
            QgsUnitTypes.AreaUnit.AreaSquareDegrees,
            QgsUnitTypes.AreaUnit.AreaSquareCentimeters,
            QgsUnitTypes.AreaUnit.AreaSquareMillimeters,
            Qgis.AreaUnit.SquareInches,
            QgsUnitTypes.AreaUnit.AreaUnknownUnit,
        ]

        for u in units:
            res, ok = QgsUnitTypes.decodeAreaUnit(QgsUnitTypes.encodeUnit(u))
            assert ok
            self.assertEqual(res, u)

        # Test decoding bad units
        res, ok = QgsUnitTypes.decodeAreaUnit("bad")
        self.assertFalse(ok)
        self.assertEqual(res, QgsUnitTypes.AreaUnit.AreaUnknownUnit)

        # Test that string is cleaned before decoding
        res, ok = QgsUnitTypes.decodeAreaUnit(" Ha  ")
        assert ok
        self.assertEqual(res, QgsUnitTypes.AreaUnit.AreaHectares)

    def testAreaUnitsToFromString(self):
        """Test converting area units to and from translated strings"""
        units = [
            QgsUnitTypes.AreaUnit.AreaSquareMeters,
            QgsUnitTypes.AreaUnit.AreaSquareKilometers,
            QgsUnitTypes.AreaUnit.AreaSquareFeet,
            QgsUnitTypes.AreaUnit.AreaSquareYards,
            QgsUnitTypes.AreaUnit.AreaSquareMiles,
            QgsUnitTypes.AreaUnit.AreaHectares,
            QgsUnitTypes.AreaUnit.AreaAcres,
            QgsUnitTypes.AreaUnit.AreaSquareNauticalMiles,
            QgsUnitTypes.AreaUnit.AreaSquareDegrees,
            QgsUnitTypes.AreaUnit.AreaSquareCentimeters,
            QgsUnitTypes.AreaUnit.AreaSquareMillimeters,
            Qgis.AreaUnit.SquareInches,
            QgsUnitTypes.AreaUnit.AreaUnknownUnit,
        ]

        for u in units:
            res, ok = QgsUnitTypes.stringToAreaUnit(QgsUnitTypes.toString(u))
            assert ok
            self.assertEqual(res, u)

        # Test converting bad strings
        res, ok = QgsUnitTypes.stringToAreaUnit("bad")
        self.assertFalse(ok)
        self.assertEqual(res, QgsUnitTypes.AreaUnit.AreaUnknownUnit)

        # Test that string is cleaned before conversion
        res, ok = QgsUnitTypes.stringToAreaUnit(
            f" {QgsUnitTypes.toString(QgsUnitTypes.AreaUnit.AreaSquareMiles).upper()}  "
        )
        assert ok
        self.assertEqual(res, QgsUnitTypes.AreaUnit.AreaSquareMiles)

    def testVolumeUnitType(self):
        """Test QgsUnitTypes::unitType() for volume units"""
        expected = {
            QgsUnitTypes.VolumeUnit.VolumeCubicMeters: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.VolumeUnit.VolumeCubicFeet: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.VolumeUnit.VolumeCubicYards: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.VolumeUnit.VolumeBarrel: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.VolumeUnit.VolumeCubicDecimeter: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.VolumeUnit.VolumeLiters: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.VolumeUnit.VolumeGallonUS: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.VolumeUnit.VolumeCubicInch: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.VolumeUnit.VolumeCubicCentimeter: QgsUnitTypes.DistanceUnitType.Standard,
            QgsUnitTypes.VolumeUnit.VolumeCubicDegrees: QgsUnitTypes.DistanceUnitType.Geographic,
            QgsUnitTypes.VolumeUnit.VolumeUnknownUnit: QgsUnitTypes.DistanceUnitType.UnknownType,
        }

        for t in list(expected.keys()):
            self.assertEqual(QgsUnitTypes.unitType(t), expected[t])

    def testEncodeDecodeVolumeUnits(self):
        """Test encoding and decoding volume units"""
        units = [
            QgsUnitTypes.VolumeUnit.VolumeCubicMeters,
            QgsUnitTypes.VolumeUnit.VolumeCubicFeet,
            QgsUnitTypes.VolumeUnit.VolumeCubicYards,
            QgsUnitTypes.VolumeUnit.VolumeBarrel,
            QgsUnitTypes.VolumeUnit.VolumeCubicDecimeter,
            QgsUnitTypes.VolumeUnit.VolumeLiters,
            QgsUnitTypes.VolumeUnit.VolumeGallonUS,
            QgsUnitTypes.VolumeUnit.VolumeCubicInch,
            QgsUnitTypes.VolumeUnit.VolumeCubicCentimeter,
            QgsUnitTypes.VolumeUnit.VolumeCubicDegrees,
            QgsUnitTypes.VolumeUnit.VolumeUnknownUnit,
        ]

        for u in units:
            res, ok = QgsUnitTypes.decodeVolumeUnit(QgsUnitTypes.encodeUnit(u))
            assert ok
            self.assertEqual(res, u)

        # Test decoding bad units
        res, ok = QgsUnitTypes.decodeVolumeUnit("bad")
        self.assertFalse(ok)
        self.assertEqual(res, QgsUnitTypes.VolumeUnit.VolumeUnknownUnit)

        # Test that string is cleaned before decoding
        res, ok = QgsUnitTypes.decodeVolumeUnit(" bbl  ")
        assert ok
        self.assertEqual(res, QgsUnitTypes.VolumeUnit.VolumeBarrel)

    def testVolumeUnitsToFromString(self):
        """Test converting volume units to and from translated strings"""
        units = [
            QgsUnitTypes.VolumeUnit.VolumeCubicMeters,
            QgsUnitTypes.VolumeUnit.VolumeCubicFeet,
            QgsUnitTypes.VolumeUnit.VolumeCubicYards,
            QgsUnitTypes.VolumeUnit.VolumeBarrel,
            QgsUnitTypes.VolumeUnit.VolumeCubicDecimeter,
            QgsUnitTypes.VolumeUnit.VolumeLiters,
            QgsUnitTypes.VolumeUnit.VolumeGallonUS,
            QgsUnitTypes.VolumeUnit.VolumeCubicInch,
            QgsUnitTypes.VolumeUnit.VolumeCubicCentimeter,
            QgsUnitTypes.VolumeUnit.VolumeCubicDegrees,
            QgsUnitTypes.VolumeUnit.VolumeUnknownUnit,
        ]

        for u in units:
            res, ok = QgsUnitTypes.stringToVolumeUnit(QgsUnitTypes.toString(u))
            assert ok
            self.assertEqual(res, u)

        # Test converting bad strings
        res, ok = QgsUnitTypes.stringToVolumeUnit("bad")
        self.assertFalse(ok)
        self.assertEqual(res, QgsUnitTypes.VolumeUnit.VolumeUnknownUnit)

        # Test that string is cleaned before conversion
        res, ok = QgsUnitTypes.stringToVolumeUnit(
            f" {QgsUnitTypes.toString(QgsUnitTypes.VolumeUnit.VolumeBarrel).upper()}  "
        )
        assert ok
        self.assertEqual(res, QgsUnitTypes.VolumeUnit.VolumeBarrel)

    def testEncodeDecodeTemporalUnits(self):
        """Test encoding and decoding temporal units"""
        units = [
            QgsUnitTypes.TemporalUnit.TemporalMilliseconds,
            QgsUnitTypes.TemporalUnit.TemporalSeconds,
            QgsUnitTypes.TemporalUnit.TemporalMinutes,
            QgsUnitTypes.TemporalUnit.TemporalHours,
            QgsUnitTypes.TemporalUnit.TemporalDays,
            QgsUnitTypes.TemporalUnit.TemporalWeeks,
            QgsUnitTypes.TemporalUnit.TemporalMonths,
            QgsUnitTypes.TemporalUnit.TemporalYears,
            QgsUnitTypes.TemporalUnit.TemporalDecades,
            QgsUnitTypes.TemporalUnit.TemporalCenturies,
            QgsUnitTypes.TemporalUnit.TemporalIrregularStep,
            QgsUnitTypes.TemporalUnit.TemporalUnknownUnit,
        ]

        for u in units:
            res, ok = QgsUnitTypes.decodeTemporalUnit(QgsUnitTypes.encodeUnit(u))
            assert ok
            self.assertEqual(res, u)

        # Test decoding bad units
        res, ok = QgsUnitTypes.decodeTemporalUnit("bad")
        self.assertFalse(ok)
        self.assertEqual(res, QgsUnitTypes.TemporalUnit.TemporalUnknownUnit)

        # Test that string is cleaned before decoding
        res, ok = QgsUnitTypes.decodeTemporalUnit(" min  ")
        assert ok
        self.assertEqual(res, QgsUnitTypes.TemporalUnit.TemporalMinutes)

    def testTemporalUnitsToFromString(self):
        """Test converting temporal units to and from translated strings"""
        units = [
            QgsUnitTypes.TemporalUnit.TemporalMilliseconds,
            QgsUnitTypes.TemporalUnit.TemporalSeconds,
            QgsUnitTypes.TemporalUnit.TemporalMinutes,
            QgsUnitTypes.TemporalUnit.TemporalHours,
            QgsUnitTypes.TemporalUnit.TemporalDays,
            QgsUnitTypes.TemporalUnit.TemporalWeeks,
            QgsUnitTypes.TemporalUnit.TemporalMonths,
            QgsUnitTypes.TemporalUnit.TemporalYears,
            QgsUnitTypes.TemporalUnit.TemporalDecades,
            QgsUnitTypes.TemporalUnit.TemporalCenturies,
            QgsUnitTypes.TemporalUnit.TemporalIrregularStep,
            QgsUnitTypes.TemporalUnit.TemporalUnknownUnit,
        ]

        for u in units:
            res, ok = QgsUnitTypes.stringToTemporalUnit(QgsUnitTypes.toString(u))
            assert ok
            self.assertEqual(res, u)

        # Test converting bad strings
        res, ok = QgsUnitTypes.stringToTemporalUnit("bad")
        self.assertFalse(ok)
        self.assertEqual(res, QgsUnitTypes.TemporalUnit.TemporalUnknownUnit)

        # Test that string is cleaned before conversion
        res, ok = QgsUnitTypes.stringToTemporalUnit(
            f" {QgsUnitTypes.toString(QgsUnitTypes.TemporalUnit.TemporalDecades).upper()}  "
        )
        assert ok
        self.assertEqual(res, QgsUnitTypes.TemporalUnit.TemporalDecades)

    def testEncodeDecodeRenderUnits(self):
        """Test encoding and decoding render units"""
        units = [
            QgsUnitTypes.RenderUnit.RenderMillimeters,
            QgsUnitTypes.RenderUnit.RenderMetersInMapUnits,
            QgsUnitTypes.RenderUnit.RenderMapUnits,
            QgsUnitTypes.RenderUnit.RenderPixels,
            QgsUnitTypes.RenderUnit.RenderPercentage,
            QgsUnitTypes.RenderUnit.RenderPoints,
            QgsUnitTypes.RenderUnit.RenderInches,
        ]

        for u in units:
            res, ok = QgsUnitTypes.decodeRenderUnit(QgsUnitTypes.encodeUnit(u))
            assert ok
            self.assertEqual(res, u)

        # Test decoding bad units
        res, ok = QgsUnitTypes.decodeRenderUnit("bad")
        self.assertFalse(ok)
        # default units should be MM
        self.assertEqual(res, QgsUnitTypes.RenderUnit.RenderMillimeters)

        # Test that string is cleaned before decoding
        res, ok = QgsUnitTypes.decodeRenderUnit(" PiXeL  ")
        assert ok
        self.assertEqual(res, QgsUnitTypes.RenderUnit.RenderPixels)

        # check some aliases - used in data defined labeling
        res, ok = QgsUnitTypes.decodeRenderUnit("Meters")
        assert ok
        res, ok = QgsUnitTypes.decodeRenderUnit("MapUnits")
        assert ok
        self.assertEqual(res, QgsUnitTypes.RenderUnit.RenderMapUnits)
        res, ok = QgsUnitTypes.decodeRenderUnit("Percent")
        assert ok
        self.assertEqual(res, QgsUnitTypes.RenderUnit.RenderPercentage)
        res, ok = QgsUnitTypes.decodeRenderUnit("Points")
        assert ok
        self.assertEqual(res, QgsUnitTypes.RenderUnit.RenderPoints)

    def testRenderUnitsString(self):
        """Test converting render units to strings"""
        units = [
            QgsUnitTypes.RenderUnit.RenderMillimeters,
            QgsUnitTypes.RenderUnit.RenderMapUnits,
            QgsUnitTypes.RenderUnit.RenderPixels,
            QgsUnitTypes.RenderUnit.RenderPercentage,
            QgsUnitTypes.RenderUnit.RenderPoints,
            QgsUnitTypes.RenderUnit.RenderInches,
        ]

        for u in units:
            self.assertTrue(QgsUnitTypes.toString(u))

    def testFromUnitToUnitFactor(self):
        """Test calculation of conversion factor between units"""

        expected = {
            QgsUnitTypes.DistanceUnit.DistanceMeters: {
                QgsUnitTypes.DistanceUnit.DistanceMeters: 1.0,
                QgsUnitTypes.DistanceUnit.DistanceKilometers: 0.001,
                QgsUnitTypes.DistanceUnit.DistanceFeet: 3.28083989501,
                QgsUnitTypes.DistanceUnit.DistanceYards: 1.0936133,
                QgsUnitTypes.DistanceUnit.DistanceMiles: 0.00062136931818182,
                QgsUnitTypes.DistanceUnit.DistanceDegrees: 0.00000898315,
                QgsUnitTypes.DistanceUnit.DistanceNauticalMiles: 0.000539957,
                QgsUnitTypes.DistanceUnit.DistanceMillimeters: 1000.0,
                QgsUnitTypes.DistanceUnit.DistanceCentimeters: 100.0,
                Qgis.DistanceUnit.Inches: 39.37007874015748,
                Qgis.DistanceUnit.ChainsInternational: 1 / 20.1168,
                Qgis.DistanceUnit.ChainsBritishBenoit1895A: 1 / 20.1167824,
                Qgis.DistanceUnit.ChainsBritishBenoit1895B: 1 / 20.116782494376,
                Qgis.DistanceUnit.ChainsBritishSears1922Truncated: 1 / 20.116756,
                Qgis.DistanceUnit.ChainsBritishSears1922: 1 / 20.11676512155,
                Qgis.DistanceUnit.ChainsClarkes: 1 / 20.1166195164,
                Qgis.DistanceUnit.ChainsUSSurvey: 1 / 20.11684023368,
                Qgis.DistanceUnit.FeetBritish1865: 1 / 0.30480083333333,
                Qgis.DistanceUnit.FeetBritish1936: 1 / 0.3048007491,
                Qgis.DistanceUnit.FeetBritishBenoit1895A: 1 / 0.30479973333333,
                Qgis.DistanceUnit.FeetBritishBenoit1895B: 1 / 0.30479973476327,
                Qgis.DistanceUnit.FeetBritishSears1922Truncated: 1 / 0.30479933333333,
                Qgis.DistanceUnit.FeetBritishSears1922: 1 / 0.30479947153868,
                Qgis.DistanceUnit.FeetClarkes: 1 / 0.3047972654,
                Qgis.DistanceUnit.FeetGoldCoast: 1 / 0.30479971018151,
                Qgis.DistanceUnit.FeetIndian: 1 / 0.30479951024815,
                Qgis.DistanceUnit.FeetIndian1937: 1 / 0.30479841,
                Qgis.DistanceUnit.FeetIndian1962: 1 / 0.3047996,
                Qgis.DistanceUnit.FeetIndian1975: 1 / 0.3047995,
                Qgis.DistanceUnit.FeetUSSurvey: 1 / 0.30480060960122,
                Qgis.DistanceUnit.LinksInternational: 1 / 0.201168,
                Qgis.DistanceUnit.LinksBritishBenoit1895A: 1 / 0.201167824,
                Qgis.DistanceUnit.LinksBritishBenoit1895B: 1 / 0.20116782494376,
                Qgis.DistanceUnit.LinksBritishSears1922Truncated: 1 / 0.20116756,
                Qgis.DistanceUnit.LinksBritishSears1922: 1 / 0.20116765121553,
                Qgis.DistanceUnit.LinksClarkes: 1 / 0.20116619516,
                Qgis.DistanceUnit.LinksUSSurvey: 1 / 0.2011684023368,
                Qgis.DistanceUnit.YardsBritishBenoit1895A: 1 / 0.9143992,
                Qgis.DistanceUnit.YardsBritishBenoit1895B: 1 / 0.9143992042898,
                Qgis.DistanceUnit.YardsBritishSears1922Truncated: 1 / 0.914398,
                Qgis.DistanceUnit.YardsBritishSears1922: 1 / 0.91439841461603,
                Qgis.DistanceUnit.YardsClarkes: 1 / 0.9143917962,
                Qgis.DistanceUnit.YardsIndian: 1 / 0.91439853074444,
                Qgis.DistanceUnit.YardsIndian1937: 1 / 0.91439523,
                Qgis.DistanceUnit.YardsIndian1962: 1 / 0.9143988,
                Qgis.DistanceUnit.YardsIndian1975: 1 / 0.9143985,
                Qgis.DistanceUnit.MilesUSSurvey: 1 / 1609.3472186944,
                Qgis.DistanceUnit.Fathoms: 1 / 1.8288,
                Qgis.DistanceUnit.MetersGermanLegal: 1 / 1.0000135965,
            },
            QgsUnitTypes.DistanceUnit.DistanceKilometers: {
                QgsUnitTypes.DistanceUnit.DistanceMeters: 1000.0,
                QgsUnitTypes.DistanceUnit.DistanceKilometers: 1.0,
                QgsUnitTypes.DistanceUnit.DistanceFeet: 3280.8398950,
                QgsUnitTypes.DistanceUnit.DistanceYards: 1093.6132983,
                QgsUnitTypes.DistanceUnit.DistanceMiles: 0.62137121212119317271,
                QgsUnitTypes.DistanceUnit.DistanceDegrees: 0.0089832,
                QgsUnitTypes.DistanceUnit.DistanceNauticalMiles: 0.53995682073432482717,
                QgsUnitTypes.DistanceUnit.DistanceMillimeters: 1000000.0,
                QgsUnitTypes.DistanceUnit.DistanceCentimeters: 100000.0,
                Qgis.DistanceUnit.Inches: 39370.078740157485,
            },
            QgsUnitTypes.DistanceUnit.DistanceFeet: {
                QgsUnitTypes.DistanceUnit.DistanceMeters: 0.3048,
                QgsUnitTypes.DistanceUnit.DistanceKilometers: 0.0003048,
                QgsUnitTypes.DistanceUnit.DistanceFeet: 1.0,
                QgsUnitTypes.DistanceUnit.DistanceYards: 0.3333333,
                QgsUnitTypes.DistanceUnit.DistanceMiles: 0.00018939375,
                QgsUnitTypes.DistanceUnit.DistanceDegrees: 2.73806498599629e-06,
                QgsUnitTypes.DistanceUnit.DistanceNauticalMiles: 0.000164579,
                QgsUnitTypes.DistanceUnit.DistanceMillimeters: 304.8,
                QgsUnitTypes.DistanceUnit.DistanceCentimeters: 30.48,
                Qgis.DistanceUnit.Inches: 12,
            },
            QgsUnitTypes.DistanceUnit.DistanceYards: {
                QgsUnitTypes.DistanceUnit.DistanceMeters: 0.9144,
                QgsUnitTypes.DistanceUnit.DistanceKilometers: 0.0009144,
                QgsUnitTypes.DistanceUnit.DistanceFeet: 3.0,
                QgsUnitTypes.DistanceUnit.DistanceYards: 1.0,
                QgsUnitTypes.DistanceUnit.DistanceMiles: 0.000568182,
                QgsUnitTypes.DistanceUnit.DistanceDegrees: 0.0000082,
                QgsUnitTypes.DistanceUnit.DistanceNauticalMiles: 0.0004937366590756,
                QgsUnitTypes.DistanceUnit.DistanceMillimeters: 914.4,
                QgsUnitTypes.DistanceUnit.DistanceCentimeters: 91.44,
                Qgis.DistanceUnit.Inches: 36,
            },
            QgsUnitTypes.DistanceUnit.DistanceDegrees: {
                QgsUnitTypes.DistanceUnit.DistanceMeters: 111319.49079327358,
                QgsUnitTypes.DistanceUnit.DistanceKilometers: 111.3194908,
                QgsUnitTypes.DistanceUnit.DistanceFeet: 365221.4264871,
                QgsUnitTypes.DistanceUnit.DistanceYards: 121740.4754957,
                QgsUnitTypes.DistanceUnit.DistanceMiles: 69.1707247,
                QgsUnitTypes.DistanceUnit.DistanceDegrees: 1.0,
                QgsUnitTypes.DistanceUnit.DistanceNauticalMiles: 60.1077164,
                QgsUnitTypes.DistanceUnit.DistanceMillimeters: 111319490.79327358,
                QgsUnitTypes.DistanceUnit.DistanceCentimeters: 11131949.079327358,
                Qgis.DistanceUnit.Inches: 4382657.117845417,
            },
            QgsUnitTypes.DistanceUnit.DistanceMiles: {
                QgsUnitTypes.DistanceUnit.DistanceMeters: 1609.3440000,
                QgsUnitTypes.DistanceUnit.DistanceKilometers: 1.6093440,
                QgsUnitTypes.DistanceUnit.DistanceFeet: 5280.0000000,
                QgsUnitTypes.DistanceUnit.DistanceYards: 1760.0000000,
                QgsUnitTypes.DistanceUnit.DistanceMiles: 1.0,
                QgsUnitTypes.DistanceUnit.DistanceDegrees: 0.0144570,
                QgsUnitTypes.DistanceUnit.DistanceNauticalMiles: 0.8689762,
                QgsUnitTypes.DistanceUnit.DistanceMillimeters: 1609344.0,
                QgsUnitTypes.DistanceUnit.DistanceCentimeters: 160934.4,
                Qgis.DistanceUnit.Inches: 63360,
            },
            QgsUnitTypes.DistanceUnit.DistanceNauticalMiles: {
                QgsUnitTypes.DistanceUnit.DistanceMeters: 1852.0,
                QgsUnitTypes.DistanceUnit.DistanceKilometers: 1.8520000,
                QgsUnitTypes.DistanceUnit.DistanceFeet: 6076.1154856,
                QgsUnitTypes.DistanceUnit.DistanceYards: 2025.3718285,
                QgsUnitTypes.DistanceUnit.DistanceMiles: 1.1507794,
                QgsUnitTypes.DistanceUnit.DistanceDegrees: 0.0166367990650,
                QgsUnitTypes.DistanceUnit.DistanceNauticalMiles: 1.0,
                QgsUnitTypes.DistanceUnit.DistanceMillimeters: 1852000.0,
                QgsUnitTypes.DistanceUnit.DistanceCentimeters: 185200.0,
                Qgis.DistanceUnit.Inches: 72913.38582677166,
            },
            QgsUnitTypes.DistanceUnit.DistanceMillimeters: {
                QgsUnitTypes.DistanceUnit.DistanceMeters: 0.001,
                QgsUnitTypes.DistanceUnit.DistanceKilometers: 0.000001,
                QgsUnitTypes.DistanceUnit.DistanceFeet: 0.00328083989501,
                QgsUnitTypes.DistanceUnit.DistanceYards: 0.0010936133,
                QgsUnitTypes.DistanceUnit.DistanceMiles: 0.00000062136931818182,
                QgsUnitTypes.DistanceUnit.DistanceDegrees: 0.00000000898315,
                QgsUnitTypes.DistanceUnit.DistanceNauticalMiles: 0.000000539957,
                QgsUnitTypes.DistanceUnit.DistanceMillimeters: 1.0,
                QgsUnitTypes.DistanceUnit.DistanceCentimeters: 0.1,
                Qgis.DistanceUnit.Inches: 0.039370086377953,
            },
            QgsUnitTypes.DistanceUnit.DistanceCentimeters: {
                QgsUnitTypes.DistanceUnit.DistanceMeters: 0.01,
                QgsUnitTypes.DistanceUnit.DistanceKilometers: 0.00001,
                QgsUnitTypes.DistanceUnit.DistanceFeet: 0.0328083989501,
                QgsUnitTypes.DistanceUnit.DistanceYards: 0.010936133,
                QgsUnitTypes.DistanceUnit.DistanceMiles: 0.0000062136931818182,
                QgsUnitTypes.DistanceUnit.DistanceDegrees: 0.0000000898315,
                QgsUnitTypes.DistanceUnit.DistanceNauticalMiles: 0.00000539957,
                QgsUnitTypes.DistanceUnit.DistanceMillimeters: 10.0,
                QgsUnitTypes.DistanceUnit.DistanceCentimeters: 1.0,
                Qgis.DistanceUnit.Inches: 0.3937007874015748,
            },
            Qgis.DistanceUnit.Inches: {
                QgsUnitTypes.DistanceUnit.DistanceMeters: 0.0254,
                QgsUnitTypes.DistanceUnit.DistanceKilometers: 2.54e-5,
                QgsUnitTypes.DistanceUnit.DistanceFeet: 0.0833333,
                QgsUnitTypes.DistanceUnit.DistanceYards: 0.0277777666667,
                QgsUnitTypes.DistanceUnit.DistanceMiles: 1.578282196971590999e-5,
                QgsUnitTypes.DistanceUnit.DistanceDegrees: 2.2817208216635843e-07,
                QgsUnitTypes.DistanceUnit.DistanceNauticalMiles: 1.371489732183071538e-5,
                QgsUnitTypes.DistanceUnit.DistanceMillimeters: 25.4,
                QgsUnitTypes.DistanceUnit.DistanceCentimeters: 2.54,
                Qgis.DistanceUnit.Inches: 1.0,
            },
            Qgis.DistanceUnit.ChainsInternational: {Qgis.DistanceUnit.Meters: 20.1168},
            Qgis.DistanceUnit.ChainsBritishBenoit1895A: {
                Qgis.DistanceUnit.Meters: 20.1167824
            },
            Qgis.DistanceUnit.ChainsBritishBenoit1895B: {
                Qgis.DistanceUnit.Meters: 20.116782494376
            },
            Qgis.DistanceUnit.ChainsBritishSears1922Truncated: {
                Qgis.DistanceUnit.Meters: 20.116756
            },
            Qgis.DistanceUnit.ChainsBritishSears1922: {
                Qgis.DistanceUnit.Meters: 20.11676512155
            },
            Qgis.DistanceUnit.ChainsClarkes: {Qgis.DistanceUnit.Meters: 20.1166195164},
            Qgis.DistanceUnit.ChainsUSSurvey: {
                Qgis.DistanceUnit.Meters: 20.11684023368
            },
            Qgis.DistanceUnit.FeetBritish1865: {
                Qgis.DistanceUnit.Meters: 0.30480083333333
            },
            Qgis.DistanceUnit.FeetBritish1936: {Qgis.DistanceUnit.Meters: 0.3048007491},
            Qgis.DistanceUnit.FeetBritishBenoit1895A: {
                Qgis.DistanceUnit.Meters: 0.30479973333333
            },
            Qgis.DistanceUnit.FeetBritishBenoit1895B: {
                Qgis.DistanceUnit.Meters: 0.30479973476327
            },
            Qgis.DistanceUnit.FeetBritishSears1922Truncated: {
                Qgis.DistanceUnit.Meters: 0.30479933333333
            },
            Qgis.DistanceUnit.FeetBritishSears1922: {
                Qgis.DistanceUnit.Meters: 0.30479947153868
            },
            Qgis.DistanceUnit.FeetClarkes: {Qgis.DistanceUnit.Meters: 0.3047972654},
            Qgis.DistanceUnit.FeetGoldCoast: {
                Qgis.DistanceUnit.Meters: 0.30479971018151
            },
            Qgis.DistanceUnit.FeetIndian: {Qgis.DistanceUnit.Meters: 0.30479951024815},
            Qgis.DistanceUnit.FeetIndian1937: {Qgis.DistanceUnit.Meters: 0.30479841},
            Qgis.DistanceUnit.FeetIndian1962: {Qgis.DistanceUnit.Meters: 0.3047996},
            Qgis.DistanceUnit.FeetIndian1975: {Qgis.DistanceUnit.Meters: 0.3047995},
            Qgis.DistanceUnit.FeetUSSurvey: {
                Qgis.DistanceUnit.Meters: 0.30480060960122
            },
            Qgis.DistanceUnit.LinksInternational: {Qgis.DistanceUnit.Meters: 0.201168},
            Qgis.DistanceUnit.LinksBritishBenoit1895A: {
                Qgis.DistanceUnit.Meters: 0.201167824
            },
            Qgis.DistanceUnit.LinksBritishBenoit1895B: {
                Qgis.DistanceUnit.Meters: 0.20116782494376
            },
            Qgis.DistanceUnit.LinksBritishSears1922Truncated: {
                Qgis.DistanceUnit.Meters: 0.20116756
            },
            Qgis.DistanceUnit.LinksBritishSears1922: {
                Qgis.DistanceUnit.Meters: 0.20116765121553
            },
            Qgis.DistanceUnit.LinksClarkes: {Qgis.DistanceUnit.Meters: 0.20116619516},
            Qgis.DistanceUnit.LinksUSSurvey: {
                Qgis.DistanceUnit.Meters: 0.2011684023368
            },
            Qgis.DistanceUnit.YardsBritishBenoit1895A: {
                Qgis.DistanceUnit.Meters: 0.9143992
            },
            Qgis.DistanceUnit.YardsBritishBenoit1895B: {
                Qgis.DistanceUnit.Meters: 0.9143992042898
            },
            Qgis.DistanceUnit.YardsBritishSears1922Truncated: {
                Qgis.DistanceUnit.Meters: 0.914398
            },
            Qgis.DistanceUnit.YardsBritishSears1922: {
                Qgis.DistanceUnit.Meters: 0.91439841461603
            },
            Qgis.DistanceUnit.YardsClarkes: {Qgis.DistanceUnit.Meters: 0.9143917962},
            Qgis.DistanceUnit.YardsIndian: {Qgis.DistanceUnit.Meters: 0.91439853074444},
            Qgis.DistanceUnit.YardsIndian1937: {Qgis.DistanceUnit.Meters: 0.91439523},
            Qgis.DistanceUnit.YardsIndian1962: {Qgis.DistanceUnit.Meters: 0.9143988},
            Qgis.DistanceUnit.YardsIndian1975: {Qgis.DistanceUnit.Meters: 0.9143985},
            Qgis.DistanceUnit.MilesUSSurvey: {
                Qgis.DistanceUnit.Meters: 1609.3472186944
            },
            Qgis.DistanceUnit.Fathoms: {Qgis.DistanceUnit.Meters: 1.8288},
            Qgis.DistanceUnit.MetersGermanLegal: {
                Qgis.DistanceUnit.Meters: 1.0000135965
            },
            QgsUnitTypes.DistanceUnit.DistanceUnknownUnit: {
                QgsUnitTypes.DistanceUnit.DistanceMeters: 1.0,
                QgsUnitTypes.DistanceUnit.DistanceKilometers: 1.0,
                QgsUnitTypes.DistanceUnit.DistanceFeet: 1.0,
                QgsUnitTypes.DistanceUnit.DistanceYards: 1.0,
                QgsUnitTypes.DistanceUnit.DistanceMiles: 1.0,
                QgsUnitTypes.DistanceUnit.DistanceDegrees: 1.0,
                QgsUnitTypes.DistanceUnit.DistanceNauticalMiles: 1.0,
                QgsUnitTypes.DistanceUnit.DistanceMillimeters: 1.0,
                QgsUnitTypes.DistanceUnit.DistanceCentimeters: 1.0,
                Qgis.DistanceUnit.Inches: 1.0,
            },
        }

        for from_unit in list(expected.keys()):
            for to_unit in list(expected[from_unit].keys()):
                expected_factor = expected[from_unit][to_unit]
                res = QgsUnitTypes.fromUnitToUnitFactor(from_unit, to_unit)
                self.assertAlmostEqual(
                    res,
                    expected_factor,
                    msg="got {:.7f}, expected {:.7f} when converting from {} to {}".format(
                        res,
                        expected_factor,
                        QgsUnitTypes.toString(from_unit),
                        QgsUnitTypes.toString(to_unit),
                    ),
                )
                # test conversion to unknown units
                res = QgsUnitTypes.fromUnitToUnitFactor(
                    from_unit, QgsUnitTypes.DistanceUnit.DistanceUnknownUnit
                )
                self.assertAlmostEqual(
                    res,
                    1.0,
                    msg=f"got {res:.7f}, expected 1.0 when converting from {QgsUnitTypes.toString(from_unit)} to unknown units",
                )

    def testAreaFromUnitToUnitFactor(self):
        """Test calculation of conversion factor between areal units"""

        expected = {
            QgsUnitTypes.AreaUnit.AreaSquareMeters: {
                QgsUnitTypes.AreaUnit.AreaSquareMeters: 1.0,
                QgsUnitTypes.AreaUnit.AreaSquareKilometers: 1e-6,
                QgsUnitTypes.AreaUnit.AreaSquareFeet: 10.7639104,
                QgsUnitTypes.AreaUnit.AreaSquareYards: 1.19599,
                QgsUnitTypes.AreaUnit.AreaSquareMiles: 3.86102e-7,
                QgsUnitTypes.AreaUnit.AreaHectares: 0.0001,
                QgsUnitTypes.AreaUnit.AreaAcres: 0.000247105,
                QgsUnitTypes.AreaUnit.AreaSquareNauticalMiles: 2.91553e-7,
                QgsUnitTypes.AreaUnit.AreaSquareDegrees: 0.000000000080697,
                QgsUnitTypes.AreaUnit.AreaSquareMillimeters: 1e6,
                QgsUnitTypes.AreaUnit.AreaSquareCentimeters: 1e4,
                Qgis.AreaUnit.SquareInches: 1550.0031000062002,
                QgsUnitTypes.AreaUnit.AreaUnknownUnit: 1.0,
            },
            QgsUnitTypes.AreaUnit.AreaSquareKilometers: {
                QgsUnitTypes.AreaUnit.AreaSquareMeters: 1e6,
                QgsUnitTypes.AreaUnit.AreaSquareKilometers: 1,
                QgsUnitTypes.AreaUnit.AreaSquareFeet: 10763910.4167097,
                QgsUnitTypes.AreaUnit.AreaSquareYards: 1195990.04630108,
                QgsUnitTypes.AreaUnit.AreaSquareMiles: 0.386102158,
                QgsUnitTypes.AreaUnit.AreaHectares: 100,
                QgsUnitTypes.AreaUnit.AreaAcres: 247.105381467,
                QgsUnitTypes.AreaUnit.AreaSquareNauticalMiles: 0.291553349598,
                QgsUnitTypes.AreaUnit.AreaSquareDegrees: 0.000080697034968,
                QgsUnitTypes.AreaUnit.AreaSquareMillimeters: 1e12,
                QgsUnitTypes.AreaUnit.AreaSquareCentimeters: 1e10,
                QgsUnitTypes.AreaUnit.AreaUnknownUnit: 1.0,
                Qgis.AreaUnit.SquareInches: 1550003100.0062,
            },
            QgsUnitTypes.AreaUnit.AreaSquareFeet: {
                QgsUnitTypes.AreaUnit.AreaSquareMeters: 0.092903,
                QgsUnitTypes.AreaUnit.AreaSquareKilometers: 9.2903e-8,
                QgsUnitTypes.AreaUnit.AreaSquareFeet: 1.0,
                QgsUnitTypes.AreaUnit.AreaSquareYards: 0.11111111111,
                QgsUnitTypes.AreaUnit.AreaSquareMiles: 3.58701e-8,
                QgsUnitTypes.AreaUnit.AreaHectares: 9.2903e-6,
                QgsUnitTypes.AreaUnit.AreaAcres: 2.29568e-5,
                QgsUnitTypes.AreaUnit.AreaSquareNauticalMiles: 2.70862e-8,
                QgsUnitTypes.AreaUnit.AreaSquareDegrees: 0.000000000007497,
                QgsUnitTypes.AreaUnit.AreaSquareMillimeters: 92903.04,
                QgsUnitTypes.AreaUnit.AreaSquareCentimeters: 929.0304,
                QgsUnitTypes.AreaUnit.AreaUnknownUnit: 1.0,
                Qgis.AreaUnit.SquareInches: 144.0,
            },
            QgsUnitTypes.AreaUnit.AreaSquareYards: {
                QgsUnitTypes.AreaUnit.AreaSquareMeters: 0.836127360,
                QgsUnitTypes.AreaUnit.AreaSquareKilometers: 8.36127e-7,
                QgsUnitTypes.AreaUnit.AreaSquareFeet: 9.0,
                QgsUnitTypes.AreaUnit.AreaSquareYards: 1.0,
                QgsUnitTypes.AreaUnit.AreaSquareMiles: 3.22831e-7,
                QgsUnitTypes.AreaUnit.AreaHectares: 8.3612736e-5,
                QgsUnitTypes.AreaUnit.AreaAcres: 0.00020661157,
                QgsUnitTypes.AreaUnit.AreaSquareNauticalMiles: 2.43776e-7,
                QgsUnitTypes.AreaUnit.AreaSquareDegrees: 0.000000000067473,
                QgsUnitTypes.AreaUnit.AreaSquareMillimeters: 836127.360,
                QgsUnitTypes.AreaUnit.AreaSquareCentimeters: 8361.27360,
                QgsUnitTypes.AreaUnit.AreaUnknownUnit: 1.0,
                Qgis.AreaUnit.SquareInches: 1296.0,
            },
            QgsUnitTypes.AreaUnit.AreaSquareMiles: {
                QgsUnitTypes.AreaUnit.AreaSquareMeters: 2589988.110336,
                QgsUnitTypes.AreaUnit.AreaSquareKilometers: 2.589988110,
                QgsUnitTypes.AreaUnit.AreaSquareFeet: 27878400,
                QgsUnitTypes.AreaUnit.AreaSquareYards: 3097600,
                QgsUnitTypes.AreaUnit.AreaSquareMiles: 1.0,
                QgsUnitTypes.AreaUnit.AreaHectares: 258.998811,
                QgsUnitTypes.AreaUnit.AreaAcres: 640,
                QgsUnitTypes.AreaUnit.AreaSquareNauticalMiles: 0.75511970898,
                QgsUnitTypes.AreaUnit.AreaSquareDegrees: 0.000209004361107,
                QgsUnitTypes.AreaUnit.AreaSquareMillimeters: 2589988110336.0,
                QgsUnitTypes.AreaUnit.AreaSquareCentimeters: 25899881103.36,
                QgsUnitTypes.AreaUnit.AreaUnknownUnit: 1.0,
                Qgis.AreaUnit.SquareInches: 4014489600.0000005,
            },
            QgsUnitTypes.AreaUnit.AreaHectares: {
                QgsUnitTypes.AreaUnit.AreaSquareMeters: 10000,
                QgsUnitTypes.AreaUnit.AreaSquareKilometers: 0.01,
                QgsUnitTypes.AreaUnit.AreaSquareFeet: 107639.1041670972,
                QgsUnitTypes.AreaUnit.AreaSquareYards: 11959.9004630,
                QgsUnitTypes.AreaUnit.AreaSquareMiles: 0.00386102,
                QgsUnitTypes.AreaUnit.AreaHectares: 1.0,
                QgsUnitTypes.AreaUnit.AreaAcres: 2.471053814,
                QgsUnitTypes.AreaUnit.AreaSquareNauticalMiles: 0.00291553,
                QgsUnitTypes.AreaUnit.AreaSquareDegrees: 0.000000806970350,
                QgsUnitTypes.AreaUnit.AreaSquareMillimeters: 10000000000.0,
                QgsUnitTypes.AreaUnit.AreaSquareCentimeters: 100000000.0,
                QgsUnitTypes.AreaUnit.AreaUnknownUnit: 1.0,
                Qgis.AreaUnit.SquareInches: 15500031.000062,
            },
            QgsUnitTypes.AreaUnit.AreaAcres: {
                QgsUnitTypes.AreaUnit.AreaSquareMeters: 4046.8564224,
                QgsUnitTypes.AreaUnit.AreaSquareKilometers: 0.00404686,
                QgsUnitTypes.AreaUnit.AreaSquareFeet: 43560,
                QgsUnitTypes.AreaUnit.AreaSquareYards: 4840,
                QgsUnitTypes.AreaUnit.AreaSquareMiles: 0.0015625,
                QgsUnitTypes.AreaUnit.AreaHectares: 0.404685642,
                QgsUnitTypes.AreaUnit.AreaAcres: 1.0,
                QgsUnitTypes.AreaUnit.AreaSquareNauticalMiles: 0.00117987,
                QgsUnitTypes.AreaUnit.AreaSquareDegrees: 0.000000326569314,
                QgsUnitTypes.AreaUnit.AreaSquareMillimeters: 4046856422.4000005,
                QgsUnitTypes.AreaUnit.AreaSquareCentimeters: 40468564.224,
                QgsUnitTypes.AreaUnit.AreaUnknownUnit: 1.0,
                Qgis.AreaUnit.SquareInches: 6272640.0,
            },
            QgsUnitTypes.AreaUnit.AreaSquareNauticalMiles: {
                QgsUnitTypes.AreaUnit.AreaSquareMeters: 3429904,
                QgsUnitTypes.AreaUnit.AreaSquareKilometers: 3.4299040,
                QgsUnitTypes.AreaUnit.AreaSquareFeet: 36919179.39391434,
                QgsUnitTypes.AreaUnit.AreaSquareYards: 4102131.04376826,
                QgsUnitTypes.AreaUnit.AreaSquareMiles: 1.324293337,
                QgsUnitTypes.AreaUnit.AreaHectares: 342.9904000000,
                QgsUnitTypes.AreaUnit.AreaAcres: 847.54773631,
                QgsUnitTypes.AreaUnit.AreaSquareNauticalMiles: 1.0,
                QgsUnitTypes.AreaUnit.AreaSquareDegrees: 0.000276783083025,
                QgsUnitTypes.AreaUnit.AreaSquareMillimeters: 3429904000000.0,
                QgsUnitTypes.AreaUnit.AreaSquareCentimeters: 34299040000.0,
                QgsUnitTypes.AreaUnit.AreaUnknownUnit: 1.0,
                Qgis.AreaUnit.SquareInches: 5316361832.723665,
            },
            QgsUnitTypes.AreaUnit.AreaSquareDegrees: {
                QgsUnitTypes.AreaUnit.AreaSquareMeters: 12392029030.5,
                QgsUnitTypes.AreaUnit.AreaSquareKilometers: 12392.029030499,
                QgsUnitTypes.AreaUnit.AreaSquareFeet: 133386690365.5682220,
                QgsUnitTypes.AreaUnit.AreaSquareYards: 14820743373.9520263,
                QgsUnitTypes.AreaUnit.AreaSquareMiles: 4784.5891573967,
                QgsUnitTypes.AreaUnit.AreaHectares: 1239202.903050,
                QgsUnitTypes.AreaUnit.AreaAcres: 3062137.060733889,
                QgsUnitTypes.AreaUnit.AreaSquareNauticalMiles: 3612.93757215,
                QgsUnitTypes.AreaUnit.AreaSquareDegrees: 1.0,
                QgsUnitTypes.AreaUnit.AreaSquareMillimeters: 12392029030500000.0,
                QgsUnitTypes.AreaUnit.AreaSquareCentimeters: 123920290305000.0,
                QgsUnitTypes.AreaUnit.AreaUnknownUnit: 1.0,
                Qgis.AreaUnit.SquareInches: 19207683412641.824,
            },
            QgsUnitTypes.AreaUnit.AreaSquareMillimeters: {
                QgsUnitTypes.AreaUnit.AreaSquareMeters: 1e-6,
                QgsUnitTypes.AreaUnit.AreaSquareKilometers: 1e-12,
                QgsUnitTypes.AreaUnit.AreaSquareFeet: 0.000010763910417,
                QgsUnitTypes.AreaUnit.AreaSquareYards: 0.000001195990046,
                QgsUnitTypes.AreaUnit.AreaSquareMiles: 3.861021585424458e-13,
                QgsUnitTypes.AreaUnit.AreaHectares: 1e-10,
                QgsUnitTypes.AreaUnit.AreaAcres: 2.471053814671653e-10,
                QgsUnitTypes.AreaUnit.AreaSquareNauticalMiles: 2.9155334959812287e-13,
                QgsUnitTypes.AreaUnit.AreaSquareDegrees: 8.069703496810251e-17,
                QgsUnitTypes.AreaUnit.AreaSquareMillimeters: 1.0,
                QgsUnitTypes.AreaUnit.AreaSquareCentimeters: 0.01,
                QgsUnitTypes.AreaUnit.AreaUnknownUnit: 1.0,
                Qgis.AreaUnit.SquareInches: 0.0015500031000062,
            },
            QgsUnitTypes.AreaUnit.AreaSquareCentimeters: {
                QgsUnitTypes.AreaUnit.AreaSquareMeters: 1e-4,
                QgsUnitTypes.AreaUnit.AreaSquareKilometers: 1e-10,
                QgsUnitTypes.AreaUnit.AreaSquareFeet: 0.0010763910417,
                QgsUnitTypes.AreaUnit.AreaSquareYards: 0.0001195990046,
                QgsUnitTypes.AreaUnit.AreaSquareMiles: 3.861021585424458e-11,
                QgsUnitTypes.AreaUnit.AreaHectares: 1e-8,
                QgsUnitTypes.AreaUnit.AreaAcres: 2.471053814671653e-8,
                QgsUnitTypes.AreaUnit.AreaSquareNauticalMiles: 2.9155334959812287e-11,
                QgsUnitTypes.AreaUnit.AreaSquareDegrees: 8.069703496810251e-15,
                QgsUnitTypes.AreaUnit.AreaSquareMillimeters: 100,
                QgsUnitTypes.AreaUnit.AreaSquareCentimeters: 1.0,
                QgsUnitTypes.AreaUnit.AreaUnknownUnit: 1.0,
                Qgis.AreaUnit.SquareInches: 0.15500031000062,
            },
            Qgis.AreaUnit.SquareInches: {
                QgsUnitTypes.AreaUnit.AreaSquareMeters: 0.00064516,
                QgsUnitTypes.AreaUnit.AreaSquareKilometers: 6.4516e-10,
                QgsUnitTypes.AreaUnit.AreaSquareFeet: 0.00694444,
                QgsUnitTypes.AreaUnit.AreaSquareYards: 0.0007716044444444,
                QgsUnitTypes.AreaUnit.AreaSquareMiles: 2.490975091827221046e-10,
                QgsUnitTypes.AreaUnit.AreaHectares: 6.451595870975627624e-8,
                QgsUnitTypes.AreaUnit.AreaAcres: 1.594224058769421271e-7,
                QgsUnitTypes.AreaUnit.AreaSquareNauticalMiles: 2.490975091827221046e-10,
                QgsUnitTypes.AreaUnit.AreaSquareDegrees: 8.069703496810251e-15,
                QgsUnitTypes.AreaUnit.AreaSquareMillimeters: 645.16,
                QgsUnitTypes.AreaUnit.AreaSquareCentimeters: 6.451599999999999,
                QgsUnitTypes.AreaUnit.AreaUnknownUnit: 1.0,
                Qgis.AreaUnit.SquareInches: 1,
            },
        }

        for from_unit in list(expected.keys()):
            for to_unit in list(expected[from_unit].keys()):
                expected_factor = expected[from_unit][to_unit]
                res = QgsUnitTypes.fromUnitToUnitFactor(from_unit, to_unit)
                self.assertAlmostEqual(
                    res,
                    expected_factor,
                    msg="got {:.15f}, expected {:.15f} when converting from {} to {}".format(
                        res,
                        expected_factor,
                        QgsUnitTypes.toString(from_unit),
                        QgsUnitTypes.toString(to_unit),
                    ),
                )
                # test conversion to unknown units
                res = QgsUnitTypes.fromUnitToUnitFactor(
                    from_unit, QgsUnitTypes.AreaUnit.AreaUnknownUnit
                )
                self.assertAlmostEqual(
                    res,
                    1.0,
                    msg=f"got {res:.7f}, expected 1.0 when converting from {QgsUnitTypes.toString(from_unit)} to unknown units",
                )

    def testDistanceToAreaUnit(self):
        """Test distanceToAreaUnit conversion"""
        expected = {
            QgsUnitTypes.DistanceUnit.DistanceMeters: QgsUnitTypes.AreaUnit.AreaSquareMeters,
            QgsUnitTypes.DistanceUnit.DistanceKilometers: QgsUnitTypes.AreaUnit.AreaSquareKilometers,
            QgsUnitTypes.DistanceUnit.DistanceFeet: QgsUnitTypes.AreaUnit.AreaSquareFeet,
            QgsUnitTypes.DistanceUnit.DistanceYards: QgsUnitTypes.AreaUnit.AreaSquareYards,
            QgsUnitTypes.DistanceUnit.DistanceMiles: QgsUnitTypes.AreaUnit.AreaSquareMiles,
            QgsUnitTypes.DistanceUnit.DistanceDegrees: QgsUnitTypes.AreaUnit.AreaSquareDegrees,
            QgsUnitTypes.DistanceUnit.DistanceCentimeters: QgsUnitTypes.AreaUnit.AreaSquareCentimeters,
            QgsUnitTypes.DistanceUnit.DistanceMillimeters: QgsUnitTypes.AreaUnit.AreaSquareMillimeters,
            QgsUnitTypes.DistanceUnit.DistanceUnknownUnit: QgsUnitTypes.AreaUnit.AreaUnknownUnit,
            QgsUnitTypes.DistanceUnit.DistanceNauticalMiles: QgsUnitTypes.AreaUnit.AreaSquareNauticalMiles,
            Qgis.DistanceUnit.Inches: Qgis.AreaUnit.SquareInches,
            Qgis.DistanceUnit.ChainsInternational: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.ChainsBritishBenoit1895A: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.ChainsBritishBenoit1895B: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.ChainsBritishSears1922Truncated: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.ChainsBritishSears1922: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.ChainsClarkes: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.ChainsUSSurvey: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.FeetBritish1865: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.FeetBritish1936: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.FeetBritishBenoit1895A: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.FeetBritishBenoit1895B: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.FeetBritishSears1922Truncated: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.FeetBritishSears1922: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.FeetClarkes: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.FeetGoldCoast: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.FeetIndian: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.FeetIndian1937: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.FeetIndian1962: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.FeetIndian1975: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.FeetUSSurvey: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.LinksInternational: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.LinksBritishBenoit1895A: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.LinksBritishBenoit1895B: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.LinksBritishSears1922Truncated: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.LinksBritishSears1922: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.LinksClarkes: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.LinksUSSurvey: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.YardsBritishBenoit1895A: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.YardsBritishBenoit1895B: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.YardsBritishSears1922Truncated: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.YardsBritishSears1922: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.YardsClarkes: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.YardsIndian: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.YardsIndian1937: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.YardsIndian1962: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.YardsIndian1975: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.MilesUSSurvey: Qgis.AreaUnit.SquareMiles,
            Qgis.DistanceUnit.Fathoms: Qgis.AreaUnit.SquareFeet,
            Qgis.DistanceUnit.MetersGermanLegal: Qgis.AreaUnit.SquareMeters,
        }

        for t in list(expected.keys()):
            self.assertEqual(QgsUnitTypes.distanceToAreaUnit(t), expected[t])

    def testAreaToDistanceUnit(self):
        """Test areaToDistanceUnit conversion"""
        expected = {
            QgsUnitTypes.AreaUnit.AreaSquareMeters: QgsUnitTypes.DistanceUnit.DistanceMeters,
            QgsUnitTypes.AreaUnit.AreaSquareKilometers: QgsUnitTypes.DistanceUnit.DistanceKilometers,
            QgsUnitTypes.AreaUnit.AreaSquareFeet: QgsUnitTypes.DistanceUnit.DistanceFeet,
            QgsUnitTypes.AreaUnit.AreaSquareYards: QgsUnitTypes.DistanceUnit.DistanceYards,
            QgsUnitTypes.AreaUnit.AreaSquareMiles: QgsUnitTypes.DistanceUnit.DistanceMiles,
            QgsUnitTypes.AreaUnit.AreaHectares: QgsUnitTypes.DistanceUnit.DistanceMeters,
            QgsUnitTypes.AreaUnit.AreaAcres: QgsUnitTypes.DistanceUnit.DistanceYards,
            QgsUnitTypes.AreaUnit.AreaSquareDegrees: QgsUnitTypes.DistanceUnit.DistanceDegrees,
            QgsUnitTypes.AreaUnit.AreaSquareCentimeters: QgsUnitTypes.DistanceUnit.DistanceCentimeters,
            QgsUnitTypes.AreaUnit.AreaSquareMillimeters: QgsUnitTypes.DistanceUnit.DistanceMillimeters,
            QgsUnitTypes.AreaUnit.AreaUnknownUnit: QgsUnitTypes.DistanceUnit.DistanceUnknownUnit,
            QgsUnitTypes.AreaUnit.AreaSquareNauticalMiles: QgsUnitTypes.DistanceUnit.DistanceNauticalMiles,
            Qgis.AreaUnit.SquareInches: Qgis.DistanceUnit.Inches,
        }

        for t in list(expected.keys()):
            self.assertEqual(QgsUnitTypes.areaToDistanceUnit(t), expected[t])

    def testVolumeFromUnitToUnitFactor(self):
        """Test calculation of conversion factor between volume units"""

        expected = {
            QgsUnitTypes.VolumeUnit.VolumeCubicMeters: {
                QgsUnitTypes.VolumeUnit.VolumeCubicMeters: 1.0,
                QgsUnitTypes.VolumeUnit.VolumeCubicFeet: 35.314666572222,
                QgsUnitTypes.VolumeUnit.VolumeCubicYards: 1.307950613786,
                QgsUnitTypes.VolumeUnit.VolumeBarrel: 6.2898107438466,
                QgsUnitTypes.VolumeUnit.VolumeCubicDecimeter: 1000,
                QgsUnitTypes.VolumeUnit.VolumeLiters: 1000,
                QgsUnitTypes.VolumeUnit.VolumeGallonUS: 264.17205124156,
                QgsUnitTypes.VolumeUnit.VolumeCubicInch: 61023.7438368,
                QgsUnitTypes.VolumeUnit.VolumeCubicCentimeter: 1000000,
                QgsUnitTypes.VolumeUnit.VolumeCubicDegrees: 7.24913798948971e-16,
                QgsUnitTypes.VolumeUnit.VolumeUnknownUnit: 1.0,
            },
            QgsUnitTypes.VolumeUnit.VolumeCubicFeet: {
                QgsUnitTypes.VolumeUnit.VolumeCubicMeters: 0.0283168,
                QgsUnitTypes.VolumeUnit.VolumeCubicFeet: 1.0,
                QgsUnitTypes.VolumeUnit.VolumeCubicYards: 0.037037,
                QgsUnitTypes.VolumeUnit.VolumeBarrel: 0.178107622,
                QgsUnitTypes.VolumeUnit.VolumeCubicDecimeter: 28.31685,
                QgsUnitTypes.VolumeUnit.VolumeLiters: 28.31685,
                QgsUnitTypes.VolumeUnit.VolumeGallonUS: 7.48052,
                QgsUnitTypes.VolumeUnit.VolumeCubicInch: 1728.000629765,
                QgsUnitTypes.VolumeUnit.VolumeCubicCentimeter: 28316.85,
                QgsUnitTypes.VolumeUnit.VolumeCubicDegrees: 2.0527272837261945e-17,
                QgsUnitTypes.VolumeUnit.VolumeUnknownUnit: 1.0,
            },
            QgsUnitTypes.VolumeUnit.VolumeCubicYards: {
                QgsUnitTypes.VolumeUnit.VolumeCubicMeters: 0.7645549,
                QgsUnitTypes.VolumeUnit.VolumeCubicFeet: 26.999998234,
                QgsUnitTypes.VolumeUnit.VolumeCubicYards: 1.0,
                QgsUnitTypes.VolumeUnit.VolumeBarrel: 4.808905491,
                QgsUnitTypes.VolumeUnit.VolumeCubicDecimeter: 764.5549,
                QgsUnitTypes.VolumeUnit.VolumeLiters: 764.5549,
                QgsUnitTypes.VolumeUnit.VolumeGallonUS: 201.974025549,
                QgsUnitTypes.VolumeUnit.VolumeCubicInch: 46656.013952472,
                QgsUnitTypes.VolumeUnit.VolumeCubicCentimeter: 764554.9,
                QgsUnitTypes.VolumeUnit.VolumeCubicDegrees: 5.542363970640507e-16,
                QgsUnitTypes.VolumeUnit.VolumeUnknownUnit: 1.0,
            },
            QgsUnitTypes.VolumeUnit.VolumeBarrel: {
                QgsUnitTypes.VolumeUnit.VolumeCubicMeters: 0.158987294928,
                QgsUnitTypes.VolumeUnit.VolumeCubicFeet: 5.614582837,
                QgsUnitTypes.VolumeUnit.VolumeCubicYards: 0.207947526,
                QgsUnitTypes.VolumeUnit.VolumeBarrel: 1.0,
                QgsUnitTypes.VolumeUnit.VolumeCubicDecimeter: 158.9873,
                QgsUnitTypes.VolumeUnit.VolumeLiters: 158.9873,
                QgsUnitTypes.VolumeUnit.VolumeGallonUS: 41.999998943,
                QgsUnitTypes.VolumeUnit.VolumeCubicInch: 9702.002677722,
                QgsUnitTypes.VolumeUnit.VolumeCubicCentimeter: 158987.3,
                QgsUnitTypes.VolumeUnit.VolumeCubicDegrees: 1.1525208762763973e-16,
                QgsUnitTypes.VolumeUnit.VolumeUnknownUnit: 1.0,
            },
            QgsUnitTypes.VolumeUnit.VolumeCubicDecimeter: {
                QgsUnitTypes.VolumeUnit.VolumeCubicMeters: 0.001,
                QgsUnitTypes.VolumeUnit.VolumeCubicFeet: 0.0353147,
                QgsUnitTypes.VolumeUnit.VolumeCubicYards: 0.00130795,
                QgsUnitTypes.VolumeUnit.VolumeBarrel: 0.00628981,
                QgsUnitTypes.VolumeUnit.VolumeCubicDecimeter: 1.0,
                QgsUnitTypes.VolumeUnit.VolumeLiters: 1.0,
                QgsUnitTypes.VolumeUnit.VolumeGallonUS: 0.264172,
                QgsUnitTypes.VolumeUnit.VolumeCubicInch: 61.02375899,
                QgsUnitTypes.VolumeUnit.VolumeCubicCentimeter: 1000,
                QgsUnitTypes.VolumeUnit.VolumeCubicDegrees: 7.24913798948971e-19,
                QgsUnitTypes.VolumeUnit.VolumeUnknownUnit: 1.0,
            },
            QgsUnitTypes.VolumeUnit.VolumeLiters: {
                QgsUnitTypes.VolumeUnit.VolumeCubicMeters: 0.001,
                QgsUnitTypes.VolumeUnit.VolumeCubicFeet: 0.0353147,
                QgsUnitTypes.VolumeUnit.VolumeCubicYards: 0.00130795,
                QgsUnitTypes.VolumeUnit.VolumeBarrel: 0.00628981,
                QgsUnitTypes.VolumeUnit.VolumeCubicDecimeter: 1.0,
                QgsUnitTypes.VolumeUnit.VolumeLiters: 1.0,
                QgsUnitTypes.VolumeUnit.VolumeGallonUS: 0.264172,
                QgsUnitTypes.VolumeUnit.VolumeCubicInch: 61.02375899,
                QgsUnitTypes.VolumeUnit.VolumeCubicCentimeter: 1000,
                QgsUnitTypes.VolumeUnit.VolumeCubicDegrees: 7.24913798948971e-19,
                QgsUnitTypes.VolumeUnit.VolumeUnknownUnit: 1.0,
            },
            QgsUnitTypes.VolumeUnit.VolumeGallonUS: {
                QgsUnitTypes.VolumeUnit.VolumeCubicMeters: 0.00378541,
                QgsUnitTypes.VolumeUnit.VolumeCubicFeet: 0.133680547,
                QgsUnitTypes.VolumeUnit.VolumeCubicYards: 0.00495113,
                QgsUnitTypes.VolumeUnit.VolumeBarrel: 0.023809524,
                QgsUnitTypes.VolumeUnit.VolumeCubicDecimeter: 3.785412,
                QgsUnitTypes.VolumeUnit.VolumeLiters: 3.785412,
                QgsUnitTypes.VolumeUnit.VolumeGallonUS: 1.0,
                QgsUnitTypes.VolumeUnit.VolumeCubicInch: 231.000069567,
                QgsUnitTypes.VolumeUnit.VolumeCubicCentimeter: 3785.412,
                QgsUnitTypes.VolumeUnit.VolumeCubicDegrees: 2.7440973935070226e-18,
                QgsUnitTypes.VolumeUnit.VolumeUnknownUnit: 1.0,
            },
            QgsUnitTypes.VolumeUnit.VolumeCubicInch: {
                QgsUnitTypes.VolumeUnit.VolumeCubicMeters: 1.63871e-5,
                QgsUnitTypes.VolumeUnit.VolumeCubicFeet: 0.000578704,
                QgsUnitTypes.VolumeUnit.VolumeCubicYards: 2.14335e-5,
                QgsUnitTypes.VolumeUnit.VolumeBarrel: 0.000103072,
                QgsUnitTypes.VolumeUnit.VolumeCubicDecimeter: 0.0163871,
                QgsUnitTypes.VolumeUnit.VolumeLiters: 0.0163871,
                QgsUnitTypes.VolumeUnit.VolumeGallonUS: 0.004329,
                QgsUnitTypes.VolumeUnit.VolumeCubicInch: 1.0,
                QgsUnitTypes.VolumeUnit.VolumeCubicCentimeter: 16.38706,
                QgsUnitTypes.VolumeUnit.VolumeCubicDegrees: 1.187916242337679e-20,
                QgsUnitTypes.VolumeUnit.VolumeUnknownUnit: 1.0,
            },
            QgsUnitTypes.VolumeUnit.VolumeCubicCentimeter: {
                QgsUnitTypes.VolumeUnit.VolumeCubicMeters: 1e-6,
                QgsUnitTypes.VolumeUnit.VolumeCubicFeet: 3.53147e-5,
                QgsUnitTypes.VolumeUnit.VolumeCubicYards: 1.30795e-6,
                QgsUnitTypes.VolumeUnit.VolumeBarrel: 6.28981e-6,
                QgsUnitTypes.VolumeUnit.VolumeCubicDecimeter: 0.001,
                QgsUnitTypes.VolumeUnit.VolumeLiters: 0.001,
                QgsUnitTypes.VolumeUnit.VolumeGallonUS: 0.000264172,
                QgsUnitTypes.VolumeUnit.VolumeCubicInch: 0.061023759,
                QgsUnitTypes.VolumeUnit.VolumeCubicCentimeter: 1.0,
                QgsUnitTypes.VolumeUnit.VolumeCubicDegrees: 7.24913798948971e-22,
                QgsUnitTypes.VolumeUnit.VolumeUnknownUnit: 1.0,
            },
            QgsUnitTypes.VolumeUnit.VolumeCubicDegrees: {
                QgsUnitTypes.VolumeUnit.VolumeCubicMeters: 1379474361572186.2500000,
                QgsUnitTypes.VolumeUnit.VolumeCubicFeet: 39062363874236.74,
                QgsUnitTypes.VolumeUnit.VolumeCubicYards: 1054683882564386.8,
                QgsUnitTypes.VolumeUnit.VolumeBarrel: 219318904165585.66,
                QgsUnitTypes.VolumeUnit.VolumeCubicDecimeter: 1379474361572.1863,
                QgsUnitTypes.VolumeUnit.VolumeLiters: 1379474361572.1863,
                QgsUnitTypes.VolumeUnit.VolumeGallonUS: 5221878801987.693,
                QgsUnitTypes.VolumeUnit.VolumeCubicInch: 22605446363.083416,
                QgsUnitTypes.VolumeUnit.VolumeCubicCentimeter: 1379474361.5721862,
                QgsUnitTypes.VolumeUnit.VolumeCubicDegrees: 1.0,
                QgsUnitTypes.VolumeUnit.VolumeUnknownUnit: 1.0,
            },
        }

        for from_unit in list(expected.keys()):
            for to_unit in list(expected[from_unit].keys()):
                expected_factor = expected[from_unit][to_unit]
                res = QgsUnitTypes.fromUnitToUnitFactor(from_unit, to_unit)
                self.assertAlmostEqual(
                    res,
                    expected_factor,
                    msg="got {:.15f}, expected {:.15f} when converting from {} to {}".format(
                        res,
                        expected_factor,
                        QgsUnitTypes.toString(from_unit),
                        QgsUnitTypes.toString(to_unit),
                    ),
                )
                # test conversion to unknown units
                res = QgsUnitTypes.fromUnitToUnitFactor(
                    from_unit, QgsUnitTypes.VolumeUnit.VolumeUnknownUnit
                )
                self.assertAlmostEqual(
                    res,
                    1.0,
                    msg=f"got {res:.7f}, expected 1.0 when converting from {QgsUnitTypes.toString(from_unit)} to unknown units",
                )

    def testTemporalFromUnitToUnitFactor(self):
        """Test calculation of conversion factor between temporal units"""

        expected = {
            QgsUnitTypes.TemporalUnit.TemporalMilliseconds: {
                QgsUnitTypes.TemporalUnit.TemporalMilliseconds: 1.0,
                QgsUnitTypes.TemporalUnit.TemporalSeconds: 0.001,
                QgsUnitTypes.TemporalUnit.TemporalMinutes: 1.66667e-5,
                QgsUnitTypes.TemporalUnit.TemporalHours: 2.7777777777777776e-07,
                QgsUnitTypes.TemporalUnit.TemporalDays: 1.157554211999884014e-8,
                QgsUnitTypes.TemporalUnit.TemporalWeeks: 1.65344e-9,
                QgsUnitTypes.TemporalUnit.TemporalMonths: 3.805172816249e-10,
                QgsUnitTypes.TemporalUnit.TemporalYears: 3.170980821917834278e-11,
                QgsUnitTypes.TemporalUnit.TemporalDecades: 3.170980821917834117e-12,
                QgsUnitTypes.TemporalUnit.TemporalCenturies: 3.170980821917834319e-13,
                QgsUnitTypes.TemporalUnit.TemporalUnknownUnit: 1.0,
                QgsUnitTypes.TemporalUnit.TemporalIrregularStep: 1.0,
            },
            QgsUnitTypes.TemporalUnit.TemporalSeconds: {
                QgsUnitTypes.TemporalUnit.TemporalMilliseconds: 1000.0,
                QgsUnitTypes.TemporalUnit.TemporalSeconds: 1,
                QgsUnitTypes.TemporalUnit.TemporalMinutes: 0.016666666666666666,
                QgsUnitTypes.TemporalUnit.TemporalHours: 0.0002777777777777778,
                QgsUnitTypes.TemporalUnit.TemporalDays: 1.157408000000009502e-5,
                QgsUnitTypes.TemporalUnit.TemporalWeeks: 1.653440000000013514e-6,
                QgsUnitTypes.TemporalUnit.TemporalMonths: 3.8580246913580245e-7,
                QgsUnitTypes.TemporalUnit.TemporalYears: 3.170980821917834046e-8,
                QgsUnitTypes.TemporalUnit.TemporalDecades: 3.170980821917834046e-9,
                QgsUnitTypes.TemporalUnit.TemporalCenturies: 3.170980821917834149e-10,
                QgsUnitTypes.TemporalUnit.TemporalUnknownUnit: 1.0,
                QgsUnitTypes.TemporalUnit.TemporalIrregularStep: 1.0,
            },
            QgsUnitTypes.TemporalUnit.TemporalMinutes: {
                QgsUnitTypes.TemporalUnit.TemporalMilliseconds: 60000.0,
                QgsUnitTypes.TemporalUnit.TemporalSeconds: 60,
                QgsUnitTypes.TemporalUnit.TemporalMinutes: 1,
                QgsUnitTypes.TemporalUnit.TemporalHours: 0.016666666666666666,
                QgsUnitTypes.TemporalUnit.TemporalDays: 0.0006944444444444445,
                QgsUnitTypes.TemporalUnit.TemporalWeeks: 9.92063492063492e-5,
                QgsUnitTypes.TemporalUnit.TemporalMonths: 2.3148148148148147e-05,
                QgsUnitTypes.TemporalUnit.TemporalYears: 1.901285268841737e-6,
                QgsUnitTypes.TemporalUnit.TemporalDecades: 1.901285268841737e-7,
                QgsUnitTypes.TemporalUnit.TemporalCenturies: 1.9028288416436452e-8,
                QgsUnitTypes.TemporalUnit.TemporalUnknownUnit: 1.0,
                QgsUnitTypes.TemporalUnit.TemporalIrregularStep: 1.0,
            },
            QgsUnitTypes.TemporalUnit.TemporalHours: {
                QgsUnitTypes.TemporalUnit.TemporalMilliseconds: 3600000.0,
                QgsUnitTypes.TemporalUnit.TemporalSeconds: 3600,
                QgsUnitTypes.TemporalUnit.TemporalMinutes: 60,
                QgsUnitTypes.TemporalUnit.TemporalHours: 1,
                QgsUnitTypes.TemporalUnit.TemporalDays: 0.041666666666666664,
                QgsUnitTypes.TemporalUnit.TemporalWeeks: 0.005952380952380952,
                QgsUnitTypes.TemporalUnit.TemporalMonths: 0.001388888888888889,
                QgsUnitTypes.TemporalUnit.TemporalYears: 0.00011407711613050422,
                QgsUnitTypes.TemporalUnit.TemporalDecades: 1.1407711613050422e-5,
                QgsUnitTypes.TemporalUnit.TemporalCenturies: 1.1407711613050422e-6,
                QgsUnitTypes.TemporalUnit.TemporalUnknownUnit: 1.0,
                QgsUnitTypes.TemporalUnit.TemporalIrregularStep: 1.0,
            },
            QgsUnitTypes.TemporalUnit.TemporalDays: {
                QgsUnitTypes.TemporalUnit.TemporalMilliseconds: 8.64e7,
                QgsUnitTypes.TemporalUnit.TemporalSeconds: 86400,
                QgsUnitTypes.TemporalUnit.TemporalMinutes: 1440,
                QgsUnitTypes.TemporalUnit.TemporalHours: 24,
                QgsUnitTypes.TemporalUnit.TemporalDays: 1,
                QgsUnitTypes.TemporalUnit.TemporalWeeks: 0.14285714285714285,
                QgsUnitTypes.TemporalUnit.TemporalMonths: 0.03333333333333333,
                QgsUnitTypes.TemporalUnit.TemporalYears: 0.0027378507871321013,
                QgsUnitTypes.TemporalUnit.TemporalDecades: 0.0002737850787132101,
                QgsUnitTypes.TemporalUnit.TemporalCenturies: 2.7378507871321012e-5,
                QgsUnitTypes.TemporalUnit.TemporalUnknownUnit: 1.0,
                QgsUnitTypes.TemporalUnit.TemporalIrregularStep: 1.0,
            },
            QgsUnitTypes.TemporalUnit.TemporalWeeks: {
                QgsUnitTypes.TemporalUnit.TemporalMilliseconds: 6.048e8,
                QgsUnitTypes.TemporalUnit.TemporalSeconds: 604800,
                QgsUnitTypes.TemporalUnit.TemporalMinutes: 10080,
                QgsUnitTypes.TemporalUnit.TemporalHours: 168,
                QgsUnitTypes.TemporalUnit.TemporalDays: 7,
                QgsUnitTypes.TemporalUnit.TemporalWeeks: 1,
                QgsUnitTypes.TemporalUnit.TemporalMonths: 0.23333333333333334,
                QgsUnitTypes.TemporalUnit.TemporalYears: 0.019164955509924708,
                QgsUnitTypes.TemporalUnit.TemporalDecades: 0.0019164955509924709,
                QgsUnitTypes.TemporalUnit.TemporalCenturies: 0.0001916495550992471,
                QgsUnitTypes.TemporalUnit.TemporalUnknownUnit: 1.0,
                QgsUnitTypes.TemporalUnit.TemporalIrregularStep: 1.0,
            },
            QgsUnitTypes.TemporalUnit.TemporalMonths: {
                QgsUnitTypes.TemporalUnit.TemporalMilliseconds: 2592000000.0,
                QgsUnitTypes.TemporalUnit.TemporalSeconds: 2592000.0,
                QgsUnitTypes.TemporalUnit.TemporalMinutes: 43200.0,
                QgsUnitTypes.TemporalUnit.TemporalHours: 720.0,
                QgsUnitTypes.TemporalUnit.TemporalDays: 30.0,
                QgsUnitTypes.TemporalUnit.TemporalWeeks: 4.285714285714286,
                QgsUnitTypes.TemporalUnit.TemporalMonths: 1,
                QgsUnitTypes.TemporalUnit.TemporalYears: 0.08213552361396304,
                QgsUnitTypes.TemporalUnit.TemporalDecades: 0.008213552361396304,
                QgsUnitTypes.TemporalUnit.TemporalCenturies: 0.0008213552361396304,
                QgsUnitTypes.TemporalUnit.TemporalUnknownUnit: 1.0,
                QgsUnitTypes.TemporalUnit.TemporalIrregularStep: 1.0,
            },
            QgsUnitTypes.TemporalUnit.TemporalYears: {
                QgsUnitTypes.TemporalUnit.TemporalMilliseconds: 31557600000.0,
                QgsUnitTypes.TemporalUnit.TemporalSeconds: 31557600.0,
                QgsUnitTypes.TemporalUnit.TemporalMinutes: 525960.0,
                QgsUnitTypes.TemporalUnit.TemporalHours: 8766.0,
                QgsUnitTypes.TemporalUnit.TemporalDays: 365.25,
                QgsUnitTypes.TemporalUnit.TemporalWeeks: 52.17857142857143,
                QgsUnitTypes.TemporalUnit.TemporalMonths: 12.175,
                QgsUnitTypes.TemporalUnit.TemporalYears: 1,
                QgsUnitTypes.TemporalUnit.TemporalDecades: 0.1,
                QgsUnitTypes.TemporalUnit.TemporalCenturies: 0.01,
                QgsUnitTypes.TemporalUnit.TemporalUnknownUnit: 1.0,
                QgsUnitTypes.TemporalUnit.TemporalIrregularStep: 1.0,
            },
            QgsUnitTypes.TemporalUnit.TemporalDecades: {
                QgsUnitTypes.TemporalUnit.TemporalMilliseconds: 315576000000.0,
                QgsUnitTypes.TemporalUnit.TemporalSeconds: 315576000.0,
                QgsUnitTypes.TemporalUnit.TemporalMinutes: 5259600.0,
                QgsUnitTypes.TemporalUnit.TemporalHours: 87660.0,
                QgsUnitTypes.TemporalUnit.TemporalDays: 3652.5,
                QgsUnitTypes.TemporalUnit.TemporalWeeks: 521.7857142857143,
                QgsUnitTypes.TemporalUnit.TemporalMonths: 121.75,
                QgsUnitTypes.TemporalUnit.TemporalYears: 10,
                QgsUnitTypes.TemporalUnit.TemporalDecades: 1,
                QgsUnitTypes.TemporalUnit.TemporalCenturies: 0.1,
                QgsUnitTypes.TemporalUnit.TemporalUnknownUnit: 1.0,
                QgsUnitTypes.TemporalUnit.TemporalIrregularStep: 1.0,
            },
            QgsUnitTypes.TemporalUnit.TemporalCenturies: {
                QgsUnitTypes.TemporalUnit.TemporalMilliseconds: 3155760000000.0,
                QgsUnitTypes.TemporalUnit.TemporalSeconds: 3155760000.0,
                QgsUnitTypes.TemporalUnit.TemporalMinutes: 52596000.0,
                QgsUnitTypes.TemporalUnit.TemporalHours: 876600.0,
                QgsUnitTypes.TemporalUnit.TemporalDays: 36525.0,
                QgsUnitTypes.TemporalUnit.TemporalWeeks: 5217.857142857143,
                QgsUnitTypes.TemporalUnit.TemporalMonths: 1217.5,
                QgsUnitTypes.TemporalUnit.TemporalYears: 100,
                QgsUnitTypes.TemporalUnit.TemporalDecades: 10,
                QgsUnitTypes.TemporalUnit.TemporalCenturies: 1,
                QgsUnitTypes.TemporalUnit.TemporalUnknownUnit: 1.0,
                QgsUnitTypes.TemporalUnit.TemporalIrregularStep: 1.0,
            },
        }

        for from_unit in list(expected.keys()):
            for to_unit in list(expected[from_unit].keys()):
                expected_factor = expected[from_unit][to_unit]
                res = QgsUnitTypes.fromUnitToUnitFactor(from_unit, to_unit)
                self.assertAlmostEqual(
                    res,
                    expected_factor,
                    places=10,
                    msg="got {:.15f}, expected {:.15f} when converting from {} to {}".format(
                        res,
                        expected_factor,
                        QgsUnitTypes.toString(from_unit),
                        QgsUnitTypes.toString(to_unit),
                    ),
                )
                # test conversion to unknown units
                res = QgsUnitTypes.fromUnitToUnitFactor(
                    from_unit, QgsUnitTypes.TemporalUnit.TemporalUnknownUnit
                )
                self.assertAlmostEqual(
                    res,
                    1.0,
                    msg=f"got {res:.7f}, expected 1.0 when converting from {QgsUnitTypes.toString(from_unit)} to unknown units",
                )
                res = QgsUnitTypes.fromUnitToUnitFactor(
                    from_unit, QgsUnitTypes.TemporalUnit.TemporalIrregularStep
                )
                self.assertAlmostEqual(
                    res,
                    1.0,
                    msg=f"got {res:.7f}, expected 1.0 when converting from {QgsUnitTypes.toString(from_unit)} to unknown units",
                )

    def testDistanceToVolumeUnit(self):
        """Test distanceToVolumeUnit conversion"""
        expected = {
            QgsUnitTypes.DistanceUnit.DistanceMeters: QgsUnitTypes.VolumeUnit.VolumeCubicMeters,
            QgsUnitTypes.DistanceUnit.DistanceKilometers: QgsUnitTypes.VolumeUnit.VolumeCubicMeters,
            QgsUnitTypes.DistanceUnit.DistanceFeet: QgsUnitTypes.VolumeUnit.VolumeCubicFeet,
            QgsUnitTypes.DistanceUnit.DistanceYards: QgsUnitTypes.VolumeUnit.VolumeCubicYards,
            QgsUnitTypes.DistanceUnit.DistanceMiles: QgsUnitTypes.VolumeUnit.VolumeCubicFeet,
            QgsUnitTypes.DistanceUnit.DistanceDegrees: QgsUnitTypes.VolumeUnit.VolumeCubicDegrees,
            QgsUnitTypes.DistanceUnit.DistanceCentimeters: QgsUnitTypes.VolumeUnit.VolumeCubicCentimeter,
            QgsUnitTypes.DistanceUnit.DistanceMillimeters: QgsUnitTypes.VolumeUnit.VolumeCubicCentimeter,
            QgsUnitTypes.DistanceUnit.DistanceUnknownUnit: QgsUnitTypes.VolumeUnit.VolumeUnknownUnit,
            QgsUnitTypes.DistanceUnit.DistanceNauticalMiles: QgsUnitTypes.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.Inches: QgsUnitTypes.VolumeUnit.VolumeCubicInch,
            Qgis.DistanceUnit.ChainsInternational: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.ChainsBritishBenoit1895A: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.ChainsBritishBenoit1895B: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.ChainsBritishSears1922Truncated: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.ChainsBritishSears1922: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.ChainsClarkes: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.ChainsUSSurvey: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.FeetBritish1865: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.FeetBritish1936: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.FeetBritishBenoit1895A: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.FeetBritishBenoit1895B: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.FeetBritishSears1922Truncated: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.FeetBritishSears1922: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.FeetClarkes: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.FeetGoldCoast: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.FeetIndian: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.FeetIndian1937: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.FeetIndian1962: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.FeetIndian1975: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.FeetUSSurvey: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.LinksInternational: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.LinksBritishBenoit1895A: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.LinksBritishBenoit1895B: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.LinksBritishSears1922Truncated: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.LinksBritishSears1922: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.LinksClarkes: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.LinksUSSurvey: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.YardsBritishBenoit1895A: Qgis.VolumeUnit.VolumeCubicYards,
            Qgis.DistanceUnit.YardsBritishBenoit1895B: Qgis.VolumeUnit.VolumeCubicYards,
            Qgis.DistanceUnit.YardsBritishSears1922Truncated: Qgis.VolumeUnit.VolumeCubicYards,
            Qgis.DistanceUnit.YardsBritishSears1922: Qgis.VolumeUnit.VolumeCubicYards,
            Qgis.DistanceUnit.YardsClarkes: Qgis.VolumeUnit.VolumeCubicYards,
            Qgis.DistanceUnit.YardsIndian: Qgis.VolumeUnit.VolumeCubicYards,
            Qgis.DistanceUnit.YardsIndian1937: Qgis.VolumeUnit.VolumeCubicYards,
            Qgis.DistanceUnit.YardsIndian1962: Qgis.VolumeUnit.VolumeCubicYards,
            Qgis.DistanceUnit.YardsIndian1975: Qgis.VolumeUnit.VolumeCubicYards,
            Qgis.DistanceUnit.MilesUSSurvey: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.Fathoms: Qgis.VolumeUnit.VolumeCubicFeet,
            Qgis.DistanceUnit.MetersGermanLegal: Qgis.VolumeUnit.VolumeCubicMeters,
        }

        for t in list(expected.keys()):
            self.assertEqual(QgsUnitTypes.distanceToVolumeUnit(t), expected[t])

    def testVolumeToDistanceUnit(self):
        """Test volumeToDistanceUnit conversion"""
        expected = {
            QgsUnitTypes.VolumeUnit.VolumeCubicMeters: QgsUnitTypes.DistanceUnit.DistanceMeters,
            QgsUnitTypes.VolumeUnit.VolumeCubicFeet: QgsUnitTypes.DistanceUnit.DistanceFeet,
            QgsUnitTypes.VolumeUnit.VolumeCubicYards: QgsUnitTypes.DistanceUnit.DistanceYards,
            QgsUnitTypes.VolumeUnit.VolumeBarrel: QgsUnitTypes.DistanceUnit.DistanceFeet,
            QgsUnitTypes.VolumeUnit.VolumeCubicDecimeter: QgsUnitTypes.DistanceUnit.DistanceCentimeters,
            QgsUnitTypes.VolumeUnit.VolumeLiters: QgsUnitTypes.DistanceUnit.DistanceMeters,
            QgsUnitTypes.VolumeUnit.VolumeGallonUS: QgsUnitTypes.DistanceUnit.DistanceFeet,
            QgsUnitTypes.VolumeUnit.VolumeCubicInch: Qgis.DistanceUnit.Inches,
            QgsUnitTypes.VolumeUnit.VolumeCubicCentimeter: QgsUnitTypes.DistanceUnit.DistanceCentimeters,
            QgsUnitTypes.VolumeUnit.VolumeCubicDegrees: QgsUnitTypes.DistanceUnit.DistanceDegrees,
        }

        for t in list(expected.keys()):
            self.assertEqual(QgsUnitTypes.volumeToDistanceUnit(t), expected[t])

    def testEncodeDecodeAngleUnits(self):
        """Test encoding and decoding angle units"""
        units = [
            QgsUnitTypes.AngleUnit.AngleDegrees,
            QgsUnitTypes.AngleUnit.AngleRadians,
            QgsUnitTypes.AngleUnit.AngleGon,
            QgsUnitTypes.AngleUnit.AngleMinutesOfArc,
            QgsUnitTypes.AngleUnit.AngleSecondsOfArc,
            QgsUnitTypes.AngleUnit.AngleTurn,
            QgsUnitTypes.AngleUnit.AngleMilliradiansSI,
            QgsUnitTypes.AngleUnit.AngleMilNATO,
            QgsUnitTypes.AngleUnit.AngleUnknownUnit,
        ]

        for u in units:
            res, ok = QgsUnitTypes.decodeAngleUnit(QgsUnitTypes.encodeUnit(u))
            assert ok, f"could not decode unit {QgsUnitTypes.toString(u)}"
            self.assertEqual(res, u)

        # Test decoding bad units
        res, ok = QgsUnitTypes.decodeAngleUnit("bad")
        self.assertFalse(ok)
        self.assertEqual(res, QgsUnitTypes.AngleUnit.AngleUnknownUnit)

        # Test that string is cleaned before decoding
        res, ok = QgsUnitTypes.decodeAngleUnit(" MoA  ")
        assert ok
        self.assertEqual(res, QgsUnitTypes.AngleUnit.AngleMinutesOfArc)

    def testAngleToString(self):
        """Test converting angle unit to string"""
        units = [
            QgsUnitTypes.AngleUnit.AngleDegrees,
            QgsUnitTypes.AngleUnit.AngleRadians,
            QgsUnitTypes.AngleUnit.AngleGon,
            QgsUnitTypes.AngleUnit.AngleMinutesOfArc,
            QgsUnitTypes.AngleUnit.AngleSecondsOfArc,
            QgsUnitTypes.AngleUnit.AngleTurn,
            QgsUnitTypes.AngleUnit.AngleMilliradiansSI,
            QgsUnitTypes.AngleUnit.AngleMilNATO,
            QgsUnitTypes.AngleUnit.AngleUnknownUnit,
        ]

        dupes = set()

        # can't test result as it may be translated, so make sure it's non-empty and not a duplicate
        for u in units:
            s = QgsUnitTypes.toString(u)
            assert len(s) > 0
            self.assertNotIn(s, dupes)
            dupes.add(s)

    def testAngleFromUnitToUnitFactor(self):
        """Test calculation of conversion factor between angular units"""

        expected = {
            QgsUnitTypes.AngleUnit.AngleDegrees: {
                QgsUnitTypes.AngleUnit.AngleDegrees: 1.0,
                QgsUnitTypes.AngleUnit.AngleRadians: 0.0174533,
                QgsUnitTypes.AngleUnit.AngleGon: 1.1111111,
                QgsUnitTypes.AngleUnit.AngleMinutesOfArc: 60,
                QgsUnitTypes.AngleUnit.AngleSecondsOfArc: 3600,
                QgsUnitTypes.AngleUnit.AngleTurn: 0.00277777777778,
                QgsUnitTypes.AngleUnit.AngleMilliradiansSI: 17.453292519943297,
                QgsUnitTypes.AngleUnit.AngleMilNATO: 17.77777777777778,
            },
            QgsUnitTypes.AngleUnit.AngleRadians: {
                QgsUnitTypes.AngleUnit.AngleDegrees: 57.2957795,
                QgsUnitTypes.AngleUnit.AngleRadians: 1.0,
                QgsUnitTypes.AngleUnit.AngleGon: 63.6619772,
                QgsUnitTypes.AngleUnit.AngleMinutesOfArc: 3437.7467708,
                QgsUnitTypes.AngleUnit.AngleSecondsOfArc: 206264.8062471,
                QgsUnitTypes.AngleUnit.AngleTurn: 0.159154943092,
                QgsUnitTypes.AngleUnit.AngleMilliradiansSI: 1000.0,
                QgsUnitTypes.AngleUnit.AngleMilNATO: 1018.5916357881301,
            },
            QgsUnitTypes.AngleUnit.AngleGon: {
                QgsUnitTypes.AngleUnit.AngleDegrees: 0.9000000,
                QgsUnitTypes.AngleUnit.AngleRadians: 0.015707968623450838802,
                QgsUnitTypes.AngleUnit.AngleGon: 1.0,
                QgsUnitTypes.AngleUnit.AngleMinutesOfArc: 54.0000000,
                QgsUnitTypes.AngleUnit.AngleSecondsOfArc: 3240.0000000,
                QgsUnitTypes.AngleUnit.AngleTurn: 0.0025,
                QgsUnitTypes.AngleUnit.AngleMilliradiansSI: 15.707963267948967,
                QgsUnitTypes.AngleUnit.AngleMilNATO: 16,
            },
            QgsUnitTypes.AngleUnit.AngleMinutesOfArc: {
                QgsUnitTypes.AngleUnit.AngleDegrees: 0.016666672633390722247,
                QgsUnitTypes.AngleUnit.AngleRadians: 0.00029088831280398030638,
                QgsUnitTypes.AngleUnit.AngleGon: 0.018518525464057963154,
                QgsUnitTypes.AngleUnit.AngleMinutesOfArc: 1.0,
                QgsUnitTypes.AngleUnit.AngleSecondsOfArc: 60.0,
                QgsUnitTypes.AngleUnit.AngleTurn: 4.62962962962963e-05,
                QgsUnitTypes.AngleUnit.AngleMilliradiansSI: 0.29088820866572157,
                QgsUnitTypes.AngleUnit.AngleMilNATO: 0.29629629629629634,
            },
            QgsUnitTypes.AngleUnit.AngleSecondsOfArc: {
                QgsUnitTypes.AngleUnit.AngleDegrees: 0.00027777787722304257169,
                QgsUnitTypes.AngleUnit.AngleRadians: 4.848138546730629518e-6,
                QgsUnitTypes.AngleUnit.AngleGon: 0.0003086420910674814405,
                QgsUnitTypes.AngleUnit.AngleMinutesOfArc: 0.016666672633325253783,
                QgsUnitTypes.AngleUnit.AngleSecondsOfArc: 1.0,
                QgsUnitTypes.AngleUnit.AngleTurn: 7.71604938271605e-07,
                QgsUnitTypes.AngleUnit.AngleMilliradiansSI: 0.0048481482527009582897,
                QgsUnitTypes.AngleUnit.AngleMilNATO: 0.0049382716049382715,
            },
            QgsUnitTypes.AngleUnit.AngleTurn: {
                QgsUnitTypes.AngleUnit.AngleDegrees: 360.0,
                QgsUnitTypes.AngleUnit.AngleRadians: 6.2831853071795,
                QgsUnitTypes.AngleUnit.AngleGon: 400.0,
                QgsUnitTypes.AngleUnit.AngleMinutesOfArc: 21600,
                QgsUnitTypes.AngleUnit.AngleSecondsOfArc: 1296000,
                QgsUnitTypes.AngleUnit.AngleTurn: 1,
                QgsUnitTypes.AngleUnit.AngleMilliradiansSI: 6283.185307179586,
                QgsUnitTypes.AngleUnit.AngleMilNATO: 6400,
            },
            QgsUnitTypes.AngleUnit.AngleMilliradiansSI: {
                QgsUnitTypes.AngleUnit.AngleDegrees: 0.057295779513082325,
                QgsUnitTypes.AngleUnit.AngleRadians: 0.001,
                QgsUnitTypes.AngleUnit.AngleGon: 0.06366197723675814,
                QgsUnitTypes.AngleUnit.AngleMinutesOfArc: 3.4377467707849396,
                QgsUnitTypes.AngleUnit.AngleSecondsOfArc: 206.26480624709637,
                QgsUnitTypes.AngleUnit.AngleTurn: 0.0015707963267948967,
                QgsUnitTypes.AngleUnit.AngleMilliradiansSI: 1.0,
                QgsUnitTypes.AngleUnit.AngleMilNATO: 1.0185916357881302,
            },
            QgsUnitTypes.AngleUnit.AngleMilNATO: {
                QgsUnitTypes.AngleUnit.AngleDegrees: 0.05625,
                QgsUnitTypes.AngleUnit.AngleRadians: 0.0009817477042468104,
                QgsUnitTypes.AngleUnit.AngleGon: 0.0625,
                QgsUnitTypes.AngleUnit.AngleMinutesOfArc: 3.375,
                QgsUnitTypes.AngleUnit.AngleSecondsOfArc: 202.5,
                QgsUnitTypes.AngleUnit.AngleTurn: 0.000015625,
                QgsUnitTypes.AngleUnit.AngleMilliradiansSI: 0.9817477042468102,
                QgsUnitTypes.AngleUnit.AngleMilNATO: 1.0,
            },
        }

        for from_unit in list(expected.keys()):
            for to_unit in list(expected[from_unit].keys()):
                expected_factor = expected[from_unit][to_unit]
                res = QgsUnitTypes.fromUnitToUnitFactor(from_unit, to_unit)
                self.assertAlmostEqual(
                    res,
                    expected_factor,
                    msg="got {:.7f}, expected {:.7f} when converting from {} to {}".format(
                        res,
                        expected_factor,
                        QgsUnitTypes.toString(from_unit),
                        QgsUnitTypes.toString(to_unit),
                    ),
                )
                # test conversion to unknown units
                res = QgsUnitTypes.fromUnitToUnitFactor(
                    from_unit, QgsUnitTypes.AngleUnit.AngleUnknownUnit
                )
                self.assertAlmostEqual(
                    res,
                    1.0,
                    msg=f"got {res:.7f}, expected 1.0 when converting from {QgsUnitTypes.toString(from_unit)} to unknown units",
                )

    def testFormatAngle(self):
        """Test formatting angles"""
        self.assertEqual(
            QgsUnitTypes.formatAngle(45, 3, QgsUnitTypes.AngleUnit.AngleDegrees),
            "45.000°",
        )
        self.assertEqual(
            QgsUnitTypes.formatAngle(1, 2, QgsUnitTypes.AngleUnit.AngleRadians),
            "1.00 rad",
        )
        self.assertEqual(
            QgsUnitTypes.formatAngle(1, 0, QgsUnitTypes.AngleUnit.AngleGon), "1 gon"
        )
        self.assertEqual(
            QgsUnitTypes.formatAngle(
                1.11111111, 4, QgsUnitTypes.AngleUnit.AngleMinutesOfArc
            ),
            "1.1111′",
        )
        self.assertEqual(
            QgsUnitTypes.formatAngle(
                1.99999999, 2, QgsUnitTypes.AngleUnit.AngleSecondsOfArc
            ),
            "2.00″",
        )
        self.assertEqual(
            QgsUnitTypes.formatAngle(1, 2, QgsUnitTypes.AngleUnit.AngleTurn), "1.00 tr"
        )
        self.assertEqual(
            QgsUnitTypes.formatAngle(1, 2, QgsUnitTypes.AngleUnit.AngleMilliradiansSI),
            "1.00 millirad",
        )
        self.assertEqual(
            QgsUnitTypes.formatAngle(1, 2, QgsUnitTypes.AngleUnit.AngleMilNATO),
            "1.00 mil",
        )
        self.assertEqual(
            QgsUnitTypes.formatAngle(1, 2, QgsUnitTypes.AngleUnit.AngleUnknownUnit),
            "1.00",
        )

        self.assertEqual(
            QgsUnitTypes.formatAngle(45, -1, QgsUnitTypes.AngleUnit.AngleDegrees), "45°"
        )
        self.assertEqual(
            QgsUnitTypes.formatAngle(1, -1, QgsUnitTypes.AngleUnit.AngleRadians),
            "1.00 rad",
        )
        self.assertEqual(
            QgsUnitTypes.formatAngle(1, -1, QgsUnitTypes.AngleUnit.AngleGon), "1 gon"
        )
        self.assertEqual(
            QgsUnitTypes.formatAngle(
                1.11111111, -1, QgsUnitTypes.AngleUnit.AngleMinutesOfArc
            ),
            "1′",
        )
        self.assertEqual(
            QgsUnitTypes.formatAngle(
                1.99999999, -1, QgsUnitTypes.AngleUnit.AngleSecondsOfArc
            ),
            "2″",
        )
        self.assertEqual(
            QgsUnitTypes.formatAngle(1, -1, QgsUnitTypes.AngleUnit.AngleTurn),
            "1.000 tr",
        )
        self.assertEqual(
            QgsUnitTypes.formatAngle(1, -1, QgsUnitTypes.AngleUnit.AngleMilliradiansSI),
            "1 millirad",
        )
        self.assertEqual(
            QgsUnitTypes.formatAngle(1, -1, QgsUnitTypes.AngleUnit.AngleMilNATO),
            "1 mil",
        )
        self.assertEqual(
            QgsUnitTypes.formatAngle(1, -1, QgsUnitTypes.AngleUnit.AngleUnknownUnit),
            "1.00",
        )

    def testFormatDistance(self):
        """Test formatting distances"""
        # keep base unit
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                100, 3, QgsUnitTypes.DistanceUnit.DistanceMeters, True
            ),
            "100.000 m",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                10, 2, QgsUnitTypes.DistanceUnit.DistanceKilometers, True
            ),
            "10.00 km",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                1, 0, QgsUnitTypes.DistanceUnit.DistanceFeet, True
            ),
            "1 ft",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                1.11111111, 4, QgsUnitTypes.DistanceUnit.DistanceNauticalMiles, True
            ),
            "1.1111 NM",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                1.99999999, 2, QgsUnitTypes.DistanceUnit.DistanceYards, True
            ),
            "2.00 yd",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                1, 2, QgsUnitTypes.DistanceUnit.DistanceMiles, True
            ),
            "1.00 mi",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                1, 2, QgsUnitTypes.DistanceUnit.DistanceDegrees, True
            ),
            "1.00 deg",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                1, 2, QgsUnitTypes.DistanceUnit.DistanceCentimeters, True
            ),
            "1.00 cm",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                1, 2, QgsUnitTypes.DistanceUnit.DistanceMillimeters, True
            ),
            "1.00 mm",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(1, 2, Qgis.DistanceUnit.Inches, True), "1.00 in"
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                1, 2, QgsUnitTypes.DistanceUnit.DistanceUnknownUnit, True
            ),
            "1.00",
        )

        # don't keep base unit
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                10, 3, QgsUnitTypes.DistanceUnit.DistanceMeters, False
            ),
            "10.000 m",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                1001, 3, QgsUnitTypes.DistanceUnit.DistanceMeters, False
            ),
            "1.001 km",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                0.05, 2, QgsUnitTypes.DistanceUnit.DistanceMeters, False
            ),
            "5.00 cm",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                0.005, 2, QgsUnitTypes.DistanceUnit.DistanceMeters, False
            ),
            "5.00 mm",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                10, 2, QgsUnitTypes.DistanceUnit.DistanceKilometers, False
            ),
            "10.00 km",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                0.5, 2, QgsUnitTypes.DistanceUnit.DistanceKilometers, False
            ),
            "500.00 m",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                10, 2, QgsUnitTypes.DistanceUnit.DistanceFeet, False
            ),
            "10.00 ft",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                6000, 2, QgsUnitTypes.DistanceUnit.DistanceFeet, False
            ),
            "1.14 mi",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                10, 2, QgsUnitTypes.DistanceUnit.DistanceYards, False
            ),
            "10.00 yd",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                2500, 2, QgsUnitTypes.DistanceUnit.DistanceYards, False
            ),
            "1.42 mi",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                1, 2, QgsUnitTypes.DistanceUnit.DistanceMiles, False
            ),
            "1.00 mi",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                0.5, 2, QgsUnitTypes.DistanceUnit.DistanceMiles, False
            ),
            "2640.00 ft",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                1.11111111, 4, QgsUnitTypes.DistanceUnit.DistanceNauticalMiles, False
            ),
            "1.1111 NM",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                0.001, 4, QgsUnitTypes.DistanceUnit.DistanceDegrees, False
            ),
            "0.0010 deg",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                100, 2, QgsUnitTypes.DistanceUnit.DistanceCentimeters, False
            ),
            "100.00 cm",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                1000, 2, QgsUnitTypes.DistanceUnit.DistanceMillimeters, False
            ),
            "1000.00 mm",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(1000, 2, Qgis.DistanceUnit.Inches, False),
            "1000.00 in",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                1, 2, QgsUnitTypes.DistanceUnit.DistanceUnknownUnit, False
            ),
            "1.00",
        )

        # small values should not be displayed as zeroes, instead fallback to scientific notation
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                0.00168478, 2, QgsUnitTypes.DistanceUnit.DistanceMeters, False
            ),
            "1.68 mm",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                0.00000168478, 2, QgsUnitTypes.DistanceUnit.DistanceMeters, False
            ),
            "1.68e-06 m",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                0.00168478, 2, QgsUnitTypes.DistanceUnit.DistanceMeters, True
            ),
            "1.68e-03 m",
        )

        # test different locales
        QLocale.setDefault(QLocale(QLocale.Language.Italian))
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                10, 3, QgsUnitTypes.DistanceUnit.DistanceMeters, False
            ),
            "10,000 m",
        )
        self.assertEqual(
            QgsUnitTypes.formatDistance(
                0.5, 2, QgsUnitTypes.DistanceUnit.DistanceMiles, False
            ),
            "2.640,00 ft",
        )

    def testFormatArea(self):
        """Test formatting areas"""
        # keep base unit
        self.assertEqual(
            QgsUnitTypes.formatArea(
                100, 3, QgsUnitTypes.AreaUnit.AreaSquareMeters, True
            ),
            "100.000 m²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                10, 2, QgsUnitTypes.AreaUnit.AreaSquareKilometers, True
            ),
            "10.00 km²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(1, 0, QgsUnitTypes.AreaUnit.AreaSquareFeet, True),
            "1 ft²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                1.11111111, 4, QgsUnitTypes.AreaUnit.AreaSquareYards, True
            ),
            "1.1111 yd²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                1.99999999, 2, QgsUnitTypes.AreaUnit.AreaSquareMiles, True
            ),
            "2.00 mi²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(1, 2, QgsUnitTypes.AreaUnit.AreaHectares, True),
            "1.00 ha",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(1, 2, QgsUnitTypes.AreaUnit.AreaAcres, True),
            "1.00 ac",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                1, 2, QgsUnitTypes.AreaUnit.AreaSquareNauticalMiles, True
            ),
            "1.00 NM²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                1, 2, QgsUnitTypes.AreaUnit.AreaSquareDegrees, True
            ),
            "1.00 deg²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                1, 2, QgsUnitTypes.AreaUnit.AreaSquareCentimeters, True
            ),
            "1.00 cm²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                1, 2, QgsUnitTypes.AreaUnit.AreaSquareMillimeters, True
            ),
            "1.00 mm²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(1, 2, Qgis.AreaUnit.SquareInches, True), "1.00 in²"
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(1, 2, QgsUnitTypes.AreaUnit.AreaUnknownUnit, True),
            "1.00",
        )

        # don't keep base unit
        self.assertEqual(
            QgsUnitTypes.formatArea(
                100, 2, QgsUnitTypes.AreaUnit.AreaSquareMeters, False
            ),
            "100.00 m²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                2000000, 2, QgsUnitTypes.AreaUnit.AreaSquareMeters, False
            ),
            "2.00 km²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                10001, 2, QgsUnitTypes.AreaUnit.AreaSquareMeters, False
            ),
            "1.00 ha",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                100, 2, QgsUnitTypes.AreaUnit.AreaSquareKilometers, False
            ),
            "100.00 km²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                0.5, 2, QgsUnitTypes.AreaUnit.AreaSquareKilometers, False
            ),
            "0.50 km²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                27879000, 2, QgsUnitTypes.AreaUnit.AreaSquareFeet, False
            ),
            "1.00 mi²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                2787, 2, QgsUnitTypes.AreaUnit.AreaSquareFeet, False
            ),
            "2787.00 ft²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                3099000, 2, QgsUnitTypes.AreaUnit.AreaSquareYards, False
            ),
            "1.00 mi²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                309, 2, QgsUnitTypes.AreaUnit.AreaSquareYards, False
            ),
            "309.00 yd²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                10, 2, QgsUnitTypes.AreaUnit.AreaSquareMiles, False
            ),
            "10.00 mi²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                0.05, 2, QgsUnitTypes.AreaUnit.AreaSquareMiles, False
            ),
            "0.05 mi²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(10, 2, QgsUnitTypes.AreaUnit.AreaHectares, False),
            "10.00 ha",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(110, 2, QgsUnitTypes.AreaUnit.AreaHectares, False),
            "1.10 km²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(10, 2, QgsUnitTypes.AreaUnit.AreaAcres, False),
            "10.00 ac",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(650, 2, QgsUnitTypes.AreaUnit.AreaAcres, False),
            "1.02 mi²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                0.01, 2, QgsUnitTypes.AreaUnit.AreaSquareNauticalMiles, False
            ),
            "0.01 NM²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                100, 2, QgsUnitTypes.AreaUnit.AreaSquareNauticalMiles, False
            ),
            "100.00 NM²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                0.0001, 4, QgsUnitTypes.AreaUnit.AreaSquareDegrees, False
            ),
            "0.0001 deg²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                0.0001, 4, QgsUnitTypes.AreaUnit.AreaSquareDegrees, False
            ),
            "0.0001 deg²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                1000, 4, QgsUnitTypes.AreaUnit.AreaSquareMillimeters, False
            ),
            "0.0010 m²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                100, 3, QgsUnitTypes.AreaUnit.AreaSquareCentimeters, False
            ),
            "0.010 m²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                10, 2, QgsUnitTypes.AreaUnit.AreaUnknownUnit, False
            ),
            "10.00",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(1000, 2, Qgis.AreaUnit.SquareInches, False),
            "1000.00 in²",
        )

        # small values should not be displayed as zeroes, instead fallback to scientific notation
        self.assertEqual(
            QgsUnitTypes.formatArea(
                0.00168478, 4, QgsUnitTypes.AreaUnit.AreaSquareMeters, False
            ),
            "0.0017 m²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                0.00168478, 2, QgsUnitTypes.AreaUnit.AreaSquareMeters, False
            ),
            "1.68e-03 m²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                0.00168478, 2, QgsUnitTypes.AreaUnit.AreaSquareMeters, True
            ),
            "1.68e-03 m²",
        )

        # test different locales
        QLocale.setDefault(QLocale(QLocale.Language.Italian))
        self.assertEqual(
            QgsUnitTypes.formatArea(
                100, 2, QgsUnitTypes.AreaUnit.AreaSquareKilometers, False
            ),
            "100,00 km²",
        )
        self.assertEqual(
            QgsUnitTypes.formatArea(
                2787, 2, QgsUnitTypes.AreaUnit.AreaSquareFeet, False
            ),
            "2.787,00 ft²",
        )

    def testEncodeDecodeLayoutUnits(self):
        """Test encoding and decoding layout units"""
        units = [
            QgsUnitTypes.LayoutUnit.LayoutMillimeters,
            QgsUnitTypes.LayoutUnit.LayoutCentimeters,
            QgsUnitTypes.LayoutUnit.LayoutMeters,
            QgsUnitTypes.LayoutUnit.LayoutInches,
            QgsUnitTypes.LayoutUnit.LayoutFeet,
            QgsUnitTypes.LayoutUnit.LayoutPoints,
            QgsUnitTypes.LayoutUnit.LayoutPicas,
            QgsUnitTypes.LayoutUnit.LayoutPixels,
        ]

        for u in units:
            res, ok = QgsUnitTypes.decodeLayoutUnit(QgsUnitTypes.encodeUnit(u))
            assert ok
            self.assertEqual(res, u)

        # Test decoding bad units
        res, ok = QgsUnitTypes.decodeLayoutUnit("bad")
        self.assertFalse(ok)
        # default units should be MM
        self.assertEqual(res, QgsUnitTypes.LayoutUnit.LayoutMillimeters)

        # Test that string is cleaned before decoding
        res, ok = QgsUnitTypes.decodeLayoutUnit(" px  ")
        assert ok
        self.assertEqual(res, QgsUnitTypes.LayoutUnit.LayoutPixels)

    def testAbbreviateRenderUnits(self):
        """Test abbreviating render units"""
        units = [
            QgsUnitTypes.RenderUnit.RenderMillimeters,
            QgsUnitTypes.RenderUnit.RenderMapUnits,
            QgsUnitTypes.RenderUnit.RenderPixels,
            QgsUnitTypes.RenderUnit.RenderPercentage,
            QgsUnitTypes.RenderUnit.RenderPoints,
            QgsUnitTypes.RenderUnit.RenderInches,
            QgsUnitTypes.RenderUnit.RenderUnknownUnit,
            QgsUnitTypes.RenderUnit.RenderMetersInMapUnits,
        ]

        used = set()
        for u in units:
            self.assertTrue(QgsUnitTypes.toString(u))
            self.assertTrue(QgsUnitTypes.toAbbreviatedString(u))
            self.assertNotIn(QgsUnitTypes.toAbbreviatedString(u), used)
            used.add(QgsUnitTypes.toAbbreviatedString(u))

    def testAbbreviateLayoutUnits(self):
        """Test abbreviating layout units"""
        units = [
            QgsUnitTypes.LayoutUnit.LayoutMillimeters,
            QgsUnitTypes.LayoutUnit.LayoutCentimeters,
            QgsUnitTypes.LayoutUnit.LayoutMeters,
            QgsUnitTypes.LayoutUnit.LayoutInches,
            QgsUnitTypes.LayoutUnit.LayoutFeet,
            QgsUnitTypes.LayoutUnit.LayoutPoints,
            QgsUnitTypes.LayoutUnit.LayoutPicas,
            QgsUnitTypes.LayoutUnit.LayoutPixels,
        ]

        used = set()
        for u in units:
            self.assertTrue(QgsUnitTypes.toString(u))
            self.assertTrue(QgsUnitTypes.toAbbreviatedString(u))
            self.assertNotIn(QgsUnitTypes.toAbbreviatedString(u), used)
            used.add(QgsUnitTypes.toAbbreviatedString(u))


if __name__ == "__main__":
    unittest.main()
