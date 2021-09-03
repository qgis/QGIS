# -*- coding: utf-8 -*-

"""
***************************************************************************
    __init__.py
    ---------------------
    Date                 : June 2021
    Copyright            : (C) 2021 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'June 2021'
__copyright__ = '(C) 2021, Alexander Bruy'


def classFactory(iface):
    from otbprovider.OtbProviderPlugin import OtbProviderPlugin
    return OtbProviderPlugin()
