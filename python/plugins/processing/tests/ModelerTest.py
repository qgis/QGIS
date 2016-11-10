# -*- coding: utf-8 -*-

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

__author__ = 'Nyall Dawson'
__date__ = 'November 2016'
__copyright__ = '(C) 2016, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.testing import start_app, unittest

from processing.modeler.ModelerAlgorithm import (ModelerAlgorithm, ModelerParameter)
from processing.modeler.ModelerParametersDialog import (ModelerParametersDialog)
from processing.core.parameters import (ParameterFile,
                                        ParameterNumber,
                                        ParameterString,
                                        ParameterTableField)
start_app()


class ModelerTest(unittest.TestCase):

    def testModelerParametersDialogAvailableValuesOfType(self):
        # test getAvailableValuesOfType from ModelerParametersDialog

        m = ModelerAlgorithm()
        string_param_1 = ModelerParameter(ParameterString('string', 'string desc'))
        m.addParameter(string_param_1)
        string_param_2 = ModelerParameter(ParameterString('string2', 'string desc'))
        m.addParameter(string_param_2)
        num_param = ModelerParameter(ParameterNumber('number', 'number desc'))
        m.addParameter(num_param)
        table_field_param = ModelerParameter(ParameterTableField('field', 'field desc'))
        m.addParameter(table_field_param)
        file_param = ModelerParameter(ParameterFile('file', 'file desc'))
        m.addParameter(file_param)

        dlg = ModelerParametersDialog(m, m)
        # test single types
        self.assertEqual(set(p.name for p in dlg.getAvailableValuesOfType(ParameterNumber)),
                         set(['number']))
        self.assertEqual(set(p.name for p in dlg.getAvailableValuesOfType(ParameterTableField)),
                         set(['field']))
        self.assertEqual(set(p.name for p in dlg.getAvailableValuesOfType(ParameterFile)),
                         set(['file']))

        # test multiple types
        self.assertEqual(set(p.name for p in dlg.getAvailableValuesOfType([ParameterString, ParameterNumber, ParameterFile])),
                         set(['string', 'string2', 'number', 'file']))


if __name__ == '__main__':
    unittest.main()
