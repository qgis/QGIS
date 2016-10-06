import sys
from nose2.tests._common import FunctionalTestCase, support_file
import nose2.plugins.loader.eggdiscovery #@UnusedImport This is not very elegant, but it allows eggdiscovery to be found in Travis (or when run with PYTHONPATH=.)

try:
    import pkg_resources

except ImportError:
    pkg_resources = None

else:
    class EggDiscoveryFunctionalTest(FunctionalTestCase):
        def setUp(self):
            for m in [m for m in sys.modules if m.startswith('pkgegg')]:
                del sys.modules[m]
            self.egg_path = support_file('scenario/tests_in_zipped_eggs/pkgegg-0.0.0-py2.7.egg')
            sys.path.append(self.egg_path)
    
        def tearDown(self):
            if self.egg_path in sys.path:
                sys.path.remove(self.egg_path)
            for m in [m for m in sys.modules if m.startswith('pkgegg')]:
                del sys.modules[m]
        
        def test_non_egg_discoverer_does_not_fail_when_looking_in_egg(self):
            proc = self.runIn(
                'scenario/tests_in_zipped_eggs',
                '-v',
                'pkgegg')
            self.assertTestRunOutputMatches(proc, stderr='Ran 0 tests in')
        
        def test_can_discover_test_modules_in_zipped_eggs(self):
            proc = self.runIn(
                'scenario/tests_in_zipped_eggs', 
                '-v',
                '--plugin=nose2.plugins.loader.eggdiscovery',
                'pkgegg')
            self.assertTestRunOutputMatches(proc, stderr='FAILED \(failures=5, errors=1, skipped=1\)')
    
        def test_eggdiscovery_failure_does_not_exist(self):
            proc = self.runIn(
                'scenario', 
                '-v',
                '--plugin=nose2.plugins.loader.eggdiscovery',
                '--exclude-plugin=nose2.plugins.loader.discovery',
                '-s',
                'tests_in_zipped_eggs_BAD')
            self.assertTestRunOutputMatches(proc, stderr='tests_in_zipped_eggs_BAD does not exist')
    
    class UnzippedEggDiscoveryFunctionalTest(FunctionalTestCase):
        def setUp(self):
            for m in [m for m in sys.modules if m.startswith('pkgegg')]:
                del sys.modules[m]
            self.egg_path = support_file('scenario/tests_in_unzipped_eggs/pkgunegg-0.0.0-py2.7.egg')
            sys.path.append(self.egg_path)
    
        def tearDown(self):
            if self.egg_path in sys.path:
                sys.path.remove(self.egg_path)
            for m in [m for m in sys.modules if m.startswith('pkgunegg')]:
                del sys.modules[m]
        
        def test_eggdiscovery_ignores_unzipped_eggs(self):
            proc = self.runIn(
                'scenario/tests_in_unzipped_eggs', 
                '-v',
                '--plugin=nose2.plugins.loader.eggdiscovery',
                'pkgunegg')
            self.assertTestRunOutputMatches(proc, stderr='FAILED \(failures=5, errors=1, skipped=1\)')
    