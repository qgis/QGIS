"""A custom storage for QgsAuthManager that stores the authentication
   configurations in memory.

   WARNING: For testing purposes only.

"""

import json
import uuid
from qgis.core import (
    Qgis,
    QgsAuthConfigurationStorage,
    QgsAuthMethodConfig,
    QgsAuthConfigSslServer,
    QgsAuthCertUtils,
)

from qgis.PyQt.QtNetwork import QSslCertificate
from qgis.PyQt.QtCore import QVariant


class QgsAuthConfigurationCustomStorage(QgsAuthConfigurationStorage):
    """Only for testing purposes: supports authentication
    configuration storage in memory"""

    def __init__(self, params: dict = {}):
        super().__init__(params)
        self.configs = {}
        self.payloads = {}
        self._initialized = False
        self._id = str(uuid.uuid4())
        self.is_encrypted = params.get("is_encrypted", "true") in [
            "true",
            "True",
            "1",
            "TRUE",
            "on",
            "ON",
            "YES",
            "yes",
        ]

    def settingsParams(self):
        param = QgsAuthConfigurationStorage.SettingParam(
            "is_encrypted", "Whether the storage is encrypted or not", QVariant.Bool
        )
        return [param]

    def isEncrypted(self):
        return self.is_encrypted

    def name(self):
        return "Custom"

    def description(self):
        return "Custom storage"

    def type(self):
        return "custom"

    def id(self):
        return self._id

    def capabilities(self):
        return (
            Qgis.AuthConfigurationStorageCapability.ReadConfiguration
            | Qgis.AuthConfigurationStorageCapability.DeleteConfiguration
            | Qgis.AuthConfigurationStorageCapability.CreateConfiguration
            | Qgis.AuthConfigurationStorageCapability.UpdateConfiguration
            | Qgis.AuthConfigurationStorageCapability.ClearStorage
        )

    def isReady(self):
        return self._initialized

    def initialize(self):
        self._initialized = True
        return True

    def authMethodConfigs(self, allowedMethods=[]):
        if len(allowedMethods) == 0:
            return self.configs.values()
        return [c for c in self.configs.values() if c.method() in allowedMethods]

    def methodConfigExists(self, id: str):
        return id in self.configs

    def storeMethodConfig(self, config: QgsAuthMethodConfig, payload: str):
        if not config.id():
            return False

        self.configs[config.id()] = config
        self.payloads[config.id()] = payload
        return True

    def removeMethodConfig(self, id: str):
        if id in self.configs:
            del self.configs[id]
            del self.payloads[id]
            return True
        return False

    def clearMethodConfigs(self):
        self.configs.clear()
        self.payloads.clear()
        return True

    def loadMethodConfig(self, id: str, full=False):
        if full:
            return self.configs.get(id, QgsAuthMethodConfig()), self.payloads[id]
        else:
            config = self.configs.get(id, QgsAuthMethodConfig())
            config.clearConfigMap()
        return self.configs.get(id, QgsAuthMethodConfig()), ""

    def authMethodConfigsWithPayload(self) -> str:
        configs = {}
        for id, config in self.configs.items():
            configs[id] = config
            configs[id].setConfig("encrypted_payload", self.payloads[id])
        return configs
