# coding=utf-8
"""Dialog test.

.. note:: This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

"""

__author__ = 'akbargumbira@gmail.com'
__date__ = '2016-05-29'
__copyright__ = 'Copyright 2016, Akbar Gumbira'


from qgis.testing import start_app, unittest
import nose2


try:
    from qgis.PyQt.QtGui import QDialogButtonBox, QDialog
except ImportError:
    from qgis.PyQt.QtWidgets import QDialogButtonBox, QDialog


from resource_sharing.gui.resource_sharing_dialog import ResourceSharingDialog
from resource_sharing.config import (
    COLLECTION_ALL_STATUS,
    COLLECTION_INSTALLED_STATUS)


class ResourceSharingDialogTest(unittest.TestCase):
    """Test dialog works."""
    @classmethod
    def setUpClass(cls):
        start_app()

    def setUp(self):
        """Runs before each test."""
        self.dialog = ResourceSharingDialog(None)

    def tearDown(self):
        """Runs after each test."""
        self.dialog = None

    def test_dialog_help(self):
        """Test we can click Help."""
        button = self.dialog.button_box.button(QDialogButtonBox.Help)
        button.click()
        result = self.dialog.result()
        # For now Rejected as we don't have any action for the help button
        self.assertEqual(result, QDialog.Rejected)

    def test_dialog_close(self):
        """Test we can click Close."""
        button = self.dialog.button_box.button(QDialogButtonBox.Close)
        button.click()
        result = self.dialog.result()
        self.assertEqual(result, QDialog.Rejected)

    def test_set_current_tab(self):
        """Test set the current tab works."""
        # Set to index 0, 1, or 2
        for index in [0, 1, 2]:
            self.dialog.menu_list_widget.setCurrentRow(index)
            if index in [0, 1]:
                # Tab All and installed
                self.assertEqual(
                    self.dialog.stacked_menu_widget.currentIndex(), 0)
                if index == 0:
                    self.assertEqual(
                        self.dialog.collection_proxy.accepted_status,
                        COLLECTION_ALL_STATUS)
                else:
                    self.assertEqual(
                        self.dialog.collection_proxy.accepted_status,
                        COLLECTION_INSTALLED_STATUS)
            else:
                # Tab Settings
                self.assertEqual(
                    self.dialog.stacked_menu_widget.currentIndex(), 1)


if __name__ == "__main__":
    nose2.main()
