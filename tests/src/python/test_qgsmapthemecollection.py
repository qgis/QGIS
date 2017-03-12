# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMapThemeCollection.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '8/03/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsMapThemeCollection,
                       QgsProject,
                       QgsVectorLayer)
from qgis.testing import start_app, unittest
from qgis.PyQt.QtTest import QSignalSpy

app = start_app()


class TestQgsMapThemeCollection(unittest.TestCase):

    def setUp(self):
        pass

    def testThemeChanged(self):
        """
        Test that the mapTheme(s)Changed signals are correctly emitted in all relevant situations
        """
        project = QgsProject()
        collection = QgsMapThemeCollection(project)

        record = QgsMapThemeCollection.MapThemeRecord()

        theme_changed_spy = QSignalSpy(collection.mapThemeChanged)
        themes_changed_spy = QSignalSpy(collection.mapThemesChanged)

        collection.insert('theme1', record)
        self.assertEqual(len(theme_changed_spy), 1)
        self.assertEqual(theme_changed_spy[-1][0], 'theme1')
        self.assertEqual(len(themes_changed_spy), 1)

        # reinsert
        collection.insert('theme1', record)
        self.assertEqual(len(theme_changed_spy), 2)
        self.assertEqual(theme_changed_spy[-1][0], 'theme1')
        self.assertEqual(len(themes_changed_spy), 2)

        # update
        collection.update('theme1', record)
        self.assertEqual(len(theme_changed_spy), 3)
        self.assertEqual(theme_changed_spy[-1][0], 'theme1')
        self.assertEqual(len(themes_changed_spy), 3)

        # remove invalid
        collection.removeMapTheme('i wish i was a slave to an age old trade... like riding around on rail cars and working long days')
        self.assertEqual(len(theme_changed_spy), 3)
        self.assertEqual(len(themes_changed_spy), 3)
        # remove valid
        collection.removeMapTheme('theme1')
        self.assertEqual(len(theme_changed_spy), 3) # not changed - removed!
        self.assertEqual(len(themes_changed_spy), 4)

        # reinsert
        collection.insert('theme1', record)
        self.assertEqual(len(theme_changed_spy), 4)
        self.assertEqual(len(themes_changed_spy), 5)

        # clear
        collection.clear()
        self.assertEqual(len(theme_changed_spy), 4)  # not changed - removed!
        self.assertEqual(len(themes_changed_spy), 6)

        # check that mapThemeChanged is emitted if layer is removed
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")
        layer2 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer2", "memory")
        project.addMapLayers([layer, layer2])

        # record for layer1
        record.addLayerRecord(QgsMapThemeCollection.MapThemeLayerRecord(layer))
        collection.insert('theme1', record)
        self.assertEqual(len(theme_changed_spy), 5)
        self.assertEqual(len(themes_changed_spy), 7)

        # now kill layer 2
        project.removeMapLayer(layer2)
        self.assertEqual(len(theme_changed_spy), 5) # signal should not be emitted - layer is not in record
        # now kill layer 1
        project.removeMapLayer(layer)
        app.processEvents()
        self.assertEqual(len(theme_changed_spy), 6) # signal should be emitted - layer is in record


if __name__ == '__main__':
    unittest.main()
