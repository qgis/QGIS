# The following has been generated automatically from src/gui/symbology/qgscategorizedsymbolrendererwidget.h
QgsCategorizedSymbolRendererWidget.CustomRoles = QgsCategorizedSymbolRendererWidget.CustomRole
# monkey patching scoped based enum
QgsCategorizedSymbolRendererWidget.ValueRole = QgsCategorizedSymbolRendererWidget.CustomRole.Value
QgsCategorizedSymbolRendererWidget.CustomRoles.ValueRole = QgsCategorizedSymbolRendererWidget.CustomRole.Value
QgsCategorizedSymbolRendererWidget.ValueRole.is_monkey_patched = True
QgsCategorizedSymbolRendererWidget.ValueRole.__doc__ = "Category value"
QgsCategorizedSymbolRendererWidget.CustomRole.__doc__ = """Custom model roles.

.. note::

   Prior to QGIS 3.36 this was available as QgsCategorizedSymbolRendererWidget.CustomRoles

.. versionadded:: 3.36

* ``Value``: Category value

  Available as ``QgsCategorizedSymbolRendererWidget.ValueRole`` in older QGIS releases.


"""
# --
QgsCategorizedSymbolRendererWidget.CustomRole.baseClass = QgsCategorizedSymbolRendererWidget
try:
    QgsCategorizedSymbolRendererWidget.create = staticmethod(QgsCategorizedSymbolRendererWidget.create)
    QgsCategorizedSymbolRendererWidget.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
