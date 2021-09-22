#!/usr/bin/env bash

# This test checks for use of deprecated/outdated methods and suggests their replacement

declare -a KEYWORDS=()
declare -a HINTS=()

KEYWORDS[0]="\-DBL_MAX"
HINTS[0]="Use the type-safe method std::numeric_limits<double>::lowest() instead"

KEYWORDS[1]="DBL_MAX"
HINTS[1]="Use the type-safe method std::numeric_limits<double>::max() instead"

KEYWORDS[2]="DBL_MIN"
HINTS[2]="Use the type-safe method std::numeric_limits<double>::min() instead (but be careful - maybe you actually want lowest!!)"

KEYWORDS[3]="DBL_EPSILON"
HINTS[3]="Use the type-safe method std::numeric_limits<double>::epsilon() instead"

KEYWORDS[4]="INT_MIN"
HINTS[4]="Use the type-safe method std::numeric_limits<int>::min() instead"

KEYWORDS[5]="INT_MAX"
HINTS[5]="Use the type-safe method std::numeric_limits<int>::max() instead"

KEYWORDS[6]="\bqMin("
HINTS[6]="Use std::min instead"

KEYWORDS[7]="\bqMax("
HINTS[7]="Use std::max instead"

KEYWORDS[8]="\bqAbs("
HINTS[8]="Use std::fabs instead"

KEYWORDS[9]="\bqRound("
HINTS[9]="Use std::round instead"

KEYWORDS[10]="\bqSort("
HINTS[10]="Use std::sort instead"

KEYWORDS[11]="@param"
HINTS[11]="Use \param instead (works correct with Python docstrings)"

KEYWORDS[12]="@return"
HINTS[12]="Use \returns instead (works correct with Python docstrings)"

KEYWORDS[13]="@note"
HINTS[13]="Use \note instead (works correct with Python docstrings)"

KEYWORDS[14]="@since"
HINTS[14]="Use \since instead (works correct with Python docstrings)"

KEYWORDS[15]="@warning"
HINTS[15]="Use \warning instead (works correct with Python docstrings)"

KEYWORDS[11]="@deprecated"
HINTS[11]="Use \deprecated instead (works correct with Python docstrings)"

KEYWORDS[12]="\bqIsFinite("
HINTS[12]="Use std::isfinite instead"

KEYWORDS[13]="\bqIsInf("
HINTS[13]="Use std::isinf instead"

KEYWORDS[14]="\bqIsNaN("
HINTS[14]="Use std::isnan instead"

KEYWORDS[15]="\bqCopy("
HINTS[15]="Use std::copy instead"

KEYWORDS[16]="\bqCount("
HINTS[16]="Use std::count instead"

KEYWORDS[17]="\bqEqual("
HINTS[17]="Use std::equal instead"

KEYWORDS[18]="\bqFill("
HINTS[18]="Use std::fill instead"

KEYWORDS[19]="\bqFind("
HINTS[19]="Use std::find instead"

KEYWORDS[20]="\bqGreater("
HINTS[20]="Use std::greater instead"

KEYWORDS[21]="\bqLess("
HINTS[21]="Use std::less instead"

KEYWORDS[22]="\bqLowerBound("
HINTS[22]="Use std::lower_bound instead"

KEYWORDS[23]="\bqStableSort("
HINTS[23]="Use std::stable_sort instead"

KEYWORDS[24]="\bqSwap("
HINTS[24]="Use std::swap instead"

KEYWORDS[25]="\bqUpperBound("
HINTS[25]="Use std::upper_bound instead"

KEYWORDS[26]="QScopedPointer"
HINTS[26]="Use std::unique_ptr instead"

KEYWORDS[27]="QSharedPointer"
HINTS[27]="Use std::shared_ptr instead"

KEYWORDS[28]="QOverload"
HINTS[28]="Use qOverload instead"

KEYWORDS[29]="qFloor"
HINTS[29]="Use std::floor instead"

KEYWORDS[30]="qCeil"
HINTS[30]="Use std::ceil instead"

KEYWORDS[31]="qSqrt"
HINTS[31]="Use std::sqrt instead"

KEYWORDS[32]="QStringLiteral()"
HINTS[32]="Use QString() instead"

KEYWORDS[33]="QStringLiteral( \"\" )"
HINTS[33]="Use QString() instead"

KEYWORDS[34]="QLatin1String( \"\" )"
HINTS[34]="Use QString() instead"

KEYWORDS[35]="@see"
HINTS[35]="Use \see instead (works correct with Python docstrings)"

KEYWORDS[36]="@brief"
HINTS[36]="Use \brief instead (works correct with Python docstrings)"

KEYWORDS[37]="Q_FOREACH"
HINTS[37]="Use range based for loops instead"

KEYWORDS[38]="foreach"
HINTS[38]="Use range based for loops instead"

KEYWORDS[39]="\bqBound("
HINTS[39]="Use std::clamp instead (but be careful of the different argument order!!)"

RES=
DIR=$(git rev-parse --show-toplevel)

pushd "${DIR}" > /dev/null || exit

for i in "${!KEYWORDS[@]}"
do
  FOUND=$(git grep "${KEYWORDS[$i]}" -- 'src/*.h' 'src/*.cpp' -- ':!*qtermwidget*' | grep --invert-match skip-keyword-check)

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

