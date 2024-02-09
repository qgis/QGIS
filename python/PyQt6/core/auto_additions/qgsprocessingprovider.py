# The following has been generated automatically from src/core/processing/qgsprocessingprovider.h
QgsProcessingProvider.FlagDeemphasiseSearchResults = QgsProcessingProvider.Flag.FlagDeemphasiseSearchResults
QgsProcessingProvider.FlagCompatibleWithVirtualRaster = QgsProcessingProvider.Flag.FlagCompatibleWithVirtualRaster
QgsProcessingProvider.Flags = lambda flags=0: QgsProcessingProvider.Flag(flags)
def _force_int(v): return v if isinstance(v, int) else int(v.value)


QgsProcessingProvider.Flag.__bool__ = lambda flag: bool(_force_int(flag))
QgsProcessingProvider.Flag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsProcessingProvider.Flag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsProcessingProvider.Flag.__or__ = lambda flag1, flag2: QgsProcessingProvider.Flag(_force_int(flag1) | _force_int(flag2))
