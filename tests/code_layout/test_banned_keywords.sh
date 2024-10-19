#!/usr/bin/env bash

# This test checks for use of deprecated/outdated methods and suggests their replacement

declare -a KEYWORDS=()
declare -a HINTS=()
declare -a PATHS=()

KEYWORDS[0]="\-DBL_MAX"
HINTS[0]="Use the type-safe method std::numeric_limits<double>::lowest() instead"
PATHS[0]="."

KEYWORDS[1]="DBL_MAX"
HINTS[1]="Use the type-safe method std::numeric_limits<double>::max() instead"
PATHS[1]="."

KEYWORDS[2]="DBL_MIN"
HINTS[2]="Use the type-safe method std::numeric_limits<double>::min() instead (but be careful - maybe you actually want lowest!!)"
PATHS[2]="."

KEYWORDS[3]="DBL_EPSILON"
HINTS[3]="Use the type-safe method std::numeric_limits<double>::epsilon() instead"
PATHS[3]="."

KEYWORDS[4]="INT_MIN"
HINTS[4]="Use the type-safe method std::numeric_limits<int>::min() instead"
PATHS[4]="."

KEYWORDS[5]="INT_MAX"
HINTS[5]="Use the type-safe method std::numeric_limits<int>::max() instead"
PATHS[5]="."

KEYWORDS[6]="\bqMin("
HINTS[6]="Use std::min instead"
PATHS[6]="."

KEYWORDS[7]="\bqMax("
HINTS[7]="Use std::max instead"
PATHS[7]="."

KEYWORDS[8]="\bqAbs("
HINTS[8]="Use std::fabs instead"
PATHS[8]="."

KEYWORDS[9]="\bqRound("
HINTS[9]="Use std::round instead"
PATHS[9]="."

KEYWORDS[10]="\bqSort("
HINTS[10]="Use std::sort instead"
PATHS[10]="."

KEYWORDS[11]="@param"
HINTS[11]="Use \param instead (works correct with Python docstrings)"
PATHS[11]="."

KEYWORDS[12]="@return"
HINTS[12]="Use \returns instead (works correct with Python docstrings)"
PATHS[12]="."

KEYWORDS[13]="@note"
HINTS[13]="Use \note instead (works correct with Python docstrings)"
PATHS[13]="."

KEYWORDS[14]="@since"
HINTS[14]="Use \since instead (works correct with Python docstrings)"
PATHS[14]="."

KEYWORDS[15]="@warning"
HINTS[15]="Use \warning instead (works correct with Python docstrings)"
PATHS[15]="."

KEYWORDS[16]="@deprecated"
HINTS[16]="Use \deprecated instead (works correct with Python docstrings)"
PATHS[16]="."

KEYWORDS[17]="\bqIsFinite("
HINTS[17]="Use std::isfinite instead"
PATHS[17]="."

KEYWORDS[18]="\bqIsInf("
HINTS[18]="Use std::isinf instead"
PATHS[18]="."

KEYWORDS[19]="\bqIsNaN("
HINTS[19]="Use std::isnan instead"
PATHS[19]="."

KEYWORDS[20]="\bqCopy("
HINTS[20]="Use std::copy instead"
PATHS[20]="."

KEYWORDS[21]="\bqCount("
HINTS[21]="Use std::count instead"
PATHS[21]="."

KEYWORDS[22]="\bqEqual("
HINTS[22]="Use std::equal instead"
PATHS[22]="."

KEYWORDS[23]="\bqFill("
HINTS[23]="Use std::fill instead"
PATHS[23]="."

KEYWORDS[24]="\bqFind("
HINTS[24]="Use std::find instead"
PATHS[24]="."

KEYWORDS[25]="\bqGreater("
HINTS[25]="Use std::greater instead"
PATHS[25]="."

KEYWORDS[26]="\bqLess("
HINTS[26]="Use std::less instead"
PATHS[26]="."

KEYWORDS[27]="\bqLowerBound("
HINTS[27]="Use std::lower_bound instead"
PATHS[27]="."

KEYWORDS[28]="\bqStableSort("
HINTS[28]="Use std::stable_sort instead"
PATHS[29]="."

KEYWORDS[29]="\bqSwap("
HINTS[29]="Use std::swap instead"
PATHS[29]="."

KEYWORDS[30]="\bqUpperBound("
HINTS[30]="Use std::upper_bound instead"
PATHS[30]="."

KEYWORDS[31]="QScopedPointer"
HINTS[31]="Use std::unique_ptr instead"
PATHS[31]="."

KEYWORDS[32]="QSharedPointer"
HINTS[32]="Use std::shared_ptr instead"
PATHS[32]="."

KEYWORDS[33]="QOverload"
HINTS[33]="Use qOverload instead"
PATHS[33]="."

KEYWORDS[34]="qFloor"
HINTS[34]="Use std::floor instead"
PATHS[34]="."

KEYWORDS[35]="qCeil"
HINTS[35]="Use std::ceil instead"
PATHS[35]="."

KEYWORDS[36]="qSqrt"
HINTS[36]="Use std::sqrt instead"
PATHS[36]="."

KEYWORDS[37]="QStringLiteral()"
HINTS[37]="Use QString() instead"
PATHS[37]="."

KEYWORDS[38]="QStringLiteral( \"\" )"
HINTS[38]="Use QString() instead"
PATHS[38]="."

KEYWORDS[39]="QLatin1String( \"\" )"
HINTS[39]="Use QString() instead"
PATHS[39]="."

KEYWORDS[40]="@see"
HINTS[40]="Use \see instead (works correct with Python docstrings)"
PATHS[40]="."

KEYWORDS[41]="@brief"
HINTS[41]="Use \brief instead (works correct with Python docstrings)"
PATHS[41]="."

KEYWORDS[42]="Q_FOREACH"
HINTS[42]="Use range based for loops instead"
PATHS[42]="."

KEYWORDS[43]="foreach"
HINTS[43]="Use range based for loops instead"
PATHS[43]="."

KEYWORDS[44]="\bqBound("
HINTS[44]="Use std::clamp instead (but be careful of the different argument order!!)"
PATHS[44]="."

KEYWORDS[45]="^\s*\* @"
HINTS[45]="Use '\param', '\returns' format for doxygen annotations, not '@param', '@returns'"
PATHS[45]="."

KEYWORDS[46]="QgsProject::instance()"
HINTS[46]="Do not introduce new use of QgsProject::instance() in core library! Find alternative ways to achieve what you are doing here."
PATHS[46]="core"

RES=
DIR=$(git rev-parse --show-toplevel)

pushd "${DIR}" > /dev/null || exit

for i in "${!KEYWORDS[@]}"
do
  FOUND=$(git grep "${KEYWORDS[$i]}" -- "src/${PATHS[$i]}/*.h" "src/${PATHS[$i]}/*.cpp" | grep --invert-match skip-keyword-check)

  if [[  ${FOUND} ]]; then
    echo "Found source files with banned keyword: ${KEYWORDS[$i]}!"
    echo " -> ${HINTS[$i]}"
    echo "    or mark with // skip-keyword-check"
    echo
    echo "${FOUND}"
    echo
    RES=1
  fi

done

popd > /dev/null || exit

if [ $RES ]; then
  echo " *** Found banned keywords"
  exit 1
fi

