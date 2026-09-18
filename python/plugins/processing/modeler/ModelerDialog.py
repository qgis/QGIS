"""
***************************************************************************
    ModelerDialog.py
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

from qgis.gui import (
    QgsModelDesignerDialog,
    QgsProcessingContextGenerator,
)

from processing.tools.dataobjects import createContext


class ModelerDialog(QgsModelDesignerDialog):
    dlgs = []

    @staticmethod
    def create(model=None):
        """
        Workaround crappy sip handling of QMainWindow. It doesn't know that we are using the deleteonclose
        flag, so happily just deletes dialogs as soon as they go out of scope. The only workaround possible
        while we still have to drag around this Python code is to store a reference to the sip wrapper so that
        sip doesn't get confused. The underlying object will still be deleted by the deleteonclose flag though!
        """
        dlg = ModelerDialog(model)
        ModelerDialog.dlgs.append(dlg)
        return dlg

    def __init__(self, model=None, parent=None):
        super().__init__(parent)

        self.processing_context = createContext()

        class ContextGenerator(QgsProcessingContextGenerator):
            def __init__(self, context):
                super().__init__()
                self.processing_context = context

            def processingContext(self):
                return self.processing_context

        self.context_generator = ContextGenerator(self.processing_context)
        self.registerProcessingContextGenerator(self.context_generator)

        if model is not None:
            _model = model.create()
            _model.setSourceFilePath(model.sourceFilePath())
            self.setModel(_model)
