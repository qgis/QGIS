from qgis.core import Qgis
from qgis.core import geoWkt
from qgis.core import projectScales
from qgis.core import geoProj4
from qgis.core import geoEpsgCrsAuthId
from qgis.core import geoNone

from qgis import core

Qgis.QGIS_VERSION = Qgis.version()
Qgis.QGIS_VERSION_INT = Qgis.versionInt()
Qgis.QGIS_VERSION_RELEASE_NAME = Qgis.releaseName()

core.GEOWKT = geoWkt()
core.PROJECT_SCALES = projectScales()
core.GEOPROJ4 = geoProj4()
core.GEO_EPSG_CRS_AUTHID = geoEpsgCrsAuthId()
core.GEO_NONE = geoNone()
