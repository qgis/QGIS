# The following has been generated automatically from src/core/qgsproperty.h
QgsPropertyDefinition.Boolean = QgsPropertyDefinition.StandardPropertyTemplate.Boolean
QgsPropertyDefinition.Integer = QgsPropertyDefinition.StandardPropertyTemplate.Integer
QgsPropertyDefinition.IntegerPositive = QgsPropertyDefinition.StandardPropertyTemplate.IntegerPositive
QgsPropertyDefinition.IntegerPositiveGreaterZero = QgsPropertyDefinition.StandardPropertyTemplate.IntegerPositiveGreaterZero
QgsPropertyDefinition.Double = QgsPropertyDefinition.StandardPropertyTemplate.Double
QgsPropertyDefinition.DoublePositive = QgsPropertyDefinition.StandardPropertyTemplate.DoublePositive
QgsPropertyDefinition.Double0To1 = QgsPropertyDefinition.StandardPropertyTemplate.Double0To1
QgsPropertyDefinition.Rotation = QgsPropertyDefinition.StandardPropertyTemplate.Rotation
QgsPropertyDefinition.String = QgsPropertyDefinition.StandardPropertyTemplate.String
QgsPropertyDefinition.Opacity = QgsPropertyDefinition.StandardPropertyTemplate.Opacity
QgsPropertyDefinition.RenderUnits = QgsPropertyDefinition.StandardPropertyTemplate.RenderUnits
QgsPropertyDefinition.ColorWithAlpha = QgsPropertyDefinition.StandardPropertyTemplate.ColorWithAlpha
QgsPropertyDefinition.ColorNoAlpha = QgsPropertyDefinition.StandardPropertyTemplate.ColorNoAlpha
QgsPropertyDefinition.PenJoinStyle = QgsPropertyDefinition.StandardPropertyTemplate.PenJoinStyle
QgsPropertyDefinition.BlendMode = QgsPropertyDefinition.StandardPropertyTemplate.BlendMode
QgsPropertyDefinition.Point = QgsPropertyDefinition.StandardPropertyTemplate.Point
QgsPropertyDefinition.Size = QgsPropertyDefinition.StandardPropertyTemplate.Size
QgsPropertyDefinition.Size2D = QgsPropertyDefinition.StandardPropertyTemplate.Size2D
QgsPropertyDefinition.LineStyle = QgsPropertyDefinition.StandardPropertyTemplate.LineStyle
QgsPropertyDefinition.StrokeWidth = QgsPropertyDefinition.StandardPropertyTemplate.StrokeWidth
QgsPropertyDefinition.FillStyle = QgsPropertyDefinition.StandardPropertyTemplate.FillStyle
QgsPropertyDefinition.CapStyle = QgsPropertyDefinition.StandardPropertyTemplate.CapStyle
QgsPropertyDefinition.HorizontalAnchor = QgsPropertyDefinition.StandardPropertyTemplate.HorizontalAnchor
QgsPropertyDefinition.VerticalAnchor = QgsPropertyDefinition.StandardPropertyTemplate.VerticalAnchor
QgsPropertyDefinition.SvgPath = QgsPropertyDefinition.StandardPropertyTemplate.SvgPath
QgsPropertyDefinition.Offset = QgsPropertyDefinition.StandardPropertyTemplate.Offset
QgsPropertyDefinition.DateTime = QgsPropertyDefinition.StandardPropertyTemplate.DateTime
QgsPropertyDefinition.Custom = QgsPropertyDefinition.StandardPropertyTemplate.Custom
QgsPropertyDefinition.DataTypeString = QgsPropertyDefinition.DataType.DataTypeString
QgsPropertyDefinition.DataTypeNumeric = QgsPropertyDefinition.DataType.DataTypeNumeric
QgsPropertyDefinition.DataTypeBoolean = QgsPropertyDefinition.DataType.DataTypeBoolean
try:
    QgsProperty.propertyMapToVariantMap = staticmethod(QgsProperty.propertyMapToVariantMap)
    QgsProperty.variantMapToPropertyMap = staticmethod(QgsProperty.variantMapToPropertyMap)
    QgsProperty.fromExpression = staticmethod(QgsProperty.fromExpression)
    QgsProperty.fromField = staticmethod(QgsProperty.fromField)
    QgsProperty.fromValue = staticmethod(QgsProperty.fromValue)
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsProperty_setTransformer = QgsProperty.setTransformer
    def __QgsProperty_setTransformer_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsProperty_setTransformer(self, arg)
    QgsProperty.setTransformer = _functools.update_wrapper(__QgsProperty_setTransformer_wrapper, QgsProperty.setTransformer)

except (NameError, AttributeError):
    pass
