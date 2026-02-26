"""QGIS Unit tests for the unit tests ;)

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import ast
import os
import re
import sys
import unittest
from pathlib import Path

from qgis.testing import QgisTestCase

TEST_DIR = Path(__file__).parent.resolve()
CMAKE_FILE = TEST_DIR / "CMakeLists.txt"
TEST_CLASS_BASES = ("QgisTestCase", "TestCase")
RX_ADD_PYTHON_TEST = re.compile(
    r"\s*ADD_PYTHON_TEST\s*\((.*?) +(.*?)(?: +.*)?\)\s*", re.IGNORECASE
)


class TestQgsTests(QgisTestCase):
    @staticmethod
    def contains_test_class(filepath: Path):
        """
        Parses a Python file to check if it contains a target test class.
        """
        with open(filepath, encoding="utf-8") as f:
            tree = ast.parse(f.read(), filename=filepath)

        for node in ast.walk(tree):
            if isinstance(node, ast.ClassDef):
                is_test_class = False
                for base in node.bases:
                    if isinstance(base, ast.Name) and base.id in TEST_CLASS_BASES:
                        is_test_class = True
                        break
                    elif (
                        isinstance(base, ast.Attribute)
                        and base.attr == "TestCase"
                        and isinstance(base.value, ast.Name)
                        and base.value.id == "unittest"
                    ):
                        is_test_class = True
                        break
                if is_test_class:
                    for item in node.body:
                        # check for methods that start with 'test'. if none found, then this is
                        # a "mixin" class which will be used by another actual class which
                        # does the actual tests
                        if isinstance(item, ast.FunctionDef) and item.name.startswith(
                            "test"
                        ):
                            return True
        return False

    def test_all_tests_in_cmake(self):
        """
        Tests that all test files are actually in CMakeLists.txt, and will be used!
        """
        self.assertTrue(TEST_DIR.exists())
        self.assertTrue(TEST_DIR.is_dir())
        self.assertTrue(CMAKE_FILE.exists())

        # scan for python files which contain tests
        all_test_files = set()
        for filepath in TEST_DIR.rglob("*.py"):
            if self.contains_test_class(filepath):
                all_test_files.add(filepath.name)

        #  scan CMakeLists.txt to see which test files are actually registered
        registered_files = set()
        with open(CMAKE_FILE, encoding="utf-8") as f_in:
            cmake_content = f_in.readlines()

        for line in cmake_content:
            match = RX_ADD_PYTHON_TEST.match(line)
            if match is None:
                continue

            test_name, test_file = match.group(1), match.group(2)
            self.assertEqual(test_file[-3:], ".py", line)
            registered_files.add(test_file)

        missing_files = all_test_files - registered_files

        if missing_files:
            print(
                f"ERROR: The following Python test files are missing from {CMAKE_FILE.name}:\n"
            )
            for f in sorted(missing_files):
                print(f"  - {f}")

            print("\nPlease add them using the 'ADD_PYTHON_TEST(...)' macro.")

        self.assertFalse(missing_files)


if __name__ == "__main__":
    unittest.main()
