# coding=utf-8
from qgis.PyQt.QtCore import Qt

try:
    from qgis.PyQt.QtGui import QSortFilterProxyModel
except ImportError:
    from qgis.PyQt.QtCore import QSortFilterProxyModel

from resource_sharing.config import COLLECTION_INSTALLED_STATUS

COLLECTION_NAME_ROLE = Qt.UserRole + 1
COLLECTION_DESCRIPTION_ROLE = Qt.UserRole + 2
COLLECTION_AUTHOR_ROLE = Qt.UserRole + 3
COLLECTION_TAGS_ROLE = Qt.UserRole + 4
COLLECTION_ID_ROLE = Qt.UserRole + 5
COLLECTION_STATUS_ROLE = Qt.UserRole + 6


class CustomSortFilterProxyModel(QSortFilterProxyModel):
    """Custom QSortFilterProxyModel to be able to search on multiple data."""
    def __init__(self, parent=None):
        super(CustomSortFilterProxyModel, self).__init__(parent)
        self._accepted_status = None

    @property
    def accepted_status(self):
        return self._accepted_status

    @accepted_status.setter
    def accepted_status(self, status):
        self._accepted_status = status
        self.invalidateFilter()

    def filterAcceptsRow(self, row_num, source_parent):
        """Override this function."""
        index = self.sourceModel().index(row_num, 0, source_parent)
        name = self.filterRegExp().indexIn(
            self.sourceModel().data(index, COLLECTION_NAME_ROLE)) >= 0
        author = self.filterRegExp().indexIn(
            self.sourceModel().data(index, COLLECTION_AUTHOR_ROLE)) >= 0
        description = self.filterRegExp().indexIn(
            self.sourceModel().data(index, COLLECTION_DESCRIPTION_ROLE)) >= 0
        tags = self.filterRegExp().indexIn(
            self.sourceModel().data(index, COLLECTION_TAGS_ROLE)) >= 0

        if self.accepted_status == COLLECTION_INSTALLED_STATUS:
            # For installed collection status
            collection_status = self.sourceModel().data(
                index, COLLECTION_STATUS_ROLE)
            status = collection_status == COLLECTION_INSTALLED_STATUS
        else:
            status = True

        return (name or author or description or tags) and status
