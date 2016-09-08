/***************************************************************************
  qgsoptionalexpression - %{Cpp:License:ClassName}

 ---------------------
 begin                : 8.9.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSOPTIONALEXPRESSION_H
#define QGSOPTIONALEXPRESSION_H

#include "qgsoptional.h"
#include "qgsexpression.h"

/**
 * An expression with an additional enabled flag.
 *
 * This can be used for configuration options where an expression can be enabled
 * or diabled but when disabled it shouldn't lose it's information for the case
 * it gets re-enabled later on and a user shoulnd't be force to redo the configuration.
 *
 * Added in QGIS 3.0
 */
typedef QgsOptional<QgsExpression> QgsOptionalExpression;

#endif // QGSOPTIONALEXPRESSION_H
