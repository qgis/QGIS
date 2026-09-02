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
    QgsProcessingContext,
    QgsProcessingModelOutput,
    QgsProcessingParameterDefinition,
)
from qgis.gui import (
    QgsProcessingModelConfigWidgetFactory,
    QgsProcessingParameterWidgetContext,
)

from processing.modeler.ModelerParameterDefinitionDialog import (
    ModelerParameterDefinitionWidget,
)


class ModelConfigWidgetFactory(QgsProcessingModelConfigWidgetFactory):
    def supportsComponent(self, component):
        return isinstance(
            component,
            (QgsProcessingModelOutput,),
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

        if isinstance(component, QgsProcessingModelOutput):
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

        return None
