import sys
import inspect
import logging

from nose2 import util
from nose2.compat import unittest

log = logging.getLogger(__name__)

__unittest = True

#
# Layer suite class
#


class LayerSuite(unittest.BaseTestSuite):

    def __init__(self, tests=(), layer=None):
        super(LayerSuite, self).__init__(tests)
        self.layer = layer
        self.wasSetup = False

    def run(self, result):
        if not self._safeMethodCall(self.setUp, result):
            return
        try:
            for test in self:
                if result.shouldStop:
                    break
                self.setUpTest(test)
                try:
                    test(result)
                finally:
                    self._safeMethodCall(self.tearDownTest, result, test)
        finally:
            if self.wasSetup:
                self._safeMethodCall(self.tearDown, result)

    def setUp(self):
        # FIXME hook call
        log.debug('in setUp layer %s', self.layer)
        if self.layer is None:
            return

        setup = self._getBoundClassmethod(self.layer, 'setUp')
        if setup:
            setup()
            log.debug('setUp layer %s called', self.layer)
        self.wasSetup = True

    def setUpTest(self, test):
        # FIXME hook call
        if self.layer is None:
            return
        # skip suites, to ensure test setup only runs once around each test
        # even for sub-layer suites inside this suite.
        try:
            iter(test)
        except TypeError:
            # ok, not a suite
            pass
        else:
            # suite-like enough for skipping
            return
        if getattr(test, '_layer_wasSetUp', False):
            return
        self._allLayers(test, 'testSetUp')
        test._layer_wasSetUp = True

    def tearDownTest(self, test):
        # FIXME hook call
        if self.layer is None:
            return
        if not getattr(test, '_layer_wasSetUp', None):
            return
        self._allLayers(test, 'testTearDown', reverse=True)
        delattr(test, '_layer_wasSetUp')

    def tearDown(self):
        # FIXME hook call
        if self.layer is None:
            return

        teardown = self._getBoundClassmethod(self.layer, 'tearDown')
        if teardown:
            teardown()
            log.debug('tearDown layer %s called', self.layer)

    def _safeMethodCall(self, method, result, *args):
        try:
            method(*args)
            return True
        except KeyboardInterrupt:
            raise
        except:
            result.addError(self, sys.exc_info())
            return False
        
    def _allLayers(self, test, method, reverse=False):
        done = set()
        all_lys = util.ancestry(self.layer)
        if reverse:
            all_lys = [reversed(lys) for lys in reversed(all_lys)]
        for lys in all_lys:
            for layer in lys:
                if layer in done:
                    continue
                self._inLayer(layer, test, method)
                done.add(layer)

    def _inLayer(self, layer, test, method):
        meth = self._getBoundClassmethod(layer, method)
        if meth:
            args, _, _, _ = inspect.getargspec(meth)
            if len(args) > 1:
                meth(test)
            else:
                meth()

    def _getBoundClassmethod(self, cls, method):
        """
        Use instead of getattr to get only classmethods explicitly defined
        on cls (not methods inherited from ancestors)
        """
        descriptor = cls.__dict__.get(method, None)
        if descriptor:
            if not isinstance(descriptor, classmethod):
                raise TypeError(
                    'The %s method on a layer must be a classmethod.' % method)
            bound_method = descriptor.__get__(None, cls)
            return bound_method
        else:
            return None
