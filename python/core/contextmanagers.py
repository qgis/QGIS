import sys
from contextlib import contextmanager
from qgis.core import QgsApplication


@contextmanager
def qgisapp(args=None, guienabled=True, configpath=None, sysexit=True):
    """
    Create a new QGIS Qt application.

    You should use this before creating any Qt widgets or QGIS objects for
    your custom QGIS based application.

    usage:
        from qgis.core.contextmanagers import qgisapp

        def main(app):
            # Run your main code block

            with qgisapp(sys.argv) as app:
                main(app)

    args - args passed to the underlying QApplication.
    guienabled - True by default will create a QApplication with a GUI. Pass
                 False if you wish to create no GUI based app, e.g a server app.
    configpath - Custom config path QGIS will use to load settings.
    sysexit - Call sys.exit on app exit. True by default.
    """
    if not args:
        args = []
    app = QgsApplication(args, guienabled, configpath)
    QgsApplication.initQgis()
    yield app
    if guienabled:
        exitcode = app.exec_()
    else:
        exitcode = 0
    QgsApplication.exitQgis()
    if sysexit:
        sys.exit(exitcode)
