# The following has been generated automatically from src/core/effects/qgsblureffect.h
QgsBlurEffect.StackBlur = QgsBlurEffect.BlurMethod.StackBlur
QgsBlurEffect.GaussianBlur = QgsBlurEffect.BlurMethod.GaussianBlur
try:
    QgsBlurEffect.create = staticmethod(QgsBlurEffect.create)
    QgsBlurEffect.__overridden_methods__ = ['type', 'properties', 'readProperties', 'clone', 'draw', 'boundingRect']
    QgsBlurEffect.__group__ = ['effects']
except (NameError, AttributeError):
    pass
