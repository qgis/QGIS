from nose2 import events, session
from nose2.compat import unittest


class SessionUnitTests(unittest.TestCase):

    def test_can_create_session(self):
        session.Session()

    def test_load_plugins_from_module_can_load_plugins(self):
        class fakemod:
            pass
        f = fakemod()

        class A(events.Plugin):
            pass
        f.A = A
        s = session.Session()
        s.loadPluginsFromModule(f)
        assert s.plugins
        a = s.plugins[0]
        self.assertEqual(a.session, s)
