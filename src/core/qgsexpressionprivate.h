/***************************************************************************
 qgsexpressionprivate.h

 ---------------------
 begin                : 9.12.2015
 copyright            : (C) 2015 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXPRESSIONPRIVATE_H
#define QGSEXPRESSIONPRIVATE_H

#include <QString>
#include <QSharedPointer>

#include "qgsexpression.h"
#include "qgsdistancearea.h"
#include "qgsunittypes.h"

///@cond
/**
 * This class exists only for implicit sharing of QgsExpression
 * and is not part of the public API.
 * It should be considered an implementation detail.
 */
class QgsExpressionPrivate
{
  public:
    QgsExpressionPrivate()
        : ref( 1 )
        , mRootNode( nullptr )
        , mCalc( nullptr )
        , mDistanceUnit( QgsUnitTypes::DistanceUnknownUnit )
        , mAreaUnit( QgsUnitTypes::AreaUnknownUnit )
    {}

    QgsExpressionPrivate( const QgsExpressionPrivate& other )
        : ref( 1 )
        , mRootNode( other.mRootNode ? other.mRootNode->clone() : nullptr )
        , mParserErrorString( other.mParserErrorString )
        , mEvalErrorString( other.mEvalErrorString )
        , mExp( other.mExp )
        , mCalc( other.mCalc )
        , mDistanceUnit( other.mDistanceUnit )
        , mAreaUnit( other.mAreaUnit )
    {}

    ~QgsExpressionPrivate()
    {
      delete mRootNode;
    }

    QAtomicInt ref;

    QgsExpression::Node* mRootNode;

    QString mParserErrorString;
    QString mEvalErrorString;

    QString mExp;

    QSharedPointer<QgsDistanceArea> mCalc;
    QgsUnitTypes::DistanceUnit mDistanceUnit;
    QgsUnitTypes::AreaUnit mAreaUnit;
};
///@endcond

#endif // QGSEXPRESSIONPRIVATE_H
