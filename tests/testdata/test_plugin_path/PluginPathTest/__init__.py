# -*- coding: utf-8 -*-

import os

class Test:

    def __init__(self, iface):
        plugin_dir = os.path.dirname(__file__)

        # write to a file
        f = open( plugin_dir + '/../plugin_started.txt', 'w' )
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
