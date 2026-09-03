# The following has been generated automatically from src/core/vectorfield/qgsvectorfieldarrowsettings.h
# monkey patching scoped based enum
QgsVectorFieldArrowSettings.ArrowScalingMethod.MinMax.__doc__ = "Scale vector magnitude linearly to fit in range of vectorFilterMin() and vectorFilterMax()"
QgsVectorFieldArrowSettings.ArrowScalingMethod.Scaled.__doc__ = "Scale vector magnitude by factor scaleFactor()"
QgsVectorFieldArrowSettings.ArrowScalingMethod.Fixed.__doc__ = "Use fixed length fixedShaftLength() regardless of vector's magnitude"
QgsVectorFieldArrowSettings.ArrowScalingMethod.__doc__ = """Algorithm how to transform vector magnitude to length of arrow on the device in pixels

* ``MinMax``: Scale vector magnitude linearly to fit in range of vectorFilterMin() and vectorFilterMax()
* ``Scaled``: Scale vector magnitude by factor scaleFactor()
* ``Fixed``: Use fixed length fixedShaftLength() regardless of vector's magnitude

"""
# --
try:
    QgsVectorFieldArrowSettings.__group__ = ['vectorfield']
except (NameError, AttributeError):
    pass
