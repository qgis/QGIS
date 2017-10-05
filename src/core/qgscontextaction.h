/***************************************************************************
  qgsactionscope.h - QgsActionScope

 ---------------------
 begin                : 25.09.2017
 copyright            : (C) 2017 by C. MARCEL
 email                : clement.marcel@nwanda.fr
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCONTEXTACTION_H
#define QGSCONTEXTACTION_H

#include "qgis_core.h"
#include <QString>
#include <QAction>
#include "qgsexpressioncontext.h"

#define SIP_NO_FILE

/** \ingroup core
 * QgsContextAction is derived from QAction and contains a context expression scope.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsContextAction: public QAction
{
  public:

    /**
     * Creates a new context action.
     *
     * \since QGIS 3.0
     */
    explicit QgsContextAction( QObject *parent );
    QgsContextAction( const QString &text, QObject *parent );
    QgsContextAction( const QIcon &icon, const QString &text, QObject *parent );

    /**
     * Action context scope.
     *
     * \since QGIS 3.0
     */
    QgsExpressionContextScope expressionContextScope() const;

    /**
     * \copydoc expressionContextScope()
     */
    void setExpressionContextScope( const QgsExpressionContextScope &expressionContextScope );

  private:
    QgsExpressionContextScope mExpressionContextScope;
};

#endif // QGSCONTEXTACTION_H
