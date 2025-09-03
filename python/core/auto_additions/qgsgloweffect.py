# The following has been generated automatically from src/core/effects/qgsgloweffect.h
try:
    QgsOuterGlowEffect.create = staticmethod(QgsOuterGlowEffect.create)
    QgsOuterGlowEffect.__overridden_methods__ = ['type', 'clone', 'shadeExterior']
    QgsOuterGlowEffect.__group__ = ['effects']
except (NameError, AttributeError):
    pass
try:
    QgsInnerGlowEffect.create = staticmethod(QgsInnerGlowEffect.create)
    QgsInnerGlowEffect.__overridden_methods__ = ['type', 'clone', 'shadeExterior']
    QgsInnerGlowEffect.__group__ = ['effects']
except (NameError, AttributeError):
    pass
try:
    QgsGlowEffect.__abstract_methods__ = ['shadeExterior']
    QgsGlowEffect.__overridden_methods__ = ['flags', 'properties', 'readProperties', 'boundingRect', 'draw']
    QgsGlowEffect.__group__ = ['effects']
except (NameError, AttributeError):
    pass
