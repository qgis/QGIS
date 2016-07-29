from PyQt5.QtCore import *

from types import MethodType

_QVariant__repr__ = QVariant.__repr__
_QVariant__eq__ = QVariant.__eq__
_QVariant__ne__ = QVariant.__ne__
_QVariant__hash__ = QVariant.__hash__


def __bool__(self):
    return not self.isNull()


def __repr__(self):
    if self.isNull():
        return 'NULL'
    else:
        return _QVariant__repr__(self)


def __eq__(self, other):
    if self.isNull():
        return (isinstance(other, QVariant) and other.isNull())or other is None
    else:
        return _QVariant__eq__(self, other)


def __ne__(self, other):
    if self.isNull():
        return not (isinstance(other, QVariant) and other.isNull()) and other is not None
    else:
        return _QVariant__ne__(self, other)


def __hash__(self):
    if self.isNull():
        return 2178309
    else:
        return _QVariant__hash__(self)

QVariant.__bool__ = __bool__
QVariant.__repr__ = __repr__
QVariant.__eq__ = __eq__
QVariant.__ne__ = __ne__
QVariant.__hash__ = __hash__

NULL = QVariant(QVariant.Int)
