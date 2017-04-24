/***************************************************************************
                         qgsprocessingcontext.h
                         ----------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGCONTEXT_H
#define QGSPROCESSINGCONTEXT_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsproject.h"
#include "qgsexpressioncontext.h"

/**
 * \class QgsProcessingContext
 * \ingroup core
 * Contains information about the context in which a processing algorithm is executed.
 *
 * Contextual information includes settings such as the associated project, and
 * expression context.
 * \since QGIS 3.0
*/

class CORE_EXPORT QgsProcessingContext
{
  public:

    QgsProcessingContext() = default;

    /**
     * Returns the project in which the algorithm is being executed.
     * \see setProject()
     */
    QgsProject *project() const { return mProject; }

    /**
     * Sets the \a project in which the algorithm will be executed.
     * \see project()
     */
    void setProject( QgsProject *project ) { mProject = project; }

    /**
     * Returns the expression context.
     */
    QgsExpressionContext expressionContext() const { return mExpressionContext; }

    /**
     * Sets the expression \a context.
     */
    void setExpressionContext( const QgsExpressionContext &context ) { mExpressionContext = context; }

  private:

    QPointer< QgsProject > mProject;

    QgsExpressionContext mExpressionContext;

};

#endif // QGSPROCESSINGPARAMETERS_H




