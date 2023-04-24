/***************************************************************************
 qgsexpression_p.h

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
#include <memory>

#include "qgsexpression.h"
#include "qgsdistancearea.h"
#include "qgis.h"
#include "qgsexpressionnode.h"

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
    {}

    QgsExpressionPrivate( const QgsExpressionPrivate &other )
      : ref( 1 )
      , mRootNode( other.mRootNode ? other.mRootNode->clone() : nullptr )
      , mParserErrorString( other.mParserErrorString )
      , mEvalErrorString( other.mEvalErrorString )
      , mParserErrors( other.mParserErrors )
      , mExp( other.mExp )
      , mDaEllipsoid( other.mDaEllipsoid )
      , mCalc( other.mCalc )
      , mDistanceUnit( other.mDistanceUnit )
      , mAreaUnit( other.mAreaUnit )
      , mIsPrepared( false )
    {
      if ( other.mDaCrs )
        mDaCrs = std::make_unique<QgsCoordinateReferenceSystem>( *other.mDaCrs.get() );
      if ( other.mDaTransformContext )
        mDaTransformContext = std::make_unique<QgsCoordinateTransformContext>( *other.mDaTransformContext.get() );
    }

    ~QgsExpressionPrivate()
    {
      delete mRootNode;
    }

    QAtomicInt ref;

    QgsExpressionNode *mRootNode = nullptr;

    QString mParserErrorString;
    QString mEvalErrorString;

    QList<QgsExpression::ParserError> mParserErrors;

    QString mExp;

    QString mDaEllipsoid;
    std::unique_ptr<QgsCoordinateReferenceSystem> mDaCrs;
    std::unique_ptr<QgsCoordinateTransformContext> mDaTransformContext;

    std::shared_ptr<QgsDistanceArea> mCalc;
    Qgis::DistanceUnit mDistanceUnit = Qgis::DistanceUnit::Unknown;
    Qgis::AreaUnit mAreaUnit = Qgis::AreaUnit::Unknown;

    //! Whether prepare() has been called before evaluate()
    bool mIsPrepared = false;

    QgsExpressionPrivate &operator= ( const QgsExpressionPrivate & ) = delete;
};


///@endcond

#endif // QGSEXPRESSIONPRIVATE_H
