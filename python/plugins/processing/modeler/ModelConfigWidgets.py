"""
***************************************************************************
    ModelConfigWidgets.py
    ---------------------
    Date                 : January 2026
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

from qgis.core import (
    QgsProcessingModelChildAlgorithm,
    QgsProcessingModelParameter,
    QgsProcessingContext,
    QgsProcessingModelOutput,
    QgsProcessingParameterDefinition,
)
from qgis.gui import (
    QgsProcessingModelConfigWidgetFactory,
    QgsProcessingParameterWidgetContext,
    QgsProcessingParameterDefinitionPanelWidget,
)
from processing.modeler.ModelerParameterDefinitionDialog import (
    ModelerParameterDefinitionDialog,
    ModelerParameterDefinitionWidget,
)
from processing.modeler.ModelerParametersDialog import (
    ModelerParametersWidget,
    ModelerParametersDialog,
)


class ModelConfigWidgetFactory(QgsProcessingModelConfigWidgetFactory):

    def supportsComponent(self, component):
        return isinstance(
            component,
            (
                QgsProcessingModelParameter,
                QgsProcessingModelOutput,
                QgsProcessingModelChildAlgorithm,
            ),
        )

    def createWidget(
        self,
        component,
        context: QgsProcessingContext,
        widgetContext: QgsProcessingParameterWidgetContext,
    ):
        model = widgetContext.model()
        if not model:
            return None

        model_dialog = widgetContext.modelDesignerDialog()

        if isinstance(component, QgsProcessingModelParameter):
            component_name = component.parameterName()
            existing_param = model.parameterDefinition(component_name)
            if ModelerParameterDefinitionDialog.use_legacy_dialog(param=existing_param):
                # boo, old api, not supported for the dock
                return None

            comment = component.comment().description()
            comment_color = component.comment().color()
            old_name = existing_param.name()
            old_description = existing_param.description()

            widget = QgsProcessingParameterDefinitionPanelWidget(
                type=existing_param.type(),
                context=context,
                widgetContext=widgetContext,
                definition=existing_param,
                algorithm=model,
            )
            widget.setComments(comment)
            widget.setCommentColor(comment_color)
            if widgetContext.processingContextGenerator():
                widget.registerProcessingContextGenerator(
                    widgetContext.processingContextGenerator()
                )

            existing_param_name = existing_param.name()

            def on_widget_changed():
                nonlocal existing_param_name
                model_scene = model_dialog.modelScene()
                graphic_item = model_scene.parameterItem(component_name)
                if not graphic_item:
                    # should not happen!
                    return

                new_param = widget.createParameter(existing_param_name)
                comment = widget.comments()
                comment_color = widget.commentColor()
                existing_param_name = graphic_item.apply_new_param(
                    new_param, old_description, old_name, comment, comment_color
                )

            widget.widgetChanged.connect(on_widget_changed)
            return widget
        elif isinstance(component, QgsProcessingModelOutput):
            child_id = component.childId()
            child_output_name = component.childOutputName()

            child_alg = model.childAlgorithm(child_id)
            comment = component.comment().description()
            comment_color = component.comment().color()

            existing_param = model.modelParameterFromChildIdAndOutputName(
                component.childId(), component.name()
            )

            widget = ModelerParameterDefinitionWidget(
                model,
                param=existing_param,
            )
            widget.setComments(comment)
            widget.setCommentColor(comment_color)

            def on_widget_changed():
                model_scene = model_dialog.modelScene()
                graphic_item = model_scene.outputItem(child_id, child_output_name)
                if not graphic_item:
                    # should not happen!
                    return

                new_param = widget.create_parameter()
                graphic_item.apply_new_output(
                    name=new_param.description(),
                    description=new_param.description(),
                    default=new_param.defaultValue(),
                    mandatory=not (
                        new_param.flags()
                        & QgsProcessingParameterDefinition.Flag.FlagOptional
                    ),
                    comments=widget.comments(),
                    comment_color=widget.commentColor(),
                    child_alg=child_alg,
                )

            widget.widgetChanged.connect(on_widget_changed)
            return widget
        elif isinstance(component, QgsProcessingModelChildAlgorithm):
            algorithm = component.algorithm()
            context = widgetContext.processingContextGenerator().processingContext()

            # this is unfortunately VERY ugly. The old Python processing GUI api
            # had the ModelerParametersDialog class so enmeshed in the parameter
            # GUI classes that there is no way to rip this out without breaking
            # API. So we HAVE to do a gross hack here and make a dialog, which
            # we never show, but which allows us to keep the old API intact...
            fake_dialog = ModelerParametersDialog(
                algorithm,
                model,
                algName=component.childId(),
                configuration=component.configuration(),
            )

            widget = ModelerParametersWidget(
                algorithm,
                model,
                algName=component.childId(),
                configuration=component.configuration(),
                dialog=fake_dialog,
                context=context,
            )
            widget.setComments(component.comment().description())
            widget.setCommentColor(component.comment().color())

            # make sure fake dialog exists for lifetime of widget
            widget.__fake_dialog = fake_dialog
            return widget

        return None
