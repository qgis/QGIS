# -*- coding: utf-8 -*-

"""
***************************************************************************
    OTBTester.py
    ---------------------
    Copyright            : (C) 2013 by CS Systemes d'information (CS SI)
    Email                : otb at c-s dot fr (CS SI)
    Contributors         : Julien Malik (CS SI)
                           Oscar Picas (CS SI)
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""
__author__ = 'Julien Malik, Oscar Picas'
__copyright__ = '(C) 2013, CS Systemes d\'information  (CS SI)'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import unittest
import ConfigParser
import io

from parsing import (
    File, Command, Comment, BlankLine, Arg, parse, prettify)

from string import Template
import os
import traceback
import logging
import copy

from ConfigParser import SafeConfigParser

from processing.otb.OTBHelper import get_OTB_log

class LowerTemplate(Template):
    def safe_substitute(self, param):
        ret = super(LowerTemplate, self).safe_substitute(param).lower()
        return ret

class MakefileParser(object):
    def __init__(self):
        self.maxDiff = None
        self.parser = SafeConfigParser()
        self.parser.read('otbcfg.ini')
        if not os.path.exists('otbcfg.ini'):
            raise Exception("OTB_SOURCE_DIR and OTB_BINARY_DIR must be specified in the file otbcfg.ini")

        self.root_dir = self.parser.get('otb','checkout_dir')
        if not os.path.exists(self.root_dir):
            raise Exception("Check otbcfg.ini : OTB_SOURCE_DIR and OTB_BINARY_DIR must be specified there")
        self.build_dir = self.parser.get('otb', 'build_dir')
        if not os.path.exists(self.build_dir):
            raise Exception("Check otbcfg.ini : OTB_SOURCE_DIR and OTB_BINARY_DIR must be specified there")
        self.logger = get_OTB_log()

    def test_CMakelists(self):
        provided = {}
        provided["OTB_SOURCE_DIR"] = self.root_dir
        provided["OTB_BINARY_DIR"] = self.build_dir
        provided["OTB_DATA_LARGEINPUT_ROOT"] = os.path.normpath(os.path.join(self.root_dir, "../OTB-Data/Input"))

        try:
            with open(os.path.join(self.root_dir, "CMakeLists.txt")) as file_input:
                content = file_input.read()
                output = parse(content)

                defined_paths = [each for each in output if 'Command' in str(type(each)) and "FIND_PATH" in each.name]
                the_paths = {key.body[0].contents: [thing.contents for thing in key.body[1:]] for key in defined_paths}

                the_sets = [each for each in output if 'Command' in str(type(each)) and "SET" in each.name.upper()]
                the_sets = {key.body[0].contents: [thing.contents for thing in key.body[1:]] for key in the_sets}
                the_sets = {key : " ".join(the_sets[key]) for key in the_sets}

                the_strings = set([each.body[-1].contents for each in output if 'Command' in str(type(each)) and "STRING" in each.name.upper()] )

                def mini_clean(item):
                    if item.startswith('"') and item.endswith('"') and " " not in item:
                        return item[1:-1]
                    return item

                the_sets = {key : mini_clean(the_sets[key]) for key in the_sets}

                def templatize(item):
                    if "$" in item:
                        return Template(item)
                    return item

                for key in the_sets:
                    if key in the_strings:
                        the_sets[key] = the_sets[key].lower()

                the_sets = {key : templatize(the_sets[key]) for key in the_sets}

                for path in the_paths:
                    target_file = the_paths[path][1]
                    suggested_paths = []
                    if len(the_paths[path]) > 2:
                        suggested_paths = the_paths[path][2:]

                    try:
                        provided[path] = find_file(target_file)
                    except Exception, e:
                        for each in suggested_paths:
                            st = Template(each)
                            pac = os.path.abspath(st.safe_substitute(provided))
                            if os.path.exists(pac):
                                provided[path] = pac
                                break

                resolve_dict(provided, the_sets)
                provided.update(the_sets)

                return provided
        except Exception, e:
            traceback.print_exc()
            self.fail(e.message)

    def add_make(self, previous_context, new_file):
        input = open(new_file).read()
        output = parse(input)
        apps = [each for each in output if 'Command' in str(type(each))]
        setcommands = [each for each in apps if 'SET' in each.name.upper()]
        stringcommands = [each for each in apps if 'STRING' in each.name.upper()]

        environment = previous_context

        def mini_clean(item):
            if item.startswith('"') and item.endswith('"') and " " not in item:
                return item[1:-1]
            return item

        new_env = {}
        for command in setcommands:
            key = command.body[0].contents
            ct = " ".join([item.contents for item in command.body[1:]])
            ct = mini_clean(ct)

            if "$" in ct:
                values = Template(ct)
            else:
                values = ct

            new_env[key] = values

        for stringcommand in stringcommands:
            key = stringcommand.body[-1].contents
            ct = stringcommand.body[-2].contents
            ct = mini_clean(ct.lower())

            if "$" in ct:
                values = LowerTemplate(ct)
            else:
                values = ct
            new_env[key] = values

        resolve_dict(environment, new_env)
        environment.update(new_env)

        return environment

    def get_apps(self, the_makefile, the_dict):
        input = open(the_makefile).read()
        output = parse(input)
        apps = [each for each in output if 'Command' in str(type(each))]
        otb_apps = [each for each in apps if 'OTB_TEST_APPLICATION' in each.name.upper()]
        return otb_apps

    def get_tests(self, the_makefile, the_dict):
        input = open(the_makefile).read()
        output = parse(input)
        apps = [each for each in output if 'Command' in str(type(each))]
        otb_tests = [each for each in apps if 'ADD_TEST' in each.name.upper()]
        return otb_tests

    def get_apps_with_context(self, the_makefile, the_dict):
        input = open(the_makefile).read()
        output = parse(input)

        def is_a_command(item):
            return 'Command' in str(type(item))

        appz = []
        context = []
        for each in output:
            if is_a_command(each):
                if 'FOREACH' in each.name and 'ENDFOREACH' not in each.name:
                    args = [item.contents for item in each.body]
                    context.append(args)
                elif 'ENDFOREACH' in each.name:
                    context.pop()
                elif 'OTB_TEST_APPLICATION' in each.name.upper():
                    appz.append((each, context[:]))
        return appz

    def get_name_line(self, the_list, the_dict):
        items = ('NAME', 'APP', 'OPTIONS', 'TESTENVOPTIONS', 'VALID')
        itemz = [[], [], [], [], []]
        last_index = 0
        for each in the_list:
            if each.contents in items:
                last_index = items.index(each.contents)
            else:
                itemz[last_index].append(each.contents)
        result = itemz[0][0]
        the_string = Template(result).safe_substitute(the_dict)

        if '$' in the_string:
            neo_dict = the_dict
            the_string = Template(the_string).safe_substitute(neo_dict)
            while '$' in the_string:
                try:
                    the_string = Template(the_string).substitute(neo_dict)
                except KeyError, e:
                    self.logger.warning("Key %s is not found in makefiles" % e.message)
                    neo_dict[e.message] = ""

        if 'string.Template' in the_string:
            raise Exception("Unexpected toString call in %s" % the_string)

        return the_string

    def get_command_line(self, the_list, the_dict):
        items = ('NAME', 'APP', 'OPTIONS', 'TESTENVOPTIONS', 'VALID')
        itemz = [[], [], [], [], []]
        last_index = 0
        for each in the_list:
            if each.contents in items:
                last_index = items.index(each.contents)
            else:
                itemz[last_index].append(each.contents)
        result = []
        result.extend(["otbcli_%s" % each for each in itemz[1]])

        if len(result[0]) == 7:
            raise Exception("App name is empty !")

        result.extend(itemz[2])
        result.append("-testenv")
        result.extend(itemz[3])
        the_string = Template(" ".join(result)).safe_substitute(the_dict)

        if '$' in the_string:
            neo_dict = the_dict
            the_string = Template(" ".join(result)).safe_substitute(neo_dict)
            while '$' in the_string:
                try:
                    the_string = Template(the_string).substitute(neo_dict)
                except KeyError, e:
                    self.logger.warning("Key %s is not found in makefiles" % e.message)
                    neo_dict[e.message] = ""

        if 'string.Template' in the_string:
            raise Exception("Unexpected toString call in %s" % the_string)

        return the_string

    def get_test(self, the_list, the_dict):
        items = ('NAME', 'APP', 'OPTIONS', 'TESTENVOPTIONS', 'VALID')
        itemz = [[], [], [], [], []]
        last_index = 0
        for each in the_list:
            if each.contents in items:
                last_index = items.index(each.contents)
            else:
                itemz[last_index].append(each.contents)
        result = ["otbTestDriver"]
        result.extend(itemz[4])

        if len(result) == 1:
            return ""

        the_string = Template(" ".join(result)).safe_substitute(the_dict)

        if '$' in the_string:
            neo_dict = the_dict
            the_string = Template(" ".join(result)).safe_substitute(neo_dict)
            while '$' in the_string:
                try:
                    the_string = Template(the_string).substitute(neo_dict)
                except KeyError, e:
                    self.logger.warning("Key %s is not found in makefiles" % e.message)
                    neo_dict[e.message] = ""

        if 'string.Template' in the_string:
            raise Exception("Unexpected toString call in %s" % the_string)

        return the_string

    def test_algos(self):
        tests = {}

        algos_dir = os.path.join(self.root_dir, "Testing/Applications")
        makefiles = find_files("CMakeLists.txt", algos_dir)
        to_be_excluded = os.path.join(self.root_dir, "Testing/Applications/CMakeLists.txt")
        if to_be_excluded in makefiles:
            makefiles.remove(to_be_excluded)

        resolve_algos = {}
        for makefile in makefiles:
            intermediate_makefiles = []
            path = makefile.split(os.sep)[len(self.root_dir.split(os.sep)):-1]
            for ind in range(len(path)):
                tmp_path = path[:ind+1]
                tmp_path.append("CMakeLists.txt")
                tmp_path = os.sep.join(tmp_path)
                candidate_makefile = os.path.join(self.root_dir, tmp_path)
                if os.path.exists(candidate_makefile):
                    intermediate_makefiles.append(candidate_makefile)
            resolve_algos[makefile] = intermediate_makefiles

        dict_for_algo = {}
        for makefile in makefiles:
            basic = self.test_CMakelists()
            last_context = self.add_make(basic, os.path.join(self.root_dir, "Testing/Utilities/CMakeLists.txt"))
            for intermediate_makefile in resolve_algos[makefile]:
                last_context = self.add_make(last_context, intermediate_makefile)
            dict_for_algo[makefile] = last_context

        for makefile in makefiles:
            appz = self.get_apps_with_context(makefile, dict_for_algo[makefile])

            for app, context in appz:
                if len(context) == 0:
                    import copy
                    ddi = copy.deepcopy(dict_for_algo[makefile])
                    tk_dict = autoresolve(ddi)
                    tk_dict = autoresolve(tk_dict)

                    name_line = self.get_name_line(app.body, tk_dict)
                    command_line = self.get_command_line(app.body, tk_dict)
                    test_line = self.get_test(app.body, tk_dict)

                    if '$' in test_line or '$' in command_line:
                        if '$' in command_line:
                            self.logger.error(command_line)
                        if '$' in test_line:
                            self.logger.warning(test_line)
                    else:
                        tests[name_line] = (command_line, test_line)
                else:
                    contexts = {}
                    for iteration in context:
                        key = iteration[0]
                        values = [each[1:-1].lower() for each in iteration[1:]]
                        contexts[key] = values

                    keyorder = contexts.keys()
                    import itertools
                    pool = [each for each in itertools.product(*contexts.values())]

                    import copy
                    for poolinstance in pool:
                        neo_dict = copy.deepcopy(dict_for_algo[makefile])
                        zipped = zip(keyorder, poolinstance)
                        for each in zipped:
                            neo_dict[each[0]] = each[1]

                        ak_dict = autoresolve(neo_dict)
                        ak_dict = autoresolve(ak_dict)
                        ak_dict = autoresolve(ak_dict)

                        ddi = ak_dict

                        name_line = self.get_name_line(app.body, ddi)
                        command_line = self.get_command_line(app.body, ddi)
                        test_line = self.get_test(app.body, ddi)

                        if '$' in command_line or '$' not in test_line:
                            if '$' in command_line:
                                self.logger.error(command_line)
                            if '$' in test_line:
                                self.logger.warning(test_line)
                        else:
                            tests[name_line] = (command_line, test_line)

        return tests

def autoresolve(a_dict):
    def as_template(item, b_dict):
        if hasattr(item, 'safe_substitute'):
            return item.safe_substitute(b_dict)
        ate = Template(item)
        return ate.safe_substitute(b_dict)
    templatized = {key: as_template(a_dict[key], a_dict) for key in a_dict.keys() }
    return templatized


def find_file(file_name, base_dir = os.curdir):
    import os
    for root, dirs, files in os.walk(base_dir, topdown=False):
        for name in files:
            if name == file_name:
                return os.path.join(root, name)
    raise Exception("File not found %s" % file_name)

def find_files(file_name, base_dir = os.curdir):
    import os
    result = []
    for root, dirs, files in os.walk(base_dir, topdown=False):
        for name in files:
            if name == file_name:
                result.append(os.path.join(root, name))
    return result

def resolve_dict(adia, adib):
    init = len(adia)
    fin = len(adia) + 1
    def _resolve_dict(dia, dib):
        for key in dib:
            cand_value = dib[key]
            if hasattr(cand_value, 'safe_substitute'):
                value = cand_value.safe_substitute(dia)
                if type(value) == type(".") and "$" not in value:
                    dia[key] = value
            else:
                dia[key] = cand_value
        for key in dia:
            if key in dib:
                del dib[key]

    while(init != fin):
        init = len(adia)
        _resolve_dict(adia, adib)
        fin = len(adia)

