"""
***************************************************************************
    projectdirtyblocker.py
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

from qgis._core import QgsProjectDirtyBlocker


class ProjectDirtyBlocker:
    """
    Context manager used to block project setDirty calls.

    .. code-block:: python

        project = QgsProject.instance()
        with QgsProject.blockDirtying(project):
            # do something

    .. versionadded:: 3.2
    """

    def __init__(self, project):
        self.project = project
        self.blocker = None

    def __enter__(self):
        self.blocker = QgsProjectDirtyBlocker(self.project)
        return self.project

    def __exit__(self, ex_type, ex_value, traceback):
        del self.blocker
        return ex_type is None
