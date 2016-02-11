import sys

import six

from nose2.plugins import printhooks
from nose2 import events, session
from nose2.tests._common import TestCase


class CustomEvent(events.Event):

    _attrs = events.Event._attrs + ('args',)

    def __init__(self, args, **kw):
        self.args = args
        super(CustomEvent, self).__init__(**kw)


class TestPluginA(events.Plugin):

    def register(self):
        super(TestPluginA, self).register()
        self.addMethods('pluginHookA')

    def register_with_nested_hook(self):
        super(TestPluginA, self).register()
        self.addMethods('pluginHookB')


class TestPluginB(events.Plugin):

    def pluginHookA(self, event):
        event.handled = True
        return "TestPluginB.pluginHookA"


class TestPluginC(events.Plugin):

    def register(self):
        super(TestPluginC, self).register()
        self.addMethods('pluginHookB1')

    def pluginHookB(self, event):
        nested_event = CustomEvent('level_two_args')
        self.session.hooks.pluginHookB1(nested_event)
        event.handled = True
        return "TestPluginC.pluginHookB"


class IndentAwarePlugin(events.Plugin):

    def pluginHookA(self, event):
        event.handled = True
        return printhooks.INDENT.pop()


class TestPrintHooksPlugin(TestCase):
    tags = ['unit']

    def setUp(self):
        self.err = sys.stderr
        self.buf = six.StringIO()
        sys.stderr = self.buf
        self.addCleanup(self.restore_stderr)

        self.session = session.Session()
        self.print_hooks_plugin = printhooks.PrintHooks(session=self.session)
        self.plugin_a = TestPluginA(session=self.session)
        self.plugin_b = TestPluginB(session=self.session)

    def restore_stderr(self):
        sys.stderr = self.err

    def test_traces_hooks_created_after_own_registration(self):
        self.print_hooks_plugin.register()
        self.plugin_a.register()
        self.plugin_b.register()

        event = CustomEvent('args')
        result = self.session.hooks.pluginHookA(event)
        self.assertEqual(result, "TestPluginB.pluginHookA")
        self.assertEqual("\n"
                         "pluginHookA: "
                         "CustomEvent(handled=False, args='args')",
                         self.buf.getvalue())

    def test_traces_hooks_created_before_own_registration(self):
        self.plugin_a.register()
        self.plugin_b.register()
        self.print_hooks_plugin.register()

        event = CustomEvent('args')
        result = self.session.hooks.pluginHookA(event)
        self.assertEqual(result, "TestPluginB.pluginHookA")
        self.assertEqual("\n"
                         "pluginHookA: "
                         "CustomEvent(handled=False, args='args')",
                         self.buf.getvalue())

    def test_traces_hooks_that_nobody_implements(self):
        self.plugin_a.register()
        self.print_hooks_plugin.register()

        event = CustomEvent('args')
        result = self.session.hooks.pluginHookA(event)
        self.assertEqual(result, None)
        self.assertEqual("\n"
                         "pluginHookA: "
                         "CustomEvent(handled=False, args='args')",
                         self.buf.getvalue())

    def test_indents_nested_hooks_in_trace(self):
        self.plugin_c = TestPluginC(session=self.session)

        self.plugin_a.register_with_nested_hook()
        self.plugin_c.register()
        self.print_hooks_plugin.register()

        event = CustomEvent('level_one_args')
        result = self.session.hooks.pluginHookB(event)
        self.assertEqual(result, "TestPluginC.pluginHookB")
        self.assertEqual("\n"
                         "pluginHookB: "
                         "CustomEvent(handled=False, args='level_one_args')"
                         "\n  "
                         "pluginHookB1: "
                         "CustomEvent(handled=False, args='level_two_args')",
                         self.buf.getvalue())

    def test_hook_implementors_can_modify_trace_indent(self):
        self.indent_aware_plugin = IndentAwarePlugin(session=self.session)

        self.plugin_a.register()
        self.indent_aware_plugin.register()
        self.print_hooks_plugin.register()

        event = CustomEvent('args')
        result = self.session.hooks.pluginHookA(event)
        self.assertEqual(result, "  ")
