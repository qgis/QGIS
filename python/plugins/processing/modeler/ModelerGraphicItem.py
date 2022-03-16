# -*- coding: utf-8 -*-

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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

from qgis.PyQt.QtCore import QCoreApplication

from qgis.core import (QgsProcessingParameterDefinition,
                       QgsProcessingModelOutput,
                       QgsProject,
                       Qgis)
from qgis.gui import (
    QgsProcessingParameterDefinitionDialog,
    QgsProcessingParameterWidgetContext,
    QgsModelParameterGraphicItem,
    QgsModelChildAlgorithmGraphicItem,
    QgsModelOutputGraphicItem,
    QgsProcessingContextGenerator
)
from processing.modeler.ModelerParameterDefinitionDialog import ModelerParameterDefinitionDialog
from processing.modeler.ModelerParametersDialog import ModelerParametersDialog
from processing.tools.dataobjects import createContext
from qgis.utils import iface


class ModelerInputGraphicItem(QgsModelParameterGraphicItem):
    """
    IMPORTANT! This is intentionally a MINIMAL class, only containing code which HAS TO BE HERE
    because it contains Python code for compatibility with deprecated methods ONLY.

    Don't add anything here -- edit the c++ base class instead!
    """

    def __init__(self, element, model):
        super().__init__(element, model, None)

        self.processing_context = createContext()

        class ContextGenerator(QgsProcessingContextGenerator):

            def __init__(self, context):
                super().__init__()
                self.processing_context = context

            def processingContext(self):
                return self.processing_context

        self.context_generator = ContextGenerator(self.processing_context)

    def create_widget_context(self):
        """
        Returns a new widget context for use in the model editor
        """
        widget_context = QgsProcessingParameterWidgetContext()
        widget_context.setProject(QgsProject.instance())
        if iface is not None:
            widget_context.setMapCanvas(iface.mapCanvas())
            widget_context.setActiveLayer(iface.activeLayer())

        widget_context.setModel(self.model())
        return widget_context

    def edit(self, edit_comment=False):
        existing_param = self.model().parameterDefinition(self.component().parameterName())
        old_name = existing_param.name()
        old_description = existing_param.description()

        comment = self.component().comment().description()
        comment_color = self.component().comment().color()
        new_param = None
        if ModelerParameterDefinitionDialog.use_legacy_dialog(param=existing_param):
            # boo, old api
            dlg = ModelerParameterDefinitionDialog(self.model(),
                                                   param=existing_param)
            dlg.setComments(comment)
            dlg.setCommentColor(comment_color)
            if edit_comment:
                dlg.switchToCommentTab()
            if dlg.exec_():
                new_param = dlg.param
                comment = dlg.comments()
                comment_color = dlg.commentColor()
        else:
            # yay, use new API!
            context = createContext()
            widget_context = self.create_widget_context()
            dlg = QgsProcessingParameterDefinitionDialog(type=existing_param.type(),
                                                         context=context,
                                                         widgetContext=widget_context,
                                                         definition=existing_param,
                                                         algorithm=self.model())
            dlg.setComments(comment)
            dlg.setCommentColor(comment_color)
            dlg.registerProcessingContextGenerator(self.context_generator)

            if edit_comment:
                dlg.switchToCommentTab()

            if dlg.exec_():
                new_param = dlg.createParameter(existing_param.name())
                comment = dlg.comments()
                comment_color = dlg.commentColor()

                validChars = \
                    'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'
                safeName = ''.join(c for c in new_param.description() if c in validChars)
                new_param.setName(safeName.lower())

        if new_param is not None:
            self.aboutToChange.emit(self.tr('Edit {}').format(new_param.description()))
            self.model().removeModelParameter(self.component().parameterName())

            if new_param.description() != old_description:
                # only update name if user has changed the description -- we don't force this, as it may cause
                # unwanted name updates which could potentially break the model's API
                name = new_param.name()

                base_name = name
                i = 2
                while self.model().parameterDefinition(name):
                    name = base_name + str(i)
                    i += 1

                new_param.setName(name)

                self.model().changeParameterName(old_name, new_param.name())

            self.component().setParameterName(new_param.name())
            self.component().setDescription(new_param.name())
            self.component().comment().setDescription(comment)
            self.component().comment().setColor(comment_color)
            self.model().addModelParameter(new_param, self.component())
            self.setLabel(new_param.description())
            self.requestModelRepaint.emit()
            self.changed.emit()

    def editComponent(self):
        self.edit()

    def editComment(self):
        self.edit(edit_comment=True)


class ModelerChildAlgorithmGraphicItem(QgsModelChildAlgorithmGraphicItem):
    """
    IMPORTANT! This is intentionally a MINIMAL class, only containing code which HAS TO BE HERE
    because it contains Python code for compatibility with deprecated methods ONLY.

    Don't add anything here -- edit the c++ base class instead!
    """

    def __init__(self, element, model):
        super().__init__(element, model, None)

    def edit(self, edit_comment=False):
        elemAlg = self.component().algorithm()
        dlg = ModelerParametersDialog(elemAlg, self.model(), self.component().childId(),
                                      self.component().configuration())
        dlg.setComments(self.component().comment().description())
        dlg.setCommentColor(self.component().comment().color())
        if edit_comment:
            dlg.switchToCommentTab()
        if dlg.exec_():
            alg = dlg.createAlgorithm()
            alg.setChildId(self.component().childId())
            alg.copyNonDefinitionPropertiesFromModel(self.model())
            self.aboutToChange.emit(self.tr('Edit {}').format(alg.description()))
            self.model().setChildAlgorithm(alg)
            self.requestModelRepaint.emit()
            self.changed.emit()

            res, errors = self.model().validateChildAlgorithm(alg.childId())
            if not res:
                self.scene().showWarning(
                    QCoreApplication.translate('ModelerGraphicItem', 'Algorithm “{}” is invalid').format(alg.description()),
                    self.tr('Algorithm is Invalid'),
                    QCoreApplication.translate('ModelerGraphicItem', "<p>The “{}” algorithm is invalid, because:</p><ul><li>{}</li></ul>").format(alg.description(), '</li><li>'.join(errors)),
                    level=Qgis.Warning
                )
            else:
                self.scene().messageBar().clearWidgets()

    def editComponent(self):
        self.edit()

    def editComment(self):
        self.edit(edit_comment=True)


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
        dlg = ModelerParameterDefinitionDialog(self.model(),
                                               param=self.model().modelParameterFromChildIdAndOutputName(self.component().childId(), self.component().name()))
        dlg.setComments(self.component().comment().description())
        dlg.setCommentColor(self.component().comment().color())
        if edit_comment:
            dlg.switchToCommentTab()

        if dlg.exec_():
            model_outputs = child_alg.modelOutputs()

            model_output = QgsProcessingModelOutput(model_outputs[self.component().name()])
            del model_outputs[self.component().name()]

            model_output.setName(dlg.param.description())
            model_output.setDescription(dlg.param.description())
            model_output.setDefaultValue(dlg.param.defaultValue())
            model_output.setMandatory(not (dlg.param.flags() & QgsProcessingParameterDefinition.FlagOptional))
            model_output.comment().setDescription(dlg.comments())
            model_output.comment().setColor(dlg.commentColor())
            model_outputs[model_output.name()] = model_output
            child_alg.setModelOutputs(model_outputs)

            self.aboutToChange.emit(self.tr('Edit {}').format(model_output.description()))

            self.model().updateDestinationParameters()
            self.requestModelRepaint.emit()
            self.changed.emit()

    def editComponent(self):
        self.edit()

    def editComment(self):
        self.edit(edit_comment=True)
