"""
***************************************************************************
    common.py
    ---------------------
    Date                 : January 2024
    Copyright            : (C) 2024 by Nyall Dawson
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

import functools
from enum import Enum

def _handle_extended_enum_values(v):
    """
    If v is an Enum, returns an integer representation of its value wrapped
    in the Enum class.
    Otherwise returns v unchanged.
    """
    if isinstance(v, Enum):
       # v may be an Enum value which itself is an Enum value!
       # E.g. QNetworkRequest.Attribute(QgsNetworkRequestParameters.RequestAttributes.AttributeInitiatorClass)
       # in this case v is an Enum of the class QNetworkRequest.Attribute, while v.value is an Enum of the
       # class QgsNetworkRequestParameters.RequestAttributes. For this to work, we need to extract the integer
       # value from QgsNetworkRequestParameters.RequestAttributes.AttributeInitiatorClass and then convert it
       # to a QNetworkRequest.Attribute enum value!
       if isinstance(v.value, Enum):
          return v.__class__(int(v.value.value))
       return v.__class__(int(v.value))
    return v

def _extended_enum_wrapper(method):
    """
    Calls a function, first converting all passed Enum arguments to integer
    values so that extended enum values work correctly.
    """
    @functools.wraps(method)
    def wrapped(*args, **kwargs):
        parsed_args = [_handle_extended_enum_values(arg) for arg in args]
        parsed_kwargs = {k: _handle_extended_enum_values(v) for k, v in kwargs.items()}
        return method(*parsed_args, **parsed_kwargs)

    return wrapped


def install_extended_enum_wrapper(f):
    """
    Installs a wrapper on a function so that all passed extended Enum arguments are handled correctly.
    """
    return _extended_enum_wrapper(f)

