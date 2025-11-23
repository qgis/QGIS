#!/usr/bin/env python3
"""
***************************************************************************
    generate_unaccent_rules.py
    ---------------------
    Date                 : November 2025
    Copyright            : (C) 2025 by Tudor Bărăscu
    Email                : tudor dot barascu at qtibia dot ro
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Tudor Bărăscu"
__date__ = "November 2025"
__copyright__ = "(C) 2025, Tudor Bărăscu"

import sys
import os
import argparse
import re
from collections import defaultdict
from datetime import datetime
import unicodedata


def escape_cpp_string(s):
    """Escape string for C++ QStringLiteral."""
    s = s.replace('\\', '\\\\')
    s = s.replace('"', '\\"')
    s = s.replace('\n', '\\n')
    s = s.replace('\r', '\\r')
    s = s.replace('\t', '\\t')
    s = s.replace('\0', '\\0')
    return s


def is_combining(ch: str) -> bool:
    """Return True if character is a Unicode combining mark (Mn, Mc, Me)."""
    return unicodedata.category(ch) in ("Mn", "Mc", "Me")


def parse_unaccent_rules(filename):
    """
    Parse PostgreSQL-style unaccent.rules like in
    https://raw.githubusercontent.com/postgres/postgres/refs/heads/master/contrib/unaccent/generate_unaccent_rules.py:

    - Unicode whitespace separates the 2 fields (not only tabs)
    - Surrounding quotes are stripped
    """

    rules = defaultdict(list)
    line_count = 0
    rule_count = 0

    try:
        with open(filename, "r", encoding="utf-8") as f:
            for line_num, raw_line in enumerate(f, 1):
                line_count += 1
                line = raw_line

                if line.lstrip().startswith("#"):
                    continue

                # Remove newline characters
                line = line.rstrip("\n\r")

                # Skip if visually empty
                if not line.strip():
                    continue

                # Split on ANY unicode whitespace
                parts = re.split(r"\s+", line, maxsplit=1)

                if len(parts) == 2:
                    source, target = parts[0], parts[1]

                    # Strip surrounding quotes
                    if source.startswith('"') and source.endswith('"'):
                        source = source[1:-1]
                    if target.startswith('"') and target.endswith('"'):
                        target = target[1:-1]

                    if source == "":
                        print(
                            f"Warning: Line {line_num} has empty source, skipping",
                            file=sys.stderr,
                        )
                        continue

                    rules[len(source)].append((source, target))
                    rule_count += 1
                    continue

                if len(parts) == 1:
                    source = parts[0]

                    if len(source) == 1 and is_combining(source):
                        # Delete combining mark → target = ""
                        rules[1].append((source, ""))
                        rule_count += 1
                        continue

                    print(
                        f"Warning: Line {line_num} has fewer than 2 fields, skipping: {source}",
                        file=sys.stderr,
                    )
                    continue

    except FileNotFoundError:
        print(f"Error: File '{filename}' not found", file=sys.stderr)
        sys.exit(1)

    except Exception as e:
        print(f"Error reading file: {e}", file=sys.stderr)
        sys.exit(1)

    # Sort for nice output
    for length in rules:
        rules[length].sort(key=lambda x: x[0])

    print(
        f"Processed {line_count} lines, generated {rule_count} rules",
        file=sys.stderr
    )
    return rules


def generate_cpp_code(rules, input_filename):
    """Generate the static C++ file containing sUnaccentMap and QgsStringUtils::unaccent."""

    max_length = max(rules.keys()) if rules else 0
    total_rules = sum(len(v) for v in rules.values())

    print("/***************************************************************************")
    print("  qgsunaccent_generated_rules.cpp")
    print("  --------------------------------------")
    print(f"  Date                 : {datetime.now().strftime('%B %Y')}")
    print(" ***************************************************************************/\n")

    print("// AUTO-GENERATED FILE - DO NOT EDIT")
    print(f"// Generated from: {os.path.basename(input_filename)}")
    print(f"// Total rules: {total_rules}")
    print(f"// Max source length: {max_length}\n")

    print('#include "qgsstringutils.h"')
    print("#include <QMap>")
    print("#include <QString>\n")

    print("namespace")
    print("{\n")
    print("  static const QMap<QString, QString> sUnaccentMap = []() -> QMap<QString, QString>")
    print("  {")
    print("    QMap<QString, QString> map;\n")

    for length in sorted(rules.keys()):
        rule_list = rules[length]
        label = "Single" if length == 1 else f"{length}"
        print(f"    // {label} character mappings ({len(rule_list)} rules)")

        for source, target in rule_list:
            source_esc = escape_cpp_string(source)
            target_esc = escape_cpp_string(target)

            if target_esc == "":
                target_cpp = "QString()"  # use QString() to make CI happy
            else:
                target_cpp = f'QStringLiteral( "{target_esc}" )'

            print(
                f'    map.insert( QStringLiteral( "{source_esc}" ), {target_cpp} );'
            )

        print()


    print("    return map;")
    print("  }();\n")
    print("} // namespace\n")

    print("QString QgsStringUtils::unaccent( const QString &string )")
    print("{")
    print("  if ( string.isEmpty() )")
    print("    return string;\n")
    print("  // PostgreSQL-compatible Unicode normalization (NFKD)")
    print("  const QString normalized = string.normalized( QString::NormalizationForm_KD );\n")
    print("  QString result;")
    print("  result.reserve( normalized.length() );\n")
    print("  for ( const QChar &ch : normalized )")
    print("  {")
    print("    const QString key( ch );")
    print("    result.append( sUnaccentMap.value( key, key ) );")
    print("  }\n")
    print("  return result;")
    print("}\n")


def main():
    parser = argparse.ArgumentParser(
        description="Generate PostgreSQL-compatible unaccent C++ rules",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="Example:\n  %(prog)s unaccent.rules > qgsunaccent_generated_rules.cpp",
    )

    parser.add_argument("input_file", help="Path to unaccent.rules")
    parser.add_argument("--stats", "-s", action="store_true", help="Print rule statistics")
    args = parser.parse_args()

    rules = parse_unaccent_rules(args.input_file)

    if args.stats:
        total_rules = sum(len(v) for v in rules.values())
        max_length = max(rules.keys()) if rules else 0
        print(f"\nRules: {total_rules}")
        print(f"Max source length: {max_length}\n")

        for length in sorted(rules.keys()):
            print(f"Length {length}: {len(rules[length])} rules")
            for s, t in rules[length][:3]:
                print(f"  {s!r} → {t!r}")
            if len(rules[length]) > 3:
                print(f"  ... and {len(rules[length]) - 3} more\n")
    else:
        generate_cpp_code(rules, args.input_file)


if __name__ == "__main__":
    main()
