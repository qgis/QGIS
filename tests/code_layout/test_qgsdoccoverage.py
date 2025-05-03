"""QGIS Unit tests for API documentation coverage.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "01/02/2015"
__copyright__ = "Copyright 2016, The QGIS Project"

import json
import os
import sys

from pathlib import Path

try:
    from qgis.static_testing import unittest
except ImportError:
    import unittest

from doxygen_parser import DoxygenParser
from termcolor import colored

REGENERATE_LISTS = False


class TestQgsDocCoverage(unittest.TestCase):

    def testCoverage(self):
        print("CTEST_FULL_OUTPUT")
        prefixPath = os.environ["QGIS_PREFIX_PATH"]
        docPath = os.path.join(prefixPath, "..", "doc", "api", "xml")

        doc_test_config_file = Path(__file__).parent / "doc_test.json"
        with open(doc_test_config_file) as f:
            doc_test_config = json.loads(f.read())

        parser = DoxygenParser(
            docPath,
            doc_test_config["acceptable_missing_doc"],
            doc_test_config["acceptable_missing_added_note"],
            doc_test_config["acceptable_missing_brief"],
        )

        coverage = 100.0 * parser.documented_members / parser.documentable_members
        missing = parser.documentable_members - parser.documented_members

        if REGENERATE_LISTS:
            with open(doc_test_config_file, "w") as f:
                all_undocumented = {}
                for c in sorted(list(parser.all_undocumented_members.keys())):
                    all_undocumented[c] = sorted(parser.all_undocumented_members[c])
                f.write(
                    json.dumps(
                        {
                            "acceptable_missing_doc": all_undocumented,
                            "acceptable_missing_added_note": sorted(
                                parser.all_classes_missing_version_added
                            ),
                            "acceptable_missing_brief": [],
                        },
                        indent=2,
                    )
                )

        print("---------------------------------")
        print(f"{parser.documentable_members} total documentable members")
        print(f"{parser.documented_members} total contain valid documentation")
        print(f"Total documentation coverage {coverage}%")
        print("---------------------------------")
        print(f"{missing} members missing documentation")
        print("---------------------------------")
        print("Unacceptable missing documentation:")

        if parser.undocumented_members:
            for cls, props in list(parser.undocumented_members.items()):
                print(
                    "\n\nClass {}, {}/{} members documented\n".format(
                        colored(cls, "yellow"), props["documented"], props["members"]
                    )
                )
                for mem in props["missing_members"]:
                    print(colored('  "' + mem + '"', "yellow", attrs=["bold"]))

        if parser.noncompliant_members:
            for cls, props in list(parser.noncompliant_members.items()):
                print(
                    "\n\nClass {}, non-compliant members found\n".format(
                        colored(cls, "yellow")
                    )
                )
                for p in props:
                    for mem, error in p.items():
                        print(
                            colored("  " + mem + ": " + error, "yellow", attrs=["bold"])
                        )

        if parser.broken_links:
            for cls, props in list(parser.broken_links.items()):
                print(
                    "\n\nClass {}, broken see also links found\n".format(
                        colored(cls, "yellow")
                    )
                )
                for member, links in props.items():
                    for l in links:
                        print(
                            colored("  " + member + ": " + l, "yellow", attrs=["bold"])
                        )
        # self.assertEquals(len(parser.undocumented_string), 0, 'FAIL: new undocumented members have been introduced, please add documentation for these members')

        if parser.classes_missing_group:
            print("---------------------------------")
            print("\n")
            print(
                colored(
                    f'{len(parser.classes_missing_group)} classes have been added without Doxygen group tag ("\\ingroup"):',
                    "yellow",
                )
            )
            print("")
            print(
                "  "
                + "\n  ".join(
                    [
                        colored(cls, "yellow", attrs=["bold"])
                        for cls in parser.classes_missing_group
                    ]
                )
            )

        if parser.classes_missing_version_added:
            print("---------------------------------")
            print("\n")
            print(
                colored(
                    f'{len(parser.classes_missing_version_added)} classes have been added without a version added doxygen note ("\\since QGIS x.xx"):',
                    "yellow",
                )
            )
            print("")
            print(
                "  "
                + "\n  ".join(
                    [
                        colored(cls, "yellow", attrs=["bold"])
                        for cls in parser.classes_missing_version_added
                    ]
                )
            )

        if parser.classes_missing_brief:
            print("---------------------------------")
            print("\n")
            print(
                colored(
                    f"{len(parser.classes_missing_brief)} classes have been added without at least a brief description:",
                    "yellow",
                )
            )
            print("")
            print(
                "  "
                + "\n  ".join(
                    [
                        colored(cls, "yellow", attrs=["bold"])
                        for cls in parser.classes_missing_brief
                    ]
                )
            )

        sys.stdout.flush()
        self.assertTrue(not parser.undocumented_members, "Undocumented members found")
        self.assertTrue(
            not parser.classes_missing_group, "Classes without \\group tag found"
        )
        self.assertTrue(
            not parser.classes_missing_version_added,
            "Classes without \\since version tag found",
        )
        self.assertTrue(
            not parser.classes_missing_brief,
            "Classes without \\brief description found",
        )
        self.assertTrue(not parser.noncompliant_members, "Non compliant members found")
        self.assertTrue(not parser.broken_links, "Broken links found")


if __name__ == "__main__":
    unittest.main()
