"""QGIS Unit tests for QgsApplication.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Tim Sutton (tim@linfiniti.com)"
__date__ = "20/01/2011"
__copyright__ = "Copyright 2012, The QGIS Project"


import unittest
from qgis.testing import start_app, QgisTestCase

QGISAPP = start_app()


class TestPyQgsApplication(QgisTestCase):

    def testInvalidThemeName(self):
        """Check using an invalid theme will fallback to  'default'"""
        QGISAPP.setUITheme("fooobar")
        myExpectedResult = "default"
        myResult = QGISAPP.themeName()
        myMessage = f"Expected:\n{myExpectedResult}\nGot:\n{myResult}\n"
        assert myExpectedResult == myResult, myMessage


if __name__ == "__main__":
    unittest.main()
