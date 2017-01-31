# coding=utf-8
import hashlib
import os
import shutil
import logging
import traceback

from qgis.PyQt.QtCore import (
    pyqtSignal, QObject)

from resource_sharing import config
from resource_sharing.config import (
    COLLECTION_INSTALLED_STATUS, COLLECTION_NOT_INSTALLED_STATUS)
from resource_sharing.utilities import (
    local_collection_path,
    render_template,
    resources_path)
from resource_sharing.repository_handler import BaseRepositoryHandler
from resource_sharing.resource_handler import BaseResourceHandler

LOGGER = logging.getLogger('QGIS Resources Sharing')


class CollectionInstaller(QObject):
    finished = pyqtSignal()
    aborted = pyqtSignal()
    progress = pyqtSignal(basestring)

    def __init__(self, collection_manager, collection_id):
        QObject.__init__(self)
        self._collection_manager = collection_manager
        self._collection_id = collection_id
        self.install_status = False
        self.error_message = None
        self.killed = False

    def run(self):
        self.progress.emit('Downloading the collection...')

        # We can't really kill the process here, so let's finish it even when
        # user cancels the download process
        download_status, error_message = self._collection_manager.download(
            self._collection_id)

        # If at this point it's killed, let's abort and tell the main thread
        if self.killed:
            self.aborted.emit()
            return

        # If download fails
        if not download_status:
            self.install_status = False
            self.error_message = error_message
            self.finished.emit()
            return

        # Downloading is fine, It's not killed, let's install it
        if not self.killed:
            self.progress.emit('Installing the collection...')
            try:
                self._collection_manager.install(self._collection_id)
            except Exception as e:
                self.error_message = e
                LOGGER.exception(traceback.format_exc())
        else:
            # Downloaded but killed
            self.aborted.emit()
            return

        # If finished installing but killed here? just emit finished
        self.install_status = True
        self.finished.emit()

    def abort(self):
        self.killed = True


class CollectionManager(object):

    def __init__(self):
        """"Utilities class related to collection."""

    def get_collection_id(self, register_name, repo_url):
        """Generate id of a collection."""
        hash_object = hashlib.sha1((register_name + repo_url).encode('utf-8'))
        hex_dig = hash_object.hexdigest()
        return hex_dig

    def get_html(self, collection_id):
        """Return the detail of a collection in HTML form given the id.

        :param collection_id: The id of the collection
        :type collection_id: str
        """
        context = {
            'resources_path': resources_path(),
            'collection': config.COLLECTIONS[collection_id]
        }
        return render_template('collection_details.html', context)

    def get_installed_collections(self, repo_url=None):
        """Get all installed collections of a given repository url.

        If repository url is not specified, it will return all the installed
        collections.

        :param repo_url: The repository url.
        :type repo_url: str

        :return: Subset of config.COLLECTIONS that meet the requirement
        :rtype: dict
        """
        installed_collections = {}
        for id, collection in config.COLLECTIONS.iteritems():
            if collection['status'] != COLLECTION_INSTALLED_STATUS:
                continue

            if repo_url:
                if collection['repository_url'] != repo_url:
                    continue

            installed_collections[id] = collection

        return installed_collections

    def download(self, collection_id):
        """Download a collection given the id.

        :param collection_id: The id of the collection about to be downloaded.
        :type collection_id: str
        """
        repo_url = config.COLLECTIONS[collection_id]['repository_url']
        repo_handler = BaseRepositoryHandler.get_handler(repo_url)
        if repo_handler is None:
            message = 'There is no handler available for the given URL!'
            LOGGER.exception(message)
            raise Exception(message)
        register_name = config.COLLECTIONS[collection_id]['register_name']
        status, information = repo_handler.download_collection(
            collection_id, register_name)
        return status, information

    def install(self, collection_id):
        """Install a collection into QGIS.

        :param collection_id: The id of the collection about to be installed.
        :type collection_id: str
        """
        for resource_handler in BaseResourceHandler.registry.values():
            resource_handler_instance = resource_handler(collection_id)
            resource_handler_instance.install()

        config.COLLECTIONS[collection_id]['status'] = \
            COLLECTION_INSTALLED_STATUS

    def uninstall(self, collection_id):
        """Uninstall the collection from QGIS.

        :param collection_id: The id of the collection about to be uninstalled.
        :type collection_id: str
        """
        # Remove the collection directory
        collection_dir = local_collection_path(collection_id)
        if os.path.exists(collection_dir):
            shutil.rmtree(collection_dir)

        # Uninstall all type of resources from QGIS
        for resource_handler in BaseResourceHandler.registry.values():
            resource_handler_instance = resource_handler(collection_id)
            resource_handler_instance.uninstall()

        config.COLLECTIONS[collection_id]['status'] = \
            COLLECTION_NOT_INSTALLED_STATUS
