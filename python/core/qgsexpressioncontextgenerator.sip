/***************************************************************************
  qgsexpressioncontextgenerator.sip - QgsExpressionContextGenerator

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
/**
 * \ingroup core
 * Abstract interface for generating an expression context.
 *
 * You need to implement this interface in a class and register this class with
 * QgsFieldExpressionWidget::registerExpressionGenerator().
 *
 * Whenever this widget requires an expression context it will call the createExpressionContext()
 * method to get a context.
 *
 * @note added in QGIS 3.0
 */

class QgsExpressionContextGenerator
{
%TypeHeaderCode
#include "qgsexpressioncontextgenerator.h"
%End

  public:
    /**
     * This method needs to be reimplemented in all classes which implement this interface
     * and return an expression context.
     */
    virtual QgsExpressionContext createExpressionContext() const = 0;

    virtual ~QgsExpressionContextGenerator();
};
