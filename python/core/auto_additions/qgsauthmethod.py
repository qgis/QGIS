# The following has been generated automatically from src/core/auth/qgsauthmethod.h
try:
    QgsAuthMethod.authMethodTag = staticmethod(QgsAuthMethod.authMethodTag)
    QgsAuthMethod.__virtual_methods__ = ['editWidget', 'updateNetworkRequest', 'updateNetworkReply', 'updateDataSourceUriItems', 'updateNetworkProxy']
    QgsAuthMethod.__abstract_methods__ = ['key', 'description', 'displayDescription', 'clearCachedConfig', 'updateMethodConfig']
    QgsAuthMethod.__group__ = ['auth']
except (NameError, AttributeError):
    pass
