# test_swift.py -- Unittests for the Swift backend.
# Copyright (C) 2013 eNovance SAS <licensing@enovance.com>
#
# Author: Fabien Boucher <fabien.boucher@enovance.com>
#
# Dulwich is dual-licensed under the Apache License, Version 2.0 and the GNU
# General Public License as public by the Free Software Foundation; version 2.0
# or (at your option) any later version. You can redistribute it and/or
# modify it under the terms of either of these two licenses.
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# You should have received a copy of the licenses; if not, see
# <http://www.gnu.org/licenses/> for a copy of the GNU General Public License
# and <http://www.apache.org/licenses/LICENSE-2.0> for a copy of the Apache
# License, Version 2.0.
#

"""Tests for dulwich.contrib.swift."""

import posixpath

from time import time
from io import BytesIO
try:
    from StringIO import StringIO
except ImportError:
    from io import StringIO

import sys
from unittest import skipIf

from dulwich.tests import (
    TestCase,
    )
from dulwich.tests.test_object_store import (
    ObjectStoreTests,
    )
from dulwich.tests.utils import (
    build_pack,
    )
from dulwich.objects import (
    Blob,
    Commit,
    Tree,
    Tag,
    parse_timezone,
    )
from dulwich.pack import (
    REF_DELTA,
    write_pack_index_v2,
    PackData,
    load_pack_index_file,
    )

try:
    from simplejson import dumps as json_dumps
except ImportError:
    from json import dumps as json_dumps

missing_libs = []

try:
    import gevent
except ImportError:
    missing_libs.append("gevent")

try:
    import geventhttpclient
except ImportError:
    missing_libs.append("geventhttpclient")

try:
    from mock import patch
except ImportError:
    missing_libs.append("mock")

skipmsg = "Required libraries are not installed (%r)" % missing_libs

skipIfPY3 = skipIf(sys.version_info[0] == 3, "SWIFT module not yet ported to python3.")

if not missing_libs:
    from dulwich.contrib import swift

config_file = """[swift]
auth_url = http://127.0.0.1:8080/auth/%(version_str)s
auth_ver = %(version_int)s
username = test;tester
password = testing
region_name = %(region_name)s
endpoint_type = %(endpoint_type)s
concurrency = %(concurrency)s
chunk_length = %(chunk_length)s
cache_length = %(cache_length)s
http_pool_length = %(http_pool_length)s
http_timeout = %(http_timeout)s
"""

def_config_file = {'version_str': 'v1.0',
                   'version_int': 1,
                   'concurrency': 1,
                   'chunk_length': 12228,
                   'cache_length': 1,
                   'region_name': 'test',
                   'endpoint_type': 'internalURL',
                   'http_pool_length': 1,
                   'http_timeout': 1}


def create_swift_connector(store={}):
    return lambda root, conf: FakeSwiftConnector(root,
                                                 conf=conf,
                                                 store=store)


class Response(object):

    def __init__(self, headers={}, status=200, content=None):
        self.headers = headers
        self.status_code = status
        self.content = content

    def __getitem__(self, key):
        return self.headers[key]

    def items(self):
        return self.headers.items()

    def read(self):
        return self.content


def fake_auth_request_v1(*args, **kwargs):
    ret = Response({'X-Storage-Url':
                    'http://127.0.0.1:8080/v1.0/AUTH_fakeuser',
                    'X-Auth-Token': '12' * 10},
                   200)
    return ret


def fake_auth_request_v1_error(*args, **kwargs):
    ret = Response({},
                   401)
    return ret


def fake_auth_request_v2(*args, **kwargs):
    s_url = 'http://127.0.0.1:8080/v1.0/AUTH_fakeuser'
    resp = {'access': {'token': {'id': '12' * 10},
                       'serviceCatalog':
                       [
                           {'type': 'object-store',
                            'endpoints': [{'region': 'test',
                                          'internalURL': s_url,
                                           },
                                          ]
                            },
                       ]
                       }
            }
    ret = Response(status=200, content=json_dumps(resp))
    return ret


def create_commit(data, marker=b'Default', blob=None):
    if not blob:
        blob = Blob.from_string(b'The blob content ' + marker)
    tree = Tree()
    tree.add(b"thefile_" + marker, 0o100644, blob.id)
    cmt = Commit()
    if data:
        assert isinstance(data[-1], Commit)
        cmt.parents = [data[-1].id]
    cmt.tree = tree.id
    author = b"John Doe " + marker + b" <john@doe.net>"
    cmt.author = cmt.committer = author
    tz = parse_timezone(b'-0200')[0]
    cmt.commit_time = cmt.author_time = int(time())
    cmt.commit_timezone = cmt.author_timezone = tz
    cmt.encoding = b"UTF-8"
    cmt.message = b"The commit message " + marker
    tag = Tag()
    tag.tagger = b"john@doe.net"
    tag.message = b"Annotated tag"
    tag.tag_timezone = parse_timezone(b'-0200')[0]
    tag.tag_time = cmt.author_time
    tag.object = (Commit, cmt.id)
    tag.name = b"v_" + marker + b"_0.1"
    return blob, tree, tag, cmt


def create_commits(length=1, marker=b'Default'):
    data = []
    for i in range(0, length):
        _marker = ("%s_%s" % (marker, i)).encode()
        blob, tree, tag, cmt = create_commit(data, _marker)
        data.extend([blob, tree, tag, cmt])
    return data

@skipIf(missing_libs, skipmsg)
class FakeSwiftConnector(object):

    def __init__(self, root, conf, store=None):
        if store:
            self.store = store
        else:
            self.store = {}
        self.conf = conf
        self.root = root
        self.concurrency = 1
        self.chunk_length = 12228
        self.cache_length = 1

    def put_object(self, name, content):
        name = posixpath.join(self.root, name)
        if hasattr(content, 'seek'):
            content.seek(0)
            content = content.read()
        self.store[name] = content

    def get_object(self, name, range=None):
        name = posixpath.join(self.root, name)
        if not range:
            try:
                return BytesIO(self.store[name])
            except KeyError:
                return None
        else:
            l, r = range.split('-')
            try:
                if not l:
                    r = -int(r)
                    return self.store[name][r:]
                else:
                    return self.store[name][int(l):int(r)]
            except KeyError:
                return None

    def get_container_objects(self):
        return [{'name': k.replace(self.root + '/', '')}
                for k in self.store]

    def create_root(self):
        if self.root in self.store.keys():
            pass
        else:
            self.store[self.root] = ''

    def get_object_stat(self, name):
        name = posixpath.join(self.root, name)
        if not name in self.store:
            return None
        return {'content-length': len(self.store[name])}


@skipIf(missing_libs, skipmsg)
@skipIfPY3
class TestSwiftObjectStore(TestCase):

    def setUp(self):
        super(TestSwiftObjectStore, self).setUp()
        self.conf = swift.load_conf(file=StringIO(config_file %
                                                  def_config_file))
        self.fsc = FakeSwiftConnector('fakerepo', conf=self.conf)

    def _put_pack(self, sos, commit_amount=1, marker='Default'):
        odata = create_commits(length=commit_amount, marker=marker)
        data = [(d.type_num, d.as_raw_string()) for d in odata]
        f = BytesIO()
        build_pack(f, data, store=sos)
        sos.add_thin_pack(f.read, None)
        return odata

    def test_load_packs(self):
        store = {'fakerepo/objects/pack/pack-'+'1'*40+'.idx': '',
                 'fakerepo/objects/pack/pack-'+'1'*40+'.pack': '',
                 'fakerepo/objects/pack/pack-'+'1'*40+'.info': '',
                 'fakerepo/objects/pack/pack-'+'2'*40+'.idx': '',
                 'fakerepo/objects/pack/pack-'+'2'*40+'.pack': '',
                 'fakerepo/objects/pack/pack-'+'2'*40+'.info': ''}
        fsc = FakeSwiftConnector('fakerepo', conf=self.conf, store=store)
        sos = swift.SwiftObjectStore(fsc)
        packs = sos._load_packs()
        self.assertEqual(len(packs), 2)
        for pack in packs:
            self.assertTrue(isinstance(pack, swift.SwiftPack))

    def test_add_thin_pack(self):
        sos = swift.SwiftObjectStore(self.fsc)
        self._put_pack(sos, 1, 'Default')
        self.assertEqual(len(self.fsc.store), 3)

    def test_find_missing_objects(self):
        commit_amount = 3
        sos = swift.SwiftObjectStore(self.fsc)
        odata = self._put_pack(sos, commit_amount, 'Default')
        head = odata[-1].id
        i = sos.iter_shas(sos.find_missing_objects([],
                                                   [head, ],
                                                   progress=None,
                                                   get_tagged=None))
        self.assertEqual(len(i), commit_amount * 3)
        shas = [d.id for d in odata]
        for sha, path in i:
            self.assertIn(sha.id, shas)

    def test_find_missing_objects_with_tag(self):
        commit_amount = 3
        sos = swift.SwiftObjectStore(self.fsc)
        odata = self._put_pack(sos, commit_amount, 'Default')
        head = odata[-1].id
        peeled_sha = dict([(sha.object[1], sha.id)
                           for sha in odata if isinstance(sha, Tag)])
        get_tagged = lambda: peeled_sha
        i = sos.iter_shas(sos.find_missing_objects([],
                                                   [head, ],
                                                   progress=None,
                                                   get_tagged=get_tagged))
        self.assertEqual(len(i), commit_amount * 4)
        shas = [d.id for d in odata]
        for sha, path in i:
            self.assertIn(sha.id, shas)

    def test_find_missing_objects_with_common(self):
        commit_amount = 3
        sos = swift.SwiftObjectStore(self.fsc)
        odata = self._put_pack(sos, commit_amount, 'Default')
        head = odata[-1].id
        have = odata[7].id
        i = sos.iter_shas(sos.find_missing_objects([have, ],
                                                   [head, ],
                                                   progress=None,
                                                   get_tagged=None))
        self.assertEqual(len(i), 3)

    def test_find_missing_objects_multiple_packs(self):
        sos = swift.SwiftObjectStore(self.fsc)
        commit_amount_a = 3
        odataa = self._put_pack(sos, commit_amount_a, 'Default1')
        heada = odataa[-1].id
        commit_amount_b = 2
        odatab = self._put_pack(sos, commit_amount_b, 'Default2')
        headb = odatab[-1].id
        i = sos.iter_shas(sos.find_missing_objects([],
                                                   [heada, headb],
                                                   progress=None,
                                                   get_tagged=None))
        self.assertEqual(len(self.fsc.store), 6)
        self.assertEqual(len(i),
                         commit_amount_a * 3 +
                         commit_amount_b * 3)
        shas = [d.id for d in odataa]
        shas.extend([d.id for d in odatab])
        for sha, path in i:
            self.assertIn(sha.id, shas)

    def test_add_thin_pack_ext_ref(self):
        sos = swift.SwiftObjectStore(self.fsc)
        odata = self._put_pack(sos, 1, 'Default1')
        ref_blob_content = odata[0].as_raw_string()
        ref_blob_id = odata[0].id
        new_blob = Blob.from_string(ref_blob_content.replace('blob',
                                                             'yummy blob'))
        blob, tree, tag, cmt = \
            create_commit([], marker='Default2', blob=new_blob)
        data = [(REF_DELTA, (ref_blob_id, blob.as_raw_string())),
                (tree.type_num, tree.as_raw_string()),
                (cmt.type_num, cmt.as_raw_string()),
                (tag.type_num, tag.as_raw_string())]
        f = BytesIO()
        build_pack(f, data, store=sos)
        sos.add_thin_pack(f.read, None)
        self.assertEqual(len(self.fsc.store), 6)


@skipIf(missing_libs, skipmsg)
class TestSwiftRepo(TestCase):

    def setUp(self):
        super(TestSwiftRepo, self).setUp()
        self.conf = swift.load_conf(file=StringIO(config_file %
                                                  def_config_file))

    def test_init(self):
        store = {'fakerepo/objects/pack': ''}
        with patch('dulwich.contrib.swift.SwiftConnector',
                   new_callable=create_swift_connector,
                   store=store):
            swift.SwiftRepo('fakerepo', conf=self.conf)

    def test_init_no_data(self):
        with patch('dulwich.contrib.swift.SwiftConnector',
                   new_callable=create_swift_connector):
            self.assertRaises(Exception, swift.SwiftRepo,
                              'fakerepo', self.conf)

    def test_init_bad_data(self):
        store = {'fakerepo/.git/objects/pack': ''}
        with patch('dulwich.contrib.swift.SwiftConnector',
                   new_callable=create_swift_connector,
                   store=store):
            self.assertRaises(Exception, swift.SwiftRepo,
                              'fakerepo', self.conf)

    def test_put_named_file(self):
        store = {'fakerepo/objects/pack': ''}
        with patch('dulwich.contrib.swift.SwiftConnector',
                   new_callable=create_swift_connector,
                   store=store):
            repo = swift.SwiftRepo('fakerepo', conf=self.conf)
            desc = b'Fake repo'
            repo._put_named_file('description', desc)
        self.assertEqual(repo.scon.store['fakerepo/description'],
                         desc)

    def test_init_bare(self):
        fsc = FakeSwiftConnector('fakeroot', conf=self.conf)
        with patch('dulwich.contrib.swift.SwiftConnector',
                   new_callable=create_swift_connector,
                   store=fsc.store):
            swift.SwiftRepo.init_bare(fsc, conf=self.conf)
        self.assertIn('fakeroot/objects/pack', fsc.store)
        self.assertIn('fakeroot/info/refs', fsc.store)
        self.assertIn('fakeroot/description', fsc.store)


@skipIf(missing_libs, skipmsg)
@skipIfPY3
class TestPackInfoLoadDump(TestCase):

    def setUp(self):
        super(TestPackInfoLoadDump, self).setUp()
        conf = swift.load_conf(file=StringIO(config_file %
                                             def_config_file))
        sos = swift.SwiftObjectStore(
            FakeSwiftConnector('fakerepo', conf=conf))
        commit_amount = 10
        self.commits = create_commits(length=commit_amount, marker="m")
        data = [(d.type_num, d.as_raw_string()) for d in self.commits]
        f = BytesIO()
        fi = BytesIO()
        expected = build_pack(f, data, store=sos)
        entries = [(sha, ofs, checksum) for
                   ofs, _, _, sha, checksum in expected]
        self.pack_data = PackData.from_file(file=f, size=None)
        write_pack_index_v2(
            fi, entries, self.pack_data.calculate_checksum())
        fi.seek(0)
        self.pack_index = load_pack_index_file('', fi)

#    def test_pack_info_perf(self):
#        dump_time = []
#        load_time = []
#        for i in range(0, 100):
#            start = time()
#            dumps = swift.pack_info_create(self.pack_data, self.pack_index)
#            dump_time.append(time() - start)
#        for i in range(0, 100):
#            start = time()
#            pack_infos = swift.load_pack_info('', file=BytesIO(dumps))
#            load_time.append(time() - start)
#        print sum(dump_time) / float(len(dump_time))
#        print sum(load_time) / float(len(load_time))

    def test_pack_info(self):
        dumps = swift.pack_info_create(self.pack_data, self.pack_index)
        pack_infos = swift.load_pack_info('', file=BytesIO(dumps))
        for obj in self.commits:
            self.assertIn(obj.id, pack_infos)


@skipIf(missing_libs, skipmsg)
class TestSwiftInfoRefsContainer(TestCase):

    def setUp(self):
        super(TestSwiftInfoRefsContainer, self).setUp()
        content = \
            b"22effb216e3a82f97da599b8885a6cadb488b4c5\trefs/heads/master\n" + \
            b"cca703b0e1399008b53a1a236d6b4584737649e4\trefs/heads/dev"
        self.store = {'fakerepo/info/refs': content}
        self.conf = swift.load_conf(file=StringIO(config_file %
                                                  def_config_file))
        self.fsc = FakeSwiftConnector('fakerepo', conf=self.conf)
        self.object_store = {}

    def test_init(self):
        """info/refs does not exists"""
        irc = swift.SwiftInfoRefsContainer(self.fsc, self.object_store)
        self.assertEqual(len(irc._refs), 0)
        self.fsc.store = self.store
        irc = swift.SwiftInfoRefsContainer(self.fsc, self.object_store)
        self.assertIn(b'refs/heads/dev', irc.allkeys())
        self.assertIn(b'refs/heads/master', irc.allkeys())

    def test_set_if_equals(self):
        self.fsc.store = self.store
        irc = swift.SwiftInfoRefsContainer(self.fsc, self.object_store)
        irc.set_if_equals(b'refs/heads/dev',
                          b"cca703b0e1399008b53a1a236d6b4584737649e4", b'1'*40)
        self.assertEqual(irc[b'refs/heads/dev'], b'1'*40)

    def test_remove_if_equals(self):
        self.fsc.store = self.store
        irc = swift.SwiftInfoRefsContainer(self.fsc, self.object_store)
        irc.remove_if_equals(b'refs/heads/dev',
                             b"cca703b0e1399008b53a1a236d6b4584737649e4")
        self.assertNotIn(b'refs/heads/dev', irc.allkeys())


@skipIf(missing_libs, skipmsg)
class TestSwiftConnector(TestCase):

    def setUp(self):
        super(TestSwiftConnector, self).setUp()
        self.conf = swift.load_conf(file=StringIO(config_file %
                                                  def_config_file))
        with patch('geventhttpclient.HTTPClient.request',
                   fake_auth_request_v1):
            self.conn = swift.SwiftConnector('fakerepo', conf=self.conf)

    def test_init_connector(self):
        self.assertEqual(self.conn.auth_ver, '1')
        self.assertEqual(self.conn.auth_url,
                         'http://127.0.0.1:8080/auth/v1.0')
        self.assertEqual(self.conn.user, 'test:tester')
        self.assertEqual(self.conn.password, 'testing')
        self.assertEqual(self.conn.root, 'fakerepo')
        self.assertEqual(self.conn.storage_url,
                         'http://127.0.0.1:8080/v1.0/AUTH_fakeuser')
        self.assertEqual(self.conn.token, '12' * 10)
        self.assertEqual(self.conn.http_timeout, 1)
        self.assertEqual(self.conn.http_pool_length, 1)
        self.assertEqual(self.conn.concurrency, 1)
        self.conf.set('swift', 'auth_ver', '2')
        self.conf.set('swift', 'auth_url', 'http://127.0.0.1:8080/auth/v2.0')
        with patch('geventhttpclient.HTTPClient.request',
                   fake_auth_request_v2):
            conn = swift.SwiftConnector('fakerepo', conf=self.conf)
        self.assertEqual(conn.user, 'tester')
        self.assertEqual(conn.tenant, 'test')
        self.conf.set('swift', 'auth_ver', '1')
        self.conf.set('swift', 'auth_url', 'http://127.0.0.1:8080/auth/v1.0')
        with patch('geventhttpclient.HTTPClient.request',
                   fake_auth_request_v1_error):
            self.assertRaises(swift.SwiftException,
                              lambda: swift.SwiftConnector('fakerepo',
                                                           conf=self.conf))

    def test_root_exists(self):
        with patch('geventhttpclient.HTTPClient.request',
                   lambda *args: Response()):
            self.assertEqual(self.conn.test_root_exists(), True)

    def test_root_not_exists(self):
        with patch('geventhttpclient.HTTPClient.request',
                   lambda *args: Response(status=404)):
            self.assertEqual(self.conn.test_root_exists(), None)

    def test_create_root(self):
        with patch('dulwich.contrib.swift.SwiftConnector.test_root_exists',
                lambda *args: None):
            with patch('geventhttpclient.HTTPClient.request',
                lambda *args: Response()):
                self.assertEqual(self.conn.create_root(), None)

    def test_create_root_fails(self):
        with patch('dulwich.contrib.swift.SwiftConnector.test_root_exists',
                   lambda *args: None):
            with patch('geventhttpclient.HTTPClient.request',
                       lambda *args: Response(status=404)):
                self.assertRaises(swift.SwiftException,
                                  lambda: self.conn.create_root())

    def test_get_container_objects(self):
        with patch('geventhttpclient.HTTPClient.request',
                   lambda *args: Response(content=json_dumps(
                       (({'name': 'a'}, {'name': 'b'}))))):
            self.assertEqual(len(self.conn.get_container_objects()), 2)

    def test_get_container_objects_fails(self):
        with patch('geventhttpclient.HTTPClient.request',
                   lambda *args: Response(status=404)):
            self.assertEqual(self.conn.get_container_objects(), None)

    def test_get_object_stat(self):
        with patch('geventhttpclient.HTTPClient.request',
                   lambda *args: Response(headers={'content-length': '10'})):
            self.assertEqual(self.conn.get_object_stat('a')['content-length'],
                             '10')

    def test_get_object_stat_fails(self):
        with patch('geventhttpclient.HTTPClient.request',
                   lambda *args: Response(status=404)):
            self.assertEqual(self.conn.get_object_stat('a'), None)

    def test_put_object(self):
        with patch('geventhttpclient.HTTPClient.request',
                   lambda *args, **kwargs: Response()):
            self.assertEqual(self.conn.put_object('a', BytesIO(b'content')),
                             None)

    def test_put_object_fails(self):
        with patch('geventhttpclient.HTTPClient.request',
                   lambda *args, **kwargs: Response(status=400)):
            self.assertRaises(swift.SwiftException,
                              lambda: self.conn.put_object(
                                  'a', BytesIO(b'content')))

    def test_get_object(self):
        with patch('geventhttpclient.HTTPClient.request',
                   lambda *args, **kwargs: Response(content=b'content')):
            self.assertEqual(self.conn.get_object('a').read(), b'content')
        with patch('geventhttpclient.HTTPClient.request',
                   lambda *args, **kwargs: Response(content=b'content')):
            self.assertEqual(self.conn.get_object('a', range='0-6'), b'content')

    def test_get_object_fails(self):
        with patch('geventhttpclient.HTTPClient.request',
                   lambda *args, **kwargs: Response(status=404)):
            self.assertEqual(self.conn.get_object('a'), None)

    def test_del_object(self):
        with patch('geventhttpclient.HTTPClient.request',
                   lambda *args: Response()):
            self.assertEqual(self.conn.del_object('a'), None)

    def test_del_root(self):
        with patch('dulwich.contrib.swift.SwiftConnector.del_object',
                   lambda *args: None):
            with patch('dulwich.contrib.swift.SwiftConnector.'
                       'get_container_objects',
                       lambda *args: ({'name': 'a'}, {'name': 'b'})):
                with patch('geventhttpclient.HTTPClient.request',
                           lambda *args: Response()):
                    self.assertEqual(self.conn.del_root(), None)


@skipIf(missing_libs, skipmsg)
class SwiftObjectStoreTests(ObjectStoreTests, TestCase):

    def setUp(self):
        TestCase.setUp(self)
        conf = swift.load_conf(file=StringIO(config_file %
                               def_config_file))
        fsc = FakeSwiftConnector('fakerepo', conf=conf)
        self.store = swift.SwiftObjectStore(fsc)
