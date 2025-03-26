# The following has been generated automatically from src/core/qgscolorscheme.h
try:
    QgsRecentColorScheme.addRecentColor = staticmethod(QgsRecentColorScheme.addRecentColor)
    QgsRecentColorScheme.lastUsedColor = staticmethod(QgsRecentColorScheme.lastUsedColor)
    QgsRecentColorScheme.__overridden_methods__ = ['schemeName', 'flags', 'fetchColors', 'clone']
except (NameError, AttributeError):
    pass
try:
    QgsColorScheme.__virtual_methods__ = ['flags', 'fetchColors', 'isEditable', 'setColors']
    QgsColorScheme.__abstract_methods__ = ['schemeName', 'fetchColors', 'clone']
except (NameError, AttributeError):
    pass
try:
    QgsGplColorScheme.__abstract_methods__ = ['gplFilePath']
    QgsGplColorScheme.__overridden_methods__ = ['fetchColors', 'setColors']
except (NameError, AttributeError):
    pass
try:
    QgsUserColorScheme.__overridden_methods__ = ['schemeName', 'clone', 'isEditable', 'flags', 'gplFilePath']
except (NameError, AttributeError):
    pass
try:
    QgsCustomColorScheme.__overridden_methods__ = ['schemeName', 'flags', 'fetchColors', 'isEditable', 'setColors', 'clone']
except (NameError, AttributeError):
    pass
try:
    QgsProjectColorScheme.__overridden_methods__ = ['schemeName', 'flags', 'fetchColors', 'isEditable', 'setColors', 'clone']
except (NameError, AttributeError):
    pass
