"""
***************************************************************************
    runtimeprofiler.py
    ---------------------
    Date                 : May 2020
    Copyright            : (C) 2020 by Nyall Dawson
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

from qgis._core import QgsScopedRuntimeProfile


class ScopedRuntimeProfileContextManager:
    """
    Context manager used to profile blocks of code in the QgsApplication.profiler() registry.

    .. code-block:: python

        with QgsRuntimeProfiler.profile('My operation'):
            # do something

    .. versionadded:: 3.14
    """

    def __init__(self, operation):
        self.operation = operation
        self.profiler = None

    def __enter__(self):
        self.profiler = QgsScopedRuntimeProfile(self.operation)
        return self.operation

    def __exit__(self, ex_type, ex_value, traceback):
        del self.profiler
        return ex_type is None
