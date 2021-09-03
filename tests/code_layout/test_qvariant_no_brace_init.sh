#!/usr/bin/env bash

# This test checks that brace initializers are never used for QVariant variables. On some compilers the value will be
# converted to a list, on others a list of lists.
# Always use = initialization to avoid this ambiguity!

if git grep -P 'QVariant(?!Map|List)[^\(\)&>:]+ {' &> /dev/null; then
  echo ' *** Brace initializers should never be used for QVariant variables -- the value may become a list of lists on some compilers'
  git grep -n -P 'QVariant(?!Map|List)[^\(\)&>:]+ {'
  exit 1
fi

