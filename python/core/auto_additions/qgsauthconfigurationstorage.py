# The following has been generated automatically from src/core/auth/qgsauthconfigurationstorage.h
try:
    QgsAuthConfigurationStorage.__attribute_docs__ = {'messageLog': 'Custom logging signal to relay to console output and :py:class:`QgsMessageLog`\n\n:param message: Message to send\n:param tag: Associated tag (title)\n:param level: Message log level\n\n.. seealso:: :py:class:`QgsMessageLog`\n', 'storageChanged': 'Emitted when the storage was updated.\n\n:param id: The storage id\n\n.. note::\n\n   This is a generic changed signal and it is normally\n   emitted together with the dedicated signals which are\n   provided for specific changes on the individual tables.\n', 'methodConfigChanged': 'Emitted when the storage method config table was changed.\n', 'masterPasswordChanged': 'Emitted when the storage master password table was changed.\n', 'authSettingsChanged': 'Emitted when the storage auth settings table was changed.\n', 'readOnlyChanged': 'Emitted when the storage read-only status was changed.\n', 'certIdentityChanged': 'Emitted when the storage cert identity table was changed.\n', 'certAuthorityChanged': 'Emitted when the storage cert authority table was changed.\n', 'sslCertCustomConfigChanged': 'Emitted when the storage ssl cert custom config table was changed.\n', 'sslCertTrustPolicyChanged': 'Emitted when the storage ssl cert trust policy table was changed.\n'}
    QgsAuthConfigurationStorage.__signal_arguments__ = {'messageLog': ['message: str', 'tag: str = QStringLiteral( "Authentication" )', 'level: Qgis.MessageLevel = Qgis.MessageLevel.Info'], 'storageChanged': ['id: str'], 'readOnlyChanged': ['readOnly: bool']}
    QgsAuthConfigurationStorage.__group__ = ['auth']
except (NameError, AttributeError):
    pass
try:
    QgsAuthConfigurationStorage.MasterPasswordConfig.__doc__ = """Structure that holds the (encrypted) master password elements."""
    QgsAuthConfigurationStorage.MasterPasswordConfig.__group__ = ['auth']
except (NameError, AttributeError):
    pass
try:
    QgsAuthConfigurationStorage.SettingParameter.__doc__ = """Storage configuration setting parameter."""
    QgsAuthConfigurationStorage.SettingParameter.__group__ = ['auth']
except (NameError, AttributeError):
    pass
