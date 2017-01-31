# coding=utf-8
import logging

try:
    from urlparse import urljoin
except ImportError:
    from urllib.parse import urljoin

from zipfile import ZipFile

from qgis.PyQt.QtCore import QTemporaryFile

from resource_sharing.repository_handler.base import BaseRepositoryHandler
from resource_sharing.utilities import local_collection_path
from resource_sharing.network_manager import NetworkManager


LOGGER = logging.getLogger('QGIS Resources Sharing')


class RemoteZipHandler(BaseRepositoryHandler):
    """Class to handle remote zip repository."""
    IS_DISABLED = False

    def __init__(self, url):
        """Constructor."""
        BaseRepositoryHandler.__init__(self, url)

    def can_handle(self):
        if not self.is_git_repository:
            if self._parsed_url.scheme in ['http', 'https']:
                return True
        return False

    def fetch_metadata(self):
        """Fetch metadata file from the url."""
        # Download the metadata
        network_manager = NetworkManager(self.metadata_url, self.auth_cfg)
        status, description = network_manager.fetch()
        if status:
            self.metadata = network_manager.content
        return status, description

    def download_collection(self, id, register_name):
        """Download a collection given its ID.

        For zip collection, we will download the zip, and extract the
        collection to collections dir.

        :param id: The ID of the collection.
        :type id: str

        :param register_name: The register name of the collection (the
            section name of the collection)
        :type register_name: unicode
        """
        # Download the zip first
        collection_path = 'collections/%s.zip' % register_name
        network_manager = NetworkManager(self.file_url(collection_path))
        status, description = network_manager.fetch()

        if not status:
            return False, description

        # Create the zip file
        zip_file = QTemporaryFile()
        if zip_file.open():
            zip_file.write(network_manager.content)
            zip_file.close()

        zf = ZipFile(zip_file.fileName())
        zf.extractall(path=local_collection_path(id))
        return True, None

    def file_url(self, relative_path):
        return urljoin(self.url, relative_path)
