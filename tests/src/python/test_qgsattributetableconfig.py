# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsAttributeTableConfig.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '07/06/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsAttributeTableConfig,
                       QgsVectorLayer
                       )
from qgis.testing import start_app, unittest

start_app()


class TestQgsAttributeTableConfig(unittest.TestCase):

    def testLayerConfig(self):
        """ test retrieving attribute table config from a layer """

        # make a layer
        point_layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                                     "pointlayer", "memory")

        # make sure attribute table config is initially populated
        config = point_layer.attributeTableConfig()
        self.assertFalse(config.isEmpty())
        self.assertEqual(config.columns()[0].name, 'fldtxt')
        self.assertEqual(config.columns()[1].name, 'fldint')

        # try replacing it
        config.setColumns([config.columns()[1], config.columns()[0]])
        point_layer.setAttributeTableConfig(config)

        # and make sure changes were applied
        config = point_layer.attributeTableConfig()
        self.assertFalse(config.isEmpty())
        self.assertEqual(config.columns()[0].name, 'fldint')
        self.assertEqual(config.columns()[1].name, 'fldtxt')

if __name__ == '__main__':
    unittest.main()
