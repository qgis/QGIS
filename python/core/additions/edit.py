# -*- coding: utf-8 -*-

"""
***************************************************************************
    edit.py
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

from builtins import object


class QgsEditError(Exception):

    def __init__(self, value):
        self.value = value

    def __str__(self):
        return repr(self.value)


class edit(object):

    def __init__(self, layer):
        self.layer = layer

    def __enter__(self):
        assert self.layer.startEditing()
        return self.layer

    def __exit__(self, ex_type, ex_value, traceback):
        if ex_type is None:
            if not self.layer.commitChanges():
                raise QgsEditError(self.layer.commitErrors())
            return True
        else:
            self.layer.rollBack()
            return False
