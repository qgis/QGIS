# The following has been generated automatically from src/core/layout/qgslayoutmanagermodel.h
QgsLayoutManagerModel.Role = QgsLayoutManagerModel.CustomRole
# monkey patching scoped based enum
QgsLayoutManagerModel.LayoutRole = QgsLayoutManagerModel.CustomRole.Layout
QgsLayoutManagerModel.Role.LayoutRole = QgsLayoutManagerModel.CustomRole.Layout
QgsLayoutManagerModel.LayoutRole.is_monkey_patched = True
QgsLayoutManagerModel.LayoutRole.__doc__ = "Layout object"
QgsLayoutManagerModel.CustomRole.__doc__ = """Custom model roles.

.. note::

   Prior to QGIS 3.36 this was available as QgsLayoutManagerModel.Role

.. versionadded:: 3.36

* ``Layout``: Layout object

  Available as ``QgsLayoutManagerModel.LayoutRole`` in older QGIS releases.


"""
# --
QgsLayoutManagerModel.CustomRole.baseClass = QgsLayoutManagerModel
QgsLayoutManagerProxyModel.Filters.baseClass = QgsLayoutManagerProxyModel
Filters = QgsLayoutManagerProxyModel  # dirty hack since SIP seems to introduce the flags in module
try:
    QgsLayoutManagerProxyModel.__overridden_methods__ = ['filterAcceptsRowInternal']
    QgsLayoutManagerProxyModel.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutManagerModel.__group__ = ['layout']
except (NameError, AttributeError):
    pass
