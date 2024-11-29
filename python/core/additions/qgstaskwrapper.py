"""
***************************************************************************
    qgstaskwrapper.py
    ---------------------
    Date                 : May 2018
    Copyright            : (C) 2018 by Denis Rouzaud
    Email                : denis@opengis.ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

from qgis._core import QgsTask


class QgsTaskWrapper(QgsTask):

    def __init__(self, description, flags, function, on_finished, *args, **kwargs):
        QgsTask.__init__(self, description, flags)
        self.args = args
        self.kwargs = kwargs
        self.function = function
        self.on_finished = on_finished
        self.returned_values = None
        self.exception = None

    def run(self):
        try:
            self.returned_values = self.function(self, *self.args, **self.kwargs)
        except Exception as ex:
            # report error
            self.exception = ex
            return False

        return True

    def finished(self, result):
        if not self.on_finished:
            return

        if not result and self.exception is None:
            self.exception = Exception("Task canceled")

        try:
            if self.returned_values:
                self.on_finished(self.exception, self.returned_values)
            else:
                self.on_finished(self.exception)
        except Exception as ex:
            self.exception = ex
