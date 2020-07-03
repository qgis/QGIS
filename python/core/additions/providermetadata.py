# -*- coding: utf-8 -*-

"""
***************************************************************************
    providermetadata.py
    ---------------------
    Date                 : June 2019
    Copyright            : (C) 2019 by Martin Dobias
    Email                : wonder dot sk at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

from qgis._core import QgsProviderMetadata


class PyProviderMetadata(QgsProviderMetadata):
    """ wrapper around QgsProviderMetadata to keep the existing Python code running which registers
        data providers by passing a custom python createProvider() function to QgsProviderMetadata
        constructor. The proper new way of doing it is to subclass QgsProviderMetadata and implement
        its virtual functions.

        TODO: QGIS 4 - remove this wrapper (only subclassing of QgsProviderMetadata should be used)
    """

    # this is a workaround to keep references to metadata classes
    # so they are not removed when the variable gets out of scope
    _kept_refs = []

    def __init__(self, key, description, library_or_create_func=None):
        super().__init__(key, description)
        if callable(library_or_create_func):
            self.createProvider = library_or_create_func
            PyProviderMetadata._kept_refs.append(self)


PyProviderMetadata.__doc__ = QgsProviderMetadata.__doc__
