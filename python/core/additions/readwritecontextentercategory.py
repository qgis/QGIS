# -*- coding: utf-8 -*-

"""
***************************************************************************
    readwritecontextentercategory.py
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


class ReadWriteContextEnterCategory():
    """
    Push a category to the stack

    .. code-block:: python

        context = QgsReadWriteContext()
        with QgsReadWriteContext.enterCategory(context, category, details):
            # do something

    .. versionadded:: 3.2
    """

    def __init__(self, context, category_name, details=None):
        self.context = context
        self.category_name = category_name
        self.details = details
        self.popper = None

    def __enter__(self):
        self.popper = self.context._enterCategory(self.category_name, self.details)
        return self.context

    def __exit__(self, ex_type, ex_value, traceback):
        del self.popper
        return True
