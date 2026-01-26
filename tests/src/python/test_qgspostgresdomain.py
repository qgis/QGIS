"""QGIS Unit tests for Postgres domains.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Denis Rouzaud"
__date__ = "10/02/2018"
__copyright__ = "Copyright 2018, The QGIS Project"

import os

from qgis.core import QgsProject, QgsVectorLayer
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsPostgresDomain(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """
        Setup the involved layer
        :return:
        """
        super().setUpClass()
        cls.dbconn = "service='qgis_test'"
        if "QGIS_PGTEST_DB" in os.environ:
            cls.dbconn = os.environ["QGIS_PGTEST_DB"]
        # Create test layer
        cls.vl = QgsVectorLayer(
            cls.dbconn + ' sslmode=disable key=\'pk\' table="qgis_test"."colors" sql=',
            "colors",
            "postgres",
        )

        QgsProject.instance().addMapLayer(cls.vl)

    def test_postgres_domain(self):
        self.assertEqual(self.vl.dataProvider().enumValues(1), ["red", "green", "blue"])
        self.assertEqual(
            self.vl.dataProvider().enumValues(2), ["yellow", "cyan", "magenta"]
        )
        self.assertEqual(
            self.vl.dataProvider().enumValues(3),
            ["Alchemilla", "Alstroemeria", "Alyssum"],
        )


if __name__ == "__main__":
    unittest.main()
