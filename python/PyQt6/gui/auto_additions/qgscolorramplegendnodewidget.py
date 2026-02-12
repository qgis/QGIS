# The following has been generated automatically from src/gui/qgscolorramplegendnodewidget.h
# monkey patching scoped based enum
QgsColorRampLegendNodeWidget.Capability.Prefix.__doc__ = "Allow editing legend prefix"
QgsColorRampLegendNodeWidget.Capability.Suffix.__doc__ = "Allow editing legend suffix"
QgsColorRampLegendNodeWidget.Capability.NumberFormat.__doc__ = "Allow editing number format"
QgsColorRampLegendNodeWidget.Capability.DefaultMinimum.__doc__ = "Allow resetting minimum label to default"
QgsColorRampLegendNodeWidget.Capability.DefaultMaximum.__doc__ = "Allow resetting maximum label to default"
QgsColorRampLegendNodeWidget.Capability.AllCapabilities.__doc__ = "All capabilities"
QgsColorRampLegendNodeWidget.Capability.__doc__ = """Capabilities to expose in the widget.

.. versionadded:: 3.38

* ``Prefix``: Allow editing legend prefix
* ``Suffix``: Allow editing legend suffix
* ``NumberFormat``: Allow editing number format
* ``DefaultMinimum``: Allow resetting minimum label to default
* ``DefaultMaximum``: Allow resetting maximum label to default
* ``AllCapabilities``: All capabilities

"""
# --
QgsColorRampLegendNodeWidget.Capability.baseClass = QgsColorRampLegendNodeWidget
QgsColorRampLegendNodeWidget.Capabilities = lambda flags=0: QgsColorRampLegendNodeWidget.Capability(flags)
QgsColorRampLegendNodeWidget.Capabilities.baseClass = QgsColorRampLegendNodeWidget
Capabilities = QgsColorRampLegendNodeWidget  # dirty hack since SIP seems to introduce the flags in module
