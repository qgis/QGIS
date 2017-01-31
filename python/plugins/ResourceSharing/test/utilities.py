# coding=utf-8
import os

from qgis.PyQt.QtCore import QUrl


def test_data_path(*args):
    """Return the absolute path to the InaSAFE test data or directory path.

    :param args: List of path e.g. ['control', 'files',
        'test-error-message.txt'] or ['control', 'scenarios'] to get the path
        to scenarios dir.
    :type args: list

    :return: Absolute path to the test data or dir path.
    :rtype: str

    """
    path = os.path.dirname(__file__)
    path = os.path.abspath(os.path.join(path, 'data'))
    for item in args:
        path = os.path.abspath(os.path.join(path, item))

    return path


def test_repository_url():
    """Return the test repository URL on file system.

    :return: The test repository URL string
    :rtype: str
    """
    return QUrl.fromLocalFile(test_data_path()).toString()
