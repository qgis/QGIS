# The following has been generated automatically from src/core/qgsfieldformatter.h
QgsFieldFormatter.CanProvideAvailableValues = QgsFieldFormatter.Flag.CanProvideAvailableValues
QgsFieldFormatter.Flags = lambda flags=0: QgsFieldFormatter.Flag(flags)
try:
    QgsFieldFormatter.__attribute_docs__ = {'NULL_VALUE': "Will be saved in the configuration when a value is NULL.\nIt's the magic UUID {2839923C-8B7D-419E-B84B-CA2FE9B80EC7}"}
    QgsFieldFormatter.__annotations__ = {'NULL_VALUE': str}
    QgsFieldFormatter.__virtual_methods__ = ['representValue', 'sortValue', 'alignmentFlag', 'createCache', 'availableValues']
    QgsFieldFormatter.__abstract_methods__ = ['id']
except (NameError, AttributeError):
    pass
