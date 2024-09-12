# The following has been generated automatically from src/gui/qgssourceselectprovider.h
QgsSourceSelectProvider.OrderLocalProvider = QgsSourceSelectProvider.Ordering.OrderLocalProvider
QgsSourceSelectProvider.OrderDatabaseProvider = QgsSourceSelectProvider.Ordering.OrderDatabaseProvider
QgsSourceSelectProvider.OrderRemoteProvider = QgsSourceSelectProvider.Ordering.OrderRemoteProvider
QgsSourceSelectProvider.OrderSearchProvider = QgsSourceSelectProvider.Ordering.OrderSearchProvider
QgsSourceSelectProvider.OrderOtherProvider = QgsSourceSelectProvider.Ordering.OrderOtherProvider
# monkey patching scoped based enum
QgsSourceSelectProvider.Capability.NoCapabilities.__doc__ = "No capabilities"
QgsSourceSelectProvider.Capability.ConfigureFromUri.__doc__ = "The source select widget can be configured from a URI"
QgsSourceSelectProvider.Capability.__doc__ = """The Capability enum describes the capabilities of the source select implementation.

.. versionadded:: 3.38

* ``NoCapabilities``: No capabilities
* ``ConfigureFromUri``: The source select widget can be configured from a URI

"""
# --
QgsSourceSelectProvider.Capability.baseClass = QgsSourceSelectProvider
QgsSourceSelectProvider.Capabilities = lambda flags=0: QgsSourceSelectProvider.Capability(flags)
QgsSourceSelectProvider.Capabilities.baseClass = QgsSourceSelectProvider
Capabilities = QgsSourceSelectProvider  # dirty hack since SIP seems to introduce the flags in module
