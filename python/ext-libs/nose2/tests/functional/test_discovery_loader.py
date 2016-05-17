from nose2.tests._common import FunctionalTestCase, TestCase, support_file
from nose2 import events, loader, session
from nose2.plugins.loader.discovery import DiscoveryLoader


class Watcher(events.Plugin):

    def __init__(self):
        self.called = []

    def loadTestsFromModule(self, event):
        self.called.append(event)


class DiscoveryFunctionalTest(FunctionalTestCase):

    def setUp(self):
        self.session = session.Session()
        self.plug = DiscoveryLoader(session=self.session)
        self.loader = loader.PluggableTestLoader(self.session)
        self.watcher = Watcher(session=self.session)
        self.watcher.register()

    def test_can_discover_test_modules_in_packages(self):
        self.session.startDir = support_file('scenario/tests_in_package')
        event = events.LoadFromNamesEvent(self.loader, [], None)
        result = self.session.hooks.loadTestsFromNames(event)
        assert isinstance(result, self.loader.suiteClass)
        self.assertEqual(len(result._tests), 1)
        self.assertEqual(len(self.watcher.called), 1)
        self.assertEqual(self.watcher.called[0].module.__name__,
                         'pkg1.test.test_things')

    def test_discovery_supports_code_in_lib_dir(self):
        self.session.startDir = support_file('scenario/package_in_lib')
        event = events.LoadFromNamesEvent(self.loader, [], None)
        result = self.session.hooks.loadTestsFromNames(event)
        assert isinstance(result, self.loader.suiteClass)
        self.assertEqual(len(result._tests), 1)
        self.assertEqual(len(self.watcher.called), 1)
        self.assertEqual(self.watcher.called[0].module.__name__, 'tests')

    def test_match_path_event_can_prevent_discovery(self):
        class NoTestsForYou(events.Plugin):

            def matchPath(self, event):
                event.handled = True
                return False
        mp = NoTestsForYou(session=self.session)
        mp.register()
        self.session.startDir = support_file('scenario/tests_in_package')
        event = events.LoadFromNamesEvent(self.loader, [], None)
        result = self.session.hooks.loadTestsFromNames(event)
        assert isinstance(result, self.loader.suiteClass)
        self.assertEqual(len(result._tests), 0)
        self.assertEqual(len(self.watcher.called), 0)

    def test_handle_file_event_can_add_tests(self):
        class TextTest(TestCase):

            def test(self):
                pass

        class TestsInText(events.Plugin):

            def handleFile(self, event):
                if event.path.endswith('.txt'):
                    event.extraTests.append(TextTest('test'))
        mp = TestsInText(session=self.session)
        mp.register()
        self.session.startDir = support_file('scenario/tests_in_package')
        event = events.LoadFromNamesEvent(self.loader, [], None)
        result = self.session.hooks.loadTestsFromNames(event)
        assert isinstance(result, self.loader.suiteClass)
        self.assertEqual(len(result._tests), 2)
        self.assertEqual(len(self.watcher.called), 1)
