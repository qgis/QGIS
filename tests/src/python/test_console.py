"""QGIS Unit tests for the console

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Matthias Kuhn"
__date__ = "15.4.2016"
__copyright__ = "Copyright 2015, The QGIS Project"

import unittest

from console import console
from qgis.core import QgsSettings
from qgis.testing import QgisTestCase, start_app

start_app()


class TestConsole(QgisTestCase):
    def setUp(self):
        QgsSettings().setValue("pythonConsole/contextHelpOnFirstLaunch", False)

    def test_show_console(self):
        my_console = console.show_console()
        my_console_widget = my_console.console


if __name__ == "__main__":
    unittest.main()
