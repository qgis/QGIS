"""
***************************************************************************
    ModelerTest
    ---------------------
    Date                 : November 2016
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************8
"""

__author__ = "Nyall Dawson"
__date__ = "November 2016"
__copyright__ = "(C) 2016, Nyall Dawson"

import unittest
from qgis.testing import start_app, QgisTestCase

from qgis.core import (
    QgsProcessingModelAlgorithm,
    QgsProcessingModelParameter,
    QgsProcessingParameterString,
    QgsProcessingParameterNumber,
    QgsProcessingParameterDistance,
    QgsProcessingParameterField,
    QgsProcessingParameterFile,
)
from processing.modeler.ModelerParametersDialog import ModelerParametersDialog

start_app()


class ModelerTest(QgisTestCase):

    def testModelerParametersDialogAvailableValuesOfType(self):
        # test getAvailableValuesOfType from ModelerParametersDialog

        m = QgsProcessingModelAlgorithm()

        string_param_1 = QgsProcessingModelParameter("string")
        m.addModelParameter(QgsProcessingParameterString("string"), string_param_1)

        string_param_2 = QgsProcessingModelParameter("string2")
        m.addModelParameter(QgsProcessingParameterString("string2"), string_param_2)

        num_param = QgsProcessingModelParameter("number")
        m.addModelParameter(QgsProcessingParameterNumber("number"), num_param)

        table_field_param = QgsProcessingModelParameter("field")
        m.addModelParameter(QgsProcessingParameterField("field"), table_field_param)

        file_param = QgsProcessingModelParameter("file")
        m.addModelParameter(QgsProcessingParameterFile("file"), file_param)

        dlg = ModelerParametersDialog(m, m)
        # test single types
        self.assertEqual(
            {
                p.parameterName()
                for p in dlg.getAvailableValuesOfType(QgsProcessingParameterNumber)
            },
            {"number"},
        )
        self.assertEqual(
            {
                p.parameterName()
                for p in dlg.getAvailableValuesOfType(QgsProcessingParameterField)
            },
            {"field"},
        )
        self.assertEqual(
            {
                p.parameterName()
                for p in dlg.getAvailableValuesOfType(QgsProcessingParameterFile)
            },
            {"file"},
        )

        # test multiple types
        self.assertEqual(
            {
                p.parameterName()
                for p in dlg.getAvailableValuesOfType(
                    [
                        QgsProcessingParameterString,
                        QgsProcessingParameterNumber,
                        QgsProcessingParameterFile,
                    ]
                )
            },
            {"string", "string2", "number", "file"},
        )


if __name__ == "__main__":
    unittest.main()
