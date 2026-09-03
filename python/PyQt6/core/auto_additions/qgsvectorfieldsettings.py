# The following has been generated automatically from src/core/vectorfield/qgsvectorfieldsettings.h
# monkey patching scoped based enum
QgsVectorFieldSettings.Symbology.Arrows.__doc__ = "Displaying vector dataset with arrows"
QgsVectorFieldSettings.Symbology.Streamlines.__doc__ = "Displaying vector dataset with streamlines"
QgsVectorFieldSettings.Symbology.Traces.__doc__ = "Displaying vector dataset with particle traces"
QgsVectorFieldSettings.Symbology.WindBarbs.__doc__ = "Displaying vector dataset with wind barbs"
QgsVectorFieldSettings.Symbology.__doc__ = """Defines the symbology of vector rendering

.. versionadded:: 3.12

* ``Arrows``: Displaying vector dataset with arrows
* ``Streamlines``: Displaying vector dataset with streamlines
* ``Traces``: Displaying vector dataset with particle traces
* ``WindBarbs``: Displaying vector dataset with wind barbs

"""
# --
try:
    QgsVectorFieldSettings.__group__ = ['vectorfield']
except (NameError, AttributeError):
    pass
