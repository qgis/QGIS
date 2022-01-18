/***************************************************************************
                         qgspointcouldexpression_p.h
                         ---------------------------
    begin                : January 2022
    copyright            : (C) 2022 Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDEXPRESSIONPRIVATE_H
#define QGSPOINTCLOUDEXPRESSIONPRIVATE_H

#include <QString>
#include <memory>

#include "qgspointcloudexpression.h"
#include "qgspointcloudexpressionnode.h"

///@cond

/**
 * This class exists only for implicit sharing of QgsPointcloudExpression
 * and is not part of the public API.
 * It should be considered an implementation detail.
 */
class QgsPointcloudExpressionPrivate
{
  public:
    QgsPointcloudExpressionPrivate()
      : ref( 1 )
    {}

    QgsPointcloudExpressionPrivate( const QgsPointcloudExpressionPrivate &other )
      : ref( 1 )
      , mRootNode( other.mRootNode ? other.mRootNode->clone() : nullptr )
      , mParserErrorString( other.mParserErrorString )
      , mEvalErrorString( other.mEvalErrorString )
      , mParserErrors( other.mParserErrors )
      , mExp( other.mExp )
    {}

    ~QgsPointcloudExpressionPrivate()
    {
      delete mRootNode;
    }

    QAtomicInt ref;

    QgsPointcloudExpressionNode *mRootNode = nullptr;

    QString mParserErrorString;
    QString mEvalErrorString;

    QList<QgsPointcloudExpression::ParserError> mParserErrors;

    QString mExp;

    //! Whether prepare() has been called before evaluate()
    bool mIsPrepared = false;

    QgsPointcloudExpressionPrivate &operator= ( const QgsPointcloudExpressionPrivate & ) = delete;
};

///@endcond

#endif // QGSPOINTCLOUDEXPRESSIONPRIVATE_H
