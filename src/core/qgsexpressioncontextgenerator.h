/***************************************************************************
  qgsexpressioncontextgenerator.h - QgsExpressionContextGenerator

 ---------------------
 begin                : 1.8.2016
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
#ifndef QGSEXPRESSIONCONTEXTGENERATOR_H
#define QGSEXPRESSIONCONTEXTGENERATOR_H

#include "qgsexpressioncontext.h"

/**
 * \ingroup core
 * Abstract interface for generating an expression context.
 *
 * You need to implement this interface in a class and register this class with
 * QgsFieldExpressionWidget::registerExpressionGenerator().
 *
 * This is used for example in QgsPropertyOverrideButton or QgsFieldExpressionWidget
 * classes which will ask for a new QgsExpressionContext every time the expression
 * editor is opened. This way they are able to provide an up-to-date expression
 * editor even when the environment changes.
 *
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsExpressionContextGenerator
{
  public:

    /**
     * This method needs to be reimplemented in all classes which implement this interface
     * and return an expression context.
     *
     * \since QGIS 3.0
     */
    virtual QgsExpressionContext createExpressionContext() const = 0;

    virtual ~QgsExpressionContextGenerator() = default;
};

#endif // QGSEXPRESSIONCONTEXTGENERATOR_H
