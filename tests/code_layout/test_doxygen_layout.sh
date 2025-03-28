#!/usr/bin/env bash

PATHS=${@-"$(cd $(dirname "$0")/../../ && pwd)"}

echo "Checking doxygen layout in ${PATHS}"


# check that \return(s) is placed before \note and \since
output=$(unbuffer ag --noaffinity --file-search-regex '\.h$' --multiline '\\(note|since)[^\n]+(\n\s*\* [^\n]+)*\n\s*\* \\return' ${PATHS} | tee /dev/stderr)
if [[ -n $output ]]; then
  echo -e "\n\x1B[31m*** Docstring computation: \\\return(s) should be placed before \\\note and \\since\x1B[0m"
  exit 1
fi

# check that \since and \deprecated are placed at the end of the command block
output=$(unbuffer  ag --noaffinity --file-search-regex '\.h$' --multiline '(\\(deprecated|since)[^\n]+\n)+\s*\*[^\/](?!\s*\\(deprecated|since))' ${PATHS} | tee /dev/stderr)
if [[ -n $output ]]; then
  echo -e "\n\x1B[31m*** Docstring computation: \\\deprecated and \\\since should be placed at the end of command blocks\x1B[0m"
  echo -e "To fix it, you may want to run (multiple times) at the top level directory:"
  echo 'sed -i -r '"'"'$!N;s/^(\s*\*\s+\\(deprecated|since)[^\n]+)\n(\s*\*([^\/].*|$))/\3\n\1/;P;D'"'"' $(ag -c --noaffinity --file-search-regex '"'"'\.h$'"'"' --multiline '"'"'(\\(deprecated|since)[^\n]+\n)+\s*\*[^\/]'"'"' . | cut -d: -f1)'
  exit 1
fi

# check that \return(s) is placed before \note and \since
output=$(unbuffer ag --noaffinity --file-search-regex '\.h$'  '\\ingroup 3d' ${PATHS} | tee /dev/stderr)
if [[ -n $output ]]; then
  echo -e "\n\x1B[31m*** Docstring computation: Use \\\ingroup qgis_3d not \\\ingroup 3d\x1B[0m"
  exit 1
fi

# code snippets command
output=$(unbuffer ag --noaffinity --file-search-regex '\.h$' --multiline '~~~\{\.\w+\}' ${PATHS} | tee /dev/stderr)
if [[ -n $output ]]; then
  echo -e "\n\x1B[31m*** Docstring computation: code snippets should use \\\code{.xx} rather than ~~~{.xx} \x1B[0m"
  exit 1
fi
