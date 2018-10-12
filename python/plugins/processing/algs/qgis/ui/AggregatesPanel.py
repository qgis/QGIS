# -*- coding: utf-8 -*-

"""
***************************************************************************
    AggregatesPanel.py
    ---------------------
    Date                 : February 2017
    Copyright            : (C) 2017 by Arnaud Morvan
    Email                : arnaud dot morvan at camptocamp dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Arnaud Morvan'
__date__ = 'February 2017'
__copyright__ = '(C) 2017, Arnaud Morvan'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.PyQt.QtCore import (
    QCoreApplication,
    QVariant,
    Qt,
)
from qgis.PyQt.QtWidgets import (
    QComboBox,
    QStyledItemDelegate,
)

from qgis.core import QgsExpression

from processing.algs.qgis.ui.FieldsMappingPanel import (
    ExpressionDelegate,
    FieldsMappingModel,
    FieldsMappingPanel,
    FieldsMappingWidgetWrapper,
    FieldTypeDelegate,
)

AGGREGATES = dict()
AGGREGATES['first_value'] = QCoreApplication.translate('aggregation', 'Returns the value from the first feature')
for function in QgsExpression.Functions():
    if function.name()[0] == '_':
        continue
    if function.isDeprecated():
        continue
    # if ( func->isContextual() ):
    if "Aggregates" in function.groups():
        if function.name() in ('aggregate',
                               'relation_aggregate'):
            continue
        AGGREGATES[function.name()] = function.helpText()


class AggregatesModel(FieldsMappingModel):

    def configure(self):
        self.columns = [{
            'name': 'input',
            'type': QgsExpression,
            'header': self.tr("Input expression"),
            'persistentEditor': True
        }, {
            'name': 'aggregate',
            'type': QVariant.String,
            'header': self.tr("Aggregate function"),
            'persistentEditor': True
        }, {
            'name': 'delimiter',
            'type': QVariant.String,
            'header': self.tr("Delimiter")
        }, {
            'name': 'name',
            'type': QVariant.String,
            'header': self.tr("Output field name")
        }, {
            'name': 'type',
            'type': QVariant.Type,
            'header': self.tr("Type"),
            'persistentEditor': True
        }, {
            'name': 'length',
            'type': QVariant.Int,
            'header': self.tr("Length")
        }, {
            'name': 'precision',
            'type': QVariant.Int,
            'header': self.tr("Precision")
        }]

    def newField(self, field=None):
        if field is None:
            return {
                'input': '',
                'aggregate': '',
                'delimiter': '',
                'name': '',
                'type': QVariant.Invalid,
                'length': 0,
                'precision': 0,
            }

        default_aggregate = ''
        if field.type() in (QVariant.Int,
                            QVariant.Double,
                            QVariant.LongLong):
            default_aggregate = 'sum'
        if field.type() == QVariant.DateTime:
            default_aggregate = ''
        if field.type() == QVariant.String:
            default_aggregate = 'concatenate'

        return {
            'input': QgsExpression.quotedColumnRef(field.name()),
            'aggregate': default_aggregate,
            'delimiter': ',',
            'name': field.name(),
            'type': field.type(),
            'length': field.length(),
            'precision': field.precision(),
        }


class AggregateDelegate(QStyledItemDelegate):

    def __init__(self, parent=None):
        super(AggregateDelegate, self).__init__(parent)

    def createEditor(self, parent, option, index):
        editor = QComboBox(parent)
        for i, aggregate in enumerate(sorted(AGGREGATES.keys())):
            editor.insertItem(i, aggregate, aggregate)
            editor.setItemData(i, AGGREGATES[aggregate], Qt.ToolTipRole)
        return editor

    def setEditorData(self, editor, index):
        if not editor:
            return
        value = index.model().data(index, Qt.EditRole)
        editor.setCurrentIndex(editor.findData(value))

    def setModelData(self, editor, model, index):
        if not editor:
            return
        value = editor.currentData()
        if value is None:
            value = QVariant.Invalid
        model.setData(index, value)


class AggregatesPanel(FieldsMappingPanel):

    def configure(self):
        self.model = AggregatesModel()
        self.fieldsView.setModel(self.model)
        self.model.rowsInserted.connect(self.on_model_rowsInserted)

        self.setDelegate('input', ExpressionDelegate(self))
        self.setDelegate('aggregate', AggregateDelegate(self))
        self.setDelegate('type', FieldTypeDelegate(self))


class AggregatesWidgetWrapper(FieldsMappingWidgetWrapper):

    def createPanel(self, parentLayerParameterName='INPUT'):
        self._parentLayerParameter = parentLayerParameterName
        return AggregatesPanel()
