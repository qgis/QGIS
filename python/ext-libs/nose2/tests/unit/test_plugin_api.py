from nose2 import events, session
from nose2.tests._common import TestCase


class Example(events.Plugin):
    commandLineSwitch = ('X', 'xxx', 'triple x')

    def testOutcome(self, event):
        pass


class TestPluginApi(TestCase):

    def setUp(self):
        self.session = session.Session()
        self.plug = Example(session=self.session)
        super(TestCase, self).setUp()

    def test_add_option_adds_option(self):
        helpt = self.session.argparse.format_help()
        assert '-X, --xxx' in helpt, \
            "commandLineSwitch arg not found in help text: %s" % helpt

    def test_short_opt_registers_plugin(self):
        args, argv = self.session.argparse.parse_known_args(['-X'])
        assert self.plug in self.session.plugins
        assert self.plug in self.session.hooks.testOutcome.plugins, \
            "short opt did not register plugin"

    def test_long_opt_registers_plugin(self):
        args, argv = self.session.argparse.parse_known_args(['--xxx'])
        assert self.plug in self.session.plugins
        assert self.plug in self.session.hooks.testOutcome.plugins, \
            "long opt did not register plugin"
