#!/usr/bin/env bash


# check that returns are placed before note and since
output=$(unbuffer ag --noaffinity --file-search-regex '\.h$' --multiline '\\(note|since)[^\n]+(\n\s*\* [^\n]+)*\n\s*\* \\return' ${TRAVIS_BUILD_DIR} | tee /dev/stderr)
if [[ ! -z $output ]]; then
  echo -e "\n\x1B[31m*** Docstring computation: \\\return(s) should be placed before \\\note and \\since\x1B[0m"
  exit 1
fi

# code snippets command
output=$(unbuffer ag --noaffinity --file-search-regex '\.h$' --multiline '~~~\{\.\w+\}' ${TRAVIS_BUILD_DIR} | tee /dev/stderr)
if [[ ! -z $output ]]; then
  echo -e "\n\x1B[31m*** Docstring computation: code snippets should use \\\code{.xx} rather than ~~~{.xx} \x1B[0m"
  exit 1
fi
