import os

if 'PROJ_DIR' in os.environ:
    pyproj_datadir = os.environ['PROJ_DIR']
else:
    pyproj_datadir = os.sep.join([os.path.dirname(__file__), 'data'])
