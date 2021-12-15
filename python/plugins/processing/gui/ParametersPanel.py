# -*- coding: utf-8 -*-

"""
***************************************************************************
    ParametersPanel.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
                           (C) 2013 by CS Systemes d'information (CS SI)
    Email                : volayaf at gmail dot com
                           otb at c-s dot fr (CS SI)
    Contributors         : Victor Olaya

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

from qgis.core import (QgsProcessingParameterDefinition,
                       QgsProcessingParameterExtent,
                       QgsProject,
                       QgsProcessingModelAlgorithm,
                       QgsProcessingOutputLayerDefinition)
from qgis.gui import (QgsProcessingContextGenerator,
                      QgsProcessingParameterWidgetContext,
                      QgsProcessingParametersWidget,
                      QgsGui,
                      QgsProcessingGui,
                      QgsProcessingParametersGenerator,
                      QgsProcessingHiddenWidgetWrapper)
from qgis.utils import iface

from processing.gui.wrappers import WidgetWrapperFactory, WidgetWrapper
from processing.gui.AlgorithmDialogBase import AlgorithmDialogBase
from processing.tools.dataobjects import createContext


class ParametersPanel(QgsProcessingParametersWidget):

    def __init__(self, parent, alg, in_place=False, active_layer=None):
        super().__init__(alg, parent)
        self.in_place = in_place
        self.active_layer = active_layer

        self.wrappers = {}

        self.extra_parameters = {}

        self.processing_context = createContext()

        class ContextGenerator(QgsProcessingContextGenerator):

            def __init__(self, context):
                super().__init__()
                self.processing_context = context

            def processingContext(self):
                return self.processing_context

        self.context_generator = ContextGenerator(self.processing_context)

        self.initWidgets()

        QgsProject.instance().layerWasAdded.connect(self.layerRegistryChanged)
        QgsProject.instance().layersWillBeRemoved.connect(self.layerRegistryChanged)

    def layerRegistryChanged(self, layers):
        for wrapper in list(self.wrappers.values()):
            try:
                wrapper.refresh()
            except AttributeError:
                pass

    def initWidgets(self):
        super().initWidgets()

        widget_context = QgsProcessingParameterWidgetContext()
        widget_context.setProject(QgsProject.instance())
        if iface is not None:
            widget_context.setMapCanvas(iface.mapCanvas())
            widget_context.setBrowserModel(iface.browserModel())
            widget_context.setActiveLayer(iface.activeLayer())

        widget_context.setMessageBar(self.parent().messageBar())
        if isinstance(self.algorithm(), QgsProcessingModelAlgorithm):
            widget_context.setModel(self.algorithm())

        in_place_input_parameter_name = 'INPUT'
        if hasattr(self.algorithm(), 'inputParameterName'):
            in_place_input_parameter_name = self.algorithm().inputParameterName()

        # Create widgets and put them in layouts
        for param in self.algorithm().parameterDefinitions():
            if param.flags() & QgsProcessingParameterDefinition.FlagHidden:
                continue

            if param.isDestination():
                continue
            else:
                if self.in_place and param.name() in (in_place_input_parameter_name, 'OUTPUT'):
                    # don't show the input/output parameter widgets in in-place mode
                    # we still need to CREATE them, because other wrappers may need to interact
                    # with them (e.g. those parameters which need the input layer for field
                    # selections/crs properties/etc)
                    self.wrappers[param.name()] = QgsProcessingHiddenWidgetWrapper(param, QgsProcessingGui.Standard, self)
                    self.wrappers[param.name()].setLinkedVectorLayer(self.active_layer)
                    continue

                wrapper = WidgetWrapperFactory.create_wrapper(param, self.parent())
                wrapper.setWidgetContext(widget_context)
                wrapper.registerProcessingContextGenerator(self.context_generator)
                wrapper.registerProcessingParametersGenerator(self)
                self.wrappers[param.name()] = wrapper

                # For compatibility with 3.x API, we need to check whether the wrapper is
                # the deprecated WidgetWrapper class. If not, it's the newer
                # QgsAbstractProcessingParameterWidgetWrapper class
                # TODO QGIS 4.0 - remove
                is_python_wrapper = issubclass(wrapper.__class__, WidgetWrapper)
                stretch = 0
                if not is_python_wrapper:
                    widget = wrapper.createWrappedWidget(self.processing_context)
                    stretch = wrapper.stretch()
                else:
                    widget = wrapper.widget

                if widget is not None:
                    if is_python_wrapper:
                        widget.setToolTip(param.toolTip())

                    label = None
                    if not is_python_wrapper:
                        label = wrapper.createWrappedLabel()
                    else:
                        label = wrapper.label

                    if label is not None:
                        self.addParameterLabel(param, label)
                    elif is_python_wrapper:
                        desc = param.description()
                        if isinstance(param, QgsProcessingParameterExtent):
                            desc += self.tr(' (xmin, xmax, ymin, ymax)')
                        if param.flags() & QgsProcessingParameterDefinition.FlagOptional:
                            desc += self.tr(' [optional]')
                        widget.setText(desc)

                    self.addParameterWidget(param, widget, stretch)

        for output in self.algorithm().destinationParameterDefinitions():
            if output.flags() & QgsProcessingParameterDefinition.FlagHidden:
                continue

            if self.in_place and output.name() in (in_place_input_parameter_name, 'OUTPUT'):
                continue

            wrapper = QgsGui.processingGuiRegistry().createParameterWidgetWrapper(output, QgsProcessingGui.Standard)
            wrapper.setWidgetContext(widget_context)
            wrapper.registerProcessingContextGenerator(self.context_generator)
            wrapper.registerProcessingParametersGenerator(self)
            self.wrappers[output.name()] = wrapper

            label = wrapper.createWrappedLabel()
            if label is not None:
                self.addOutputLabel(label)

            widget = wrapper.createWrappedWidget(self.processing_context)
            self.addOutputWidget(widget, wrapper.stretch())

            #    def skipOutputChanged(widget, checkbox, skipped):
            # TODO
            #        enabled = not skipped
            #
            #        # Do not try to open formats that are write-only.
            #        value = widget.value()
            #        if value and isinstance(value, QgsProcessingOutputLayerDefinition) and isinstance(output, (
            #                QgsProcessingParameterFeatureSink, QgsProcessingParameterVectorDestination)):
            #            filename = value.sink.staticValue()
            #            if filename not in ('memory:', ''):
            #                path, ext = os.path.splitext(filename)
            #                format = QgsVectorFileWriter.driverForExtension(ext)
            #                drv = gdal.GetDriverByName(format)
            #                if drv:
            #                    if drv.GetMetadataItem(gdal.DCAP_OPEN) is None:
            #                        enabled = False
            #
            #        checkbox.setEnabled(enabled)
            #        checkbox.setChecked(enabled)

        for wrapper in list(self.wrappers.values()):
            wrapper.postInitialize(list(self.wrappers.values()))

    def createProcessingParameters(self, flags=QgsProcessingParametersGenerator.Flags()):
        include_default = not (flags & QgsProcessingParametersGenerator.Flag.SkipDefaultValueParameters)
        parameters = {}
        for p, v in self.extra_parameters.items():
            parameters[p] = v

        for param in self.algorithm().parameterDefinitions():
            if param.flags() & QgsProcessingParameterDefinition.FlagHidden:
                continue
            if not param.isDestination():
                try:
                    wrapper = self.wrappers[param.name()]
                except KeyError:
                    continue

                # For compatibility with 3.x API, we need to check whether the wrapper is
                # the deprecated WidgetWrapper class. If not, it's the newer
                # QgsAbstractProcessingParameterWidgetWrapper class
                # TODO QGIS 4.0 - remove
                if issubclass(wrapper.__class__, WidgetWrapper):
                    widget = wrapper.widget
                else:
                    widget = wrapper.wrappedWidget()

                if not isinstance(wrapper, QgsProcessingHiddenWidgetWrapper) and widget is None:
                    continue

                value = wrapper.parameterValue()
                if param.defaultValue() != value or include_default:
                    parameters[param.name()] = value

                if not param.checkValueIsAcceptable(value):
                    raise AlgorithmDialogBase.InvalidParameterValue(param, widget)
            else:
                if self.in_place and param.name() == 'OUTPUT':
                    parameters[param.name()] = 'memory:'
                    continue

                try:
                    wrapper = self.wrappers[param.name()]
                except KeyError:
                    continue

                widget = wrapper.wrappedWidget()
                value = wrapper.parameterValue()

                dest_project = None
                if wrapper.customProperties().get('OPEN_AFTER_RUNNING'):
                    dest_project = QgsProject.instance()

                if value and isinstance(value, QgsProcessingOutputLayerDefinition):
                    value.destinationProject = dest_project
                if value and (param.defaultValue() != value or include_default):
                    parameters[param.name()] = value

                    context = createContext()
                    ok, error = param.isSupportedOutputValue(value, context)
                    if not ok:
                        raise AlgorithmDialogBase.InvalidOutputExtension(widget, error)

        return self.algorithm().preprocessParameters(parameters)

    def setParameters(self, parameters):
        self.extra_parameters = {}
        for param in self.algorithm().parameterDefinitions():
            if param.flags() & QgsProcessingParameterDefinition.FlagHidden:
                if param.name() in parameters:
                    self.extra_parameters[param.name()] = parameters[param.name()]
                continue

            if not param.name() in parameters:
                continue

            value = parameters[param.name()]

            wrapper = self.wrappers[param.name()]
            wrapper.setParameterValue(value, self.processing_context)
