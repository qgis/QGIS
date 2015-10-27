import os
import sys
import glob
import re
import fileinput
import traceback

from PyQt4.QtCore import QCoreApplication
from qgis.core import QgsApplication, QgsMessageLog, QgsExpression


def rename_builtin_func_if_exists(module_path):
    """
    Rename function in the user defined expression if built-in with
    the same name already exists
    """
    renamed = False
    f = fileinput.FileInput(module_path, inplace=True)
    for line in f:
        m = re.search(r'^def\s(.*)\(', line)
        name = m.group(1) if m else ''
        if name in QgsExpression.BuiltinFunctions():
            print line.replace(name, name + '_user').rstrip()
            renamed = True
        else:
            print line.rstrip()

    f.close()
    return renamed


def load_user_expressions(path):
    """
    Load all user expressions from the given paths
    """
    msg_title = QCoreApplication.translate("UserExpressions", "User expressions")
    # Loop all py files and import them
    modules = glob.glob(path + "/*.py")
    names = [os.path.basename(f)[:-3] for f in modules]
    for name in names:
        if name == "__init__":
            continue

        # Check if user expression contains built-in function and rename their before
        # importing the module
        has_builtin_function = rename_builtin_func_if_exists(os.path.join(expressionspath, name + '.py'))
        # As user expression functions should be registered with qgsfunction
        # just importing the file is enough to get it to load the functions into QGIS
        try:
            __import__("expressions.{0}".format(name), locals(), globals())
        except:
            error = traceback.format_exc()
            msg = QCoreApplication.translate("UserExpressions", "The user expression {0} is not valid").format(name)
            QgsMessageLog.logMessage(msg + "\n" + error, msg_title, QgsMessageLog.WARNING)

        if has_builtin_function:
            msg_builtin = QCoreApplication.translate("UserExpressions", "Some functions in user-defined expressions '{0}' "
                                                                        "have been renamed by adding the '_user' suffix "
                                                                        "because they have the same name of the built-in ones.").format(name)
            QgsMessageLog.logMessage(msg_builtin, msg_title, QgsMessageLog.WARNING)


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
    # But for now it's not the end of the world if it doesn't load the first time
    pass
