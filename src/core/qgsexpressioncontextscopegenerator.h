/***************************************************************************
  qgsexpressioncontextscopegenerator.h - QgsExpressionContextScopeGenerator

 ---------------------
 begin                : 24.11.2017
 copyright            : (C) 2017 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSEXPRESSIONCONTEXTSCOPEGENERATOR_H
#define QGSEXPRESSIONCONTEXTSCOPEGENERATOR_H

#include "qgsexpressioncontext.h"

/**
 * \ingroup core
 * Abstract interface for generating an expression context scope.
 *
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsExpressionContextScopeGenerator
{
  public:

    /**
     * This method needs to be reimplemented in all classes which implement this interface
     * and return an expression context scope.
     *
     * \since QGIS 3.0
     */
    virtual QgsExpressionContextScope *createExpressionContextScope() const = 0 SIP_FACTORY;

    virtual ~QgsExpressionContextScopeGenerator() = default;
};

#endif // QGSEXPRESSIONCONTEXTGENERATOR_H
