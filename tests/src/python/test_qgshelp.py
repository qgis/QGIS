# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsHelp class

From build dir, run: ctest -R PyQgsHelp -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Julien Cabieces'
__date__ = '2022-09-21'
__copyright__ = 'Copyright 2022, Julien Cabieces'

from qgis.PyQt.QtCore import QCoreApplication

from qgis.core import QgsSettings
from qgis.gui import QgsHelp
from qgis.testing import start_app, unittest
import mockedwebserver


class TestQgsHelp(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        cls.server, cls.port = mockedwebserver.launch()

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestPyQgsHelp")
        QCoreApplication.setApplicationName("TestPyQgsHelp")
        QgsSettings().clear()
        start_app()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        cls.server.stop()
        QgsSettings().clear()

    def testOpenUrl(self):
        """
        Tests returned url according to help key
        """

        server_url = 'http://localhost:{}/'.format(TestQgsHelp.port)

        QgsSettings().setValue(
            "help/helpSearchPath",
            [
                server_url + "first_search_path",
                server_url + "second_search_path",
            ],
        )

        handler = mockedwebserver.SequentialHandler()
        handler.add('HEAD', '/first_search_path/first_url.html', 200, {})
        handler.add('HEAD', '/first_search_path/second_url.html', 400, {})
        handler.add('HEAD', '/second_search_path/second_url.html', 200, {})
        handler.add('HEAD', '/first_search_path/error_url.html', 404, {})
        handler.add('HEAD', '/second_search_path/error_url.html', 404, {})
        with mockedwebserver.install_http_handler(handler):
            self.assertEquals(server_url + "first_search_path/first_url.html", QgsHelp.helpUrl("first_url.html").toDisplayString())
            self.assertEquals(server_url + "second_search_path/second_url.html", QgsHelp.helpUrl("second_url.html").toDisplayString())
            self.assertTrue(QgsHelp.helpUrl("error_url.html").toDisplayString().endswith("nohelp.html"))


if __name__ == '__main__':
    unittest.main()
