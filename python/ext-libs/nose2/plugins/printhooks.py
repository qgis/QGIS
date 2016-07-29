"""
This plugin is primarily useful for plugin authors who want to debug
their plugins.

It prints each hook that is called to stderr, along with details of
the event that was passed to the hook.

To do that, this plugin overrides :meth:`nose2.events.Plugin.register`
and, after registration, replaces all existing
:class:`nose2.events.Hook` instances in ``session.hooks`` with
instances of a Hook subclass that prints information about each call.
"""
import sys

from nose2 import events


INDENT = []
__unittest = True


class PrintHooks(events.Plugin):

    """Print hooks as they are called"""

    configSection = 'print-hooks'
    commandLineSwitch = ('P', 'print-hooks',
                         'Print names of hooks in order of execution')

    def register(self):
        """Override to inject noisy hook instances.

        Replaces Hook instances in ``self.session.hooks.hooks`` with
        noisier objects.

        """
        super(PrintHooks, self).register()
        # now we can be sure that all other plugins have loaded
        # and this plugin is active, patch in our hook class
        self.session.hooks.hookClass = NoisyHook
        for attr, hook in self.session.hooks.hooks.items():
            newhook = NoisyHook(attr)
            newhook.plugins = hook.plugins
            self.session.hooks.hooks[attr] = newhook


class NoisyHook(events.Hook):

    def __call__(self, event):
        _report(self.method, event)
        _indent()
        try:
            return super(NoisyHook, self).__call__(event)
        finally:
            _dedent()


def _report(method, event):
    sys.stderr.write("\n%s%s: %s" % (''.join(INDENT), method, event))


def _indent():
    INDENT.append('  ')


def _dedent():
    if INDENT:
        INDENT.pop()
