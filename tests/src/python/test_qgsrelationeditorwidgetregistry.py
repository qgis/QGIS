"""QGIS Unit tests for edit widgets.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Matthias Kuhn"
__date__ = "28/11/2015"
__copyright__ = "Copyright 2015, The QGIS Project"

from qgis.PyQt.QtWidgets import (
    QCheckBox,
    QGridLayout,
    QLabel,
)
from qgis.core import QgsRelation
from qgis.gui import (
    QgsAbstractRelationEditorConfigWidget,
    QgsAbstractRelationEditorWidget,
    QgsAbstractRelationEditorWidgetFactory,
    QgsGui,
    QgsRelationEditorConfigWidget,
    QgsRelationEditorWidget,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsRelationEditorWidgetRegistry(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """
        Setup the involved layers and relations for a n:m relation
        :return:
        """
        super().setUpClass()
        cls.registry = QgsGui.relationWidgetRegistry()

    def test_cannot_delete_relation_editor(self):
        count_before = len(self.registry.relationWidgetNames())
        self.registry.removeRelationWidget("relation_editor")
        count_after = len(self.registry.relationWidgetNames())

        self.assertEqual(count_before, count_after)
        self.assertIsInstance(
            self.registry.create("relation_editor", {}), QgsRelationEditorWidget
        )
        self.assertIsInstance(
            self.registry.createConfigWidget("relation_editor", QgsRelation()),
            QgsRelationEditorConfigWidget,
        )

    def test_returns_none_when_unknown_widget_id(self):
        self.assertIsNone(self.registry.create("babinatatitrunkina", {}))
        self.assertIsNone(
            self.registry.createConfigWidget("babinatatitrunkina", QgsRelation())
        )

    def test_creates_new_widget(self):
        # define the widget
        class QgsExampleRelationEditorWidget(QgsAbstractRelationEditorWidget):

            def __init__(self, config, parent):
                super().__init__(config, parent)

                self.setLayout(QGridLayout())
                self.label = QLabel()
                self.label.setText(
                    f'According to the configuration, the checkboxin state was {str(config.get("checkboxin", "Unknown"))}'
                )
                self.layout().addWidget(self.label)

            def config(self):
                return {}

            def setConfig(self, config):
                self.label.setText(
                    f'According to the configuration, the checkboxin state was {str(config.get("checkboxin", "Unknown"))}'
                )

        # define the config widget
        class QgsExampleRelationEditorConfigWidget(
            QgsAbstractRelationEditorConfigWidget
        ):

            def __init__(self, relation, parent):
                super().__init__(relation, parent)

                self.setLayout(QGridLayout())
                self.checkbox = QCheckBox("Is this checkbox checkboxin?")
                self.layout().addWidget(self.checkbox)

            def config(self):
                return {"checkboxin": self.checkbox.isChecked()}

            def setConfig(self, config):
                self.checkbox.setChecked(config.get("checkboxin", False))

        # define the widget factory
        class QgsExampleRelationEditorWidgetFactory(
            QgsAbstractRelationEditorWidgetFactory
        ):
            def type(self):
                return "example"

            def name(self):
                return "Example Relation Widget"

            def create(self, config, parent):
                return QgsExampleRelationEditorWidget(config, parent)

            def configWidget(self, relation, parent):
                return QgsExampleRelationEditorConfigWidget(relation, parent)

        self.assertIsNone(self.registry.create("example", {}))
        self.assertIsNone(self.registry.createConfigWidget("example", QgsRelation()))

        self.registry.addRelationWidget(QgsExampleRelationEditorWidgetFactory())

        self.assertIsInstance(
            self.registry.create("example", {}), QgsExampleRelationEditorWidget
        )
        self.assertIsInstance(
            self.registry.createConfigWidget("example", QgsRelation()),
            QgsExampleRelationEditorConfigWidget,
        )


if __name__ == "__main__":
    unittest.main()
