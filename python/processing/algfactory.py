# -*- coding: utf-8 -*-

"""
***************************************************************************
    algfactory.py
    ---------------------
    Date                 : November 2018
    Copyright            : (C) 2018 by Nathan Woodrow
    Email                : woodrow dot nathan at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nathan Woodrow'
__date__ = 'November 2018'
__copyright__ = '(C) 2018, Nathan Woodrow'

from collections import OrderedDict
from functools import partial

from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtGui import QIcon
from qgis.core import (QgsProcessingParameterDefinition,
                       QgsProcessingAlgorithm,
                       QgsProcessingParameterString,
                       QgsProcessingParameterAuthConfig,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterDistance,
                       QgsProcessingParameterDuration,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterFileDestination,
                       QgsProcessingParameterFolderDestination,
                       QgsProcessingParameterRasterDestination,
                       QgsProcessingParameterVectorDestination,
                       QgsProcessingParameterPointCloudDestination,
                       QgsProcessingParameterBand,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterExpression,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterField,
                       QgsProcessingParameterFile,
                       QgsProcessingParameterMapLayer,
                       QgsProcessingParameterMatrix,
                       QgsProcessingParameterMultipleLayers,
                       QgsProcessingParameterPoint,
                       QgsProcessingParameterGeometry,
                       QgsProcessingParameterRange,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterVectorLayer,
                       QgsProcessingParameterMeshLayer,
                       QgsProcessingParameterColor,
                       QgsProcessingParameterScale,
                       QgsProcessingParameterLayout,
                       QgsProcessingParameterLayoutItem,
                       QgsProcessingParameterDateTime,
                       QgsProcessingParameterMapTheme,
                       QgsProcessingParameterProviderConnection,
                       QgsProcessingParameterDatabaseSchema,
                       QgsProcessingParameterDatabaseTable,
                       QgsProcessingParameterCoordinateOperation,
                       QgsProcessingParameterPointCloudLayer,
                       QgsProcessingParameterAnnotationLayer,
                       QgsProcessingOutputString,
                       QgsProcessingOutputBoolean,
                       QgsProcessingOutputFile,
                       QgsProcessingOutputFolder,
                       QgsProcessingOutputHtml,
                       QgsProcessingOutputLayerDefinition,
                       QgsProcessingOutputMapLayer,
                       QgsProcessingOutputMultipleLayers,
                       QgsProcessingOutputNumber,
                       QgsProcessingOutputRasterLayer,
                       QgsProcessingOutputVectorLayer,
                       QgsProcessingOutputPointCloudLayer,
                       QgsMessageLog,
                       QgsApplication)


def _log(*args, **kw):
    """
    Log messages to the QgsMessageLog viewer
    """
    QgsMessageLog.logMessage(" ".join(map(str, args)), "Factory")


def _make_output(**args):
    """
    Create a processing output class type.
    :param args: 'cls' The class object type.
                 'name' the name of the output
                 'description' The description used on the output
    :return:
    """
    cls = args['cls']
    del args['cls']
    newargs = {
        "name": args['name'],
        "description": args['description'],
    }
    return cls(**newargs)


class ProcessingAlgFactoryException(Exception):
    """
    Exception raised when using @alg on a function.
    """

    def __init__(self, message):
        super(ProcessingAlgFactoryException, self).__init__(message)


class AlgWrapper(QgsProcessingAlgorithm):
    """
    Wrapper object used to create new processing algorithms from @alg.
    """

    def __init__(self, name=None, display=None,
                 group=None, group_id=None, inputs=None,
                 outputs=None, func=None, help=None, icon=None):
        super(AlgWrapper, self).__init__()
        self._inputs = OrderedDict(inputs or {})
        self._outputs = OrderedDict(outputs or {})
        self._icon = icon
        self._name = name
        self._group = group
        self._group_id = group_id
        self._display = display
        self._func = func
        self._help = help

    def _get_parent_id(self, parent):
        """
        Return the id of the parent object.
        """
        if isinstance(parent, str):
            return parent
        else:
            raise NotImplementedError()

    # Wrapper logic
    def define(self, name, label, group, group_label, help=None, icon=QgsApplication.iconPath("processingScript.svg")):
        self._name = name
        self._display = label
        self._group = group_label
        self._group_id = group
        self._help = help
        self._icon = icon

    def end(self):
        """
        Finalize the wrapper logic and check for any invalid config.
        """
        if not self.has_outputs:
            raise ProcessingAlgFactoryException("No outputs defined for '{}' alg"
                                                "At least one must be defined. Use @alg.output")

    def add_output(self, type, **kwargs):
        parm = self._create_param(type, output=True, **kwargs)
        self._outputs[parm.name()] = parm

    def add_help(self, helpstring, *args, **kwargs):
        self._help = helpstring

    def add_input(self, type, **kwargs):
        parm = self._create_param(type, **kwargs)
        self._inputs[parm.name()] = parm

    @property
    def inputs(self):
        return self._inputs

    @property
    def outputs(self):
        return self._outputs

    def _create_param(self, type, output=False, **kwargs):
        name = kwargs['name']
        if name in self._inputs or name in self._outputs:
            raise ProcessingAlgFactoryException("{} already defined".format(name))

        parent = kwargs.get("parent")
        if parent:
            parentname = self._get_parent_id(parent)
            if parentname == name:
                raise ProcessingAlgFactoryException("{} can't depend on itself. "
                                                    "We know QGIS is smart but it's not that smart".format(name))
            if parentname not in self._inputs and parentname not in self._outputs:
                raise ProcessingAlgFactoryException("Can't find parent named {}".format(parentname))

        kwargs['description'] = kwargs.pop("label", "")
        kwargs['defaultValue'] = kwargs.pop("default", None)
        advanced = kwargs.pop("advanced", False)
        help_str = kwargs.pop("help", "")
        try:
            if output:
                try:
                    make_func = output_type_mapping[type]
                except KeyError:
                    raise ProcessingAlgFactoryException("{} is a invalid output type".format(type))
            else:
                try:
                    make_func = input_type_mapping[type]
                except KeyError:
                    raise ProcessingAlgFactoryException("{} is a invalid input type".format(type))
            parm = make_func(**kwargs)
            if advanced:
                parm.setFlags(parm.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
            if not output:
                parm.setHelp(help_str)
            return parm
        except KeyError as ex:
            raise NotImplementedError("{} not supported".format(str(type)))

    def set_func(self, func):
        self._func = func
        # Only take the help from the function if it hasn't already been set.
        if self._func and not self._help:
            self._help = self._func.__doc__.strip()

    @property
    def has_outputs(self):
        """
        True if this alg wrapper has outputs defined.
        """
        dests = [p for p in self._inputs.values() if p.isDestination()]
        return bool(self._outputs) or bool(dests)

    @property
    def has_inputs(self):
        """
        True if this alg wrapper has outputs defined.
        """
        return bool(self._inputs)

    def _get_parameter_value(self, parm, parameters, name, context):
        """
        Extract the real value from the parameter.
        """
        if isinstance(parm, (QgsProcessingParameterString, QgsProcessingParameterAuthConfig)):
            value = self.parameterAsString(parameters, name, context)
            return value
        elif isinstance(parm, QgsProcessingParameterNumber):
            if parm.dataType() == QgsProcessingParameterNumber.Integer:
                value = self.parameterAsInt(parameters, name, context)
                return value
            if parm.dataType() == QgsProcessingParameterNumber.Double:
                value = self.parameterAsDouble(parameters, name, context)
                return value

    # Overloads
    def name(self):
        return self._name

    def displayName(self):
        return self._display

    def group(self):
        return self._group

    def groupId(self):
        return self._group_id

    def processAlgorithm(self, parameters, context, feedback):
        values = {}
        for parm in self._inputs.values():
            name = parm.name()
            values[name] = self._get_parameter_value(parm, parameters, name, context)

        output = self._func(self, parameters, context, feedback, values)
        if output is None:
            return {}
        return output

    def createInstance(self):
        return AlgWrapper(self._name, self._display,
                          self._group, self._group_id,
                          inputs=self._inputs,
                          outputs=self._outputs,
                          func=self._func,
                          help=self._help,
                          icon=self._icon)

    def initAlgorithm(self, configuration=None, p_str=None, Any=None, *args, **kwargs):
        for parm in self._inputs.values():
            self.addParameter(parm.clone())

        for parm in self._outputs.values():
            clsname = parm.__class__.__name__
            klass = globals()[clsname]
            clone = klass(parm.name(), parm.description())
            self.addOutput(clone)

    def shortHelpString(self):
        return self._help

    def icon(self):
        return QIcon(self._icon)


class ProcessingAlgFactory():
    STRING = "STRING",
    INT = "INT",
    NUMBER = "NUMBER",
    DISTANCE = "DISTANCE",
    SINK = "SINK"
    SOURCE = "SOURCE"
    FILE = "FILE",
    FOLDER = "FOLDER",
    HTML = "HTML",
    LAYERDEF = "LAYERDEF",
    MAPLAYER = "MAPLAYER",
    MULTILAYER = "MULTILAYER",
    RASTER_LAYER = "RASTER_LAYER",
    VECTOR_LAYER = "VECTOR_LAYER",
    MESH_LAYER = "MESH_LAYER",
    POINT_CLOUD_LAYER = "POINT_CLOUD_LAYER",
    FILE_DEST = "FILE_DEST",
    FOLDER_DEST = "FOLDER_DEST",
    RASTER_LAYER_DEST = "RASTER_LAYER_DEST",
    VECTOR_LAYER_DEST = "VECTOR_LAYER_DEST",
    POINTCLOUD_LAYER_DEST = "POINTCLOUD_LAYER_DEST",
    BAND = "BAND",
    BOOL = "BOOL",
    CRS = "CRS",
    ENUM = "ENUM",
    EXPRESSION = "EXPRESSION",
    EXTENT = "EXTENT",
    FIELD = "FIELD",
    MATRIX = "MATRIX",
    POINT = "POINT",
    GEOMETRY = "GEOMETRY",
    RANGE = "RANGE",
    AUTH_CFG = "AUTH_CFG"
    SCALE = "SCALE"
    COLOR = "COLOR"
    LAYOUT = "LAYOUT"
    LAYOUT_ITEM = "LAYOUT_ITEM"
    DATETIME = "DATETIME"
    MAP_THEME = "MAP_THEME"
    PROVIDER_CONNECTION = "PROVIDER_CONNECTION"
    DATABASE_SCHEMA = "DATABASE_SCHEMA"
    DATABASE_TABLE = "DATABASE_TABLE"
    COORDINATE_OPERATION = "COORDINATE_OPERATION"
    POINTCLOUD_LAYER = "POINTCLOUD_LAYER"
    ANNOTATION_LAYER = "ANNOTATION_LAYER"

    def __init__(self):
        self._current = None
        self.instances = []

    def tr(self, string):
        """
        Returns a translatable string with the self.tr() function.
        """
        return QCoreApplication.translate('Processing', string)

    @property
    def current(self):
        return self._current

    @property
    def current_defined(self):
        return self._current is not None

    def __call__(self, *args, **kwargs):
        return self._define(*args, **kwargs)

    def _initnew(self):
        self._current = AlgWrapper()

    def _pop(self):
        self.instances.append(self.current)
        self._current = None

    def _define(self, *args, **kwargs):
        self._initnew()
        self.current.define(*args, **kwargs)

        def dec(f):
            self.current.set_func(f)
            self.current.end()
            self._pop()
            return f

        return dec

    def output(self, type, *args, **kwargs):
        """
        Define a output parameter for this algorithm using @alg.output.
        Apart from `type` this method will take all arguments and pass them though to the correct `QgsProcessingOutputDefinition ` type.

        Types:
            str: QgsProcessingOutputString
            int: QgsProcessingOutputNumber
            float: QgsProcessingOutputNumber
            alg.NUMBER: QgsProcessingOutputNumber
            alg.DISTANCE: QgsProcessingOutputNumber
            alg.INT: QgsProcessingOutputNumber
            alg.STRING: QgsProcessingOutputString
            alg.FILE: QgsProcessingOutputFile
            alg.FOLDER: QgsProcessingOutputFolder
            alg.HTML: QgsProcessingOutputHtml
            alg.LAYERDEF:  QgsProcessingOutputLayerDefinition
            alg.MAPLAYER:  QgsProcessingOutputMapLayer
            alg.MULTILAYER:  QgsProcessingOutputMultipleLayers
            alg.RASTER_LAYER: QgsProcessingOutputRasterLayer
            alg.VECTOR_LAYER: QgsProcessingOutputVectorLayer
            alg.POINTCLOUD_LAYER: QgsProcessingOutputPointCloudLayer
            alg.BOOL: QgsProcessingOutputBoolean

        :param type: The type of the input. This should be a type define on `alg` like alg.STRING, alg.DISTANCE
        :keyword label: The label of the output. Will convert into `description` arg.
        :keyword parent: The string ID of the parent parameter. Parent parameter must be defined before its here.
        """

        def dec(f):
            return f

        self.current.add_output(type, *args, **kwargs)
        return dec

    def help(self, helpstring, *args, **kwargs):
        """
        Define the help for the algorithm using @alg.help. This method will
        be used instead of the doc string on the function as the help in the processing dialogs.

        :param helpstring: The help text. Use alg.tr() for translation support.
        """

        def dec(f):
            return f

        self.current.add_help(helpstring, *args, **kwargs)

        return dec

    def input(self, type, *args, **kwargs):
        """
        Define a input parameter for this algorithm using @alg.input.
        Apart from `type` this method will take all arguments and pass them though to the correct `QgsProcessingParameterDefinition` type.

        Types:

            str: QgsProcessingParameterString
            int: QgsProcessingParameterNumber
            float: QgsProcessingParameterNumber
            bool: QgsProcessingParameterBoolean
            alg.NUMBER: QgsProcessingParameterNumber
            alg.INT: QgsProcessingParameterNumber
            alg.STRING: QgsProcessingParameterString
            alg.DISTANCE: QgsProcessingParameterDistance
            alg.SINK: QgsProcessingParameterFeatureSink
            alg.SOURCE: QgsProcessingParameterFeatureSource
            alg.FILE_DEST: QgsProcessingParameterFileDestination
            alg.FOLDER_DEST: QgsProcessingParameterFolderDestination
            alg.RASTER_LAYER: QgsProcessingParameterRasterLayer
            alg.RASTER_LAYER_DEST: QgsProcessingParameterRasterDestination
            alg.VECTOR_LAYER_DEST: QgsProcessingParameterVectorDestination
            alg.POINTCLOUD_LAYER_DEST: QgsProcessingParameterPointCloudDestination
            alg.BAND: QgsProcessingParameterBand
            alg.BOOL: QgsProcessingParameterBoolean
            alg.CRS: QgsProcessingParameterCrs
            alg.ENUM: QgsProcessingParameterEnum
            alg.EXPRESSION: QgsProcessingParameterExpression
            alg.EXTENT: QgsProcessingParameterExtent
            alg.FIELD: QgsProcessingParameterField
            alg.FILE: QgsProcessingParameterFile
            alg.MAPLAYER: QgsProcessingParameterMapLayer
            alg.MATRIX: QgsProcessingParameterMatrix
            alg.MULTILAYER: QgsProcessingParameterMultipleLayers
            alg.POINT: QgsProcessingParameterPoint
            alg.GEOMETRY: QgsProcessingParameterGeometry
            alg.RANGE: QgsProcessingParameterRange
            alg.VECTOR_LAYER: QgsProcessingParameterVectorLayer
            alg.AUTH_CFG: QgsProcessingParameterAuthConfig
            alg.MESH_LAYER: QgsProcessingParameterMeshLayer
            alg.SCALE: QgsProcessingParameterScale
            alg.LAYOUT: QgsProcessingParameterLayout
            alg.LAYOUT_ITEM: QgsProcessingParameterLayoutItem
            alg.COLOR: QgsProcessingParameterColor
            alg.DATETIME: QgsProcessingParameterDateTime
            alg.MAP_THEME: QgsProcessingParameterMapTheme
            alg.PROVIDER_CONNECTION: QgsProcessingParameterProviderConnection
            alg.DATABASE_SCHEMA: QgsProcessingParameterDatabaseSchema
            alg.DATABASE_TABLE: QgsProcessingParameterDatabaseTable
            alg.COORDINATE_OPERATION: QgsProcessingParameterCoordinateOperation
            alg.POINTCLOUD_LAYER: QgsProcessingParameterPointCloudLayer
            alg.ANNOTATION_LAYER: QgsProcessingParameterAnnotationLayer

        :param type: The type of the input. This should be a type define on `alg` like alg.STRING, alg.DISTANCE
        :keyword label: The label of the output. Translates into `description` arg.
        :keyword parent: The string ID of the parent parameter. Parent parameter must be defined before its here.
        :keyword default: The default value set for that parameter. Translates into `defaultValue` arg.
        """

        def dec(f):
            return f

        self.current.add_input(type, *args, **kwargs)

        return dec


input_type_mapping = {
    str: QgsProcessingParameterString,
    int: partial(QgsProcessingParameterNumber, type=QgsProcessingParameterNumber.Integer),
    float: partial(QgsProcessingParameterNumber, type=QgsProcessingParameterNumber.Double),
    bool: QgsProcessingParameterBoolean,
    ProcessingAlgFactory.NUMBER: partial(QgsProcessingParameterNumber, type=QgsProcessingParameterNumber.Double),
    ProcessingAlgFactory.INT: partial(QgsProcessingParameterNumber, type=QgsProcessingParameterNumber.Integer),
    ProcessingAlgFactory.STRING: QgsProcessingParameterString,
    ProcessingAlgFactory.DISTANCE: QgsProcessingParameterDistance,
    ProcessingAlgFactory.SINK: QgsProcessingParameterFeatureSink,
    ProcessingAlgFactory.SOURCE: QgsProcessingParameterFeatureSource,
    ProcessingAlgFactory.FILE_DEST: QgsProcessingParameterFileDestination,
    ProcessingAlgFactory.FOLDER_DEST: QgsProcessingParameterFolderDestination,
    ProcessingAlgFactory.RASTER_LAYER: QgsProcessingParameterRasterLayer,
    ProcessingAlgFactory.RASTER_LAYER_DEST: QgsProcessingParameterRasterDestination,
    ProcessingAlgFactory.VECTOR_LAYER_DEST: QgsProcessingParameterVectorDestination,
    ProcessingAlgFactory.POINTCLOUD_LAYER_DEST: QgsProcessingParameterPointCloudDestination,
    ProcessingAlgFactory.BAND: QgsProcessingParameterBand,
    ProcessingAlgFactory.BOOL: QgsProcessingParameterBoolean,
    ProcessingAlgFactory.CRS: QgsProcessingParameterCrs,
    ProcessingAlgFactory.ENUM: QgsProcessingParameterEnum,
    ProcessingAlgFactory.EXPRESSION: QgsProcessingParameterExpression,
    ProcessingAlgFactory.EXTENT: QgsProcessingParameterExtent,
    ProcessingAlgFactory.FIELD: QgsProcessingParameterField,
    ProcessingAlgFactory.FILE: QgsProcessingParameterFile,
    ProcessingAlgFactory.MAPLAYER: QgsProcessingParameterMapLayer,
    ProcessingAlgFactory.MATRIX: QgsProcessingParameterMatrix,
    ProcessingAlgFactory.MULTILAYER: QgsProcessingParameterMultipleLayers,
    ProcessingAlgFactory.POINT: QgsProcessingParameterPoint,
    ProcessingAlgFactory.GEOMETRY: QgsProcessingParameterGeometry,
    ProcessingAlgFactory.RANGE: QgsProcessingParameterRange,
    ProcessingAlgFactory.VECTOR_LAYER: QgsProcessingParameterVectorLayer,
    ProcessingAlgFactory.AUTH_CFG: QgsProcessingParameterAuthConfig,
    ProcessingAlgFactory.MESH_LAYER: QgsProcessingParameterMeshLayer,
    ProcessingAlgFactory.SCALE: QgsProcessingParameterScale,
    ProcessingAlgFactory.LAYOUT: QgsProcessingParameterLayout,
    ProcessingAlgFactory.LAYOUT_ITEM: QgsProcessingParameterLayoutItem,
    ProcessingAlgFactory.COLOR: QgsProcessingParameterColor,
    ProcessingAlgFactory.DATETIME: QgsProcessingParameterDateTime,
    ProcessingAlgFactory.MAP_THEME: QgsProcessingParameterMapTheme,
    ProcessingAlgFactory.PROVIDER_CONNECTION: QgsProcessingParameterProviderConnection,
    ProcessingAlgFactory.DATABASE_SCHEMA: QgsProcessingParameterDatabaseSchema,
    ProcessingAlgFactory.DATABASE_TABLE: QgsProcessingParameterDatabaseTable,
    ProcessingAlgFactory.COORDINATE_OPERATION: QgsProcessingParameterCoordinateOperation,
    ProcessingAlgFactory.POINTCLOUD_LAYER: QgsProcessingParameterPointCloudLayer,
    ProcessingAlgFactory.ANNOTATION_LAYER: QgsProcessingParameterAnnotationLayer
}

output_type_mapping = {
    str: partial(_make_output, cls=QgsProcessingOutputString),
    int: partial(_make_output, cls=QgsProcessingOutputNumber),
    float: partial(_make_output, cls=QgsProcessingOutputNumber),
    ProcessingAlgFactory.NUMBER: partial(_make_output, cls=QgsProcessingOutputNumber),
    ProcessingAlgFactory.DISTANCE: partial(_make_output, cls=QgsProcessingOutputNumber),
    ProcessingAlgFactory.INT: partial(_make_output, cls=QgsProcessingOutputNumber),
    ProcessingAlgFactory.STRING: partial(_make_output, cls=QgsProcessingOutputString),
    ProcessingAlgFactory.FILE: partial(_make_output, cls=QgsProcessingOutputFile),
    ProcessingAlgFactory.FOLDER: partial(_make_output, cls=QgsProcessingOutputFolder),
    ProcessingAlgFactory.HTML: partial(_make_output, cls=QgsProcessingOutputHtml),
    ProcessingAlgFactory.LAYERDEF: partial(_make_output, cls=QgsProcessingOutputLayerDefinition),
    ProcessingAlgFactory.MAPLAYER: partial(_make_output, cls=QgsProcessingOutputMapLayer),
    ProcessingAlgFactory.MULTILAYER: partial(_make_output, cls=QgsProcessingOutputMultipleLayers),
    ProcessingAlgFactory.RASTER_LAYER: partial(_make_output, cls=QgsProcessingOutputRasterLayer),
    ProcessingAlgFactory.VECTOR_LAYER: partial(_make_output, cls=QgsProcessingOutputVectorLayer),
    ProcessingAlgFactory.POINTCLOUD_LAYER: partial(_make_output, cls=QgsProcessingOutputPointCloudLayer),
    ProcessingAlgFactory.BOOL: partial(_make_output, cls=QgsProcessingOutputBoolean),
}
