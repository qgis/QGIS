# coding=utf-8
import os
import fnmatch

from resource_sharing.resource_handler.base import BaseResourceHandler
from resource_sharing.resource_handler.symbol_resolver_mixin import \
    SymbolResolverMixin


class StyleResourceHandler(BaseResourceHandler, SymbolResolverMixin):
    """Concrete class of the style handler."""
    IS_DISABLED = False

    def __init__(self, collection_id):
        """Constructor of the base class."""
        BaseResourceHandler.__init__(self, collection_id)

    @classmethod
    def dir_name(cls):
        return 'style'

    def install(self):
        """Install the style.

        We just resolve the symbol svg/image path in the qml file
        """
        # Check if the dir exists, pass installing silently if it doesn't exist
        if not os.path.exists(self.resource_dir):
            return

        # Get all the style xml files under resource dirs
        style_files = []
        for item in os.listdir(self.resource_dir):
            file_path = os.path.join(self.resource_dir, item)
            if fnmatch.fnmatch(file_path, '*.qml'):
                style_files.append(file_path)

        # If there's no symbol files don't do anything
        if len(style_files) == 0:
            return

        for style_file in style_files:
            # Modify the style
            self.resolve_dependency(style_file)

    def uninstall(self):
        """Uninstall the style from QGIS."""
        pass
