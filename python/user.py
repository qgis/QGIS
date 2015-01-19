import os
import sys
import glob

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
startuppy = os.path.join(userpythonhome, "startup.py")

sys.path.append(userpythonhome)

# exec startup script
if os.path.exists(startuppy):
    execfile(startuppy, locals(), globals())

if not os.path.exists(expressionspath):
    os.makedirs(expressionspath)

initfile = os.path.join(expressionspath, "__init__.py")
if not os.path.exists(initfile):
    open(initfile, "w").close()

import expressions

expressions.load = load_user_expressions
expressions.load(expressionspath)

