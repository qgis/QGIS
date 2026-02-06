"""QGIS Unit tests for QgsShortcutsManager.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "28/05/2016"
__copyright__ = "Copyright 2016, The QGIS Project"

import os
from typing import List

from qgis.PyQt.QtCore import QCoreApplication, QObject, QEvent
from qgis.PyQt.QtWidgets import QAction, QShortcut, QWidget
from qgis.core import QgsSettings
from qgis.gui import QgsGui, QgsShortcutsManager
import unittest
from qgis.testing import start_app, QgisTestCase


class TestQgsShortcutsManager(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestPyQgsShortcutsManager.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsShortcutsManager")
        QgsSettings().clear()
        start_app()

    @staticmethod
    def filter_common_actions(actions: list[QObject]) -> list[QObject]:
        return [a for a in actions if a.property("commonAction") is None]

    def testInstance(self):
        """test retrieving global instance"""
        self.assertTrue(QgsGui.shortcutsManager())

        # register an action to the singleton
        action = QAction("test", None)
        QgsGui.shortcutsManager().registerAction(action)
        # check that the same instance is returned
        self.assertEqual(
            self.filter_common_actions(QgsGui.shortcutsManager().listActions()),
            [action],
        )
        s2 = QgsShortcutsManager()
        self.assertEqual(self.filter_common_actions(s2.listActions()), [])

    def testConstructor(self):
        """test constructing managers"""
        s = QgsShortcutsManager(None, "/my_path/")
        self.assertEqual(s.settingsPath(), "/my_path/")

    def testSettingsPath(self):
        """test that settings path is respected"""

        QgsSettings().clear()

        s1 = QgsShortcutsManager(None, "/path1/")
        s2 = QgsShortcutsManager(None, "/path2/")

        action1 = QAction("action", None)
        s1.registerAction(action1)
        s1.setKeySequence(action1, "B")

        action2 = QAction("action", None)
        s2.registerAction(action2)
        s2.setKeySequence(action2, "C")

        # test retrieving
        r1 = QgsShortcutsManager(None, "/path1/")
        r2 = QgsShortcutsManager(None, "/path2/")

        raction1 = QAction("action", None)
        r1.registerAction(raction1)
        raction2 = QAction("action", None)
        r2.registerAction(raction2)

        self.assertEqual(raction1.shortcut().toString(), "B")
        self.assertEqual(raction2.shortcut().toString(), "C")

    def testRegisterAction(self):
        """test registering actions"""
        QgsSettings().clear()

        s = QgsShortcutsManager(None)

        action1 = QAction("action1", None)
        action1.setShortcut("x")
        self.assertTrue(s.registerAction(action1, "A"))
        action2 = QAction("action2", None)
        action2.setShortcut("y")
        self.assertTrue(s.registerAction(action2, "B"))
        self.assertCountEqual(
            self.filter_common_actions(s.listActions()), [action1, action2]
        )

        # try re-registering an existing action - should fail, but leave action registered
        self.assertFalse(s.registerAction(action2, "B"))
        self.assertCountEqual(
            self.filter_common_actions(s.listActions()), [action1, action2]
        )

        # actions should have been set to default sequences
        self.assertEqual(action1.shortcut().toString(), "A")
        self.assertEqual(action2.shortcut().toString(), "B")

        # test that adding an action should set its shortcut automatically
        s.setKeySequence("action1", "C")
        s.setKeySequence("action2", "D")

        s = QgsShortcutsManager(None)
        self.assertTrue(s.registerAction(action1, "A"))
        self.assertTrue(s.registerAction(action2, "B"))

        # actions should have been set to previous shortcuts
        self.assertEqual(action1.shortcut().toString(), "C")
        self.assertEqual(action2.shortcut().toString(), "D")

        # test registering an action containing '&' in name
        s = QgsShortcutsManager(None)
        action = QAction("&action1", None)
        self.assertTrue(s.registerAction(action))
        self.assertEqual(action1.shortcut().toString(), "C")

    def testRegisterShortcut(self):
        """test registering shortcuts"""
        QgsSettings().clear()

        s = QgsShortcutsManager(None)

        shortcut1 = QShortcut(None)
        shortcut1.setKey("x")
        shortcut1.setObjectName("shortcut1")
        self.assertTrue(s.registerShortcut(shortcut1, "A"))
        shortcut2 = QShortcut(None)
        shortcut2.setKey("y")
        shortcut2.setObjectName("shortcut2")
        self.assertTrue(s.registerShortcut(shortcut2, "B"))

        # shortcuts should have been set to default sequences
        self.assertEqual(shortcut1.key().toString(), "A")
        self.assertEqual(shortcut2.key().toString(), "B")

        # test that adding a shortcut should set its sequence automatically
        s.setKeySequence(shortcut1, "C")
        s.setKeySequence(shortcut2, "D")

        s = QgsShortcutsManager(None)
        self.assertTrue(s.registerShortcut(shortcut1, "A"))
        self.assertTrue(s.registerShortcut(shortcut2, "B"))

        # shortcuts should have been set to previous sequences
        self.assertEqual(shortcut1.key().toString(), "C")
        self.assertEqual(shortcut2.key().toString(), "D")

    def testRegisterAll(self):
        """test registering all children"""

        w = QWidget()
        action1 = QAction("action1", w)
        shortcut1 = QShortcut(w)
        shortcut1.setObjectName("shortcut1")
        w2 = QWidget(w)
        action2 = QAction("action2", w2)
        shortcut2 = QShortcut(w2)
        shortcut2.setObjectName("shortcut2")

        # recursive
        s = QgsShortcutsManager()
        s.registerAllChildActions(w, True)
        self.assertEqual(
            set(self.filter_common_actions(s.listActions())), {action1, action2}
        )
        s.registerAllChildShortcuts(w, True)
        self.assertEqual(set(s.listShortcuts()), {shortcut1, shortcut2})

        # non recursive
        s = QgsShortcutsManager()
        s.registerAllChildActions(w, False)
        self.assertEqual(set(self.filter_common_actions(s.listActions())), {action1})
        s.registerAllChildShortcuts(w, False)
        self.assertEqual(set(s.listShortcuts()), {shortcut1})

        # recursive
        s = QgsShortcutsManager()
        s.registerAllChildren(w, True)
        self.assertEqual(
            set(self.filter_common_actions(s.listActions())), {action1, action2}
        )
        self.assertEqual(set(s.listShortcuts()), {shortcut1, shortcut2})

        # non recursive
        s = QgsShortcutsManager()
        s.registerAllChildren(w, False)
        self.assertEqual(set(self.filter_common_actions(s.listActions())), {action1})
        self.assertEqual(set(s.listShortcuts()), {shortcut1})

    def testUnregister(self):
        """test unregistering from manager"""

        QgsSettings().clear()

        s = QgsShortcutsManager(None)

        shortcut1 = QShortcut(None)
        shortcut1.setKey("x")
        shortcut1.setObjectName("shortcut1")
        shortcut2 = QShortcut(None)
        shortcut2.setKey("y")
        shortcut2.setObjectName("shortcut2")

        action1 = QAction("action1", None)
        action1.setShortcut("x")
        action2 = QAction("action2", None)
        action2.setShortcut("y")

        # try unregistering objects not registered in manager
        self.assertFalse(s.unregisterShortcut(shortcut1))
        self.assertFalse(s.unregisterAction(action1))

        # try unregistering objects from manager
        s.registerShortcut(shortcut1)
        s.registerShortcut(shortcut2)
        s.registerAction(action1)
        s.registerAction(action2)

        self.assertEqual(
            set(self.filter_common_actions(s.listActions())), {action1, action2}
        )
        self.assertEqual(set(s.listShortcuts()), {shortcut1, shortcut2})

        self.assertTrue(s.unregisterAction(action1))
        self.assertTrue(s.unregisterShortcut(shortcut1))

        self.assertEqual(set(self.filter_common_actions(s.listActions())), {action2})
        self.assertEqual(set(s.listShortcuts()), {shortcut2})

        self.assertTrue(s.unregisterAction(action2))
        self.assertTrue(s.unregisterShortcut(shortcut2))

    def testList(self):
        """test listing registered objects"""

        QgsSettings().clear()

        s = QgsShortcutsManager(None)

        self.assertEqual(self.filter_common_actions(s.listActions()), [])
        self.assertEqual(s.listShortcuts(), [])
        self.assertEqual(self.filter_common_actions(s.listAll()), [])

        shortcut1 = QShortcut(None)
        shortcut2 = QShortcut(None)
        action1 = QAction("action1", None)
        action2 = QAction("action2", None)
        s.registerShortcut(shortcut1)
        s.registerShortcut(shortcut2)
        s.registerAction(action1)
        s.registerAction(action2)

        self.assertEqual(
            set(self.filter_common_actions(s.listActions())), {action1, action2}
        )
        self.assertEqual(set(s.listShortcuts()), {shortcut1, shortcut2})
        self.assertEqual(
            set(self.filter_common_actions(s.listAll())),
            {action1, action2, shortcut1, shortcut2},
        )

    def testDefault(self):
        """test retrieving default sequences"""

        QgsSettings().clear()

        s = QgsShortcutsManager(None)

        shortcut1 = QShortcut(None)
        shortcut2 = QShortcut(None)
        action1 = QAction("action1", None)
        action2 = QAction("action2", None)

        # test while not yet registered
        self.assertEqual(s.defaultKeySequence(shortcut1), "")
        self.assertEqual(s.defaultKeySequence(action1), "")
        self.assertEqual(s.objectDefaultKeySequence(shortcut1), "")
        self.assertEqual(s.objectDefaultKeySequence(action1), "")

        # now register them
        s.registerShortcut(shortcut1, "A")
        s.registerShortcut(shortcut2, "B")
        s.registerAction(action1, "C")
        s.registerAction(action2, "D")

        self.assertEqual(s.defaultKeySequence(shortcut1), "A")
        self.assertEqual(s.defaultKeySequence(shortcut2), "B")
        self.assertEqual(s.defaultKeySequence(action1), "C")
        self.assertEqual(s.defaultKeySequence(action2), "D")
        self.assertEqual(s.objectDefaultKeySequence(shortcut1), "A")
        self.assertEqual(s.objectDefaultKeySequence(shortcut2), "B")
        self.assertEqual(s.objectDefaultKeySequence(action1), "C")
        self.assertEqual(s.objectDefaultKeySequence(action2), "D")

    def testSetSequence(self):
        """test setting key sequences"""

        QgsSettings().clear()

        s = QgsShortcutsManager(None)

        shortcut1 = QShortcut(None)
        shortcut1.setObjectName("shortcut1")
        shortcut2 = QShortcut(None)
        shortcut2.setObjectName("shortcut2")
        action1 = QAction("action1", None)
        action2 = QAction("action2", None)

        s.registerShortcut(shortcut1, "A")
        s.registerShortcut(shortcut2, "B")
        s.registerAction(action1, "C")
        s.registerAction(action2, "D")

        # test setting by action/shortcut
        self.assertTrue(s.setKeySequence(shortcut1, "E"))
        self.assertTrue(s.setKeySequence(shortcut2, "F"))
        self.assertTrue(s.setKeySequence(action1, "G"))
        self.assertTrue(s.setKeySequence(action2, "H"))

        # test that action/shortcuts have been updated
        self.assertEqual(shortcut1.key().toString(), "E")
        self.assertEqual(shortcut2.key().toString(), "F")
        self.assertEqual(action1.shortcut().toString(), "G")
        self.assertEqual(action2.shortcut().toString(), "H")

        # new manager
        s = QgsShortcutsManager(None)
        # new shortcuts
        shortcut1 = QShortcut(None)
        shortcut1.setObjectName("shortcut1")
        shortcut2 = QShortcut(None)
        shortcut2.setObjectName("shortcut2")
        action1 = QAction("action1", None)
        action2 = QAction("action2", None)

        # register them
        s.registerShortcut(shortcut1, "A")
        s.registerShortcut(shortcut2, "B")
        s.registerAction(action1, "C")
        s.registerAction(action2, "D")

        # check that previously set sequence has been restored
        self.assertEqual(shortcut1.key().toString(), "E")
        self.assertEqual(shortcut2.key().toString(), "F")
        self.assertEqual(action1.shortcut().toString(), "G")
        self.assertEqual(action2.shortcut().toString(), "H")

        # same test, using setObjectKeySequence
        QgsSettings().clear()
        s = QgsShortcutsManager(None)
        shortcut1 = QShortcut(None)
        shortcut1.setObjectName("shortcut1")
        action1 = QAction("action1", None)
        s.registerShortcut(shortcut1, "A")
        s.registerAction(action1, "C")
        self.assertTrue(s.setObjectKeySequence(shortcut1, "E"))
        self.assertTrue(s.setObjectKeySequence(action1, "G"))
        self.assertEqual(shortcut1.key().toString(), "E")
        self.assertEqual(action1.shortcut().toString(), "G")
        s = QgsShortcutsManager(None)
        shortcut1 = QShortcut(None)
        shortcut1.setObjectName("shortcut1")
        action1 = QAction("action1", None)
        s.registerShortcut(shortcut1, "A")
        s.registerAction(action1, "C")
        self.assertEqual(shortcut1.key().toString(), "E")
        self.assertEqual(action1.shortcut().toString(), "G")

        # same test, using setKeySequence by name
        QgsSettings().clear()
        s = QgsShortcutsManager(None)
        shortcut1 = QShortcut(None)
        shortcut1.setObjectName("shortcut1")
        action1 = QAction("action1", None)
        s.registerShortcut(shortcut1, "A")
        s.registerAction(action1, "C")
        self.assertFalse(s.setKeySequence("invalid_name", "E"))
        self.assertTrue(s.setKeySequence("shortcut1", "E"))
        self.assertTrue(s.setKeySequence("action1", "G"))
        self.assertEqual(shortcut1.key().toString(), "E")
        self.assertEqual(action1.shortcut().toString(), "G")
        s = QgsShortcutsManager(None)
        shortcut1 = QShortcut(None)
        shortcut1.setObjectName("shortcut1")
        action1 = QAction("action1", None)
        s.registerShortcut(shortcut1, "A")
        s.registerAction(action1, "C")
        self.assertEqual(shortcut1.key().toString(), "E")
        self.assertEqual(action1.shortcut().toString(), "G")

    def testBySequence(self):
        """test retrieving by sequence"""
        QgsSettings().clear()

        shortcut1 = QShortcut(None)
        shortcut1.setObjectName("shortcut1")
        shortcut2 = QShortcut(None)
        shortcut2.setObjectName("shortcut2")
        action1 = QAction("action1", None)
        action2 = QAction("action2", None)

        s = QgsShortcutsManager(None)
        self.assertFalse(s.actionForSequence("E"))
        self.assertFalse(s.objectForSequence("F"))

        s.registerShortcut(shortcut1, "E")
        s.registerShortcut(shortcut2, "A")
        s.registerAction(action1, "F")
        s.registerAction(action2, "B")
        # use another way of registering sequences
        self.assertTrue(s.setKeySequence(shortcut2, "G"))
        self.assertTrue(s.setKeySequence(action2, "H"))

        self.assertEqual(s.objectForSequence("E"), shortcut1)
        self.assertEqual(s.objectForSequence("F"), action1)
        self.assertEqual(s.objectForSequence("G"), shortcut2)
        self.assertEqual(s.objectForSequence("H"), action2)
        self.assertFalse(s.objectForSequence("A"))
        self.assertFalse(s.objectForSequence("B"))

        self.assertEqual(s.shortcutForSequence("E"), shortcut1)
        self.assertFalse(s.shortcutForSequence("F"))
        self.assertEqual(s.shortcutForSequence("G"), shortcut2)
        self.assertFalse(s.shortcutForSequence("H"))
        self.assertFalse(s.actionForSequence("E"))
        self.assertEqual(s.actionForSequence("F"), action1)
        self.assertFalse(s.actionForSequence("G"))
        self.assertEqual(s.actionForSequence("H"), action2)

    def testByName(self):
        """ " test retrieving actions and shortcuts by name"""
        QgsSettings().clear()

        shortcut1 = QShortcut(None)
        shortcut1.setObjectName("shortcut1")
        shortcut2 = QShortcut(None)
        shortcut2.setObjectName("shortcut2")
        action1 = QAction("action1", None)
        action2 = QAction("action2", None)

        s = QgsShortcutsManager(None)
        self.assertFalse(s.actionByName("action1"))
        self.assertFalse(s.shortcutByName("shortcut1"))

        s.registerShortcut(shortcut1)
        s.registerShortcut(shortcut2)
        s.registerAction(action1)
        s.registerAction(action2)

        self.assertEqual(s.shortcutByName("shortcut1"), shortcut1)
        self.assertFalse(s.shortcutByName("action1"))
        self.assertEqual(s.shortcutByName("shortcut2"), shortcut2)
        self.assertFalse(s.shortcutByName("action2"))
        self.assertFalse(s.actionByName("shortcut1"))
        self.assertEqual(s.actionByName("action1"), action1)
        self.assertFalse(s.actionByName("shortcut2"))
        self.assertEqual(s.actionByName("action2"), action2)

    def testTooltip(self):
        """ " test action tooltips"""
        action1 = QAction("action1", None)
        action1.setToolTip("my tooltip")
        action2 = QAction("action2", None)
        action2.setToolTip("my multiline\ntooltip")
        action3 = QAction("action3", None)
        action3.setToolTip("my tooltip (Ctrl+S)")

        s = QgsShortcutsManager(None)
        s.registerAction(action1)
        s.registerAction(action2)
        s.registerAction(action3, "Ctrl+S")

        self.assertEqual(action1.toolTip(), "<b>my tooltip</b>")
        self.assertEqual(action2.toolTip(), "<b>my multiline</b><p>tooltip</p>")
        self.assertEqual(action3.toolTip(), "<b>my tooltip</b> (Ctrl+S)")

    def test_common_actions(self):
        s = QgsShortcutsManager(None)
        reformat_code_action = [
            a
            for a in s.listActions()
            if a.property("commonAction")
            == QgsShortcutsManager.CommonAction.CodeReformat.value
        ][0]
        self.assertEqual(reformat_code_action.text(), "Reformat Code")
        self.assertEqual(reformat_code_action.shortcut().toString(), "Ctrl+Alt+F")
        self.assertEqual(
            reformat_code_action.toolTip(), "<b>Reformat code</b> (Ctrl+Alt+F)"
        )
        toggle_code_comment_action = [
            a
            for a in s.listActions()
            if a.property("commonAction")
            == QgsShortcutsManager.CommonAction.CodeToggleComment.value
        ][0]
        self.assertEqual(toggle_code_comment_action.text(), "Toggle Comment")

        # on Windows Ctrl+/ does not work, so we stick with original Ctrl+:
        # see https://github.com/qgis/QGIS/issues/63429
        toggle_comment_shortcut = "Ctrl+/"
        if os.name == "nt":
            toggle_comment_shortcut = "Ctrl+:"

        self.assertEqual(
            toggle_code_comment_action.shortcut().toString(), toggle_comment_shortcut
        )
        self.assertEqual(
            toggle_code_comment_action.toolTip(),
            f"<b>Toggle comment</b> ({toggle_comment_shortcut})",
        )

        self.assertEqual(
            s.sequenceForCommonAction(
                QgsShortcutsManager.CommonAction.CodeReformat
            ).toString(),
            "Ctrl+Alt+F",
        )
        self.assertEqual(
            s.sequenceForCommonAction(
                QgsShortcutsManager.CommonAction.CodeToggleComment
            ).toString(),
            toggle_comment_shortcut,
        )

        # link an action to a common action
        my_reformat_action1 = QAction()
        s.initializeCommonAction(
            my_reformat_action1, QgsShortcutsManager.CommonAction.CodeReformat
        )
        my_reformat_action2 = QAction()
        s.initializeCommonAction(
            my_reformat_action2, QgsShortcutsManager.CommonAction.CodeReformat
        )

        # default properties should be set
        self.assertEqual(my_reformat_action1.text(), "Reformat Code")
        self.assertEqual(my_reformat_action1.shortcut().toString(), "Ctrl+Alt+F")
        self.assertEqual(
            my_reformat_action1.toolTip(), "<b>Reformat code</b> (Ctrl+Alt+F)"
        )
        self.assertEqual(my_reformat_action2.text(), "Reformat Code")
        self.assertEqual(my_reformat_action2.shortcut().toString(), "Ctrl+Alt+F")
        self.assertEqual(
            my_reformat_action2.toolTip(), "<b>Reformat code</b> (Ctrl+Alt+F)"
        )
        my_toggle_comment_action = QAction()
        s.initializeCommonAction(
            my_toggle_comment_action, QgsShortcutsManager.CommonAction.CodeToggleComment
        )
        self.assertEqual(my_toggle_comment_action.text(), "Toggle Comment")
        self.assertEqual(
            my_toggle_comment_action.shortcut().toString(), toggle_comment_shortcut
        )
        self.assertEqual(
            my_toggle_comment_action.toolTip(),
            f"<b>Toggle comment</b> ({toggle_comment_shortcut})",
        )

        # change shortcut
        s.setKeySequence(reformat_code_action, "B")
        self.assertEqual(my_reformat_action1.shortcut().toString(), "B")
        self.assertEqual(my_reformat_action1.toolTip(), "<b>Reformat code</b> (B)")
        self.assertEqual(my_reformat_action2.shortcut().toString(), "B")
        self.assertEqual(my_reformat_action2.toolTip(), "<b>Reformat code</b> (B)")
        self.assertEqual(
            my_toggle_comment_action.shortcut().toString(), toggle_comment_shortcut
        )

        self.assertEqual(
            s.sequenceForCommonAction(
                QgsShortcutsManager.CommonAction.CodeReformat
            ).toString(),
            "B",
        )
        self.assertEqual(
            s.sequenceForCommonAction(
                QgsShortcutsManager.CommonAction.CodeToggleComment
            ).toString(),
            toggle_comment_shortcut,
        )

        s.setKeySequence(toggle_code_comment_action, "C")
        self.assertEqual(my_reformat_action1.shortcut().toString(), "B")
        self.assertEqual(my_reformat_action1.toolTip(), "<b>Reformat code</b> (B)")
        self.assertEqual(my_reformat_action2.shortcut().toString(), "B")
        self.assertEqual(my_reformat_action2.toolTip(), "<b>Reformat code</b> (B)")
        self.assertEqual(my_toggle_comment_action.shortcut().toString(), "C")
        self.assertEqual(
            my_toggle_comment_action.toolTip(), "<b>Toggle comment</b> (C)"
        )

        self.assertEqual(
            s.sequenceForCommonAction(
                QgsShortcutsManager.CommonAction.CodeReformat
            ).toString(),
            "B",
        )
        self.assertEqual(
            s.sequenceForCommonAction(
                QgsShortcutsManager.CommonAction.CodeToggleComment
            ).toString(),
            "C",
        )

        # delete local action
        my_reformat_action2.deleteLater()
        QCoreApplication.sendPostedEvents(None, QEvent.Type.DeferredDelete)
        my_reformat_action2 = None

        # should be no crash
        s.setKeySequence(reformat_code_action, "D")
        self.assertEqual(my_reformat_action1.shortcut().toString(), "D")
        self.assertEqual(my_reformat_action1.toolTip(), "<b>Reformat code</b> (D)")
        self.assertEqual(my_toggle_comment_action.shortcut().toString(), "C")
        self.assertEqual(
            my_toggle_comment_action.toolTip(), "<b>Toggle comment</b> (C)"
        )


if __name__ == "__main__":
    unittest.main()
