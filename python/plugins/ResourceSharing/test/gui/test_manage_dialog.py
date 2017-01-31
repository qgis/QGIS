# coding=utf-8
from qgis.testing import start_app, unittest
import nose2

try:
    from qgis.PyQt.QtGui import QDialogButtonBox
except ImportError:
    from qgis.PyQt.QtWidgets import QDialogButtonBox


from resource_sharing.gui.manage_dialog import ManageRepositoryDialog
from test.utilities import test_repository_url


class ManageDialogTest(unittest.TestCase):
    """Test dialog works."""
    @classmethod
    def setUpClass(cls):
        start_app()

    def setUp(self):
        """Runs before each test."""
        self.dialog = ManageRepositoryDialog(None)

    def tearDown(self):
        """Runs after each test."""
        self.dialog = None

    def test_form_changed(self):
        """Add form changed test."""
        self.assertEqual(
            self.dialog.buttonBox.button(QDialogButtonBox.Ok).isEnabled(),
            False)
        self.dialog.line_edit_name.setText('Repository Name')
        self.dialog.line_edit_url.setText(test_repository_url())
        self.assertEqual(
            self.dialog.buttonBox.button(QDialogButtonBox.Ok).isEnabled(),
            True)

if __name__ == "__main__":
    nose2.main()
