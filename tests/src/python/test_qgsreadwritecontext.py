"""QGIS Unit tests for QgsReadWriteContext.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Denis Rouzaud"
__date__ = "28.02.2018"
__copyright__ = "Copyright 2017, The QGIS Project"


from qgis.core import Qgis, QgsReadWriteContext
from qgis.testing import unittest


class TestQgsReadWriteContext(unittest.TestCase):

    def testEnterCategory(self):
        context = QgsReadWriteContext()
        context.pushMessage("msg0", Qgis.MessageLevel.Critical)
        with QgsReadWriteContext.enterCategory(context, "cat1"):
            context.pushMessage("msg1", Qgis.MessageLevel.Warning)
            with QgsReadWriteContext.enterCategory(context, "cat2", "detail2"):
                context.pushMessage("msg2")
            context.pushMessage("msg3")
        context.pushMessage("msg4")

        messages = context.takeMessages()

        self.assertEqual(messages[0].message(), "msg0")
        self.assertEqual(messages[0].level(), Qgis.MessageLevel.Critical)
        self.assertEqual(messages[0].categories(), [])

        self.assertEqual(messages[1].message(), "msg1")
        self.assertEqual(messages[1].level(), Qgis.MessageLevel.Warning)
        self.assertEqual(messages[1].categories(), ["cat1"])

        self.assertEqual(messages[2].message(), "msg2")
        self.assertEqual(messages[2].categories(), ["cat1", "cat2 :: detail2"])

        self.assertEqual(messages[3].message(), "msg3")
        self.assertEqual(messages[3].categories(), ["cat1"])

        self.assertEqual(messages[4].message(), "msg4")
        self.assertEqual(messages[4].categories(), [])

    def test_message_equality(self):
        """
        Test QgsReadWriteContext.ReadWriteMessage equality operator
        """
        m1 = QgsReadWriteContext.ReadWriteMessage(
            "m1", Qgis.MessageLevel.Info, ["cat1", "cat2"]
        )
        self.assertEqual(
            m1,
            QgsReadWriteContext.ReadWriteMessage(
                "m1", Qgis.MessageLevel.Info, ["cat1", "cat2"]
            ),
        )
        self.assertNotEqual(
            m1,
            QgsReadWriteContext.ReadWriteMessage(
                "m2", Qgis.MessageLevel.Info, ["cat1", "cat2"]
            ),
        )
        self.assertNotEqual(
            m1,
            QgsReadWriteContext.ReadWriteMessage(
                "m1", Qgis.MessageLevel.Warning, ["cat1", "cat2"]
            ),
        )
        self.assertNotEqual(
            m1,
            QgsReadWriteContext.ReadWriteMessage(
                "m1", Qgis.MessageLevel.Info, ["cat3", "cat2"]
            ),
        )


if __name__ == "__main__":
    unittest.main()
