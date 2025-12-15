#!/usr/bin/env bash
###########################################################################
#    prepare_commit_doxygen_test.sh
#    ---------------------
#    Date                 : August 2008
#    Copyright            : (C) 2008 by Juergen E. Fischer
#    Email                : jef at norbit dot de
###########################################################################
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
###########################################################################

set -e

# capture files passed by pre-commit
MODIFIED="$@"

if [ -z "$MODIFIED" ]; then
  exit 0
fi

HAS_AG=false
if command -v ag > /dev/null; then
  HAS_AG=true
fi

HAS_UNBUFFER=false
if command -v unbuffer > /dev/null; then
  HAS_UNBUFFER=true
fi

# Run doxygen layout test if requirements are met

MODIFIED_DOXYGEN_FILES=""
for f in $MODIFIED; do
  case "$f" in
    *.h|*.cpp)
      MODIFIED_DOXYGEN_FILES="$MODIFIED_DOXYGEN_FILES $f"
      ;;
  esac
done

if [ -n "$MODIFIED_DOXYGEN_FILES" ]; then
  echo "Checking doxygen layout in ${MODIFIED_DOXYGEN_FILES}"

  if test "$HAS_AG" != "true"; then
    echo "WARNING: the ag(1) executable was not found, doxygen layout checker could not run" >&2
  elif test "$HAS_UNBUFFER" != "true"; then
    echo "WARNING: the unbuffer(1) executable was not found, doxygen layout checker could not run" >&2
  else
    # check that \return(s) is placed before \note and \since
    output=$(unbuffer ag --noaffinity --file-search-regex '\.h$' --multiline '\\(note|since)[^\n]+(\n\s*\* [^\n]+)*\n\s*\* \\return' ${MODIFIED_DOXYGEN_FILES} | tee /dev/stderr)
    if [[ -n $output ]]; then
      echo -e "\n\x1B[31m*** Docstring computation: \\\return(s) should be placed before \\\note and \\since\x1B[0m"
      exit 1
    fi

    # check that \since and \deprecated are placed at the end of the command block
    output=$(unbuffer  ag --noaffinity --file-search-regex '\.h$' --multiline '(\\(deprecated|since)[^\n]+\n)+\s*\*[^\/](?!\s*\\(deprecated|since))' ${MODIFIED_DOXYGEN_FILES} | tee /dev/stderr)
    if [[ -n $output ]]; then
      echo -e "\n\x1B[31m*** Docstring computation: \\\deprecated and \\\since should be placed at the end of command blocks\x1B[0m"
      echo -e "To fix it, you may want to run (multiple times) at the top level directory:"
      echo 'sed -i -r '"'"'$!N;s/^(\s*\*\s+\\(deprecated|since)[^\n]+)\n(\s*\*([^\/].*|$))/\3\n\1/;P;D'"'"' $(ag -c --noaffinity --file-search-regex '"'"'\.h$'"'"' --multiline '"'"'(\\(deprecated|since)[^\n]+\n)+\s*\*[^\/]'"'"' . | cut -d: -f1)'
      exit 1
    fi

    # check that \ingroup qgis_3d is used, not \ingroup 3d
    output=$(unbuffer ag --noaffinity --file-search-regex '\.h$'  '\\ingroup 3d' ${MODIFIED_DOXYGEN_FILES} | tee /dev/stderr)
    if [[ -n $output ]]; then
      echo -e "\n\x1B[31m*** Docstring computation: Use \\\ingroup qgis_3d not \\\ingroup 3d\x1B[0m"
      exit 1
    fi

    # code snippets command
    output=$(unbuffer ag --noaffinity --file-search-regex '\.h$' --multiline '~~~\{\.\w+\}' ${MODIFIED_DOXYGEN_FILES} | tee /dev/stderr)
    if [[ -n $output ]]; then
      echo -e "\n\x1B[31m*** Docstring computation: code snippets should use \\\code{.xx} rather than ~~~{.xx} \x1B[0m"
      exit 1
    fi
  fi
fi

exit 0
