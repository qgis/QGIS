# coding=utf-8
import os
import fnmatch

try:
    from qgis.core import QgsStyleV2 as QgsStyle
except ImportError:
    from qgis.core import QgsStyle

from resource_sharing.resource_handler.base import BaseResourceHandler
from resource_sharing.symbol_xml_extractor import SymbolXMLExtractor
from resource_sharing.resource_handler.symbol_resolver_mixin import \
    SymbolResolverMixin


class SymbolResourceHandler(BaseResourceHandler, SymbolResolverMixin):
    """Concrete class of the Symbol handler."""
    IS_DISABLED = False

    def __init__(self, collection_id):
        """Constructor of the base class."""
        BaseResourceHandler.__init__(self, collection_id)
        # Init the default style
        self.style = QgsStyle.defaultStyle()

    @classmethod
    def dir_name(self):
        return 'symbol'

    def _get_parent_group_or_tag(self):
        """Retrieve or create the parent group (QGIS2) or tag (QGIS3) for
        the styles returns the group or tag id."""
        parent_group_name = '%s (%s)' % (
            self.collection['name'], self.collection_id)
        try:
            group = self.style.groupId(parent_group_name)
            if group != 0:
                return group
            return self.style.addGroup(parent_group_name)
        except AttributeError:
            tag = self.style.tagId(parent_group_name)
            if tag != 0:
                return tag
            return self.style.addTag(parent_group_name)

    def _get_child_group_tag(self, group_or_tag_id, file_name):
        """Retrieve or create the child group (QGIS2) or the parent tag for
        (QGIS3, no hierarchy) for the styles, returns the group or parent tag
        id with a slash and the file_name as a way to fake a tree within QGIS3.
        """
        try:
            group = self.style.groupId(file_name)
            if group != 0:
                return group
            return self.style.addGroup(file_name, group_or_tag_id)
        except AttributeError:
            tag_name = self.style.tag(group_or_tag_id) + '/' + file_name
            tag = self.style.tagId(tag_name)
            if tag != 0:
                return tag
            return self.style.addTag(tag_name)

    def _get_child_groups_tags_ids(self):
        """Retrieve child groups (QGIS2) or tags (QGIS3) ids."""
        parent_group_name = '%s (%s)' % (
            self.collection['name'], self.collection_id)
        try:
            return [self.style.groupId(n) for n in
                    self.style.childGroupNames(parent_group_name)]
        except AttributeError:
            return [self.style.tagId(tag) for tag in self.style.tags()
                    if tag.find(parent_group_name) == 0]

    def _get_symbols_for_group_or_tag(self, symbol_type,
                                      child_group_or_tag_id):
        """Return all symbols names in the group id (QGIS2) or tag id
        (QGIS3)."""
        try:
            return self.style.symbolsOfGroup(
                QgsStyle.SymbolEntity, child_group_or_tag_id)
        except AttributeError:
            return self.style.symbolsWithTag(
                QgsStyle.SymbolEntity, child_group_or_tag_id)

    def _group_or_tag(self, symbol_type, symbol_name, tag_or_group):
        """Add to group (QGIS2) or tag (QGIS3)."""
        try:
            self.style.group(QgsStyle.SymbolEntity, symbol_name, tag_or_group)
        except AttributeError:
            self.style.tagSymbol(QgsStyle.SymbolEntity, symbol_name,
                                 [self.style.tag(tag_or_group)])

    def _group_or_tag_remove(self, group_or_tag_id):
        """Remove a group or tag."""
        try:
            self.style.remove(QgsStyle.GroupEntity, group_or_tag_id)
        except AttributeError:
            self.style.remove(QgsStyle.TagEntity, group_or_tag_id)

    def install(self):
        """Install the symbol and collection from this collection into QGIS.

        We create a group with the name of the collection, a child group for
        each xml file and save all the symbols and colorramp defined that xml
        file into that child group.
        """
        # Check if the dir exists, pass installing silently if it doesn't exist
        if not os.path.exists(self.resource_dir):
            return

        # Uninstall first in case of reinstalling
        self.uninstall()

        # Get all the symbol xml files under resource dirs
        symbol_files = []
        for item in os.listdir(self.resource_dir):
            file_path = os.path.join(self.resource_dir, item)
            if fnmatch.fnmatch(file_path, '*.xml'):
                symbol_files.append(file_path)

        # If there's no symbol files don't do anything
        if len(symbol_files) == 0:
            return

        group_or_tag_id = self._get_parent_group_or_tag()

        for symbol_file in symbol_files:
            file_name = os.path.splitext(os.path.basename(symbol_file))[0]
            # FIXME: no groups in QGIS3!!!

            child_id = self._get_child_group_tag(group_or_tag_id, file_name)
            # Modify the symbol file first
            self.resolve_dependency(symbol_file)
            # Add all symbols and colorramps and group it
            symbol_xml_extractor = SymbolXMLExtractor(symbol_file)

            for symbol in symbol_xml_extractor.symbols:
                symbol_name = '%s (%s)' % (symbol['name'], self.collection_id)
                # self.resolve_dependency(symbol['symbol'])
                if self.style.addSymbol(symbol_name, symbol['symbol'], True):
                    self._group_or_tag(QgsStyle.SymbolEntity, symbol_name,
                                       child_id)

            for colorramp in symbol_xml_extractor.colorramps:
                colorramp_name = '%s (%s)' % (
                    colorramp['name'], self.collection_id)
                if self.style.addColorRamp(
                        colorramp_name, colorramp['colorramp'], True):
                    self._group_or_tag(
                        QgsStyle.ColorrampEntity, colorramp_name, child_id)

    def uninstall(self):
        """Uninstall the symbols from QGIS."""
        # Get the parent group id
        group_or_tag_id = self._get_parent_group_or_tag()
        child_groups_or_tags_ids = self._get_child_groups_tags_ids()
        for child_group_id in child_groups_or_tags_ids:
            # Get all the symbol from this child group and remove them
            symbols = self._get_symbols_for_group_or_tag(
                QgsStyle.SymbolEntity, child_group_id)
            for symbol in symbols:
                self.style.removeSymbol(symbol)
            # Get all the colorramps and remove them
            colorramps = self._get_symbols_for_group_or_tag(
                QgsStyle.ColorrampEntity, child_group_id)
            for colorramp in colorramps:
                self.style.removeColorRamp(colorramp)

            # Remove this child group
            self._group_or_tag_remove(child_group_id)

        # Remove parent group:
        self._group_or_tag_remove(group_or_tag_id)
