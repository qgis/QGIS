"""
***************************************************************************
    fromfunction.py
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

import typing as _typing
from .qgstaskwrapper import QgsTaskWrapper
from qgis._core import QgsTask


@staticmethod
def _fromFunction(
    description: str,
    function: _typing.Callable,
    *args,
    on_finished: _typing.Optional[_typing.Callable] = None,
    flags=QgsTask.Flag.AllFlags,
    **kwargs
) -> QgsTask:
    """
    Creates a new QgsTask task from a python function.

    Example
    -------

    .. code-block:: python

        def calculate(task):
            # pretend this is some complex maths and stuff we want
            # to run in the background
            return 5*6

        def calculation_finished(exception, value=None):
            if not exception:
                iface.messageBar().pushMessage(
                    'the magic number is {}'.format(value))
            else:
                iface.messageBar().pushMessage(
                    str(exception))

        task = QgsTask.fromFunction('my task', calculate,
                on_finished=calculation_finished)
        QgsApplication.taskManager().addTask(task)

    """

    assert function
    return QgsTaskWrapper(description, flags, function, on_finished, *args, **kwargs)
