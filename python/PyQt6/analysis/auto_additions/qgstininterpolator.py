# The following has been generated automatically from src/analysis/interpolation/qgstininterpolator.h
QgsTinInterpolator.Linear = QgsTinInterpolator.TinInterpolation.Linear
QgsTinInterpolator.CloughTocher = QgsTinInterpolator.TinInterpolation.CloughTocher
try:
    QgsTinInterpolator.triangulationFields = staticmethod(QgsTinInterpolator.triangulationFields)
    QgsTinInterpolator.__overridden_methods__ = ['interpolatePoint']
    QgsTinInterpolator.__group__ = ['interpolation']
except (NameError, AttributeError):
    pass
