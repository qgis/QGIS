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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os


class Test:

    def __init__(self, iface):
        plugin_dir = os.path.dirname(__file__)

        # write to a file
        f = open(plugin_dir + '/../plugin_started.txt', 'w')
        f.write("OK\n")
        f.close()

    def initGui(self):
        pass

    def unload(self):
        pass

    # run method that performs all the real work
    def run(self):
        pass


def name():
    return "plugin path test"


def description():
    return "desc"


def version():
    return "Version 0.1"


def icon():
    return "icon.png"


def qgisMinimumVersion():
    return "2.0"


def author():
    return "HM/Oslandia"


def email():
    return "hugo.mercier@oslandia.com"


def classFactory(iface):
    # load Test class from file Test
    return Test(iface)
