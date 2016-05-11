from PyQt5.QtCore import *

from types import MethodType


def __nonzero__(self):
    if self.isNull():
        return False
    else:
        return super().__nonzero__()


def __repr__(self):
    if self.isNull():
        return 'NULL'
    else:
        return super().__repr__(self)


def __eq__(self, other):
    if self.isNull():
        return (isinstance(other, QVariant) and other.isNull())or other is None
    else:
        return super().__eq__(self, other)


def __ne__(self, other):
    if self.isNull():
        return not (isinstance(other, QVariant) and other.isNull()) and other is not None
    else:
        return super().__ne__(self, other)


def __hash__(self):
    if self.isNull():
        return 2178309
    else:
        return super.__hash__(self)

QVariant.__nonzero__ = __nonzero__
QVariant.__repr__ = __repr__
QVariant.__eq__ = __eq__
QVariant.__ne__ = __ne__
QVariant.__hash__ = __hash__

NULL = QVariant(QVariant.Int)
