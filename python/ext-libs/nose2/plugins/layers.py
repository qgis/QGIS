import logging
import re

import six

from nose2 import events, util
from nose2.suite import LayerSuite
from nose2.compat import unittest, OrderedDict

BRIGHT = r'\033[1m'
RESET = r'\033[0m'

__unittest = True

log = logging.getLogger(__name__)


class Layers(events.Plugin):
    alwaysOn = True

    def startTestRun(self, event):
        event.suite = self._makeLayerSuite(event)

    def _makeLayerSuite(self, event):
        return self._sortByLayers(
            event.suite, self.session.testLoader.suiteClass)

    def _sortByLayers(self, suite, suiteClass):
        top = suiteClass()
        # first find all of the layers mentioned
        layers = OrderedDict()
        for test in self._flatten(suite):
            # split tests up into buckets by layer
            layer = getattr(test, 'layer', None)
            if layer:
                layers.setdefault(layer, LayerSuite(layer=layer)).addTest(test)
            else:
                top.addTest(test)

        # then organize layers into a tree
        remaining = list(layers.keys())
        seen = set()
        tree = {}
        while remaining:
            ly = remaining.pop()
            if ly in seen:
                continue
            seen.add(ly)
            # superclasses of this layer
            if ly is None:
                deps = []
            else:
                deps = [cls for cls in util.bases_and_mixins(ly)
                        if cls is not object]
                deps.reverse()
            if not deps:
                # layer is top-level
                self._addToTree(tree, ly, None)
            else:
                outer = ly
                while deps:
                    inner, outer = outer, deps.pop()
                    self._addToTree(tree, inner, outer)
                    if outer not in layers:
                        remaining.append(outer)
                        layers[outer] = LayerSuite(layer=outer)

        # finally build the top-level suite
        self._treeToSuite(tree, None, top, layers)
        # printtree(top)
        return top

    def _addToTree(self, tree, inner, outer):
        found = False
        for k, v in tree.items():
            if inner in v:
                found = True
                if outer is not None:
                    v.remove(inner)
                break
        if outer is not None or not found:
            tree.setdefault(outer, []).append(inner)

    def _treeToSuite(self, tree, key, suite, layers):
        mysuite = layers.get(key, None)
        if mysuite:
            suite.addTest(mysuite)
            suite = mysuite
        sublayers = tree.get(key, [])
        # ensure that layers with a set order are in order
        sublayers.sort(key=self._sortKey)
        log.debug('sorted sublayers of %s (%s): %s', mysuite,
                  getattr(mysuite, 'layer', 'no layer'), sublayers)
        for layer in sublayers:
            self._treeToSuite(tree, layer, suite, layers)

    def _flatten(self, suite):
        out = []
        for test in suite:
            try:
                out.extend(self._flatten(test))
            except TypeError:
                out.append(test)
        return out

    def _sortKey(self, layer):
        pos = getattr(layer, 'position', None)
        # ... lame
        if pos is not None:
            key = six.u("%04d") % pos
        else:
            key = layer.__name__
        return key


class LayerReporter(events.Plugin):
    commandLineSwitch = (
        None, 'layer-reporter', 'Add layer information to test reports')
    configSection = 'layer-reporter'

    def __init__(self):
        self.indent = self.config.as_str('indent', '  ')
        self.colors = self.config.as_bool('colors', False)
        self.highlight_words = self.config.as_list('highlight-words',
                                                   ['A', 'having', 'should'])
        self.highlight_re = re.compile(
            r'\b(%s)\b' % '|'.join(self.highlight_words))
        self.layersReported = set()

    def reportStartTest(self, event):
        if self.session.verbosity < 2:
            return
        test = event.testEvent.test
        layer = getattr(test, 'layer', None)
        if not layer:
            return
        for ix, lys in enumerate(util.ancestry(layer)):
            for layer in lys:
                if layer not in self.layersReported:
                    desc = self.describeLayer(layer)
                    event.stream.writeln('%s%s' % (self.indent * ix, desc))
                    self.layersReported.add(layer)
        event.stream.write(self.indent * (ix + 1))

    def describeLayer(self, layer):
        return self.format(getattr(layer, 'description', layer.__name__))

    def format(self, st):
        if self.colors:
            return self.highlight_re.sub(r'%s\1%s' % (BRIGHT, RESET), st)
        return st

    def describeTest(self, event):
        if hasattr(event.test, 'methodDescription'):
            event.description = self.format(event.test.methodDescription())
        if event.errorList and hasattr(event.test, 'layer'):
            # walk back layers to build full description
            self.describeLayers(event)

    def describeLayers(self, event):
        desc = [event.description]
        base = event.test.layer
        for layer in (base.__mro__ + getattr(base, 'mixins', ())):
            if layer is object:
                continue
            desc.append(self.describeLayer(layer))
        desc.reverse()
        event.description = ' '.join(desc)


# for debugging
def printtree(suite, indent=''):
    six.print_('%s%s ->' % (indent, getattr(suite, 'layer', 'no layer')))
    for test in suite:
        if isinstance(test, unittest.BaseTestSuite):
            printtree(test, indent + '  ')
        else:
            six.print_('%s %s' % (indent, test))
    six.print_('%s<- %s' % (indent, getattr(suite, 'layer', 'no layer')))
