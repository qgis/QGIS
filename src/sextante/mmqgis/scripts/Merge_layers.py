##layers=multiple vector
##result=output vector
from sextante.mmqgis import sextante_mmqgis_library as sxt_mmqgis
from sextante.mmqgis.DummyInterface import DummyInterface
from sextante.core.QGisLayers import QGisLayers
names=[]
for layer in layers:
	names.append(QGisLayers.getObjectFromUri(layer).name())
sxt_mmqgis.mmqgis_merge(DummyInterface(), names, result)

