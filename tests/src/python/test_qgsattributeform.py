"""QGIS Unit tests for the attribute form


Run with ctest -V -R PyQgsAttributeForm

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = "Alessandro Pasotti"
__date__ = "2019-06-06"
__copyright__ = "Copyright 2019, The QGIS Project"

from qgis.PyQt.QtCore import QVariant
from qgis.core import (
    QgsDefaultValue,
    QgsEditFormConfig,
    QgsEditorWidgetSetup,
    QgsFeature,
    QgsField,
    QgsVectorLayer,
)
from qgis.gui import (
    QgsAttributeEditorContext,
    QgsAttributeForm,
    QgsFilterLineEdit,
    QgsGui,
    QgsMapCanvas,
)
import unittest
from qgis.testing import start_app, QgisTestCase

QGISAPP = start_app()


class TestQgsAttributeForm(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.mCanvas = QgsMapCanvas()
        QgsGui.editorWidgetRegistry().initEditors(cls.mCanvas)

    @classmethod
    def createLayerWithOnePoint(cls, field_type):
        layer = QgsVectorLayer(f"Point?field=fld:{field_type}", "vl", "memory")
        pr = layer.dataProvider()
        f = QgsFeature()
        assert pr.addFeatures([f])
        assert layer.featureCount() == 1
        return layer

    @classmethod
    def createFormWithDuplicateWidget(cls, vl, field_type, widget_type):
        """Creates a form with two identical widgets for the same field"""

        config = vl.editFormConfig()
        config.setLayout(QgsEditFormConfig.EditorLayout.TabLayout)
        element = config.tabs()[0]
        element2 = element.clone(element)
        config.addTab(element2)
        vl.setEditFormConfig(config)
        vl.setEditorWidgetSetup(0, QgsEditorWidgetSetup(widget_type, {}))
        form = QgsAttributeForm(vl, next(vl.getFeatures()))
        assert form.editable()
        return form

    @classmethod
    def get_widgets_for_field(cls, vl):
        """Get compatible widget names"""

        return [
            k
            for k, v in QgsGui.editorWidgetRegistry().factories().items()
            if v.supportsField(vl, 0)
        ]

    @classmethod
    def checkForm(cls, field_type, value):
        """Creates a vector layer and an associated form with two identical widgets for the same field and test it with NULL and after setting a value"""

        vl = cls.createLayerWithOnePoint(field_type)
        assert vl.startEditing()
        for widget_type in cls.get_widgets_for_field(vl):
            form = cls.createFormWithDuplicateWidget(vl, field_type, widget_type)
            vl.changeAttributeValue(1, 0, value)
            form.setFeature(next(vl.getFeatures()))

    def test_duplicated_widgets(self):
        """
        Note: this crashed two times for datetime (see GH #29937):

        - first crash with initial NULL values, because widget's clear() triggered changed()
        - second crash when setting a value, because setDateTime() triggered changed()

        There are no assertions in this test because we are looking for a crash.
        """

        field_types = {
            "integer": 123,
            "double": 123.45,
            "string": "lorem ipsum",
            "date": "2019-01-01",
            "time": "12:12:12",
            "datetime": "2019-01-01",
            "int2": 123,
            "int4": 123,
            "int8": 123,
            "numeric": 123.45,
            "decimal": 123.45,
            "real": 123.45,
            "double precision": 123.45,
            "text": "lorem ipsum",
            "bool": True,
            # 'binary'
        }

        for field_type, value in field_types.items():
            self.checkForm(field_type, value)

    def test_duplicated_widgets_multiedit(self):
        """
        Test multiedit with duplicated widgets
        """

        field_type = "integer"
        vl = self.createLayerWithOnePoint(field_type)

        # add another point
        pr = vl.dataProvider()
        f = QgsFeature()
        assert pr.addFeatures([f])
        assert vl.featureCount() == 2

        assert vl.startEditing()

        assert vl.changeAttributeValue(1, 0, 123)
        assert vl.changeAttributeValue(2, 0, 456)

        widget_type = "TextEdit"
        form = self.createFormWithDuplicateWidget(vl, field_type, widget_type)

        fids = list()
        for feature in vl.getFeatures():
            fids.append(feature.id())

        form.setMode(QgsAttributeEditorContext.Mode.MultiEditMode)
        form.setMultiEditFeatureIds(fids)

        for children in form.findChildren(QgsFilterLineEdit):
            if children.objectName() == "fld":
                # As the values are mixed, the widget values should be empty
                assert not children.text()

        # After save the values should be unchanged
        form.save()
        featuresIterator = vl.getFeatures()
        self.assertEqual(next(featuresIterator).attribute(0), 123)
        self.assertEqual(next(featuresIterator).attribute(0), 456)

    def test_on_update(self):
        """Test live update"""

        layer = QgsVectorLayer("Point?field=age:int", "vl", "memory")
        # set default value for numbers to [1, {age}], it will depend on the field age and should update
        field = QgsField("numbers", QVariant.List, "array")
        field.setEditorWidgetSetup(QgsEditorWidgetSetup("List", {}))
        layer.dataProvider().addAttributes([field])
        layer.updateFields()
        layer.setDefaultValueDefinition(1, QgsDefaultValue("array(1, age)", True))
        layer.setEditorWidgetSetup(1, QgsEditorWidgetSetup("List", {}))
        layer.startEditing()
        form = QgsAttributeForm(layer)
        feature = QgsFeature(layer.fields())
        form.setFeature(feature)
        form.setMode(QgsAttributeEditorContext.Mode.AddFeatureMode)
        form.changeAttribute("numbers", [12])
        form.changeAttribute("age", 1)
        self.assertEqual(form.currentFormFeature()["numbers"], [1, 1])
        form.changeAttribute("age", 7)
        self.assertEqual(form.currentFormFeature()["numbers"], [1, 7])

    def test_default_value_always_updated(self):
        """Test that default values are not updated on every edit operation
        when containing an 'attribute' expression"""

        layer = QgsVectorLayer("Point?field=age:int&field=number:int", "vl", "memory")

        layer.setEditorWidgetSetup(0, QgsEditorWidgetSetup("Range", {}))

        # set default value for numbers to attribute("age"), it will depend on the field age and should not update
        layer.setDefaultValueDefinition(
            1, QgsDefaultValue("attribute(@feature, 'age')", False)
        )
        layer.setEditorWidgetSetup(1, QgsEditorWidgetSetup("Range", {}))

        layer.startEditing()

        feature = QgsFeature(layer.fields())
        feature.setAttribute("age", 15)

        form = QgsAttributeForm(layer)
        form.setMode(QgsAttributeEditorContext.Mode.AddFeatureMode)
        form.setFeature(feature)

        QGISAPP.processEvents()

        self.assertEqual(form.currentFormFeature()["age"], 15)
        # not yet update it on init
        self.assertEqual(form.currentFormFeature()["number"], None)
        # return
        form.changeAttribute("number", 12)
        form.changeAttribute("age", 1)
        self.assertEqual(form.currentFormFeature()["number"], 12)
        form.changeAttribute("age", 7)
        self.assertEqual(form.currentFormFeature()["number"], 12)

    def test_default_value_always_updated_live_edit(self):
        """
        Live update on edit, when:
        - dependency changed (depending field and any field on expression with ALL_ATTRIBUTES-dependency)
        Update on save (after editing an existing feature), when:
        - it's any other function (with volatile elements like now, rand, randf, uuid)
        Update on save (after adding a feature):
        - Keeping the values, no update.

        We have the fields:
        - age (no expression)
        - year (no expression)
        - birthday (depending on year and age): updates live when one of those changes
        - pos (depending on age and having a volatile function): updates live when age changes and always after save
        - random (depending on nothing): updates always after save
        - evaluation - todo : eval('true')

        This means on edit:
        - changing age -> update live: number, birthday, pos / not update random and year
        - changing year -> update birthday, pos / not update random, number, age
        - changing birthday -> update nothing

        On save when edit mode:
        - update pos and random because of volatile functions
        """

        layer = QgsVectorLayer(
            "Point?field=age:int&field=year:int&field=birthday:int&field=pos:int&field=random:int",
            "vl",
            "memory",
        )

        # add another field numbers
        field = QgsField("numbers", QVariant.List, subType=QVariant.Int)
        field.setEditorWidgetSetup(QgsEditorWidgetSetup("List", {}))
        layer.dataProvider().addAttributes([field])
        layer.updateFields()

        apply_on_update = True

        layer.setEditorWidgetSetup(0, QgsEditorWidgetSetup("Range", {}))
        layer.setEditorWidgetSetup(1, QgsEditorWidgetSetup("Range", {}))

        # set default value for birthday (2), it will depend on the field age and year
        layer.setDefaultValueDefinition(
            2, QgsDefaultValue("year - age", apply_on_update)
        )
        layer.setEditorWidgetSetup(2, QgsEditorWidgetSetup("Range", {}))

        # set default value for pos (3), it will depend on the field age and it contains a volatile function and should update after save (on singleeditmode)
        layer.setDefaultValueDefinition(
            3,
            QgsDefaultValue(
                "pos + age + day_of_week( now() ) - day_of_week( now() )",
                apply_on_update,
            ),
        )
        layer.setEditorWidgetSetup(3, QgsEditorWidgetSetup("Range", {}))

        # set default value for random (4), it contains a volatile function and should update after save (on singleeditmode)
        layer.setDefaultValueDefinition(
            4, QgsDefaultValue("random + random + rand(0,0)", apply_on_update)
        )
        layer.setEditorWidgetSetup(4, QgsEditorWidgetSetup("Range", {}))

        # set default value for numbers (5), it will depend on the field age
        layer.setDefaultValueDefinition(
            5, QgsDefaultValue("array(1, age)", apply_on_update)
        )
        layer.setEditorWidgetSetup(5, QgsEditorWidgetSetup("List", {}))

        layer.startEditing()
        form = QgsAttributeForm(layer)
        feature = QgsFeature(layer.fields())
        feature.setAttribute("age", 15)
        feature.setAttribute("year", 2023)
        feature.setAttribute("random", 100)
        feature.setAttribute("birthday", 1900)
        feature.setAttribute("pos", 100)
        feature.setAttribute("numbers", [12])
        form.setFeature(feature)
        form.setMode(QgsAttributeEditorContext.Mode.AddFeatureMode)

        QGISAPP.processEvents()

        # editing age
        form.changeAttribute("age", 10)
        self.assertEqual(form.currentFormFeature()["age"], 10)

        # don't update year
        self.assertEqual(form.currentFormFeature()["year"], 2023)
        # update birthday because of age
        self.assertEqual(form.currentFormFeature()["birthday"], 2013)
        # update pos because of age
        self.assertEqual(form.currentFormFeature()["pos"], 110)
        # don't update random (yet)
        self.assertEqual(form.currentFormFeature()["random"], 100)
        # update number because of age
        self.assertEqual(form.currentFormFeature()["numbers"], [1, 10])

        # editing year
        form.changeAttribute("year", 2024)
        self.assertEqual(form.currentFormFeature()["year"], 2024)

        # don't update age
        self.assertEqual(form.currentFormFeature()["age"], 10)
        # update birthday because of year
        self.assertEqual(form.currentFormFeature()["birthday"], 2014)
        # don't update pos (yet)
        self.assertEqual(form.currentFormFeature()["pos"], 110)
        # don't update random (yet)
        self.assertEqual(form.currentFormFeature()["random"], 100)
        # don't update numbers
        self.assertEqual(form.currentFormFeature()["numbers"], [1, 10])

        # save form - this leads not to any updates (because it's a newly created features)
        form.save()

        # read the feature and check, that nothing updated after save
        feature = next(layer.getFeatures())

        # don't updated age
        self.assertEqual(feature.attribute("age"), 10)
        # don't updated year
        self.assertEqual(feature.attribute("year"), 2024)
        # don't updated birthday
        self.assertEqual(feature.attribute("birthday"), 2014)
        # don't updated pos (because newly created feature)
        self.assertEqual(feature.attribute("pos"), 110)
        # don't updated random (because newly created feature)
        self.assertEqual(feature.attribute("random"), 100)
        # don't updated numbers
        self.assertEqual(feature.attribute("numbers"), [1, 10])

        # changing mode of form
        form.setMode(QgsAttributeEditorContext.Mode.SingleEditMode)
        # and set the feature
        form.setFeature(feature)

        # check if nothing updated because of loading it:
        # don't update year
        self.assertEqual(form.currentFormFeature()["year"], 2024)
        # don't update age
        self.assertEqual(form.currentFormFeature()["age"], 10)
        # don't update birthday
        self.assertEqual(form.currentFormFeature()["birthday"], 2014)
        # don't update pos (because newly created feature)
        self.assertEqual(form.currentFormFeature()["pos"], 110)
        # don't update random (because newly created feature)
        self.assertEqual(form.currentFormFeature()["random"], 100)
        # don't update numbers
        self.assertEqual(form.currentFormFeature()["numbers"], [1, 10])

        # editing birthday
        form.changeAttribute("birthday", 2200)
        self.assertEqual(form.currentFormFeature()["birthday"], 2200)

        # don't update age
        self.assertEqual(form.currentFormFeature()["age"], 10)
        # don't update year
        self.assertEqual(form.currentFormFeature()["year"], 2024)
        # don't update pos
        self.assertEqual(form.currentFormFeature()["pos"], 110)
        # don't update random
        self.assertEqual(form.currentFormFeature()["random"], 100)

        # editing age
        form.changeAttribute("age", 41)
        self.assertEqual(form.currentFormFeature()["age"], 41)

        # don't update year
        self.assertEqual(form.currentFormFeature()["year"], 2024)
        # update birthday because of age
        self.assertEqual(form.currentFormFeature()["birthday"], 1983)
        # update pos because of age
        self.assertEqual(form.currentFormFeature()["pos"], 151)
        # don't update random (yet)
        self.assertEqual(form.currentFormFeature()["random"], 100)
        # update number because of age
        self.assertEqual(form.currentFormFeature()["numbers"], [1, 41])

        # save form - this leads to updates, because existing feature
        form.save()

        # read the feature and check, that other function expressions updated (pos and rand)
        feature = next(layer.getFeatures())

        # don't updated age
        self.assertEqual(feature.attribute("age"), 41)
        # don't updated year
        self.assertEqual(feature.attribute("year"), 2024)
        # don't updated birthday
        self.assertEqual(feature.attribute("birthday"), 1983)
        # again updated pos
        self.assertEqual(feature.attribute("pos"), 192)
        # updated random
        self.assertEqual(feature.attribute("random"), 200)
        # don't updated numbers
        self.assertEqual(feature.attribute("numbers"), [1, 41])


if __name__ == "__main__":
    unittest.main()
