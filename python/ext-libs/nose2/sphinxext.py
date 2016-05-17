import types

from docutils import nodes
from docutils.statemachine import ViewList
from docutils.parsers.rst import Directive, directives

from nose2 import events, session, util


AD = u'<autodoc>'
__unittest = True


class AutoPlugin(Directive):
    required_arguments = 1
    optional_arguments = 1
    final_argument_whitespace = False
    has_content = False
    option_spec = {'module': directives.unchanged}

    def run(self):
        plugin_name = self.arguments[0]
        parent, plugin = util.object_from_name(plugin_name)
        if isinstance(plugin, types.ModuleType):
            # document all plugins in module
            module = plugin
            mod_name = module.__name__
            plugins = self.plugins(module)

        else:
            if 'module' in self.options:
                mod_name = self.options['module']
            else:
                mod_name = plugin_name[
                    0:plugin_name.index(plugin.__name__) - 1]
            plugins = [plugin]

        rst = ViewList()
        if mod_name:
            rst.append(u'.. automodule :: %s\n' % mod_name, AD)
            rst.append(u'', AD)

        for plug in plugins:
            self.document(rst, plug)

        # parse rst and generate new nodelist
        state = self.state
        node = nodes.section()
        node.document = state.document
        surrounding_title_styles = state.memo.title_styles
        surrounding_section_level = state.memo.section_level
        state.memo.title_styles = []
        state.memo.section_level = 0
        state.nested_parse(rst, 0, node, match_titles=1)
        state.memo.title_styles = surrounding_title_styles
        state.memo.section_level = surrounding_section_level

        return node.children

    def document(self, rst, plugin):
        ssn = session.Session()
        ssn.configClass = ssn.config = config = ConfigBucket()
        ssn.pluginargs = opts = OptBucket()
        plugin_name = plugin.__name__
        config = ssn.config
        obj = plugin(session=ssn)
        try:
            obj.pluginsLoaded(events.PluginsLoadedEvent([obj]))
        except AttributeError:
            pass

        # config options
        if config.vars:
            self.add_config(rst, config)

        # command-line options
        if opts.opts:
            self.headline(rst, u'Command-line options')
            for opt in opts:
                for line in opt.options():
                    rst.append(line, AD)
                rst.append('', AD)

        # class __doc__
        self.headline(rst, u'Plugin class reference: %s' % plugin_name)
        rst.append(u'.. autoclass :: %s' % plugin_name, AD)
        rst.append(u'   :members:', AD)
        rst.append(u'', AD)

    def add_config(self, rst, config):
        headline = u'Configuration [%s]' % config.section
        self.headline(rst, headline)

        for var in sorted(config.vars.keys()):
            info = config.vars[var]
            rst.append(u'.. rst:configvar :: %s' % var, AD)
            rst.append(u'  ', AD)
            rst.append(u'  :Default: %(default)s' % info, AD)
            rst.append(u'  :Type: %(type)s' % info, AD)
            rst.append(u'', AD)

        self.headline(rst, u"Sample configuration", '-')
        rst.append(u'The default configuration is equivalent to including '
                   u'the following in a unittest.cfg file.', AD)
        rst.append(u'', AD)
        rst.append(u'.. code-block:: ini', AD)
        rst.append(u'  ', AD)
        rst.append(u'  [%s]' % config.section, AD)
        for var in sorted(config.vars.keys()):
            info = config.vars[var]
            entry = '  %s = ' % (var)
            if info['type'] != 'list':
                entry = u'%s%s' % (entry, info['default'])
                rst.append(entry, AD)
            elif info['default']:
                pad = ' ' * len(entry)
                entry = u'%s%s' % (entry, info['default'][0])
                rst.append(entry, AD)
                for val in info['default'][1:]:
                    rst.append(u'%s%s' % (pad, val), AD)
            else:
                rst.append(entry, AD)
        rst.append(u'', AD)

    def headline(self, rst, headline, level=u'='):
        rst.append(headline, AD)
        rst.append(level * len(headline), AD)
        rst.append(u'', AD)

    def plugins(self, module):
        for entry in dir(module):
            try:
                item = getattr(module, entry)
            except AttributeError:
                pass
            try:
                if issubclass(item, events.Plugin):
                    yield item
            except TypeError:
                pass


def setup(app):
    app.add_directive('autoplugin', AutoPlugin)
    app.add_object_type('configvar', 'config', u'pair: %s; configvar')


DEFAULT = object()


class ConfigBucket(object):

    def __init__(self):
        self.section = None
        self.vars = {}

    def __call__(self, items):
        self.vars = dict(items)
        return self

    def has_section(self, section):
        self.section = section
        return False

    def items(self):
        return self.vars.items()

    def as_bool(self, item, default=DEFAULT):
        self.vars[item] = {'type': 'boolean',
                           'default': default}
        return default
    as_tri = as_bool

    def as_int(self, item, default=DEFAULT):
        self.vars[item] = {'type': 'integer',
                           'default': default}
        return default

    def as_float(self, item, default=DEFAULT):
        self.vars[item] = {'type': 'float',
                           'default': default}
        return default

    def as_str(self, item, default=DEFAULT):
        self.vars[item] = {'type': 'str',
                           'default': default}
        return default

    def as_list(self, item, default=DEFAULT):
        self.vars[item] = {'type': 'list',
                           'default': default}
        return default

    def __getitem__(self, item):
        self.vars[item] = {'type': None,
                           'default': DEFAULT}

    def get(self, item, default=DEFAULT):
        self.vars[item] = {'type': None,
                           'default': default}
        return default


class OptBucket(object):

    def __init__(self, doc=None, prog='nosetests'):
        self.seen = set()
        self.opts = []
        self.doc = doc
        self.prog = prog

    def __iter__(self):
        return iter(self.opts)

    def format_help(self):
        return self.doc.replace('%prog', self.prog).replace(':\n', '::\n')

    def add_argument(self, *arg, **kw):
        if not arg in self.seen:
            self.opts.append(Opt(*arg, **kw))
            self.seen.add(arg)

    def __call__(self, callback, opt=None, longOpt=None, help=None):
        opts = []
        if opt is not None:
            opts.append('-' + opt)
        if longOpt is not None:
            opts.append('--' + longOpt)
        self.add_option(*opts, help=help)


class Opt(object):

    def __init__(self, *arg, **kw):
        self.opts = arg
        self.action = kw.pop('action', None)
        self.default = kw.pop('default', None)
        self.metavar = kw.pop('metavar', None)
        self.help = kw.pop('help', None)

    def options(self):
        buf = []
        for optstring in self.opts:
            desc = optstring
            if self.action not in ('store_true', 'store_false', None):
                desc += ' %s' % self.meta(optstring)
            buf.append(desc)
            res = ['.. cmdoption :: ' + ', '.join(buf)]
            if self.help:
                res.append('')
                res.append('   %s' % self.help)
            res.append('')
        return res

    def meta(self, optstring):
        # FIXME optparser default metavar?
        return self.metavar or 'DEFAULT'
