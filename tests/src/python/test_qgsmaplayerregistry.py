# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMapLayerRegistry.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Alessandro Pasotti'
__date__ = '04/12/2015'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.core import QgsMapLayerRegistry
from qgis.testing import unittest


class TestQgsMapLayerRegistry(unittest.TestCase):

    def setUp(self):
        pass

    def test_RemoveLayerShouldNotSegFault(self):
        reg = QgsMapLayerRegistry.instance()
        # Should not segfault
        reg.removeMapLayers(['not_exists'])
        reg.removeMapLayer('not_exists2')

        # check also that the removal of an unexistant layer does not insert a null layer
        for k, layer in reg.mapLayers().items():
            assert(layer is not None)


if __name__ == '__main__':
    unittest.main()
