# The following has been generated automatically from src/core/textrenderer/qgstextblockformat.h
# monkey patching scoped based enum
QgsTextBlockFormat.BooleanValue.NotSet.__doc__ = "Property is not set"
QgsTextBlockFormat.BooleanValue.SetTrue.__doc__ = "Property is set and ``True``"
QgsTextBlockFormat.BooleanValue.SetFalse.__doc__ = "Property is set and ``False``"
QgsTextBlockFormat.BooleanValue.__doc__ = """Status values for boolean format properties

* ``NotSet``: Property is not set
* ``SetTrue``: Property is set and ``True``
* ``SetFalse``: Property is set and ``False``

"""
# --
try:
    QgsTextBlockFormat.__group__ = ['textrenderer']
except (NameError, AttributeError):
    pass
