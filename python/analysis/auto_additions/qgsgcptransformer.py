# The following has been generated automatically from src/analysis/georeferencing/qgsgcptransformer.h
# monkey patching scoped based enum
QgsGcpTransformerInterface.TransformMethod.Linear.__doc__ = "Linear transform"
QgsGcpTransformerInterface.TransformMethod.Helmert.__doc__ = "Helmert transform"
QgsGcpTransformerInterface.TransformMethod.PolynomialOrder1.__doc__ = "Polynomial order 1"
QgsGcpTransformerInterface.TransformMethod.PolynomialOrder2.__doc__ = "Polyonmial order 2"
QgsGcpTransformerInterface.TransformMethod.PolynomialOrder3.__doc__ = "Polynomial order"
QgsGcpTransformerInterface.TransformMethod.ThinPlateSpline.__doc__ = "Thin plate splines"
QgsGcpTransformerInterface.TransformMethod.Projective.__doc__ = "Projective"
QgsGcpTransformerInterface.TransformMethod.InvalidTransform.__doc__ = "Invalid transform"
QgsGcpTransformerInterface.TransformMethod.__doc__ = """Available transformation methods.

* ``Linear``: Linear transform
* ``Helmert``: Helmert transform
* ``PolynomialOrder1``: Polynomial order 1
* ``PolynomialOrder2``: Polyonmial order 2
* ``PolynomialOrder3``: Polynomial order
* ``ThinPlateSpline``: Thin plate splines
* ``Projective``: Projective
* ``InvalidTransform``: Invalid transform

"""
# --
QgsGcpTransformerInterface.TransformMethod.baseClass = QgsGcpTransformerInterface
try:
    QgsGcpTransformerInterface.methodToString = staticmethod(QgsGcpTransformerInterface.methodToString)
    QgsGcpTransformerInterface.create = staticmethod(QgsGcpTransformerInterface.create)
    QgsGcpTransformerInterface.createFromParameters = staticmethod(QgsGcpTransformerInterface.createFromParameters)
    QgsGcpTransformerInterface.__group__ = ['georeferencing']
except (NameError, AttributeError):
    pass
