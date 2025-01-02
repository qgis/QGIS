# The following has been generated automatically from src/core/effects/qgspainteffect.h
QgsPaintEffect.Modifier = QgsPaintEffect.DrawMode.Modifier
QgsPaintEffect.Render = QgsPaintEffect.DrawMode.Render
QgsPaintEffect.ModifyAndRender = QgsPaintEffect.DrawMode.ModifyAndRender
try:
    QgsDrawSourceEffect.create = staticmethod(QgsDrawSourceEffect.create)
    QgsDrawSourceEffect.__group__ = ['effects']
except (NameError, AttributeError):
    pass
try:
    QgsPaintEffect.__group__ = ['effects']
except (NameError, AttributeError):
    pass
try:
    QgsEffectPainter.__group__ = ['effects']
except (NameError, AttributeError):
    pass
