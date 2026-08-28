#!/usr/bin/env python3
###########################################################################
#    check_spelling.py
#    ---------------------
#    Date                 : December 2016
#    Copyright            : (C) 2016 by Denis Rouzaud
#    Email                : denis.rouzaud@gmail.com
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################
"""Spell check source files against spelling.dat. Requires nothing but Python 3.

The dictionary holds one `error:correction[:flags]` entry per line, where flags
may contain:

  *  never look for the error inside a longer word
  %  ignore the error in the scripts listed in EXCLUDE_SCRIPT_LIST

How an entry is looked for depends on its shape:

  australia:Australia   a fix of the case only, so searched case sensitively  #spellok
  are'nt:aren't         holds a space or an apostrophe, so searched as a  #spellok
                        whole word, case insensitively
  extint:extinct        no correctly spelled word can contain the error,  #spellok
                        so searched anywhere, even inside a longer word
  symbo:symbol          anything else, searched as a whole word case  #spellok
                        insensitively, and in camel or snake case sensitively

Append #spellok (or <!-- #spellok -->) to a line to skip it.
"""

import argparse
import os
import re
import subprocess
import sys
from bisect import bisect_right
from pathlib import Path

DIR = Path(__file__).resolve().parent

# extensions and files exempted from the entries flagged with ':%' in spelling.dat
EXCLUDE_SCRIPT_LIST = re.compile(
    r"(\.(xml|sip|pl|sh|badquote|cmake(\.in)?)"
    r"|^(debian/copyright|cmake_templates/.*|tests/testdata/labeling/README\.rst"
    r"|tests/testdata/font/QGIS-Vera/COPYRIGHT\.TXT|doc/debian/build/))$"
)

# files that are never checked
EXCLUDE_EXTERNAL_LIST = re.compile(
    r"((\.(svg|qgs|laz|las|png|lock|sip\.in))"
    r"|resources/data/.*|resources/cpt-city-qgis-min/.*|resources/server/src/.*"
    r"|src/server/services/landingpage/webapp/.*|tests/testdata/.*"
    r"|scripts/pre_commit/spell_check/spelling\.dat|doc/api_break\.dox|NEWS\.md"
    r"|python/PyQt6/.*/class_map\.yaml|python/.*/auto_(additions|generated)/.*)$"
)

# marker silencing a line, '//#spellok' or '<!-- #spellok -->'
SPELLOK = re.compile(r"#\s*spellok")

# a dictionary entry whose error part is a regular expression, not a plain word
IS_REGEX = re.compile(r"[\\()\[\]|+?{}*^$]")

# error and correction are the same word up to the case: 'australia:Australia'  #spellok
FIX_CASE = re.compile(r"(\w+):\1(:\*)?\Z", re.IGNORECASE)

# error or correction holds a space, an apostrophe or a dot: "are'nt:aren't"  #spellok
FIX_SPECIAL_CHAR = re.compile(r"(\w*[ '.…])*\w*:\w*(?(1)|[ '.…])")

# The error is safe to look for inside longer words: neither end of the correction
# is a shifted copy of the matching end of the error, so a correctly spelled word
# cannot contain the error. Try it out on https://regex101.com/r/7kznVA/14
IN_WORD = re.compile(
    r"(\w)(\w)(\w)\w*(\w)(\w)(\w):(?:(?!\2\3\w|\w\1\2).)\w*?(?:(?!\5\6\w|\w\4\5)\w\w\w)\Z"
)


class Entry:
    """One 'error:correction[:flags]' line of spelling.dat."""

    __slots__ = ("line", "error", "correction", "flags", "regex")

    def __init__(self, line):
        fields = line.split(":")
        self.line = line
        self.error = fields[0]
        self.correction = fields[1]
        self.flags = fields[2] if len(fields) > 2 else ""
        self.regex = None
        if IS_REGEX.search(self.error):
            self.regex = re.compile(self.error, re.IGNORECASE)

    def correct(self, error, match=None):
        """The correction for `error`, with the capitalization of `error` carried over."""
        correction = self.correction
        if match is not None and match.groups():
            # a regular expression entry, the correction holds %s placeholders
            correction = correction % match.groups()
        if re.search(r"[A-Z]+\Z", error):
            correction = correction.upper()
        elif re.search(r"[A-Z][a-z]+\Z", error) and correction[:1].islower():
            correction = correction[0].upper() + correction[1:]
        return correction


def trie_pattern(words):
    """A regular expression matching any of `words`, with the prefixes shared.

    Sharing the prefixes is what makes searching for the ~7800 dictionary
    entries at once affordable, a flat alternation would be orders of
    magnitude slower.
    """
    trie = {}
    for word in words:
        node = trie
        for char in word:
            node = node.setdefault(char, {})
        node[""] = {}

    def build(node):
        if len(node) == 1 and "" in node:
            return ""
        chars, alternatives = [], []
        for char in sorted(k for k in node if k != ""):
            sub = build(node[char])
            if sub:
                alternatives.append(re.escape(char) + sub)
            else:
                chars.append(char)
        if len(chars) == 1:
            alternatives.append(re.escape(chars[0]))
        elif chars:
            alternatives.append("[" + "".join(re.escape(c) for c in chars) + "]")
        if len(alternatives) == 1:
            pattern = alternatives[0]
        else:
            pattern = "(?:" + "|".join(alternatives) + ")"
        if "" in node:
            # the node also ends a word, so what follows it is optional
            pattern = pattern + "?" if len(pattern) == 1 else f"(?:{pattern})?"
        return pattern

    return build(trie) if trie else ""


def any_of(*patterns):
    patterns = [p for p in patterns if p]
    return "(?:" + "|".join(patterns) + ")" if patterns else ""


def whole_word(pattern):
    return rf"(?:\b|_){pattern}(?:\b|_)" if pattern else ""


class Dictionary:
    """The spelling.dat entries, compiled into two searchable patterns."""

    def __init__(self, path):
        entries = [
            Entry(line)
            for line in path.read_text(encoding="utf-8").splitlines()
            if line and not line.startswith("#")
        ]
        self.by_error = {e.error.lower(): e for e in entries if e.regex is None}
        self.regexes = [e for e in entries if e.regex is not None]

        fix_case, special, in_word, remains = [], [], [], []
        for entry in entries:
            if FIX_CASE.match(entry.line):
                fix_case.append(entry)
            elif FIX_SPECIAL_CHAR.match(entry.line):
                special.append(entry)
            elif IN_WORD.match(entry.line):
                in_word.append(entry)
            else:
                remains.append(entry)

        words = [e.error.lower() for e in special + remains if e.regex is None]
        bounded = any_of(trie_pattern(sorted(words)), *(e.error for e in self.regexes))
        self.ignore_case = self._compile(
            any_of(
                whole_word(bounded),
                trie_pattern(sorted(e.error.lower() for e in in_word)),
            ),
            re.IGNORECASE,
        )

        # words of at least 4 letters are also looked for in camel and snake case
        words = [e.error for e in remains if len(e.error) > 3]
        lower = trie_pattern(sorted(w.lower() for w in words))
        upper = trie_pattern(sorted(w.upper() for w in words))
        title = trie_pattern(sorted(w[:1].upper() + w[1:].lower() for w in words))
        self.case_sensitive = self._compile(
            any_of(
                whole_word(trie_pattern(sorted(e.error for e in fix_case))),
                rf"(?:\b|_)(?:{lower}_|{upper}_|{title}[_A-Z0-9])",
                rf"[_a-z0-9]{title}(?:\b|[_A-Z0-9])",
                rf"[_a-z0-9]{upper}(?:\b|[_a-z0-9])",
            ),
            0,
        )

    @staticmethod
    def _compile(pattern, flags):
        return re.compile(pattern, flags) if pattern else None

    @property
    def patterns(self):
        return [p for p in (self.ignore_case, self.case_sensitive) if p]

    def lookup(self, text):
        """Resolve matched text to (offset, error, entry, match).

        A match swallows the underscore or the neighbouring letter the pattern
        used as a word boundary, strip those to find the entry back.
        """
        lower = text.lower()
        for start, end in ((0, len(text)), (1, len(text)), (0, -1), (1, -1)):
            candidate = lower[start:end]
            if not candidate:
                continue
            entry = self.by_error.get(candidate)
            if entry is not None:
                return start, text[start:end], entry, None
            for entry in self.regexes:
                match = entry.regex.fullmatch(candidate)
                if match:
                    return start, text[start:end], entry, match
        return -1, "", None, None


class Misspelling:
    __slots__ = ("path", "line", "column", "text", "error", "correction")

    def __init__(self, path, line, column, text, error, correction):
        self.path = path
        self.line = line
        self.column = column
        self.text = text
        self.error = error
        self.correction = correction


def check(path, lines, dictionary):
    """The misspellings of `lines`, in order, without overlaps."""
    content = "".join(lines)
    starts, offset = [], 0
    for line in lines:
        starts.append(offset)
        offset += len(line)

    is_script = bool(EXCLUDE_SCRIPT_LIST.search(path))
    found = {}
    for pattern in dictionary.patterns:
        for match in pattern.finditer(content):
            shift, error, entry, groups = dictionary.lookup(match.group())
            if entry is None:
                print(
                    f"{path}: cannot map {match.group()!r} to a spelling.dat entry",
                    file=sys.stderr,
                )
                continue
            if is_script and "%" in entry.flags:
                continue
            start = match.start() + shift
            index = bisect_right(starts, start) - 1
            if SPELLOK.search(lines[index]):
                continue
            found.setdefault(
                start,
                Misspelling(
                    path,
                    index + 1,
                    start - starts[index],
                    lines[index],
                    error,
                    entry.correct(error, groups),
                ),
            )

    misspellings, end = [], -1
    for start in sorted(found):
        if start >= end:
            misspellings.append(found[start])
            end = start + len(found[start].error)
    return misspellings


def read_lines(path):
    """The lines of `path`, or None if it is binary or unreadable."""
    try:
        data = Path(path).read_bytes()
    except OSError as error:
        print(f"{path}: {error}", file=sys.stderr)
        return None
    if b"\0" in data:
        return None
    return data.decode("utf-8", errors="surrogateescape").splitlines(keepends=True)


def fix(path, lines, misspellings):
    """Apply the corrections in place, right to left so the offsets stay valid."""
    for misspelling in reversed(misspellings):
        line = lines[misspelling.line - 1]
        start = misspelling.column
        end = start + len(misspelling.error)
        lines[misspelling.line - 1] = line[:start] + misspelling.correction + line[end:]
    Path(path).write_bytes("".join(lines).encode("utf-8", errors="surrogateescape"))


def ignore_pattern():
    """The .agignore entries, as one case insensitive regular expression."""
    lines = []
    for line in (DIR / ".agignore").read_text(encoding="utf-8").splitlines():
        line = re.sub(r"\s*#.*$", "", line).strip()
        if line:
            lines.append(line)
    return re.compile("|".join(lines), re.IGNORECASE)


def files_to_check(paths, everything):
    if not paths:
        paths = os.environ.get("ALL_CHANGED_FILES", "").split()
    if not paths:
        if not everything:
            print("no file given, pass --all to check the whole tree", file=sys.stderr)
            return []
        paths = subprocess.run(
            ["git", "ls-files", "-z"], capture_output=True, text=True, check=True
        ).stdout.split("\0")
    ignored = ignore_pattern()
    return [
        path
        for path in paths
        if path
        and not ignored.search(path)
        and not EXCLUDE_EXTERNAL_LIST.search(path)
        and os.path.isfile(path)
    ]


def report(misspelling, color):
    bold, red, green, reset = color
    text = misspelling.text.rstrip("\r\n")
    # keep the tabs so that the carets line up with the error
    indent = "".join("\t" if c == "\t" else " " for c in text[: misspelling.column])
    where = f"{misspelling.path}:{misspelling.line}:{misspelling.column + 1}"
    print(
        f"{bold}{where}{reset} {red}{misspelling.error}{reset}"
        f" should be {green}{misspelling.correction}{reset}\n"
        f"  {text}\n"
        f"  {indent}{'^' * len(misspelling.error)}"
    )


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("files", nargs="*", help="the files to check")
    parser.add_argument("--all", action="store_true", help="check all tracked files")
    parser.add_argument("--fix", action="store_true", help="correct the files in place")
    parser.add_argument("-l", "--log", help="log 'file line error correction'")
    args = parser.parse_args()

    paths = files_to_check(args.files, args.all)
    if not paths:
        return 0

    dictionary = Dictionary(DIR / "spelling.dat")
    color = (
        ("\033[1m", "\033[31m", "\033[32m", "\033[0m")
        if sys.stdout.isatty() and not os.environ.get("NO_COLOR")
        else ("", "", "", "")
    )
    log = open(args.log, "a", encoding="utf-8") if args.log else None

    count = 0
    for path in paths:
        lines = read_lines(path)
        if lines is None:
            continue
        misspellings = check(path, lines, dictionary)
        if not misspellings:
            continue
        count += len(misspellings)
        for m in misspellings:
            report(m, color)
            if log:
                print(f"{m.path} {m.line} {m.error} {m.correction}", file=log)
        if args.fix:
            fix(path, lines, misspellings)

    if log:
        log.close()
    if not count:
        return 0
    plural = "" if count == 1 else "s"
    outcome = "fixed" if args.fix else "found, fix them or append #spellok to the line"
    print(f"\n{color[0]}{count} spelling error{plural} {outcome}{color[3]}")
    return 1


if __name__ == "__main__":
    sys.exit(main())
