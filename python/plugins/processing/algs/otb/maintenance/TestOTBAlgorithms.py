# -*- coding: utf-8 -*-

"""
***************************************************************************
    TestOTBAlgorithms.py
    ---------------------
    Copyright            : (C) 2013 by CS Systemes d'information
    Email                : otb at c-s dot fr
    Contributors         : Oscar Picas
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import unittest
import signal
import os
import shlex
import subprocess
import shelve

try:
    import processing
except ImportError as e:
    raise Exception("Processing must be installed and available in PYTHONPATH")

try:
    import otbApplication
except ImportError as e:
    raise Exception("OTB python plugins must be installed and available in PYTHONPATH")

from processing.algs.otb.OTBHelper import get_OTB_log, create_xml_descriptors
from processing.algs.otb.OTBTester import MakefileParser


class Alarm(Exception):
    pass


def alarm_handler(signum, frame):
    raise Alarm


class AlgoTestCase(unittest.TestCase):

    def setUp(self):
        self.logger = get_OTB_log()
        self.the_files = [os.path.join(os.path.join(os.path.abspath(os.curdir), 'description'), each) for each in os.listdir(os.path.join(os.path.abspath(os.curdir), 'description')) if '.xml' in each]

    def tearDown(self):
        self.logger = None


class TestSequence(unittest.TestCase):

    def setUp(self):
        self.data = shelve.open("tests.shelve", writeback=True)

    def tearDown(self):
        self.data.close()


def ut_generator(test_name, a_tuple):
    def test(self):
        logger = get_OTB_log()

        needs_update = False
        if test_name not in self.data:
            needs_update = True
        if test_name in self.data:
            if (self.data[test_name][0] != a_tuple[0]) or (self.data[test_name][1] != a_tuple[1]) or (self.data[test_name][2] is False):
                needs_update = True

        if needs_update:
            signal.signal(signal.SIGALRM, alarm_handler)
            signal.alarm(6 * 60)  # 6 minutes

            black_list = []

            ut_command = a_tuple[0]
            self.assertTrue(ut_command is not None)
            self.assertTrue(ut_command != "")

            ut_command_validation = a_tuple[1]
            self.assertTrue(ut_command_validation is not None)
            self.assertTrue(ut_command_validation != "")

            if ut_command.split(" ")[0] in black_list:
                raise Exception("Blacklisted test !")

            args = shlex.split(ut_command)
            failed = False
            logger.info("Running [%s]" % ut_command)
            p = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            (pout, perr) = p.communicate()
            if ("ERROR" in pout or "ERROR" in perr) or ("FATAL" in pout or "FATAL" in perr) or ("CRITICAL" in pout or "CRITICAL" in perr):
                error_text = "Command [%s] returned [%s]" % (ut_command, pout)
                if "Invalid image filename" in pout or "Invalid vector data filename" in pout or "Failed to open" in pout:
                    logger.warning(error_text)
                else:
                    logger.error(error_text)
                    self.fail(error_text)
                failed = True
            else:
                logger.info(pout)

            if (len(ut_command_validation) > 0) and not failed:
                new_ut_command_validation = ut_command_validation + " Execute " + ut_command

                logger.info("Running Unit test [%s]" % new_ut_command_validation)
                argz = shlex.split(new_ut_command_validation)
                q = subprocess.Popen(argz, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                (qout, qerr) = q.communicate()

                if not ("Test EXIT SUCCESS" in qout or "Test EXIT SUCCESS" in qerr):
                    error_text = "Unit test [%s] returned [%s]" % (new_ut_command_validation, qout)
                    if "Invalid image filename" in qout or "Invalid vector data filename" in qout or "Failed to open" in qout:
                        logger.warning(error_text)
                    else:
                        logger.error(error_text)
                        self.fail(error_text)
                else:
                    logger.info(qout)

            signal.alarm(0)
            self.data[test_name] = [a_tuple[0], a_tuple[1], failed]
        else:
            logger.info("Passed test: %s" % test_name)

    return test


def get_client_apps():
    app_clients = []
    for available_app in otbApplication.Registry.GetAvailableApplications():
        app_instance = otbApplication.Registry.CreateApplication(available_app)
        app_instance.UpdateParameters()
        ct = "otbcli_" + available_app
        app_clients.append(ct)
    return app_clients


def unfiltered_processing_mapping():
    mkf = MakefileParser()
    the_tests = mkf.test_algos()
    for t in the_tests:
        test_name = 'test_std_%s' % t
        if the_tests[t][1] is None:
            skip = True
        else:
            if the_tests[t][1] == "":
                skip = True

        if not skip:
            test = ut_generator(test_name, the_tests[t])
            setattr(TestSequence, test_name, test)

    suite = unittest.TestLoader().loadTestsFromTestCase(TestSequence)
    unittest.TextTestRunner(verbosity=2).run(suite)


def test_processing_mapping():
    mkf = MakefileParser()
    the_tests = mkf.test_algos()
    clients = get_client_apps()

    already_tested = set()

    for t in the_tests:
        test_name = 'test_%s' % t
        if the_tests[t][0].split(" ")[0] in clients:
            skip = False
            if the_tests[t][1] is None:
                skip = True
            else:
                if the_tests[t][1] == "":
                    skip = True

            if not skip:
                runnable = the_tests[t][0].split(" ")[0]
                if runnable not in already_tested:
                    test = ut_generator(test_name, the_tests[t])
                    setattr(TestSequence, test_name, test)
                    already_tested.add(runnable)

    suite = unittest.TestLoader().loadTestsFromTestCase(TestSequence)
    unittest.TextTestRunner(verbosity=2).run(suite)


def test_xml_generation():
    create_xml_descriptors()

if __name__ == '__main__':
    mkf = MakefileParser()
    the_tests = mkf.test_algos()
    for t in the_tests:
        test_name = 'test_%s' % t
        test = ut_generator(test_name, the_tests[t])
        setattr(TestSequence, test_name, test)
    unittest.main()
