#!/usr/bin/env python3

#
# This file is part of KDToolBox.
#
# SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# Author: Jesper K. Pedersen <jesper.pedersen@kdab.com>
#
# SPDX-License-Identifier: MIT
#

"""
Script to add inclusion of mocs to files recursively.
"""

# pylint: disable=redefined-outer-name

import argparse
import os
import re
import sys

dirty = False


def stripInitialSlash(path):
    if path and path.startswith("/"):
        path = path[1:]
    return path


# Returns true if the path is to be excluded from the search


def shouldExclude(root, path):
    # pylint: disable=used-before-assignment,possibly-used-before-assignment
    if not args.excludes:
        return False  # No excludes provided

    assert root.startswith(args.root)
    root = stripInitialSlash(root[len(args.root) :])

    if args.headerPrefix:
        assert root.startswith(args.headerPrefix)
        root = stripInitialSlash(root[len(args.headerPrefix) :])

    return (path in args.excludes) or (root + "/" + path in args.excludes)


regexp = re.compile("\\s*(Q_OBJECT|Q_GADGET|Q_NAMESPACE)\\s*")
# Returns true if the header file provides contains a Q_OBJECT, Q_GADGET or Q_NAMESPACE macro


def hasMacro(fileName):
    with open(fileName, encoding="ISO-8859-1") as fileHandle:
        for line in fileHandle:
            if regexp.match(line):
                return True
        return False


# returns the matching .cpp file for the given .h file


def matchingCPPFile(root, fileName):
    assert root.startswith(args.root)
    root = stripInitialSlash(root[len(args.root) :])

    if args.headerPrefix:
        assert root.startswith(args.headerPrefix)
        root = stripInitialSlash(root[len(args.headerPrefix) :])

    if args.sourcePrefix:
        root = args.sourcePrefix + "/" + root

    return (
        args.root
        + "/"
        + root
        + ("/" if root != "" else "")
        + fileNameWithoutExtension(fileName)
        + ".cpp"
    )


def fileNameWithoutExtension(fileName):
    return os.path.splitext(os.path.basename(fileName))[0]


# returns true if the specifies .cpp file already has the proper include


def cppHasMOCInclude(fileName):
    includeStatement = '#include "moc_%s.cpp"' % fileNameWithoutExtension(fileName)
    with open(fileName, encoding="utf8") as fileHandle:
        return includeStatement in fileHandle.read()


def getMocInsertionLocation(filename, content):
    headerIncludeRegex = re.compile(
        r'#include "%s\.h".*\n' % fileNameWithoutExtension(filename), re.M
    )
    match = headerIncludeRegex.search(content)
    if match:
        return match.end()
    return 0


def trimExistingMocInclude(content, cppFileName):
    mocStrRegex = re.compile(
        r'#include "moc_%s\.cpp"\n' % fileNameWithoutExtension(cppFileName)
    )
    match = mocStrRegex.search(content)
    if match:
        return content[: match.start()] + content[match.end() :]
    return content


def processFile(root, fileName):
    # pylint: disable=global-statement
    global dirty
    macroFound = hasMacro(root + "/" + fileName)
    logVerbose(
        "Inspecting %s %s"
        % (
            root + "/" + fileName,
            "[Has Q_OBJECT / Q_GADGET / Q_NAMESPACE]" if macroFound else "",
        )
    )

    if macroFound:
        cppFileName = matchingCPPFile(root, fileName)
        logVerbose("  -> %s" % cppFileName)

        if not os.path.exists(cppFileName):
            log("file %s didn't exist (which might not be an error)" % cppFileName)
            return

        if args.replaceExisting or not cppHasMOCInclude(cppFileName):
            dirty = True
            if args.dryRun:
                log("Missing moc include file: %s" % cppFileName)
            else:
                log("Updating %s" % cppFileName)

                with open(cppFileName, encoding="utf8") as f:
                    content = f.read()

                if args.replaceExisting:
                    content = trimExistingMocInclude(content, cppFileName)

                loc = getMocInsertionLocation(cppFileName, content)
                if args.insertAtEnd:
                    with open(cppFileName, "a", encoding="utf8") as f:
                        f.write(
                            '\n#include "moc_%s.cpp"\n'
                            % fileNameWithoutExtension(cppFileName)
                        )
                else:
                    with open(cppFileName, "w", encoding="utf8") as f:
                        f.write(
                            content[:loc]
                            + (
                                '#include "moc_%s.cpp"\n'
                                % fileNameWithoutExtension(cppFileName)
                            )
                            + content[loc:]
                        )


def log(content):
    if not args.quiet:
        print(content)


def logVerbose(content):
    if args.verbose:
        print(content)


# MAIN
if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="""Script to add inclusion of mocs to files recursively.
        The source files either need to be in the same directories as the header files or in parallel directories,
        where the root of the headers are specified using --header-prefix and the root of the sources are specified using --source-prefix.
        If either header-prefix or source-prefix is the current directory, then they may be omitted."""
    )
    parser.add_argument(
        "--dry-run",
        "-n",
        dest="dryRun",
        action="store_true",
        help="only report files to be updated",
    )
    parser.add_argument(
        "--quiet", "-q", dest="quiet", action="store_true", help="suppress output"
    )
    parser.add_argument("--verbose", "-v", dest="verbose", action="store_true")
    parser.add_argument(
        "--header-prefix",
        metavar="directory",
        dest="headerPrefix",
        help="This directory will be replaced with source-prefix when "
        "searching for matching source files",
    )
    parser.add_argument(
        "--source-prefix",
        metavar="directory",
        dest="sourcePrefix",
        help="see --header-prefix",
    )
    parser.add_argument(
        "--excludes",
        metavar="directory",
        dest="excludes",
        nargs="*",
        help="directories to be excluded, might either be in the form of a directory name, "
        "e.g. 3rdparty or a partial directory prefix from the root, e.g 3rdparty/parser",
    )
    parser.add_argument(
        "--insert-at-end",
        dest="insertAtEnd",
        action="store_true",
        help="insert the moc include at the end of the file instead of the beginning",
    )
    parser.add_argument(
        "--replace-existing",
        dest="replaceExisting",
        action="store_true",
        help="delete and readd existing MOC include statements",
    )
    parser.add_argument(
        dest="root",
        default=".",
        metavar="directory",
        nargs="?",
        help="root directory for the operation",
    )

    args = parser.parse_args()

    root = args.root
    if args.headerPrefix:
        root += "/" + args.headerPrefix

    path = os.walk(root)
    for root, directories, files in path:
        # Filter out directories specified in --exclude
        directories[:] = [d for d in directories if not shouldExclude(root, d)]

        for file in files:
            if file.endswith(".h") or file.endswith(".hpp"):
                processFile(root, file)

    if not dirty:
        log("No changes needed")

    sys.exit(-1 if dirty else 0)
