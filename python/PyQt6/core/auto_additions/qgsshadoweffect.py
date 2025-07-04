# The following has been generated automatically from src/core/effects/qgsshadoweffect.h
try:
    QgsDropShadowEffect.create = staticmethod(QgsDropShadowEffect.create)
    QgsDropShadowEffect.__overridden_methods__ = ['type', 'clone', 'exteriorShadow']
    QgsDropShadowEffect.__group__ = ['effects']
except (NameError, AttributeError):
    pass
try:
    QgsInnerShadowEffect.create = staticmethod(QgsInnerShadowEffect.create)
    QgsInnerShadowEffect.__overridden_methods__ = ['type', 'clone', 'exteriorShadow']
    QgsInnerShadowEffect.__group__ = ['effects']
except (NameError, AttributeError):
    pass
try:
    QgsShadowEffect.__abstract_methods__ = ['exteriorShadow']
    QgsShadowEffect.__overridden_methods__ = ['flags', 'properties', 'readProperties', 'boundingRect', 'draw']
    QgsShadowEffect.__group__ = ['effects']
except (NameError, AttributeError):
    pass
