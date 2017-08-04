# -*- coding: utf-8 -*-

"""
***************************************************************************
    ParametersTest
    ---------------------
    Date                 : August 2017
    Copyright            : (C) 2017 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nyall Dawson'
__date__ = 'August 2017'
__copyright__ = '(C) 2017, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.testing import start_app, unittest
from qgis.core import QgsApplication

from processing.gui.AlgorithmDialog import AlgorithmDialog

start_app()


class AlgorithmDialogTest(unittest.TestCase):

    def testCreation(self):
        alg = QgsApplication.processingRegistry().algorithmById('native:centroids')
        a = AlgorithmDialog(alg)
        self.assertEqual(a.mainWidget().alg, alg)


if __name__ == '__main__':
    unittest.main()
