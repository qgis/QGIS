"""QGIS Unit tests for QgsHtmlRenderSettings.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.core import QgsHtmlRenderSettings, QgsNetworkAccessManager
from qgis.PyQt.QtCore import QUrl
from qgis.testing import QgisTestCase, unittest
from utilities import start_app, unitTestDataPath

start_app()


class TestQgsHtmlRenderSettings(QgisTestCase):
    def test_settings(self):
        settings = QgsHtmlRenderSettings()
        # should default to same user agent as QGIS uses
        self.assertEqual(settings.userAgent(), QgsNetworkAccessManager.userAgent())
        self.assertIn("QGIS", settings.userAgent())
        self.assertIn("Mozilla", settings.userAgent())

        settings.setBaseUrl(QUrl("http://qgis.org"))
        self.assertEqual(settings.baseUrl(), QUrl("http://qgis.org"))

        settings.setUserStylesheet("""body: {margin: 1000px !important}""")
        self.assertEqual(
            settings.userStylesheet(), """body: {margin: 1000px !important}"""
        )

        settings.setFeatureJson({"geometry": "point"})
        self.assertEqual(settings.featureJson(), {"geometry": "point"})

        self.assertEqual(
            settings.asMap(),
            {
                "baseUrl": QUrl("http://qgis.org"),
                "feature": {"geometry": "point"},
                "userAgent": QgsNetworkAccessManager.userAgent(),
                "userStylesheet": "body: {margin: 1000px !important}",
            },
        )

    def test_empty(self):
        settings = QgsHtmlRenderSettings()
        self.assertFalse(settings.isEmpty())
        settings.setUserAgent("")
        self.assertTrue(settings.isEmpty())

        settings.setBaseUrl(QUrl("http://qgis.org"))
        self.assertFalse(settings.isEmpty())
        settings.setBaseUrl(QUrl())
        self.assertTrue(settings.isEmpty())

        settings.setUserStylesheet("""body: {margin: 1000px !important}""")
        self.assertFalse(settings.isEmpty())
        settings.setUserStylesheet("")
        self.assertTrue(settings.isEmpty())

        settings.setFeatureJson({"geometry": "point"})
        self.assertFalse(settings.isEmpty())
        settings.setFeatureJson({})
        self.assertTrue(settings.isEmpty())


if __name__ == "__main__":
    unittest.main()
