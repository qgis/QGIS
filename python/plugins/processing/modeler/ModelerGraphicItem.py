"""
***************************************************************************
    ModelerGraphicItem.py
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

__author__ = "Victor Olaya"
__date__ = "August 2012"
__copyright__ = "(C) 2012, Victor Olaya"

from qgis.core import (
    QgsProcessingModelOutput,
    QgsProcessingParameterDefinition,
)
from qgis.gui import (
    QgsModelOutputGraphicItem,
)

from processing.modeler.ModelerParameterDefinitionDialog import (
    ModelerParameterDefinitionDialog,
)


class ModelerOutputGraphicItem(QgsModelOutputGraphicItem):
    """
    IMPORTANT! This is intentionally a MINIMAL class, only containing code which HAS TO BE HERE
    because it contains Python code for compatibility with deprecated methods ONLY.

    Don't add anything here -- edit the c++ base class instead!
    """

    def __init__(self, element, model):
        super().__init__(element, model, None)

    def edit(self, edit_comment=False):
        child_alg = self.model().childAlgorithm(self.component().childId())
        dlg = ModelerParameterDefinitionDialog(
            self.model(),
            param=self.model().modelParameterFromChildIdAndOutputName(
                self.component().childId(), self.component().name()
            ),
        )
        dlg.setComments(self.component().comment().description())
        dlg.setCommentColor(self.component().comment().color())
        if edit_comment:
            dlg.switchToCommentTab()

        if dlg.exec():
            new_param = dlg.create_parameter()
            self.apply_new_output(
                name=new_param.description(),
                description=new_param.description(),
                default=new_param.defaultValue(),
                mandatory=not (
                    new_param.flags()
                    & QgsProcessingParameterDefinition.Flag.FlagOptional
                ),
                comments=dlg.comments(),
                comment_color=dlg.commentColor(),
                child_alg=child_alg,
            )

    def apply_new_output(
        self,
        name: str,
        description: str,
        default,
        mandatory: bool,
        comments: str,
        comment_color,
        child_alg,
    ):
        model_outputs = child_alg.modelOutputs()

        model_output = QgsProcessingModelOutput(model_outputs[self.component().name()])
        del model_outputs[self.component().name()]

        model_output.setName(name)
        model_output.setDescription(description)
        model_output.setDefaultValue(default)
        model_output.setMandatory(mandatory)
        model_output.comment().setDescription(comments)
        model_output.comment().setColor(comment_color)
        model_outputs[model_output.name()] = model_output
        child_alg.setModelOutputs(model_outputs)

        undo_command_id = f"output:{name}"
        self.aboutToChange.emit(
            self.tr("Edit {}").format(model_output.description()), undo_command_id
        )

        self.model().updateDestinationParameters()
        self.requestModelRepaint.emit()
        self.changed.emit()

    def editComponent(self):
        self.edit()

    def editComment(self):
        self.edit(edit_comment=True)
