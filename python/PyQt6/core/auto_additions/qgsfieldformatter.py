# The following has been generated automatically from src/core/qgsfieldformatter.h
QgsFieldFormatter.CanProvideAvailableValues = QgsFieldFormatter.Flag.CanProvideAvailableValues
QgsFieldFormatter.Flags = lambda flags=0: QgsFieldFormatter.Flag(flags)
try:
    QgsFieldFormatter.__virtual_methods__ = ['representValue', 'sortValue', 'alignmentFlag', 'createCache', 'availableValues']
    QgsFieldFormatter.__abstract_methods__ = ['id']
except (NameError, AttributeError):
    pass
