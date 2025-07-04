# The following has been generated automatically from src/core/effects/qgspainteffect.h
try:
    QgsDrawSourceEffect.create = staticmethod(QgsDrawSourceEffect.create)
    QgsDrawSourceEffect.__overridden_methods__ = ['flags', 'type', 'clone', 'properties', 'readProperties', 'draw']
    QgsDrawSourceEffect.__group__ = ['effects']
except (NameError, AttributeError):
    pass
try:
    QgsPaintEffect.__virtual_methods__ = ['flags', 'saveProperties', 'render', 'begin', 'end', 'boundingRect']
    QgsPaintEffect.__abstract_methods__ = ['type', 'clone', 'properties', 'readProperties', 'draw']
    QgsPaintEffect.__group__ = ['effects']
except (NameError, AttributeError):
    pass
try:
    QgsEffectPainter.__group__ = ['effects']
except (NameError, AttributeError):
    pass
