# coding=utf-8
import csv
import os
import pickle

from qgis.PyQt.QtCore import QObject, QSettings, QTemporaryFile

from resource_sharing.utilities import (
    repo_settings_group, local_collection_path, repositories_cache_path)
from resource_sharing.repository_handler import BaseRepositoryHandler
from resource_sharing.network_manager import NetworkManager
from resource_sharing.collection_manager import CollectionManager
from resource_sharing.config import COLLECTION_INSTALLED_STATUS
from resource_sharing import config
from resource_sharing.exception import MetadataError


class RepositoryManager(QObject):
    """Class to handle repositories."""

    DIRECTORY_URL = ('https://raw.githubusercontent.com/qgis/'
                     'QGIS-Resources/master/directory.csv')

    def __init__(self):
        """Constructor.

        ..note:
        - Directories is a list of repository that are registered in user's
        QGIS. Data structure of directories:
        self._directories = {
            'QGIS Official Repository': {
                'url': 'git@github.com:anitagraser/QGIS-style-repo-dummy.git',
                'auth_cfg': '0193jkad'
             }
        }

        - Repositories is a dictionary of repository with all the collections
        contained in that repository. Data structure of repositories:
        self._repositories = {
            repo_name: [{
                'register_name': collection,
                'author': author,
                'author_email': email,
                'repository_url': self.url,
                'status': COLLECTION_NOT_INSTALLED_STATUS,
                'name': parser.get(collection, 'name'),
                'tags': parser.get(collection, 'tags'),
                'description': parser.get(collection, 'description'),
                'qgis_min_version': '2.0',
                'qgis_max_version': '2.99'
                'preview': ['preview/image1.png', 'preview/image2.png']
            },
            .... //other collections from this repository
            ],
            ... //other repository
        }
        """
        QObject.__init__(self)
        # Online directories from the DIRECTORY_URL
        self._online_directories = {}
        # Registered directories
        self._directories = {}
        # Registered repositories
        self._repositories = {}
        # Collection manager instance to deal with collections
        self._collections_manager = CollectionManager()
        # Fetch online directories
        self.fetch_online_directories()
        # Load directory of repositories from settings
        self.load_directories()
        # Load repositories from cache
        self.load_repositories()

    @property
    def directories(self):
        """Directories contains all the repositories (name and URL)
        registered in setting.

        :returns: Dictionary of repositories registered
        :rtype: dict
        """
        return self._directories

    def fetch_online_directories(self):
        """Fetch online directory of repositories."""
        downloader = NetworkManager(self.DIRECTORY_URL)
        status, _ = downloader.fetch()
        if status:
            directory_file = QTemporaryFile()
            if directory_file.open():
                directory_file.write(downloader.content)
                directory_file.close()

            with open(directory_file.fileName()) as csv_file:
                reader = csv.DictReader(csv_file, fieldnames=('name', 'url'))
                for row in reader:
                    self._online_directories[row['name']] = row['url'].strip()
            # Save it to cache
            settings = QSettings()
            settings.beginGroup(repo_settings_group())
            settings.setValue('online_directories', self._online_directories)
            settings.endGroup()
        else:
            # Just use cache from previous use
            settings = QSettings()
            settings.beginGroup(repo_settings_group())
            self._online_directories = settings.value('online_directories', {})
            settings.endGroup()

    def load_directories(self):
        """Load directories of repository registered in settings."""
        self._directories = {}
        settings = QSettings()
        settings.beginGroup(repo_settings_group())

        # Write online directory first to QSettings if needed
        for online_dir_name in self._online_directories:
            repo_present = False
            for repo_name in settings.childGroups():
                url = settings.value(repo_name + '/url', '', type=unicode)
                if url == self._online_directories[online_dir_name]:
                    repo_present = True
                    break
            if not repo_present:
                self.add_directory(
                    online_dir_name, self._online_directories[online_dir_name])

        for repo_name in settings.childGroups():
            self._directories[repo_name] = {}
            url = settings.value(
                repo_name + '/url', '', type=unicode)
            self._directories[repo_name]['url'] = url
            auth_cfg = settings.value(
                repo_name + '/auth_cfg', '', type=unicode).strip()
            self._directories[repo_name]['auth_cfg'] = auth_cfg
        settings.endGroup()

    def add_directory(self, repo_name, url, auth_cfg=None):
        """Add a directory to settings and add the collections from that repo.

        :param repo_name: The name of the repository
        :type repo_name: str

        :param url: The URL of the repository
        :type url: str
        """
        repo_handler = BaseRepositoryHandler.get_handler(url)
        if repo_handler is None:
            raise Exception('There is no handler available for the given URL!')

        if auth_cfg:
            repo_handler.auth_cfg = auth_cfg

        # Fetch metadata
        status, description = repo_handler.fetch_metadata()
        if status:
            # Parse metadata
            try:
                collections = repo_handler.parse_metadata()
            except MetadataError:
                raise
            # Add the repo and the collections
            self._repositories[repo_name] = collections
            self.rebuild_collections()
            # Add to QSettings
            settings = QSettings()
            settings.beginGroup(repo_settings_group())
            settings.setValue(repo_name + '/url', url)
            if auth_cfg:
                settings.setValue(repo_name + '/auth_cfg', auth_cfg)
            settings.endGroup()
            # Serialize repositories every time we successfully added a repo
            self.serialize_repositories()

        return status, description

    def edit_directory(
            self,
            old_repo_name,
            new_repo_name,
            old_url,
            new_url,
            new_auth_cfg):
        """Edit a directory and update the collections.

        :param old_repo_name: The old name of the repository
        :type old_repo_name: str

        :param new_repo_name: The new name of the repository
        :type new_repo_name: str

        :param old_url: The old URL of the repository
        :type old_url: str

        :param new_url: The new URL of the repository
        :type new_url: str

        :param new_auth_cfg: The auth config id.
        :type new_auth_cfg: str
        """
        # Fetch the metadata from the new url
        repo_handler = BaseRepositoryHandler.get_handler(new_url)
        if repo_handler is None:
            raise Exception('There is no handler available for the given URL!')

        if new_auth_cfg:
            repo_handler.auth_cfg = new_auth_cfg

        status, description = repo_handler.fetch_metadata()

        if status:
            # Parse metadata
            try:
                new_collections = repo_handler.parse_metadata()
            except MetadataError:
                raise

            old_collections = self._repositories.get(old_repo_name, [])
            # Get all the installed collections from the old repository
            installed_old_collections = []
            for old_collection in old_collections:
                if old_collection['status'] == COLLECTION_INSTALLED_STATUS:
                    installed_old_collections.append(old_collection)

            # Beware of the installed collections
            # Old collection exists in the new URL are identified by its
            # register name. Cases for installed collections:
            # 1. Old collection exists in the new URL, same URL: use the new
            # one, update the status to INSTALLED
            # 2. Old collection exists in the new URL, different URL: keep them
            # both (add the old one). Because they should be treated as
            # different collection
            # 3. Old collection doesn't exist in the new URL, same URL: keep
            # the old collection
            # 4. Old collection doesn't exist in the new URL, different URL:
            # same with 3
            for installed_collection in installed_old_collections:
                reg_name = installed_collection['register_name']
                is_present = False

                for collection in new_collections:
                    if collection['register_name'] == reg_name:
                        is_present = True
                        if old_url == new_url:
                            collection['status'] = COLLECTION_INSTALLED_STATUS
                        else:
                            new_collections.append(installed_collection)
                        break

                # Get to this point could be because it's present or the old
                # installed collection doesn't exist in the new URL
                if not is_present:
                    new_collections.append(installed_collection)

            # Remove old repository and add new one
            self._repositories.pop(old_repo_name, None)
            self._repositories[new_repo_name] = new_collections
            self.rebuild_collections()

            # Update QSettings
            settings = QSettings()
            settings.beginGroup(repo_settings_group())
            settings.remove(old_repo_name)
            settings.setValue(new_repo_name + '/url', new_url)
            settings.setValue(new_repo_name + '/auth_cfg', new_auth_cfg)
            settings.endGroup()
            # Serialize repositories every time we successfully edited repo
            self.serialize_repositories()
        return status, description

    def remove_directory(self, repo_name):
        """Remove a directory and all the collections of that repository.

        :param repo_name: The old name of the repository
        :type repo_name: str
        """
        self._repositories.pop(repo_name, None)
        self.rebuild_collections()
        # Remove repo from QSettings
        settings = QSettings()
        settings.beginGroup(repo_settings_group())
        settings.remove(repo_name)
        settings.endGroup()
        # Serialize repositories every time successfully removed a repo
        self.serialize_repositories()

    def reload_directory(self, repo_name, url, auth_cfg):
        """Re-fetch the directory and update the collections registry.

        :param repo_name: The name of the repository
        :type repo_name: str

        :param url: The URL of the repository
        :type url: str
        """
        # We're basically editing a directory with the same repo name and url
        status, description = self.edit_directory(
            repo_name,
            repo_name,
            url,
            url,
            auth_cfg
        )
        return status, description

    def rebuild_collections(self):
        """Rebuild collections from repositories."""
        config.COLLECTIONS = {}
        for repo in self._repositories.keys():
            repo_collections = self._repositories[repo]
            for collection in repo_collections:
                collection_id = self._collections_manager.get_collection_id(
                    collection['register_name'],
                    collection['repository_url']
                )
                config.COLLECTIONS[collection_id] = collection

                # Check in the file system if the collection exists for all
                # installed collections. If not, also uninstall resources
                current_status = config.COLLECTIONS[collection_id]['status']
                if current_status == COLLECTION_INSTALLED_STATUS:
                    collection_path = local_collection_path(collection_id)
                    if not os.path.exists(collection_path):
                        # Uninstall the collection
                        self._collections_manager.uninstall(collection_id)

    def resync_repository(self):
        """Resync from collections as opposed to rebuild_collections."""
        for repo in self._repositories.keys():
            repo_collections = self._repositories[repo]
            synced_repo_collections = []
            for collection in repo_collections:
                collection_id = self._collections_manager.get_collection_id(
                    collection['register_name'],
                    collection['repository_url']
                )
                synced_repo_collections.append(
                    config.COLLECTIONS[collection_id]
                )
            self._repositories[repo] = synced_repo_collections

    def serialize_repositories(self):
        """Save repositories to cache."""
        if not os.path.exists(os.path.dirname(repositories_cache_path())):
            os.makedirs(os.path.dirname(repositories_cache_path()))

        self.resync_repository()
        with open(repositories_cache_path(), 'wb') as f:
            pickle.dump(self._repositories, f)

    def load_repositories(self):
        """Load repositories from cache and rebuild collections."""
        repo_collections = {}
        if os.path.exists(repositories_cache_path()):
            with open(repositories_cache_path(), 'rb') as f:
                repo_collections = pickle.load(f)
        self._repositories = repo_collections
        self.rebuild_collections()
