"""
***************************************************************************
    ContextAction.py
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

from functools import wraps
from warnings import warn

from qgis.gui import QgsProcessingToolboxContextAction
from qgis.PyQt.QtCore import QCoreApplication


class ContextAction(QgsProcessingToolboxContextAction):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._parent_inited = True

    def __init_subclass__(cls, **kwargs):
        warn(
            "The ContextAction class is deprecated, use QgsProcessingToolboxContextAction",
            DeprecationWarning,
            2,
        )
        super().__init_subclass__(**kwargs)

        # Check if the subclass overrides __init__ directly -- if so, we have to
        # do some grossness to FORCE that __init__ to call the super class __init__,
        # or sip will raise an exception. And since we didn't require this super
        # call in the older API, we have to do this with "magic" to avoid breakage...
        if "__init__" in cls.__dict__:
            orig_init = cls.__dict__["__init__"]

            @wraps(orig_init)
            def wrapped_init(self, *args, **user_kwargs):
                # Ensure parent initialization runs first if it hasn't already
                if not getattr(self, "_parent_inited", False):
                    super().__init__()
                    self._parent_inited = True
                orig_init(self, *args, **user_kwargs)

            cls.__init__ = wrapped_init

    def tr(self, string, context=""):
        if context == "":
            context = self.__class__.__name__
        return QCoreApplication.translate(context, string)


def _get_name(self):
    return self.actionName()


def _set_name(self, _name):
    self.setActionName(_name)


ContextAction.name = property(_get_name)
ContextAction.name = ContextAction.name.setter(_set_name)
