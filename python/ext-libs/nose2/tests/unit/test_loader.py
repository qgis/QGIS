from nose2 import events, loader, session
from nose2.tests._common import TestCase


class TestPluggableTestLoader(TestCase):

    def setUp(self):
        self.session = session.Session()
        self.loader = loader.PluggableTestLoader(self.session)

    def test_load_from_module_calls_hook(self):
        self.session.hooks.register('loadTestsFromModule', FakePlugin())
        evt = events.LoadFromModuleEvent(self.loader, 'some_module')
        self.session.hooks.loadTestsFromModule(evt)
        self.assertTrue(evt.fakeLoadFromModule,
                        "FakePlugin.loadTestsFromModule() was not called")

    def test_load_from_name_calls_hook(self):
        self.session.hooks.register('loadTestsFromName', FakePlugin())
        evt = events.LoadFromNameEvent(self.loader,
                                       'some_name',
                                       'some_module')
        self.session.hooks.loadTestsFromName(evt)
        self.assertTrue(evt.fakeLoadFromName,
                        "FakePlugin.fakeLoadFromName() was not called")

    def test_load_from_names_calls_hook(self):
        self.session.hooks.register('loadTestsFromNames', FakePlugin())
        evt = events.LoadFromNamesEvent(self.loader,
                                        ['some_name'],
                                        'some_module')
        self.session.hooks.loadTestsFromNames(evt)
        self.assertTrue(evt.fakeLoadFromNames,
                        "FakePlugin.fakeLoadFromNames() was not called")

    def test_loader_from_names_calls_module_hook(self):
        fake_plugin = FakePlugin()
        self.session.hooks.register('loadTestsFromModule', fake_plugin)
        self.loader.loadTestsFromNames([], 'some_module')
        self.assertTrue(fake_plugin.fakeLoadFromModule,
                        "FakePlugin.loadTestsFromModule() was not called")

    def test_loader_from_names_calls_name_hook(self):
        fake_plugin = FakePlugin()
        self.session.hooks.register('loadTestsFromName', fake_plugin)
        self.loader.loadTestsFromNames(['some_name'])
        self.assertTrue(fake_plugin.fakeLoadFromName,
                        "FakePlugin.loadTestsFromName() was not called")

    def test_loader_from_names_calls_names_hook(self):
        fake_plugin = FakePlugin()
        self.session.hooks.register('loadTestsFromNames', fake_plugin)
        self.loader.loadTestsFromNames(['some_name'])
        self.assertTrue(fake_plugin.fakeLoadFromNames,
                        "FakePlugin.loadTestsFromNames() was not called")


class FakePlugin(object):

    def __init__(self):
        self.fakeLoadFromModule = False
        self.fakeLoadFromName = False
        self.fakeLoadFromNames = False

    def loadTestsFromModule(self, event):
        event.fakeLoadFromModule = True
        self.fakeLoadFromModule = True

    def loadTestsFromName(self, event):
        event.fakeLoadFromName = True
        self.fakeLoadFromName = True

    def loadTestsFromNames(self, event):
        event.fakeLoadFromNames = True
        self.fakeLoadFromNames = True