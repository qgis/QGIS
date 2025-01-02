# The following has been generated automatically from src/core/textrenderer/qgstextcharacterformat.h
# monkey patching scoped based enum
QgsTextCharacterFormat.BooleanValue.NotSet.__doc__ = "Property is not set"
QgsTextCharacterFormat.BooleanValue.SetTrue.__doc__ = "Property is set and ``True``"
QgsTextCharacterFormat.BooleanValue.SetFalse.__doc__ = "Property is set and ``False``"
QgsTextCharacterFormat.BooleanValue.__doc__ = """Status values for boolean format properties

* ``NotSet``: Property is not set
* ``SetTrue``: Property is set and ``True``
* ``SetFalse``: Property is set and ``False``

"""
# --
try:
    QgsTextCharacterFormat.__group__ = ['textrenderer']
except (NameError, AttributeError):
    pass
