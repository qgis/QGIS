# coding=utf-8
"""This module contains the base class of repository handler."""
try:
    from io import StringIO
except:
    from StringIO import StringIO

import logging

from six import add_metaclass
try:
    from ConfigParser import SafeConfigParser
    from urlparse import urlparse
    from qgis.core import QGis as Qgis
except ImportError:
    from configparser import SafeConfigParser
    from urllib.parse import urlparse
    from qgis.core import Qgis

from ext_libs.giturlparse import validate as git_validate
from resource_sharing.config import COLLECTION_NOT_INSTALLED_STATUS
from resource_sharing.exception import MetadataError
from resource_sharing.version_compare import isCompatible


LOGGER = logging.getLogger('QGIS Resources Sharing')


class RepositoryHandlerMeta(type):
    """Handler meta class definition."""
    def __init__(cls, name, bases, dct):
        if not hasattr(cls, 'registry'):
            # This is the base class.  Create an empty registry
            cls.registry = {}
        else:
            # This is a derived class.
            # Add the class if it's not disabled
            if not cls.IS_DISABLED:
                interface_id = name.lower()
                cls.registry[interface_id] = cls

        super(RepositoryHandlerMeta, cls).__init__(name, bases, dct)


@add_metaclass(RepositoryHandlerMeta)
class BaseRepositoryHandler(object):
    """Abstract class of handler."""

    METADATA_FILE = 'metadata.ini'
    IS_DISABLED = False

    def __init__(self, url):
        """Constructor of the base class."""
        self._url = None
        self._auth_cfg = None
        self._metadata = None
        self._parsed_url = None

        # Call proper setters here
        self.url = url

    def can_handle(self):
        """Checking if handler can handle this URL."""
        raise NotImplementedError

    @classmethod
    def get_handler(cls, url):
        """Get the right repository handler instance for given URL.

        :param url: The url of the repository
        :type url: str

        :return: The handler instance. None if no handler found.
        :rtype: BaseHandler, None
        """
        repo_handler = None
        for handler in cls.registry.values():
            handler_instance = handler(url)
            if handler_instance.can_handle():
                repo_handler = handler_instance
                break
        return repo_handler

    @property
    def url(self):
        """The URL to the repository.

        Example:
        - https://github.com/anitagraser/QGIS-style-repo-dummy.git
        - file://home/akbar/dev/qgis-style-repo-dummy
        """
        return self._url

    @url.setter
    def url(self, url):
        """Setter to the repository's URL."""
        self._url = url
        self._parsed_url = urlparse(url)

    @property
    def auth_cfg(self):
        """The authentication configuration id."""
        return self._auth_cfg

    @auth_cfg.setter
    def auth_cfg(self, auth_cfg):
        """Setter to the authentication configuration id."""
        self._auth_cfg = auth_cfg

    @property
    def is_git_repository(self):
        """Flag if a repository is a git repository."""
        return git_validate(self._url)

    @property
    def metadata_url(self):
        """Return the absolute URL to the metadata."""
        return self.file_url(self.METADATA_FILE)

    @property
    def metadata(self):
        """Metadata content."""
        return self._metadata

    @metadata.setter
    def metadata(self, metadata):
        self._metadata = metadata

    def fetch_metadata(self):
        """Fetch the content of the metadata."""
        raise NotImplementedError

    def parse_metadata(self):
        """Parse str metadata to collection dict."""
        if not self.metadata:
            msg = 'The metadata content is None'
            LOGGER.error(msg)
            raise MetadataError(msg)

        collections = []

        try:  # Py3/Qt5
            metadata_file = StringIO(self.metadata)
        except:  # Py2/Q4
            metadata_file = StringIO(bytes(self.metadata).decode('utf-8'))

        try:
            parser = SafeConfigParser()
            parser.readfp(metadata_file)
            collections_str = parser.get('general', 'collections')
        except Exception as e:
            raise MetadataError('Error parsing metadata: %s' % e)

        collection_list = [
            collection.strip() for collection in collections_str.split(',')]
        # Read all the collections
        for collection in collection_list:
            # Parse the version
            qgis_min_version = parser.has_option(
                collection, 'qgis_minimum_version') and parser.get(
                collection, 'qgis_minimum_version') or None
            qgis_max_version = parser.has_option(
                collection, 'qgis_maximum_version') and parser.get(
                collection, 'qgis_maximum_version') or None
            if not qgis_min_version:
                qgis_min_version = '2.0'
            if not qgis_max_version:
                qgis_max_version = '3.99'
            if not isCompatible(
                    Qgis.QGIS_VERSION, qgis_min_version, qgis_max_version):
                LOGGER.info(
                    'Collection %s is not compatible with current QGIS '
                    'version. QGIS ver:%s, QGIS min ver:%s, QGIS max ver: '
                    '%s' % (
                        collection, Qgis.QGIS_VERSION, qgis_min_version,
                        qgis_max_version))
                break

            # Collection is compatible, continue parsing
            try:
                # Parse general information
                author = parser.get(collection, 'author')
                email = parser.get(collection, 'email')
                name = parser.get(collection, 'name')
                tags = parser.get(collection, 'tags')
                description = parser.get(collection, 'description')

                # Parse licensing stuffs
                license_str = parser.has_option(
                    collection, 'license') and parser.get(
                    collection, 'license') or None
                license_path = parser.has_option(
                    collection, 'license_file') and parser.get(
                    collection, 'license_file') or None
                license_url = None
                if license_path:
                    license_url = self.collection_file_url(
                        collection,
                        license_path.strip()
                    )

                # Parse the preview urls
                preview_str = parser.has_option(collection, 'preview') and \
                    parser.get(collection, 'preview') or ''
                preview_list = []
                for preview in preview_str.split(','):
                    if preview.strip() != '':
                        preview_url = self.collection_file_url(
                            collection,
                            preview.strip()
                        )
                        preview_list.append(preview_url)

            except Exception as e:
                raise MetadataError('Error parsing metadata: %s' % e)

            collection_dict = {
                'register_name': collection,
                'author': author,
                'author_email': email,
                'repository_url': self.url,
                'status': COLLECTION_NOT_INSTALLED_STATUS,
                'name': name,
                'tags': tags,
                'description': description,
                'qgis_min_version': qgis_min_version,
                'qgis_max_version': qgis_max_version,
                'preview': preview_list,
                'license': license_str,
                'license_url': license_url
            }
            collections.append(collection_dict)

        return collections

    def download_collection(self, id, register_name):
        """Download a collection given its ID.

        :param id: The ID of the collection.
        :type id: str

        :param register_name: The register name of the collection (the
            section name of the collection)
        :type register_name: str
        """
        raise NotImplementedError

    def file_url(self, relative_path):
        """Return the URL to a path given the relative path to repository root.

        This depends on the repository type so it's implemented in each
        of the concrete repository handler classes.

        :param relative_path: The relative path to the root of the repository.
        :type relative_path: str

        :return: The absolute URL to the file.
        :rtype: str
        """
        raise NotImplementedError

    def collection_file_url(self, collection_name, file_path):
        """Return the URL of a file relative the collection root

        ..e.g If it's file repository, calling
            self.collection_file_url('test_collection', 'preview/prev1.png')
            will return file:///<the_repository_path>/collections
            /test_collection/preview/prev1.png

        :param collection_name: The register name of the collection.
        :type collection_name: str

        :param file_path: The file path relative to the collection root.
        :type file_path: str
        """
        rel_path = 'collections/%s/%s' % (collection_name, file_path)
        return self.file_url(rel_path)
