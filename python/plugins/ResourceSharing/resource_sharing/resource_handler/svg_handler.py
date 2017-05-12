# coding=utf-8
import os

from qgis.PyQt.QtCore import QSettings
try:
    from qgis.core import Qgis
except ImportError:
    from qgis.core import QGis as Qgis

from resource_sharing.resource_handler.base import BaseResourceHandler
from resource_sharing.utilities import local_collection_path


class SVGResourceHandler(BaseResourceHandler):
    """Concrete class of the SVG resource handler."""
    IS_DISABLED = False

    def __init__(self, collection_id):
        """Constructor of the base class."""
        BaseResourceHandler.__init__(self, collection_id)

    @classmethod
    def svg_search_paths(cls):
        """Return a list of SVG paths read from settings"""
        settings = QSettings()
        search_paths_str = settings.value('svg/searchPathsForSVG')

        # QGIS 3: it's already a list!
        is_list = False
        if not search_paths_str:
            search_paths = []
        else:
            if Qgis.QGIS_VERSION_INT < 29900:
                search_paths = search_paths_str.split('|')
            else:
                search_paths = search_paths_str
        return search_paths

    @classmethod
    def set_svg_search_paths(cls, paths):
        """Set a list of SVG paths read from settings"""
        settings = QSettings()

        if Qgis.QGIS_VERSION_INT < 29900:
            settings.setValue('svg/searchPathsForSVG', '|'.join(paths))
        else:
            settings.setValue('svg/searchPathsForSVG', paths)

    @classmethod
    def dir_name(cls):
        return 'svg'

    def install(self):
        """Install the SVGs from this collection in to QGIS.

        We simply just add the path to the collection root directory to search
        path in QGIS.
        """
        # Check if the dir exists, pass installing silently if it doesn't exist
        if not os.path.exists(self.resource_dir):
            return

        # Add to the search paths for SVG
        search_paths = self.svg_search_paths()

        if local_collection_path() not in search_paths:
            search_paths.append(local_collection_path())

        self.set_svg_search_paths(search_paths)

    def uninstall(self):
        """Uninstall the SVGs from QGIS."""
        # Remove from the searchPaths if the dir empty of collection
        search_paths = self.svg_search_paths()
        collection_directories = os.listdir(local_collection_path())

        if len(collection_directories) == 0:
            if local_collection_path() in search_paths:
                search_paths.remove(local_collection_path())

        self.set_svg_search_paths(search_paths)
