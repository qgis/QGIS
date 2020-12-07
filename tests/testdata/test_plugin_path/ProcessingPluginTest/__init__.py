# -*- coding: utf-8 -*-

"""
***************************************************************************
    __init__.py
    ---------------------
    Date                 : July 2013
    Copyright            : (C) 2013 by Hugo Mercier
    Email                : hugo dot mercier at oslandia dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Hugo Mercier'
__date__ = 'July 2013'
__copyright__ = '(C) 2013, Hugo Mercier'

import os


class Test:

    def __init__(self, iface):
        pass

    def initGui(self):
        assert False

    def initProcessing(self):
        pass

    def unload(self):
        pass


def classFactory(iface):
    # load Test class from file Test
    return Test(iface)
