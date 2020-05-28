# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsPropertyOverrideButton.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '11/01/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsPropertyDefinition,
                       QgsProperty,
                       QgsApplication,
                       QgsProjectColorScheme)

from qgis.gui import (QgsColorButton,
                      QgsPropertyOverrideButton)

from qgis.testing import start_app, unittest
from qgis.PyQt.QtGui import QColor

start_app()


class TestQgsPropertyOverrideButton(unittest.TestCase):

    def testProjectColor(self):
        scheme = [s for s in QgsApplication.colorSchemeRegistry().schemes() if isinstance(s, QgsProjectColorScheme)][0]
        scheme.setColors([])

        definition = QgsPropertyDefinition('test', 'test', QgsPropertyDefinition.ColorWithAlpha)
        button = QgsPropertyOverrideButton()
        button.init(0, QgsProperty(), definition)

        button.aboutToShowMenu()

        self.assertIn('Project Color', [a.text() for a in button.menu().actions()])
        self.assertIn('Color', [a.text() for a in button.menu().actions()])
        color_action = [a for a in button.menu().actions() if a.text() == 'Color'][0]
        self.assertEqual([a.text() for a in color_action.menu().actions()][0], 'No colors set')

        # add some project colors
        scheme.setColors([[QColor(255, 0, 0), 'color 1'], [QColor(255, 255, 0), 'burnt marigold']])

        button.aboutToShowMenu()
        self.assertIn('Project Color', [a.text() for a in button.menu().actions()])
        self.assertIn('Color', [a.text() for a in button.menu().actions()])
        color_action = [a for a in button.menu().actions() if a.text() == 'Color'][0]
        self.assertEqual([a.text() for a in color_action.menu().actions()], ['color 1', 'burnt marigold'])

        button.menuActionTriggered(color_action.menu().actions()[1])
        self.assertTrue(button.toProperty().isActive())
        self.assertEqual(button.toProperty().asExpression(), 'project_color(\'burnt marigold\')')

        button.menuActionTriggered(color_action.menu().actions()[0])
        self.assertTrue(button.toProperty().isActive())
        self.assertEqual(button.toProperty().asExpression(), 'project_color(\'color 1\')')

        button.setToProperty(QgsProperty.fromExpression('project_color(\'burnt marigold\')'))
        button.aboutToShowMenu()
        color_action = [a for a in button.menu().actions() if a.text() == 'Color'][0]
        self.assertTrue(color_action.isChecked())
        self.assertEqual([a.isChecked() for a in color_action.menu().actions()], [False, True])

        # should also see color menu for ColorNoAlpha properties
        definition = QgsPropertyDefinition('test', 'test', QgsPropertyDefinition.ColorNoAlpha)
        button = QgsPropertyOverrideButton()
        button.init(0, QgsProperty(), definition)

        button.aboutToShowMenu()
        self.assertIn('Project Color', [a.text() for a in button.menu().actions()])
        self.assertIn('Color', [a.text() for a in button.menu().actions()])

        # but no color menu for other types
        definition = QgsPropertyDefinition('test', 'test', QgsPropertyDefinition.Double)
        button = QgsPropertyOverrideButton()
        button.init(0, QgsProperty(), definition)

        button.aboutToShowMenu()
        self.assertNotIn('Project Color', [a.text() for a in button.menu().actions()])
        self.assertNotIn('Color', [a.text() for a in button.menu().actions()])

    def testLinkedColorButton(self):
        definition = QgsPropertyDefinition('test', 'test', QgsPropertyDefinition.ColorWithAlpha)
        button = QgsPropertyOverrideButton()
        button.init(0, QgsProperty(), definition)
        cb = QgsColorButton()
        button.registerLinkedWidget(cb)

        project_scheme = [s for s in QgsApplication.colorSchemeRegistry().schemes() if isinstance(s, QgsProjectColorScheme)][0]
        project_scheme.setColors([[QColor(255, 0, 0), 'col1'], [QColor(0, 255, 0), 'col2']])

        button.setToProperty(QgsProperty.fromValue('#ff0000'))
        self.assertTrue(cb.isEnabled())
        self.assertFalse(cb.linkedProjectColorName())
        button.setActive(False)
        self.assertTrue(cb.isEnabled())
        self.assertFalse(cb.linkedProjectColorName())
        button.setToProperty(QgsProperty.fromExpression('project_color(\'Cthulhu\'s delight\')'))
        self.assertTrue(cb.isEnabled())
        self.assertFalse(cb.linkedProjectColorName())
        button.setToProperty(QgsProperty.fromExpression('project_color(\'col1\')'))
        self.assertTrue(cb.isEnabled())
        self.assertEqual(cb.linkedProjectColorName(), 'col1')
        button.setActive(False)
        self.assertTrue(cb.isEnabled())
        self.assertFalse(cb.linkedProjectColorName())
        button.setActive(True)
        self.assertTrue(cb.isEnabled())
        self.assertEqual(cb.linkedProjectColorName(), 'col1')


if __name__ == '__main__':
    unittest.main()
