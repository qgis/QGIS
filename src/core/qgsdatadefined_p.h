/***************************************************************************
    qgsdatadefined_p.h
     -----------------
    Date                 : May-2015
    Copyright            : (C) 2015 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDATADEFINED_PRIVATE_H
#define QGSDATADEFINED_PRIVATE_H

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include <QStringList>
#include <QMap>
#include <QSharedData>
#include "qgsexpression.h"


class QgsDataDefinedPrivate : public QSharedData
{
  public:

    QgsDataDefinedPrivate( bool active = false,
                           bool useExpression = false,
                           const QString& expressionString = QString(),
                           const QString& field = QString() )
        : expression( nullptr )
        , active( active )
        , useExpression( useExpression )
        , expressionString( expressionString )
        , field( field )
        , expressionPrepared( false )
    {
    }

    QgsDataDefinedPrivate( const QgsDataDefinedPrivate& other )
        : QSharedData( other )
        , expression( nullptr )
        , active( other.active )
        , useExpression( other.useExpression )
        , expressionString( other.expressionString )
        , field( other.field )
        , expressionParams( other.expressionParams )
        , expressionPrepared( false )
        , exprRefColumns( other.exprRefColumns )
    {
    }

    ~QgsDataDefinedPrivate()
    {
      delete expression;
    }

    bool operator==( const QgsDataDefinedPrivate& other ) const
    {
      return (( active == other.active ) && ( useExpression == other.useExpression )
              && ( expressionString == other.expressionString ) && ( field == other.field ) );
    }

    QgsExpression* expression;

    bool active;
    bool useExpression;
    QString expressionString;
    QString field;

    QMap<QString, QVariant> expressionParams;
    bool expressionPrepared;
    QStringList exprRefColumns;
};

/// @endcond

#endif // QGSDATADEFINED_PRIVATE_H
