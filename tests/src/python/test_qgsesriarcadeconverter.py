"""QGIS Unit tests for QgsEsriArcadeConverter.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.core import QgsEsriArcadeConverter
from qgis.testing import unittest


class TestQgsEsriArcadeConverter(unittest.TestCase):
    @staticmethod
    def convert_script(script: str):
        converter = QgsEsriArcadeConverter(script)
        assert converter.result() == QgsEsriArcadeConverter.Result.Success
        return converter.expression()

    @staticmethod
    def convert_script_error(script: str):
        converter = QgsEsriArcadeConverter(script)
        return converter.result()

    def test_arcade(self):
        """
        Test Arcade expression conversion
        """
        self.assertEqual(self.convert_script(""), "")
        self.assertEqual(self.convert_script("$feature.Name"), '"Name"')
        self.assertEqual(
            self.convert_script(
                "return $feature.Name",
            ),
            '"Name"',
        )
        self.assertEqual(
            self.convert_script(
                "Return   $feature.Name  ",
            ),
            '"Name"',
        )
        self.assertEqual(
            self.convert_script(
                "$feature.A TEXT FIELD",
            ),
            '"A TEXT FIELD"',
        )
        self.assertEqual(
            self.convert_script(
                "Upper($feature.Name)",
            ),
            'upper("Name")',
        )
        self.assertEqual(
            self.convert_script(
                "Lower($feature.Name)",
            ),
            'lower("Name")',
        )
        self.assertEqual(
            self.convert_script(
                "return Lower($feature.Name)",
            ),
            'lower("Name")',
        )
        self.assertEqual(
            self.convert_script(
                '$feature["Name_Field"]',
            ),
            '"Name_Field"',
        )
        self.assertEqual(
            self.convert_script(
                '$feature["Name Field"]',
            ),
            '"Name Field"',
        )

        self.assertEqual(
            self.convert_script(
                "$feature['Salmonids?']",
            ),
            '"Salmonids?"',
        )
        self.assertEqual(
            self.convert_script("$feature.SAMPLE TYP"),
            '"SAMPLE TYP"',
        )
        self.assertEqual(
            self.convert_script(
                "$feature['Shape.STLength()']",
            ),
            "length(@geometry)",
        )
        self.assertEqual(
            self.convert_script(
                "$feature['SHAPE.STArea()']",
            ),
            "area(@geometry)",
        )
        self.assertEqual(self.convert_script("$view.scale"), "@map_scale")
        self.assertEqual(
            self.convert_script('proper($feature["SITE_NAME"])'),
            'title("SITE_NAME")',
        )
        self.assertEqual(
            self.convert_script('proper("abc")'),
            "title('abc')",
        )
        self.assertEqual(
            self.convert_script("$feature.Name + $feature.Value"),
            '"Name" + "Value"',
        )
        self.assertEqual(
            self.convert_script(
                "$feature.Name + TextFormatting.NewLine + $feature.Value",
            ),
            '"Name" + \'\\n\' + "Value"',
        )
        self.assertEqual(
            self.convert_script(
                """proper($feature["SITE_NAME"])+ TextFormatting.NewLine + "Removed: "+ $feature["REMOVAL_DATE"]""",
            ),
            """title("SITE_NAME") + '\\n' + 'Removed: ' + "REMOVAL_DATE\"""",
        )
        self.assertEqual(
            self.convert_script("""DomainName($feature, 'FACILITY_TYPE')"""),
            'represent_value("FACILITY_TYPE")',
        )

        self.assertEqual(
            self.convert_script("""proper(DomainName($feature, 'FACILITY_TYPE'))"""),
            'title(represent_value("FACILITY_TYPE"))',
        )

        # more complex forms of DomainName can't be converted
        self.assertEqual(
            self.convert_script_error(
                """DomainName($feature, 'FACILITY_TYPE', 'some code')""",
            ),
            QgsEsriArcadeConverter.Result.InvalidScriptIncompatible,
        )

        self.assertEqual(
            self.convert_script("""($feature.Heading / $feature.Importance) * 100"""),
            """("Heading" / "Importance") * 100""",
        )

        self.assertEqual(
            self.convert_script(
                """DefaultValue($feature.Ba, $feature.Kb)""",
            ),
            """coalesce("Ba", "Kb")""",
        )
        self.assertEqual(
            self.convert_script(
                """"Project/plan" + TextFormatting.Newline + $feature.Kn""",
            ),
            "'Project/plan' + '\\n' + \"Kn\"",
        )

        self.assertEqual(
            self.convert_script(
                "IsEmpty($feature.SITENAME)",
            ),
            '"SITENAME" IS NULL',
        )

        self.assertEqual(
            self.convert_script(
                """"Natural2" + "-(" + $feature.LENGTH_RANK + ")" + TextFormatting.NewLine + When(IsEmpty($feature.SITENAME), '(no value)','(' + $feature.SITENAME + ')') + TextFormatting.NewLine + Round($feature.Shape_Length / 1, 2) + "m" """,
            ),
            """'Natural2' + '-(' + "LENGTH_RANK" + ')' + '\\n' + (CASE WHEN "SITENAME" IS NULL THEN '(no value)' ELSE '(' + "SITENAME" + ')' END) + '\\n' + round("Shape_Length" / 1, 2) + 'm'""",
        )

        self.assertEqual(
            self.convert_script(
                """IIf(IsEmpty($feature.SSS),"Not Complete","Complete")""",
            ),
            """if("SSS" IS NULL, 'Not Complete', 'Complete')""",
        )
        self.assertEqual(
            self.convert_script(
                """IIf($feature.SSS=="abc","Not Complete","Complete")""",
            ),
            """if("SSS" = \'abc\', \'Not Complete\', \'Complete\')""",
        )

        self.assertEqual(
            self.convert_script(
                """IIf(IsEmpty($feature.SS1) && IsEmpty($feature.SS2),"Not Complete","Complete")""",
            ),
            """if(("SS1" IS NULL) AND ("SS2" IS NULL), 'Not Complete', 'Complete')""",
        )

        self.assertEqual(
            self.convert_script(
                """IIf(IsEmpty($feature.SS1) || IsEmpty($feature.SS2),"Not Complete","Complete")""",
            ),
            """if(("SS1" IS NULL) OR ("SS2" IS NULL), 'Not Complete', 'Complete')""",
        )

        self.assertEqual(
            self.convert_script(
                'Decode(\n    $feature.TITLE_DESCRIPTION,\n    "cc", "dd",\n    "ee", "ff",\n    "gg", "hh",\n    "ii", "jj",\n    "Private", "Private",\n    "unknown"\n)\n',
            ),
            """coalesce(map_get(map('cc', 'dd', 'ee', 'ff', 'gg', 'hh', 'ii', 'jj', 'Private', 'Private'), "TITLE_DESCRIPTION"), 'unknown')""",
        )

        self.assertEqual(
            self.convert_script_error(
                """var slotNo = Mid($feature.GEN_ATTR, 10, 5); var lotNo = Number(slotNo);return Text(lotNo);""",
            ),
            QgsEsriArcadeConverter.Result.InvalidScriptMultiStatements,
        )

        self.assertEqual(
            self.convert_script(
                """if ($feature.historicna == \'aa, cc, dd\') {\n    return \'aa cc dd\' + TextFormatting.NewLine + "(" + $feature.shard_id + ")";\n} else {\n    return $feature.historicna + TextFormatting.NewLine + "(" + $feature.shard_id + ")"\n}\n\n\n""",
            ),
            """if("historicna" = 'aa, cc, dd', 'aa cc dd' + '\\n' + '(' + "shard_id" + ')', "historicna" + '\\n' + '(' + "shard_id" + ')')""",
        )

        self.assertEqual(
            self.convert_script(
                """-($feature['rotation'])""",
            ),
            '-"rotation"',
        )

        self.assertEqual(
            self.convert_script(
                '''domainname($feature,\'MODELNAME\')+" (" + $feature.IDCENTRALE + "-R-0" + $feature.FIELD_NAME + "-" +  Text( Number( Right ($feature.OBJECT_NAME, Count($feature.OBJECT_NAME)-Find(\'_\', $feature.OBJECT_NAME, 4)-1))) + ")"''',
            ),
            """represent_value("MODELNAME") + ' (' + "IDCENTRALE" + '-R-0' + "FIELD_NAME" + '-' + to_string(to_real(right("OBJECT_NAME", length("OBJECT_NAME") - (strpos(substr('_', 4), "OBJECT_NAME") + 4) - 1))) + ')'""",
        )

        self.assertEqual(
            self.convert_script(
                "Left($feature.NETWORKEQUIPMENT, 3)",
            ),
            """left("NETWORKEQUIPMENT", 3)""",
        )

        self.assertEqual(
            self.convert_script_error(
                'var Z_raw = $feature.ssss;\nvar L = $feature.llll;\n\n// NULL / Empty check\nif (IsEmpty(Z_raw) || IsEmpty(L)) return "OTHER";',
            ),
            QgsEsriArcadeConverter.Result.InvalidScriptMultiStatements,
        )

        self.assertEqual(
            self.convert_script_error(
                """
            var v = $feature.TYPE_CODE

if (v > 0 && v < 100) {
return 1; // green
}

if ((v >= 100 && v < 200) || (v >= 300 && v < 400)) {
return 2; // orange
}

if ((v >= 200 && v < 300) || (v >= 400 && v < 500)) {
return 3; // yellow
}


return 0; // no color / default""",
            ),
            QgsEsriArcadeConverter.Result.InvalidScriptMultiStatements,
        )

        self.assertEqual(
            self.convert_script(
                "Max($feature.LineWt / 100 * 2.83, 1)",
            ),
            """max(("LineWt" / 100) * 2.83, 1)""",
        )

        self.assertEqual(
            self.convert_script(
                """Split('red,green,blue,orange', ',')""",
            ),
            """string_to_array('red,green,blue,orange', ',')""",
        )

        self.assertEqual(
            self.convert_script_error(
                'var a = $feature.YEAR\nvar b = Split(a, "-")\nvar c = b[0]\nreturn c\n',
            ),
            QgsEsriArcadeConverter.Result.InvalidScriptMultiStatements,
        )

        self.assertEqual(
            self.convert_script_error(
                """
var a = $feature.Type
if (a=="C"){return"Operating Area"}""",
            ),
            QgsEsriArcadeConverter.Result.InvalidScriptMultiStatements,
        )

        self.assertEqual(
            self.convert_script(
                "Concatenate([$feature['LIC'], ', ', $feature['ZEPA']])",
            ),
            """array_to_string(array("LIC", ', ', "ZEPA"))""",
        )

        self.assertEqual(
            self.convert_script(
                'TextFormatting.NewLine + $feature.number + TextFormatting.NewLine + $feature.location_number + TextFormatting.NewLine + Replace($feature.height, " Feet" ,"") + "-" + $feature.class',
            ),
            ''''\\n' + "number" + '\\n' + "location_number" + '\\n' + replace("height", ' Feet', '') + '-' + "class"''',
        )

        self.assertEqual(
            self.convert_script(
                """(($feature['ModulNr'] + ' ') + $feature['Modul'])""",
            ),
            '''"ModulNr" + ' ' + "Modul"''',
        )

        self.assertEqual(
            self.convert_script(
                """If(IsEmpty($feature.SPP1) && IsEmpty($feature.SPP2)) "Not Complete" else "Complete\"""",
            ),
            """if(("SPP1" IS NULL) AND ("SPP2" IS NULL), 'Not Complete', 'Complete')""",
        )

        self.assertEqual(
            self.convert_script(
                "3",
            ),
            "3",
        )

        self.assertEqual(
            self.convert_script(
                "null",
            ),
            "NULL",
        )

        self.assertEqual(
            self.convert_script_error(
                "Return Concatenate([$feature.aec_Name,Text($feature.aec, '#####.## ')],' \r RIM:') + ' \r' + $feature.aec_Network",
            ),
            QgsEsriArcadeConverter.Result.InvalidScriptIncompatible,
        )


if __name__ == "__main__":
    unittest.main()
