# import qgis libs so that ve set the correct sip api version
import qgis   # pylint: disable=W0611  # NOQA
import sys
import os
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), os.path.pardir, 'ext_libs')))
