# The following has been generated automatically from src/core/elevation/qgsterrainprovider.h
try:
    QgsAbstractTerrainProvider.__virtual_methods__ = ['resolveReferences']
    QgsAbstractTerrainProvider.__abstract_methods__ = ['equals', 'readXml', 'writeXml', 'type', 'clone', 'prepare', 'crs', 'heightAt']
    QgsAbstractTerrainProvider.__group__ = ['elevation']
except (NameError, AttributeError):
    pass
try:
    QgsFlatTerrainProvider.__overridden_methods__ = ['type', 'readXml', 'writeXml', 'crs', 'heightAt', 'clone', 'prepare', 'equals']
    QgsFlatTerrainProvider.__group__ = ['elevation']
except (NameError, AttributeError):
    pass
try:
    QgsRasterDemTerrainProvider.__overridden_methods__ = ['type', 'resolveReferences', 'readXml', 'writeXml', 'crs', 'heightAt', 'clone', 'equals', 'prepare']
    QgsRasterDemTerrainProvider.__group__ = ['elevation']
except (NameError, AttributeError):
    pass
try:
    QgsMeshTerrainProvider.__overridden_methods__ = ['type', 'resolveReferences', 'readXml', 'writeXml', 'crs', 'heightAt', 'clone', 'equals', 'prepare']
    QgsMeshTerrainProvider.__group__ = ['elevation']
except (NameError, AttributeError):
    pass
