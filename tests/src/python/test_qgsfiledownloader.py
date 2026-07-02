"""
Test the QgsFileDownloader class

Run test with:
LC_ALL=en_US.UTF-8 ctest -V -R PyQgsFileDownloader

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os
import tempfile
import unittest
from functools import partial

from qgis.core import QgsFileDownloader
from qgis.PyQt.QtCore import QEventLoop, QUrl
from qgis.testing import QgisTestCase, start_app

__author__ = "Alessandro Pasotti"
__date__ = "08/11/2016"
__copyright__ = "Copyright 2016, The QGIS Project"

start_app()


class TestQgsFileDownloader(QgisTestCase):
    """
    This class tests the QgsFileDownloader class
    """

    def _make_download(self, url, destination, cancel=False):
        self.completed_was_called = False
        self.error_was_called = False
        self.canceled_was_called = False
        self.progress_was_called = False
        self.exited_was_called = False

        loop = QEventLoop()

        downloader = QgsFileDownloader(QUrl(url), destination)
        downloader.downloadCompleted.connect(partial(self._set_slot, "completed"))
        downloader.downloadExited.connect(partial(self._set_slot, "exited"))
        downloader.downloadCanceled.connect(partial(self._set_slot, "canceled"))
        downloader.downloadError.connect(partial(self._set_slot, "error"))
        downloader.downloadProgress.connect(partial(self._set_slot, "progress"))

        downloader.downloadExited.connect(loop.quit)

        if cancel:
            downloader.downloadProgress.connect(downloader.cancelDownload)

        loop.exec()

    @unittest.skipIf(
        os.environ.get("QGIS_CONTINUOUS_INTEGRATION_RUN", "true"),
        "Test with http://www.qgis.org unstable. Needs local server.",
    )
    def test_validDownload(self):
        """Tests a valid download"""
        destination = tempfile.mktemp()
        self._make_download("http://www.qgis.org", destination)
        self.assertTrue(self.exited_was_called)
        self.assertTrue(self.completed_was_called)
        self.assertTrue(self.progress_was_called)
        self.assertFalse(self.canceled_was_called)
        self.assertFalse(self.error_was_called)
        self.assertTrue(os.path.isfile(destination))
        self.assertGreater(os.path.getsize(destination), 0)

    def test_inValidDownload(self):
        """Tests an invalid download"""
        destination = tempfile.mktemp()
        self._make_download("http://www.doesnotexistofthatimsure.qgis", destination)
        self.assertTrue(self.exited_was_called)
        self.assertFalse(self.completed_was_called)
        self.assertTrue(self.progress_was_called)
        self.assertFalse(self.canceled_was_called)
        self.assertTrue(self.error_was_called)
        self.assertEqual(
            self.error_args[1],
            ["Download failed: Host www.doesnotexistofthatimsure.qgis not found"],
        )
        self.assertFalse(os.path.isfile(destination))

    @unittest.skipIf(
        os.environ.get("QGIS_CONTINUOUS_INTEGRATION_RUN", "true"),
        "Test with http://www.github.com unstable. Needs local server.",
    )
    def test_dowloadCanceled(self):
        """Tests user canceled download"""
        destination = tempfile.mktemp()
        self._make_download(
            "https://github.com/qgis/QGIS/archive/master.zip", destination, True
        )
        self.assertTrue(self.exited_was_called)
        self.assertFalse(self.completed_was_called)
        self.assertTrue(self.canceled_was_called)
        self.assertFalse(self.error_was_called)
        self.assertFalse(os.path.isfile(destination))

    def test_InvalidUrl(self):
        destination = tempfile.mktemp()
        self._make_download("xyz://www", destination)
        self.assertTrue(self.exited_was_called)
        self.assertFalse(self.completed_was_called)
        self.assertFalse(self.canceled_was_called)
        self.assertTrue(self.error_was_called)
        self.assertFalse(os.path.isfile(destination))
        self.assertEqual(
            self.error_args[1], ['Download failed: Protocol "xyz" is unknown']
        )

    @unittest.skipIf(
        os.environ.get("QGIS_CONTINUOUS_INTEGRATION_RUN", "true"),
        "Test with http://www.github.com unstable. Needs local server.",
    )
    def test_InvalidFile(self):
        self._make_download("https://github.com/qgis/QGIS/archive/master.zip", "")
        self.assertTrue(self.exited_was_called)
        self.assertFalse(self.completed_was_called)
        self.assertFalse(self.canceled_was_called)
        self.assertTrue(self.error_was_called)
        self.assertEqual(self.error_args[1], ["No output filename specified"])

    def test_BlankUrl(self):
        destination = tempfile.mktemp()
        self._make_download("", destination)
        self.assertTrue(self.exited_was_called)
        self.assertFalse(self.completed_was_called)
        self.assertFalse(self.canceled_was_called)
        self.assertTrue(self.error_was_called)
        self.assertFalse(os.path.isfile(destination))
        self.assertEqual(
            self.error_args[1], ['Download failed: Protocol "" is unknown']
        )

    def ssl_compare(self, name, url, error):
        destination = tempfile.mktemp()
        self._make_download(url, destination)
        msg = f"Failed in {name}: {url}\n"
        self.assertTrue(self.exited_was_called)
        self.assertFalse(self.completed_was_called, msg)
        self.assertFalse(self.canceled_was_called, msg)
        self.assertTrue(self.error_was_called, msg)
        self.assertFalse(os.path.isfile(destination), msg)
        result = sorted(self.error_args[1])
        result = ";".join(result)
        self.assertTrue(
            result.startswith(error), msg + f"- Expected: {error}\n- Actual: {result}\n"
        )

    def test_sslExpired(self):

        # badssl.com is really unstable, so prefer setting up a local instance if you want to reproduce
        # like it's done in CI. To do so, see tests folder README.md, section "Local badssl server"

        expired_url = os.environ.get(
            "QGIS_BADSSL_URL_EXPIRED", "https://expired.badssl.com/"
        )
        print(f"expired url: {expired_url}")
        self.ssl_compare(
            "expired",
            expired_url,
            "SSL Errors: ;The certificate has expired",
        )

        selfsigned_url = os.environ.get(
            "QGIS_BADSSL_URL_SELFSIGNED", "https://self-signed.badssl.com/"
        )
        print(f"selfsigned url: {selfsigned_url}")
        self.ssl_compare(
            "self-signed",
            selfsigned_url,
            "SSL Errors: ;The certificate is self-signed, and untrusted",
        )

        untrusted_url = os.environ.get(
            "QGIS_BADSSL_URL_UNTRUSTED", "https://untrusted-root.badssl.com/"
        )
        print(f"untrusted_url: {untrusted_url}")
        self.ssl_compare(
            "untrusted-root",
            untrusted_url,
            "SSL Errors: ;The root certificate of the certificate chain is self-signed, and untrusted",
        )

    def _set_slot(self, *args, **kwargs):
        # print('_set_slot(%s) called' % args[0])
        setattr(self, args[0] + "_was_called", True)
        setattr(self, args[0] + "_args", args)


if __name__ == "__main__":
    unittest.main()
