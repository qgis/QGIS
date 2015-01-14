import os
import sys
import glob
import expressions

from qgis.core import QgsApplication 

def load_user_expressions(path):
    """
    Load all user expressions from the given paths
    """
    #Loop all py files and import them
    modules = glob.glob(path + "/*.py")
    names = [os.path.basename(f)[:-3] for f in modules]
    for name in names:
        if name == "__init__":
            continue
        # As user expression functions should be registed with qgsfunction
        # just importing the file is enough to get it to load the functions into QGIS
        __import__("expressions.{0}".format(name), locals(), globals())


userpythonhome = os.path.join(QgsApplication.qgisSettingsDirPath(), "python")
expressionspath = os.path.join(userpythonhome, "expressions")

# TODO Look for the startup.py and execfile it

expressions.load = load_user_expressions
expressions.load(expressionspath)

