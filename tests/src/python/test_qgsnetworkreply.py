"""QGIS Unit tests for QgsNetworkReplyContent

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "20/06/2022"
__copyright__ = "Copyright 2022, The QGIS Project"


from qgis.core import QgsNetworkReplyContent
import unittest
from qgis.testing import start_app, QgisTestCase

app = start_app()


class TestQgsNetworkReply(QgisTestCase):

    def test_content_disposition_filename(self):
        self.assertEqual(
            QgsNetworkReplyContent.extractFileNameFromContentDispositionHeader("x"), ""
        )
        self.assertEqual(
            QgsNetworkReplyContent.extractFileNameFromContentDispositionHeader(
                "attachment; filename=content.txt"
            ),
            "content.txt",
        )
        self.assertEqual(
            QgsNetworkReplyContent.extractFileNameFromContentDispositionHeader(
                "attachment; filename*=UTF-8''filename.txt"
            ),
            "filename.txt",
        )
        self.assertEqual(
            QgsNetworkReplyContent.extractFileNameFromContentDispositionHeader(
                "attachment; filename=\"EURO rates\"; filename*=utf-8''%e2%82%ac%20rates"
            ),
            "€ rates",
        )
        self.assertEqual(
            QgsNetworkReplyContent.extractFileNameFromContentDispositionHeader(
                'attachment; filename="omáèka.jpg"'
            ),
            "omáèka.jpg",
        )


if __name__ == "__main__":
    unittest.main()
