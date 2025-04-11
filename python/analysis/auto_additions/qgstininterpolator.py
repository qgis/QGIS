# The following has been generated automatically from src/analysis/interpolation/qgstininterpolator.h
# monkey patching scoped based enum
QgsTinInterpolator.TinInterpolation.Linear.__doc__ = "Linear interpolation"
QgsTinInterpolator.TinInterpolation.CloughTocher.__doc__ = "Clough-Tocher interpolation"
QgsTinInterpolator.TinInterpolation.__doc__ = """Indicates the type of interpolation to be performed

* ``Linear``: Linear interpolation
* ``CloughTocher``: Clough-Tocher interpolation

"""
# --
try:
    QgsTinInterpolator.triangulationFields = staticmethod(QgsTinInterpolator.triangulationFields)
    QgsTinInterpolator.__overridden_methods__ = ['interpolatePoint']
    QgsTinInterpolator.__group__ = ['interpolation']
except (NameError, AttributeError):
    pass
