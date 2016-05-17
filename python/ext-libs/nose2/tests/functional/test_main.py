from nose2.tests._common import FunctionalTestCase


class TestPluggableTestProgram(FunctionalTestCase):

    def test_run_in_empty_dir_succeeds(self):
        proc = self.runIn('scenario/no_tests')
        stdout, stderr = proc.communicate()
        self.assertEqual(proc.poll(), 0, stderr)

    def test_extra_hooks(self):
        class Check(object):
            ran = False

            def startTestRun(self, event):
                self.ran = True

        check = Check()
        proc = self.runIn('scenario/no_tests',
                          extraHooks=[('startTestRun', check)])
        stdout, stderr = proc.communicate()
        self.assertEqual(proc.poll(), 0, stderr)
        assert check.ran, "Extra hook did not execute"

    def test_run_in_module_from_its_main(self):
        proc = self.runModuleAsMain('scenario/one_test/tests.py')
        self.assertTestRunOutputMatches(proc, stderr='Ran 1 test')
        self.assertEqual(proc.poll(), 0)