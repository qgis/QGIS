# -*- coding: utf-8 -*-
"""QGIS Unit tests for the console

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Matthias Kuhn'
__date__ = '15.4.2016'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.testing import unittest, start_app
from console import console

start_app()


class TestConsole(unittest.TestCase):

    def test_show_console(self):
        my_console = console.show_console()
        my_console_widget = my_console.console

        for action in my_console_widget.classMenu.actions():
            action.trigger()


if __name__ == "__main__":
    unittest.main()
