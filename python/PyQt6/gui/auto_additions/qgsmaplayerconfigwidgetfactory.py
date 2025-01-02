# The following has been generated automatically from src/gui/qgsmaplayerconfigwidgetfactory.h
# monkey patching scoped based enum
QgsMapLayerConfigWidgetFactory.ParentPage.NoParent.__doc__ = "Factory creates pages itself, not sub-components"
QgsMapLayerConfigWidgetFactory.ParentPage.Temporal.__doc__ = "Factory creates sub-components of the temporal properties page (only supported for raster layer temporal properties)"
QgsMapLayerConfigWidgetFactory.ParentPage.__doc__ = """Available parent pages, for factories which create a widget which is a sub-component
of a standard page.

.. versionadded:: 3.20

* ``NoParent``: Factory creates pages itself, not sub-components
* ``Temporal``: Factory creates sub-components of the temporal properties page (only supported for raster layer temporal properties)

"""
# --
