"""
***************************************************************************
    FieldPyculator.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Victor Olaya & NextGIS"
__date__ = "August 2012"
__copyright__ = "(C) 2012, Victor Olaya & NextGIS"

import sys

from qgis.PyQt.QtCore import QMetaType
from qgis.core import (
    Qgis,
    QgsProcessingException,
    QgsField,
    QgsFields,
    QgsFeatureSink,
    QgsProcessing,
    QgsProcessingParameterFeatureSource,
    QgsProcessingParameterString,
    QgsProcessingParameterEnum,
    QgsProcessingParameterNumber,
    QgsProcessingParameterFeatureSink,
    QgsVariantUtils,
)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class FieldsPyculator(QgisAlgorithm):
    INPUT = "INPUT"
    FIELD_NAME = "FIELD_NAME"
    FIELD_TYPE = "FIELD_TYPE"
    FIELD_LENGTH = "FIELD_LENGTH"
    FIELD_PRECISION = "FIELD_PRECISION"
    GLOBAL = "GLOBAL"
    FORMULA = "FORMULA"
    OUTPUT = "OUTPUT"
    RESULT_VAR_NAME = "value"

    def flags(self):
        # This algorithm represents a security risk, due to the use
        # of the Python "exec" function
        return super().flags() | Qgis.ProcessingAlgorithmFlag.SecurityRisk

    def group(self):
        return self.tr("Vector table")

    def groupId(self):
        return "vectortable"

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(
            QgsProcessingParameterFeatureSource(
                self.INPUT,
                self.tr("Input layer"),
                types=[QgsProcessing.SourceType.TypeVector],
            )
        )
        self.addParameter(
            QgsProcessingParameterString(
                self.FIELD_NAME, self.tr("Result field name"), defaultValue="NewField"
            )
        )

        types = [
            (QMetaType.Type.Int, QMetaType.Type.UnknownType),
            (QMetaType.Type.Double, QMetaType.Type.UnknownType),
            (QMetaType.Type.QString, QMetaType.Type.UnknownType),
            (QMetaType.Type.Bool, QMetaType.Type.UnknownType),
            (QMetaType.Type.QDate, QMetaType.Type.UnknownType),
            (QMetaType.Type.QTime, QMetaType.Type.UnknownType),
            (QMetaType.Type.QDateTime, QMetaType.Type.UnknownType),
            (QMetaType.Type.QByteArray, QMetaType.Type.UnknownType),
            (QMetaType.Type.QStringList, QMetaType.Type.QString),
            (QMetaType.Type.QVariantList, QMetaType.Type.Int),
            (QMetaType.Type.QVariantList, QMetaType.Type.Double),
        ]
        type_names = []
        type_icons = []
        for type_name, subtype_name in types:
            type_names.append(
                QgsVariantUtils.typeToDisplayString(type_name, subtype_name)
            )
            type_icons.append(QgsFields.iconForFieldType(type_name, subtype_name))
        param = QgsProcessingParameterEnum(
            "FIELD_TYPE", "Field type", options=type_names
        )
        param.setMetadata({"widget_wrapper": {"icons": type_icons}})
        self.addParameter(param)

        self.addParameter(
            QgsProcessingParameterNumber(
                self.FIELD_LENGTH, self.tr("Field length"), minValue=0, defaultValue=10
            )
        )
        self.addParameter(
            QgsProcessingParameterNumber(
                self.FIELD_PRECISION,
                self.tr("Field precision"),
                minValue=0,
                maxValue=15,
                defaultValue=3,
            )
        )
        self.addParameter(
            QgsProcessingParameterString(
                self.GLOBAL, self.tr("Global expression"), multiLine=True, optional=True
            )
        )
        self.addParameter(
            QgsProcessingParameterString(
                self.FORMULA,
                self.tr("Formula"),
                defaultValue="value = ",
                multiLine=True,
            )
        )
        self.addParameter(
            QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr("Calculated"))
        )

    def name(self):
        return "advancedpythonfieldcalculator"

    def displayName(self):
        return self.tr("Advanced Python field calculator")

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(
                self.invalidSourceError(parameters, self.INPUT)
            )

        field_name = self.parameterAsString(parameters, self.FIELD_NAME, context)

        field_type = QMetaType.Type.UnknownType
        field_sub_type = QMetaType.Type.UnknownType
        field_type_parameter = self.parameterAsEnum(
            parameters, self.FIELD_TYPE, context
        )
        if field_type_parameter == 0:  # Integer
            field_type = QMetaType.Type.Int
        elif field_type_parameter == 1:  # Float
            field_type = QMetaType.Type.Double
        elif field_type_parameter == 2:  # String
            field_type = QMetaType.Type.QString
        elif field_type_parameter == 3:  # Boolean
            field_type = QMetaType.Type.Bool
        elif field_type_parameter == 4:  # Date
            field_type = QMetaType.Type.QDate
        elif field_type_parameter == 5:  # Time
            field_type = QMetaType.Type.QTime
        elif field_type_parameter == 6:  # DateTime
            field_type = QMetaType.Type.QDateTime
        elif field_type_parameter == 7:  # Binary
            field_type = QMetaType.Type.QByteArray
        elif field_type_parameter == 8:  # StringList
            field_type = QMetaType.Type.QStringList
            field_sub_type = QMetaType.Type.QString
        elif field_type_parameter == 9:  # IntegerList
            field_type = QMetaType.Type.QVariantList
            field_sub_type = QMetaType.Type.Int
        elif field_type_parameter == 10:  # DoubleList
            field_type = QMetaType.Type.QVariantList
            field_sub_type = QMetaType.Type.Double

        width = self.parameterAsInt(parameters, self.FIELD_LENGTH, context)
        precision = self.parameterAsInt(parameters, self.FIELD_PRECISION, context)
        code = self.parameterAsString(parameters, self.FORMULA, context)
        globalExpression = self.parameterAsString(parameters, self.GLOBAL, context)

        fields = source.fields()
        field = QgsField(
            field_name, field_type, "", width, precision, "", field_sub_type
        )
        fields.append(field)
        new_ns = {}

        (sink, dest_id) = self.parameterAsSink(
            parameters,
            self.OUTPUT,
            context,
            fields,
            source.wkbType(),
            source.sourceCrs(),
        )
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        # Run global code
        if globalExpression.strip() != "":
            try:
                bytecode = compile(globalExpression, "<string>", "exec")
                exec(bytecode, new_ns)
            except:
                raise QgsProcessingException(
                    self.tr(
                        "FieldPyculator code execute error. Global code block can't be executed!\n{0}\n{1}"
                    ).format(str(sys.exc_info()[0].__name__), str(sys.exc_info()[1]))
                )

        # Replace all fields tags
        fields = source.fields()
        for num, field in enumerate(fields):
            field_name = str(field.name())
            replval = "__attr[" + str(num) + "]"
            code = code.replace("<" + field_name + ">", replval)
        # Replace all special vars
        code = code.replace("$id", "__id")
        code = code.replace("$geom", "__geom")
        need_id = code.find("__id") != -1
        need_geom = code.find("__geom") != -1
        need_attrs = code.find("__attr") != -1

        # Compile
        try:
            bytecode = compile(code, "<string>", "exec")
        except:
            raise QgsProcessingException(
                self.tr(
                    "FieldPyculator code execute error. Field code block can't be executed!\n{0}\n{1}"
                ).format(str(sys.exc_info()[0].__name__), str(sys.exc_info()[1]))
            )

        # Run
        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0

        for current, feat in enumerate(features):
            if feedback.isCanceled():
                break

            feedback.setProgress(int(current * total))
            attrs = feat.attributes()
            feat_id = feat.id()

            # Add needed vars
            if need_id:
                new_ns["__id"] = feat_id

            if need_geom:
                geom = feat.geometry()
                new_ns["__geom"] = geom

            if need_attrs:
                pyattrs = [a for a in attrs]
                new_ns["__attr"] = pyattrs

            # Clear old result
            if self.RESULT_VAR_NAME in new_ns:
                del new_ns[self.RESULT_VAR_NAME]

            # Exec
            exec(bytecode, new_ns)

            # Check result
            if self.RESULT_VAR_NAME not in new_ns:
                raise QgsProcessingException(
                    self.tr(
                        "FieldPyculator code execute error\n"
                        "Field code block does not return '{0}' variable! "
                        "Please declare this variable in your code!"
                    ).format(self.RESULT_VAR_NAME)
                )

            # Write feature
            attrs.append(new_ns[self.RESULT_VAR_NAME])
            feat.setAttributes(attrs)
            sink.addFeature(feat, QgsFeatureSink.Flag.FastInsert)

        sink.finalize()
        return {self.OUTPUT: dest_id}

    def checkParameterValues(self, parameters, context):
        # TODO check that formula is correct and fields exist
        return super().checkParameterValues(parameters, context)
