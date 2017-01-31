# coding=utf-8
import os
import shutil
import logging
import traceback

from qgis.core import QgsApplication

from giturlparse import parse, validate
from dulwich import porcelain
from resource_sharing.repository_handler.base import BaseRepositoryHandler
from resource_sharing.network_manager import NetworkManager
from resource_sharing.utilities import local_collection_path


LOGGER = logging.getLogger('QGIS Resources Sharing')


class RemoteGitHandler(BaseRepositoryHandler):
    """Class to handle generic git remote repository."""
    IS_DISABLED = True

    def __init__(self, url):
        """Constructor."""
        BaseRepositoryHandler.__init__(self, url)
        self._git_platform = None
        self._git_host = None
        self._git_owner = None
        self._git_repository = None

        # Call proper setters here
        self.url = url

    def can_handle(self):
        return False

    @BaseRepositoryHandler.url.setter
    def url(self, url):
        """Setter to the repository's URL."""
        if validate(url):
            self._url = url
            git_parse = parse(url)
            self._git_platform = git_parse.platform
            self._git_host = git_parse.host
            self._git_owner = git_parse.owner
            self._git_repository = git_parse.repo

    @property
    def git_platform(self):
        return self._git_platform

    @property
    def git_host(self):
        return self._git_host

    @property
    def git_owner(self):
        return self._git_owner

    @property
    def git_repository(self):
        return self._git_repository

    def fetch_metadata(self):
        """Fetch metadata file from the repository."""
        # Fetch the metadata
        network_manager = NetworkManager(self.metadata_url, self.auth_cfg)
        status, description = network_manager.fetch()
        if status:
            self.metadata = network_manager.content
        return status, description

    def download_collection(self, id, register_name):
        """Download a collection given its ID.

        For remote git repositories, we will clone the repository first (or
        pull if the repo is already cloned before) and copy the collection to
        collections dir.

        :param id: The ID of the collection.
        :type id: str

        :param register_name: The register name of the collection (the
            section name of the collection)
        :type register_name: unicode
        """
        # Clone or pull the repositories first
        local_repo_dir = os.path.join(
            QgsApplication.qgisSettingsDirPath(),
            'resource_sharing',
            'repositories',
            self.git_host, self.git_owner, self.git_repository
        )
        if not os.path.exists(os.path.join(local_repo_dir, '.git')):
            os.makedirs(local_repo_dir)
            try:
                repo = porcelain.clone(
                    self.url, local_repo_dir
                )
            except Exception as e:
                # Try to clone with https if it's ssh url
                git_parsed = parse(self.url)
                if self.url == git_parsed.url2ssh:
                    try:
                        repo = porcelain.clone(
                            git_parsed.url2https, local_repo_dir)
                    except Exception as e:
                        error_message = 'Error: %s' % str(e)
                        LOGGER.exception(traceback.format_exc())
                        return False, error_message
                else:
                    error_message = 'Error: %s' % str(e)
                    LOGGER.exception(traceback.format_exc())
                    return False, error_message

            if not repo:
                error_message = ('Error: Cloning the repository of the '
                                 'collection failed.')
                return False, error_message
        else:
            try:
                porcelain.pull(
                    local_repo_dir,
                    self.url,
                    b'refs/heads/master'
                )
            except Exception as e:
                # Try to pull with https if it's ssh url
                git_parsed = parse(self.url)
                if self.url == git_parsed.url2ssh:
                    try:
                        porcelain.pull(
                            local_repo_dir,
                            git_parsed.url2https,
                            b'refs/heads/master'
                        )
                    except Exception as e:
                        error_message = 'Error: %s' % str(e)
                        LOGGER.exception(traceback.format_exc())
                        return False, error_message
                else:
                    error_message = 'Error: %s' % str(e)
                    LOGGER.exception(traceback.format_exc())
                    return False, error_message

        # Copy the specific downloaded collection to collections dir
        src_dir = os.path.join(local_repo_dir, 'collections', register_name)
        if not os.path.exists(src_dir):
            error_message = ('Error: The collection does not exist in the '
                             'repository.')
            return False, error_message

        dest_dir = local_collection_path(id)
        if os.path.exists(dest_dir):
            shutil.rmtree(dest_dir)
        shutil.copytree(src_dir, dest_dir)

        return True, None
