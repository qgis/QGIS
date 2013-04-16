# -*- coding: utf-8 -*-

"""
***************************************************************************
    SextanteTests.py
    ---------------------
    Date                 : April 2013
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
__date__ = 'April 2013'
__copyright__ = '(C) 2013, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

'''Convenience module to create a test suite will all SEXTANTE tests'''
import unittest
from sextante.tests import QgisAlgsTest
from sextante.tests import ParametersTest
from sextante.tests import ModelerAlgorithmTest
from sextante.tests import SextanteToolsTest
from sextante.tests import ScriptTest
from sextante.tests import SagaTest
from sextante.tests import GeoAlgorithmTest
from sextante.tests import GdalTest

def suite():
    suite = unittest.TestSuite()
    suite.addTests(QgisAlgsTest.suite())
    suite.addTests(ModelerAlgorithmTest.suite())
    suite.addTests(SagaTest.suite())
    suite.addTests(GdalTest.suite())
    suite.addTests(ScriptTest.suite())
    suite.addTests(SextanteToolsTest.suite())
    #suite.addTests(ParametersTest.suite())
    suite.addTests(GeoAlgorithmTest.suite())
    return suite

def runtests():
    result = unittest.TestResult()
    testsuite = suite()
    testsuite.run(result)
    return result