#!/usr/bin/env bash

# This test checks for use of Qt module-wide imports such as #include <QtCore>, 
# which slow compilation dramatically
# See https://www.kdab.com/beware-of-qt-module-wide-includes/

declare -a IMPORTS=()
declare -a HINTS=()

IMPORTS[0]="#include <QtCore>"
HINTS[0]="Avoid module-wide import of QtCore, which results in all Qt core headers being imported"

IMPORTS[1]="#include <QtCore/QtCore>"
HINTS[1]="Avoid module-wide import of QtCore, which results in all Qt core headers being imported"

IMPORTS[2]="#include <QtGui>"
HINTS[2]="Avoid module-wide import of QtGui, which results in all Qt GUI headers being imported"

IMPORTS[3]="#include <QtGui/QtGui>"
HINTS[3]="Avoid module-wide import of QtGui, which results in all Qt GUI headers being imported"

IMPORTS[4]="#include <QtMultimedia>"
HINTS[4]="Avoid module-wide import of QtMultimedia, which results in all Qt Multimedia headers being imported"

IMPORTS[4]="#include <QtMultimedia/QtMultimedia>"
HINTS[4]="Avoid module-wide import of QtMultimedia, which results in all Qt Multimedia headers being imported"

IMPORTS[5]="#include <QtMultimediaWidgets>"
HINTS[5]="Avoid module-wide import of QtMultimediaWidgets, which results in all Qt Multimedia widget headers being imported"

IMPORTS[6]="#include <QtMultimediaWidgets/QtMultimediaWidgets>"
HINTS[6]="Avoid module-wide import of QtMultimediaWidgets, which results in all Qt Multimedia widget headers being imported"

IMPORTS[5]="#include <QtMultimediaWidgets>"
HINTS[5]="Avoid module-wide import of QtMultimediaWidgets, which results in all Qt Multimedia widget headers being imported"

IMPORTS[6]="#include <QtMultimediaWidgets/QtMultimediaWidgets>"
HINTS[6]="Avoid module-wide import of QtMultimediaWidgets, which results in all Qt Multimedia widget headers being imported"

IMPORTS[7]="#include <QtNetwork>"
HINTS[7]="Avoid module-wide import of QtNetwork, which results in all Qt Network headers being imported"

IMPORTS[8]="#include <QtNetwork/QtNetwork>"
HINTS[8]="Avoid module-wide import of QtNetwork, which results in all Qt Network headers being imported"

IMPORTS[9]="#include <QtQml>"
HINTS[9]="Avoid module-wide import of QtQml, which results in all Qt QML headers being imported"

IMPORTS[10]="#include <QtQml/QtQml>"
HINTS[10]="Avoid module-wide import of QtQml, which results in all Qt QML headers being imported"

IMPORTS[11]="#include <QtSql>"
HINTS[11]="Avoid module-wide import of QtSql, which results in all Qt SQL headers being imported"

IMPORTS[12]="#include <QtSql/QtSql>"
HINTS[12]="Avoid module-wide import of QtSql, which results in all Qt SQL headers being imported"

IMPORTS[13]="#include <QtTest>"
HINTS[13]="Avoid module-wide import of QtTest, which results in all Qt Test headers being imported. Suggest using #include <QTest> instead."

IMPORTS[14]="#include <QtTest/QtTest>"
HINTS[14]="Avoid module-wide import of QtTest, which results in all Qt Test headers being imported. Suggest using #include <QTest> instead."

IMPORTS[15]="#include <QtWidgets>"
HINTS[15]="Avoid module-wide import of QtWidgets, which results in all Qt Widget headers being imported."

IMPORTS[16]="#include <QtWidgets/QtWidgets>"
HINTS[16]="Avoid module-wide import of QtWidgets, which results in all Qt Widget headers being imported."

RES=
DIR=$(git rev-parse --show-toplevel)

pushd "${DIR}" > /dev/null || exit

for i in "${!IMPORTS[@]}"
do
  FOUND=$(git grep -n "${IMPORTS[$i]}" -- ':!*external/qt3dextra-headers*' ':!external/o2/examples*' ':!ChangeLog' ':!tests/code_layout/test_qt_imports.sh')

  if [[  ${FOUND} ]]; then
    echo "Found source files with a module-wide Qt import: ${IMPORTS[$i]}!"
    echo " -> ${HINTS[$i]}"
    echo
    echo "${FOUND}"
    echo
    RES=1
  fi

done

popd > /dev/null || exit

if [ $RES ]; then
  echo " *** Found module-wide Qt imports"
  exit 1
fi

