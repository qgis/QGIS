##layers=multiple vector
##result=output vector
from sextante.mmqgis import mmqgis_library as mmqgis
from sextante.core.Sextante import Sextante
from sextante.mmqgis.DummyInterface import DummyInterface
from sextante.core.QGisLayers import QGisLayers
names=[]
for layer in layers:
	names.append(QGisLayers.getObjectFromUri(layer).name())
mmqgis.mmqgis_merge(DummyInterface(), names, result)

