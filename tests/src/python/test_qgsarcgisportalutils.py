# -*- coding: utf-8 -*-
"""QGIS Unit tests for the AFS provider.

From build dir, run: ctest -R PyQgsArcGisPortalUtils -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2018-02-16'
__copyright__ = 'Copyright 2018, Nyall Dawson'

import hashlib
import os
import tempfile

from qgis.PyQt.QtCore import QCoreApplication, QObject
from qgis.core import (QgsArcGisPortalUtils,
                       QgsApplication,
                       QgsSettings
                       )
from qgis.testing import (start_app,
                          unittest
                          )


def sanitize(endpoint, x):
    if not os.path.exists(endpoint):
        os.makedirs(endpoint)
    if x.startswith('/query'):
        x = x[len('/query'):]
        endpoint = endpoint + '_query'

    if len(endpoint + x) > 150:
        ret = endpoint + hashlib.md5(x.encode()).hexdigest()
        # print('Before: ' + endpoint + x)
        # print('After:  ' + ret)
        return ret
    return endpoint + x.replace('?', '_').replace('&', '_').replace('<', '_').replace('>', '_').replace('"',
                                                                                                        '_').replace(
        "'", '_').replace(' ', '_').replace(':', '_').replace('/', '_').replace('\n', '_')


class MessageLogger(QObject):

    def __init__(self, tag=None):
        QObject.__init__(self)
        self.log = []
        self.tag = tag

    def __enter__(self):
        QgsApplication.messageLog().messageReceived.connect(self.logMessage)
        return self

    def __exit__(self, type, value, traceback):
        QgsApplication.messageLog().messageReceived.disconnect(self.logMessage)

    def logMessage(self, msg, tag, level):
        if tag == self.tag or not self.tag:
            self.log.append(msg.encode('UTF-8'))

    def messages(self):
        return self.log


class TestPyQgsArcGisPortalUtils(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestPyQgsAFSProvider.com")
        QCoreApplication.setApplicationName("TestPyQgsAFSProvider")
        QgsSettings().clear()
        start_app()

        # On Windows we must make sure that any backslash in the path is
        # replaced by a forward slash so that QUrl can process it
        cls.basetestpath = tempfile.mkdtemp().replace('\\', '/')

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        QgsSettings().clear()
        # shutil.rmtree(cls.basetestpath, True)

    def testUserInfoSelf(self):
        """
        Test retrieving logged on user info
        """
        print(self.basetestpath)
        endpoint = self.basetestpath + '/user_fake_qgis_http_endpoint'
        with open(sanitize(endpoint, '/self?f=json'), 'wb') as f:
            f.write("""{
  "username": "me",
  "id": "2a",
  "groups": [
    {
      "id": "c4",
      "title": "A Group"
    },
    {
      "id": "d4",
      "title": "Another Group"
    }
  ]
}""".encode('UTF-8'))

        res = QgsArcGisPortalUtils.retrieveUserInfo('http://' + endpoint, '', '')
        # no errors
        self.assertFalse(res[1])
        self.assertFalse(res[2])
        self.assertEqual(res[0], {'groups': [{'id': 'c4', 'title': 'A Group'}, {'id': 'd4', 'title': 'Another Group'}],
                                  'id': '2a', 'username': 'me'})

    def testUserInfoExplicit(self):
        """
        Test retrieving explicitly specified user info
        """
        print(self.basetestpath)
        endpoint = self.basetestpath + '/user_fake_qgis_http_endpoint'

        with open(sanitize(endpoint + '_users/', 'some_user?f=json'), 'wb') as f:
            f.write("""{
  "username": "some_user",
  "id": "2b",
  "groups": [
    {
      "id": "c4",
      "title": "A Group"
    },
    {
      "id": "d4",
      "title": "Another Group"
    }
  ]
}""".encode('UTF-8'))

        headers = {'referer': 'http://google.com'}
        res = QgsArcGisPortalUtils.retrieveUserInfo('http://' + endpoint, 'some_user', '', headers)
        # no errors
        self.assertFalse(res[1])
        self.assertFalse(res[2])
        self.assertEqual(res[0], {'groups': [{'id': 'c4', 'title': 'A Group'}, {'id': 'd4', 'title': 'Another Group'}],
                                  'id': '2b', 'username': 'some_user'})

    def test_retrieve_groups(self):
        """
        Test retrieving user groups
        """
        print(self.basetestpath)
        endpoint = self.basetestpath + '/group_fake_qgis_http_endpoint'

        with open(sanitize(endpoint + '_users/', 'some_user?f=json'), 'wb') as f:
            f.write("""{
          "username": "some_user",
          "id": "2b",
          "groups": [
            {
              "id": "c4",
              "title": "A Group"
            },
            {
              "id": "d4",
              "title": "Another Group"
            }
          ]
        }""".encode('UTF-8'))

        res = QgsArcGisPortalUtils.retrieveUserGroups('http://' + endpoint, 'some_user', '')
        # no errors
        self.assertFalse(res[1])
        self.assertFalse(res[2])
        self.assertEqual(res[0], [{'id': 'c4', 'title': 'A Group'}, {'id': 'd4', 'title': 'Another Group'}])

    def test_retrieve_group_items(self):
        """
        Test retrieving group content
        """
        print(self.basetestpath)
        endpoint = self.basetestpath + '/group_items_fake_qgis_http_endpoint'

        with open(sanitize(endpoint + '_groups/', 'ab1?f=json&start=1&num=2'), 'wb') as f:
            f.write("""{
  "total": 3,
  "start": 1,
  "num": 2,
  "nextStart": 3,
  "items": [
    {
      "id": "74",
      "title": "Item 1"
    },
    {
      "id": "20",
      "title": "Item 2"
    }
  ]
}""".encode('UTF-8'))

            with open(sanitize(endpoint + '_groups/', 'ab1?f=json&start=3&num=2'), 'wb') as f:
                f.write("""{
          "total": 3,
          "start": 3,
          "num": 1,
          "nextStart": 3,
          "items": [
            {
              "id": "75",
              "title": "Item 3"
            }
          ]
        }""".encode('UTF-8'))
        res = QgsArcGisPortalUtils.retrieveGroupContent('http://' + endpoint, 'ab1', '', pageSize=2)
        # no errors
        self.assertFalse(res[1])
        self.assertFalse(res[2])
        self.assertEqual(res[0], [{'id': '74', 'title': 'Item 1'}, {'id': '20', 'title': 'Item 2'},
                                  {'id': '75', 'title': 'Item 3'}])

    def test_retrieve_group_items_filtered(self):
        """
        Test retrieving group content
        """
        print(self.basetestpath)
        endpoint = self.basetestpath + '/groupf_items_fake_qgis_http_endpoint'

        with open(sanitize(endpoint + '_groups/', 'ab1?f=json&start=1&num=2'), 'wb') as f:
            f.write("""{
  "total": 3,
  "start": 1,
  "num": 2,
  "nextStart": 3,
  "items": [
    {
      "id": "74",
      "title": "Item 1",
      "type":"Feature Service"
    },
    {
      "id": "20",
      "title": "Item 2",
      "type":"Map Service"
    }
  ]
}""".encode('UTF-8'))

            with open(sanitize(endpoint + '_groups/', 'ab1?f=json&start=3&num=2'), 'wb') as f:
                f.write("""{
          "total": 3,
          "start": 3,
          "num": 1,
          "nextStart": 3,
          "items": [
            {
              "id": "75",
              "title": "Item 3",
              "type":"Image Service"
            }
          ]
        }""".encode('UTF-8'))
        res = QgsArcGisPortalUtils.retrieveGroupItemsOfType('http://' + endpoint, 'ab1', '',
                                                            [QgsArcGisPortalUtils.FeatureService], pageSize=2)
        # no errors
        self.assertFalse(res[1])
        self.assertFalse(res[2])
        self.assertEqual(res[0], [{'id': '74', 'title': 'Item 1', 'type': 'Feature Service'}])
        res = QgsArcGisPortalUtils.retrieveGroupItemsOfType('http://' + endpoint, 'ab1', '',
                                                            [QgsArcGisPortalUtils.MapService], pageSize=2)
        # no errors
        self.assertFalse(res[1])
        self.assertFalse(res[2])
        self.assertEqual(res[0], [{'id': '20', 'title': 'Item 2', 'type': 'Map Service'}])
        res = QgsArcGisPortalUtils.retrieveGroupItemsOfType('http://' + endpoint, 'ab1', '',
                                                            [QgsArcGisPortalUtils.ImageService], pageSize=2)
        # no errors
        self.assertFalse(res[1])
        self.assertFalse(res[2])
        self.assertEqual(res[0], [{'id': '75', 'title': 'Item 3', 'type': 'Image Service'}])
        res = QgsArcGisPortalUtils.retrieveGroupItemsOfType('http://' + endpoint, 'ab1', '',
                                                            [QgsArcGisPortalUtils.FeatureService,
                                                             QgsArcGisPortalUtils.MapService], pageSize=2)
        # no errors
        self.assertFalse(res[1])
        self.assertFalse(res[2])
        self.assertEqual(res[0], [{'id': '74', 'title': 'Item 1', 'type': 'Feature Service'},
                                  {'id': '20', 'title': 'Item 2', 'type': 'Map Service'}])


if __name__ == '__main__':
    unittest.main()
