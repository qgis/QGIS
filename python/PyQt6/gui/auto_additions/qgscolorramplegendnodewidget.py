# The following has been generated automatically from src/gui/qgscolorramplegendnodewidget.h
# monkey patching scoped based enum
QgsColorRampLegendNodeWidget.Capability.Prefix.__doc__ = "Allow editing legend prefix"
QgsColorRampLegendNodeWidget.Capability.Suffix.__doc__ = "Allow editing legend suffix"
QgsColorRampLegendNodeWidget.Capability.NumberFormat.__doc__ = "Allow editing number format"
QgsColorRampLegendNodeWidget.Capability.DefaultMinimum.__doc__ = "Allow resetting minimum label to default"
QgsColorRampLegendNodeWidget.Capability.DefaultMaximum.__doc__ = "Allow resetting maximum label to default"
QgsColorRampLegendNodeWidget.Capability.AllCapabilities.__doc__ = "All capabilities"
QgsColorRampLegendNodeWidget.Capability.__doc__ = "Capabilities to expose in the widget.\n\n.. versionadded:: 3.38\n\n" + '* ``Prefix``: ' + QgsColorRampLegendNodeWidget.Capability.Prefix.__doc__ + '\n' + '* ``Suffix``: ' + QgsColorRampLegendNodeWidget.Capability.Suffix.__doc__ + '\n' + '* ``NumberFormat``: ' + QgsColorRampLegendNodeWidget.Capability.NumberFormat.__doc__ + '\n' + '* ``DefaultMinimum``: ' + QgsColorRampLegendNodeWidget.Capability.DefaultMinimum.__doc__ + '\n' + '* ``DefaultMaximum``: ' + QgsColorRampLegendNodeWidget.Capability.DefaultMaximum.__doc__ + '\n' + '* ``AllCapabilities``: ' + QgsColorRampLegendNodeWidget.Capability.AllCapabilities.__doc__
# --
QgsColorRampLegendNodeWidget.Capability.baseClass = QgsColorRampLegendNodeWidget
QgsColorRampLegendNodeWidget.Capabilities = lambda flags=0: QgsColorRampLegendNodeWidget.Capability(flags)
QgsColorRampLegendNodeWidget.Capabilities.baseClass = QgsColorRampLegendNodeWidget
Capabilities = QgsColorRampLegendNodeWidget  # dirty hack since SIP seems to introduce the flags in module
