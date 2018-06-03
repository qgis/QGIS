#!/usr/bin/env bash

# This test checks that all source files correctly have license headers

INCLUDE_EXTENSIONS="h|cpp|hpp|py|c"
EXCLUDE_LIST="(.*\\/(qtermwidget)\\/|ui_defaults\\.h|CREDITS|TODO|README|URI|^[^.]*$|.*\\.(?!($INCLUDE_EXTENSIONS)$))"
LICENSE_CHECK="external/licensecheck/licensecheck.pl"

DIR=$(git rev-parse --show-toplevel)

pushd "${DIR}" || exit > /dev/null
missing=$(! { "$LICENSE_CHECK" -r -i "$EXCLUDE_LIST" src & "$LICENSE_CHECK" -r -i "$EXCLUDE_LIST" python; } | grep UNKNOWN)

popd || exit > /dev/null

if [[ $missing ]]; then
  echo " *** Found source files without valid license headers"
  echo "$missing"
  exit 1
fi

