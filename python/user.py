import os
import sys
import glob

def startup(userpythonhome):
    """
    Run startup logic for this QGIS user
    """
    try:
        # Start up can be a startup.py or a startup package
        import startup
    except ImportError:
        pass 

    expressionspath = os.path.join(userpythonhome, "expressions")
    load_user_expressions([expressionspath])


def load_user_expressions(paths):
    """
    Load all user expressions from the given paths
    """
    #Loop all py files and import them
    for path in paths:
        sys.path.append(path)
        modules = glob.glob(path + "/*.py")
        names = [os.path.basename(f)[:-3] for f in modules]
        for name in names:
            if name == "__init__":
                continue
            # As user expression functions should be registed with qgsfunction
            # just importing the file is enough to get it to load the functions into QGIS
            __import__(name, locals(), globals())



