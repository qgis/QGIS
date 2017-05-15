/***************************************************************************
                               qgsexpressionutils.cpp
                             -------------------
    begin                : May 2017
    copyright            : (C) 2017 Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexpressionutils.h"
#include "qgsexpressionnode.h"

constexpr QgsExpressionUtils::TVL QgsExpressionUtils::AND[3][3];
constexpr QgsExpressionUtils::TVL QgsExpressionUtils::OR[3][3];
constexpr QgsExpressionUtils::TVL QgsExpressionUtils::NOT[3];
