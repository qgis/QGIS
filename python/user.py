import os
import sys
import glob
import traceback

from PyQt4.QtCore import QCoreApplication
from qgis.core import QgsApplication, QgsMessageLog


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
        # As user expression functions should be registered with qgsfunction
        # just importing the file is enough to get it to load the functions into QGIS
        try:
            __import__("expressions.{0}".format(name), locals(), globals())
        except:
            error = traceback.format_exc()
            msgtitle = QCoreApplication.translate("UserExpressions", "User expressions")
            msg = QCoreApplication.translate("UserExpressions", "The user expression {0} is not valid").format(name)
            QgsMessageLog.logMessage(msg + "\n" + error, msgtitle, QgsMessageLog.WARNING)


userpythonhome = os.path.join(QgsApplication.qgisSettingsDirPath(), "python")
expressionspath = os.path.join(userpythonhome, "expressions")
startuppy = os.path.join(userpythonhome, "startup.py")

sys.path.append(userpythonhome)

# exec startup script
if os.path.exists(startuppy):
    exec(compile(open(startuppy).read(), startuppy, 'exec'), locals(), globals())

if not os.path.exists(expressionspath):
    os.makedirs(expressionspath)

initfile = os.path.join(expressionspath, "__init__.py")
if not os.path.exists(initfile):
    open(initfile, "w").close()

template = """\"\"\"
Define new functions using @qgsfunction. feature and parent must always be the
last args. Use args=-1 to pass a list of values as arguments
\"\"\"

from qgis.core import *
from qgis.gui import *

@qgsfunction(args='auto', group='Custom')
def func(value1, feature, parent):
    return value1
"""


try:
    import expressions

    expressions.load = load_user_expressions
    expressions.load(expressionspath)
    expressions.template = template
except ImportError:
    # We get a import error and crash for some reason even if we make the expressions package
    # TODO Fix the crash on first load with no expressions folder
    # But for now it's not the end of the world if it doesn't laod the first time
    pass
