# The following has been generated automatically from src/core/network/qgsblockingnetworkrequest.h
QgsBlockingNetworkRequest.NoError = QgsBlockingNetworkRequest.ErrorCode.NoError
QgsBlockingNetworkRequest.NetworkError = QgsBlockingNetworkRequest.ErrorCode.NetworkError
QgsBlockingNetworkRequest.TimeoutError = QgsBlockingNetworkRequest.ErrorCode.TimeoutError
QgsBlockingNetworkRequest.ServerExceptionError = QgsBlockingNetworkRequest.ErrorCode.ServerExceptionError
# monkey patching scoped based enum
QgsBlockingNetworkRequest.RequestFlag.EmptyResponseIsValid.__doc__ = "Do not generate an error if getting an empty response (e.g. HTTP 204)"
QgsBlockingNetworkRequest.RequestFlag.__doc__ = "Request flags\n\n.. versionadded:: 3.40\n\n" + '* ``EmptyResponseIsValid``: ' + QgsBlockingNetworkRequest.RequestFlag.EmptyResponseIsValid.__doc__
# --
QgsBlockingNetworkRequest.RequestFlag.baseClass = QgsBlockingNetworkRequest
QgsBlockingNetworkRequest.RequestFlags = lambda flags=0: QgsBlockingNetworkRequest.RequestFlag(flags)
QgsBlockingNetworkRequest.RequestFlags.baseClass = QgsBlockingNetworkRequest
RequestFlags = QgsBlockingNetworkRequest  # dirty hack since SIP seems to introduce the flags in module
