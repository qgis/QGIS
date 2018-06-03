#!/usr/bin/env bash

# This test checks that all source files correctly have license headers

INCLUDE_EXTENSIONS="h|cpp|hpp|py|c"
EXCLUDE_LIST="(.*\/(qtermwidget)\/|ui_defaults\\.h|CREDITS|TODO|README|URI|^[^.]*$|.*\\.(?!($INCLUDE_EXTENSIONS)$))"

# check for existance of licensecheck first
has_licensecheck=$( licensecheck )
if licensecheck ; then
  echo "licensecheck installed!"
else
  echo "licensecheck not installed!"
  exit 1
fi

DIR=$(git rev-parse --show-toplevel)

pushd ${DIR} > /dev/null
missing=$(! { licensecheck -r -i "$EXCLUDE_LIST" src & licensecheck -r -i "$EXCLUDE_LIST" python; } | grep UNKNOWN)

popd > /dev/null

if [[ $missing ]]; then
  echo " *** Found source files without valid license headers"
  echo "$missing"
  exit 1
fi

