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

import os

from functools import partial

from qgis.core import (QgsProcessingParameterDefinition,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterRasterDestination,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterVectorDestination,
                       QgsProcessingOutputLayerDefinition,
                       QgsProject,
                       QgsProcessingModelAlgorithm,
                       QgsVectorFileWriter)
from qgis.gui import (QgsProcessingContextGenerator,
                      QgsProcessingParameterWidgetContext,
                      QgsProcessingLayerOutputDestinationWidget,
                      QgsProcessingParametersWidget)
from qgis.utils import iface

from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtWidgets import (
    QLabel,
    QCheckBox
)
from osgeo import gdal

from processing.gui.wrappers import WidgetWrapperFactory, WidgetWrapper
from processing.tools.dataobjects import createContext


class ParametersPanel(QgsProcessingParametersWidget):

    def __init__(self, parent, alg, in_place=False):
        super().__init__(alg, parent)
        self.in_place = in_place

        self.wrappers = {}
        self.outputWidgets = {}
        self.checkBoxes = {}

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

        # Create widgets and put them in layouts
        for param in self.algorithm().parameterDefinitions():
            if param.flags() & QgsProcessingParameterDefinition.FlagHidden:
                continue

            if param.isDestination():
                continue
            else:
                wrapper = WidgetWrapperFactory.create_wrapper(param, self.parent())
                wrapper.setWidgetContext(widget_context)
                wrapper.registerProcessingContextGenerator(self.context_generator)
                self.wrappers[param.name()] = wrapper

                # For compatibility with 3.x API, we need to check whether the wrapper is
                # the deprecated WidgetWrapper class. If not, it's the newer
                # QgsAbstractProcessingParameterWidgetWrapper class
                # TODO QGIS 4.0 - remove
                is_python_wrapper = issubclass(wrapper.__class__, WidgetWrapper)
                if not is_python_wrapper:
                    widget = wrapper.createWrappedWidget(self.processing_context)
                else:
                    widget = wrapper.widget

                if self.in_place and param.name() in ('INPUT', 'OUTPUT'):
                    # don't show the input/output parameter widgets in in-place mode
                    # we still need to CREATE them, because other wrappers may need to interact
                    # with them (e.g. those parameters which need the input layer for field
                    # selections/crs properties/etc)
                    continue

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

                    self.addParameterWidget(param, widget)

        for output in self.algorithm().destinationParameterDefinitions():
            if output.flags() & QgsProcessingParameterDefinition.FlagHidden:
                continue

            if self.in_place and param.name() in ('INPUT', 'OUTPUT'):
                continue

            label = QLabel(output.description())
            widget = QgsProcessingLayerOutputDestinationWidget(output, False)
            widget.setWidgetContext(widget_context)

            self.addOutputLabel(label)
            self.addOutputWidget(widget)
            if isinstance(output, (QgsProcessingParameterRasterDestination, QgsProcessingParameterFeatureSink,
                                   QgsProcessingParameterVectorDestination)):
                check = QCheckBox()
                check.setText(QCoreApplication.translate('ParametersPanel', 'Open output file after running algorithm'))

                def skipOutputChanged(widget, checkbox, skipped):

                    enabled = not skipped

                    # Do not try to open formats that are write-only.
                    value = widget.value()
                    if value and isinstance(value, QgsProcessingOutputLayerDefinition) and isinstance(output, (
                            QgsProcessingParameterFeatureSink, QgsProcessingParameterVectorDestination)):
                        filename = value.sink.staticValue()
                        if filename not in ('memory:', ''):
                            path, ext = os.path.splitext(filename)
                            format = QgsVectorFileWriter.driverForExtension(ext)
                            drv = gdal.GetDriverByName(format)
                            if drv:
                                if drv.GetMetadataItem(gdal.DCAP_OPEN) is None:
                                    enabled = False

                    checkbox.setEnabled(enabled)
                    checkbox.setChecked(enabled)

                check.setChecked(not widget.outputIsSkipped())
                check.setEnabled(not widget.outputIsSkipped())
                widget.skipOutputChanged.connect(partial(skipOutputChanged, widget, check))
                self.addOutputWidget(check)
                self.checkBoxes[output.name()] = check

            self.outputWidgets[output.name()] = widget

        for wrapper in list(self.wrappers.values()):
            wrapper.postInitialize(list(self.wrappers.values()))

    def setParameters(self, parameters):
        for param in self.algorithm().parameterDefinitions():
            if param.flags() & QgsProcessingParameterDefinition.FlagHidden:
                continue

            if not param.name() in parameters:
                continue

            if not param.isDestination():
                value = parameters[param.name()]

                wrapper = self.wrappers[param.name()]
                wrapper.setParameterValue(value, self.processing_context)
            else:
                dest_widget = self.outputWidgets[param.name()]
                dest_widget.setValue(parameters[param.name()])
