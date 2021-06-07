/***************************************************************************
     qgsproperty_p.h
     ---------------
    Date                 : January 2017
    Copyright            : (C) 2020 by Wang Peng
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPROPERTYPRIVATE_H
#define QGSPROPERTYPRIVATE_H

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#define SIP_NO_FILE

#include "qgis_core.h"
#include <QSharedData>
#include <QVariant>
#include "qgsexpression.h"
#include "qgspropertytransformer.h"

class QgsPropertyPrivate : public QSharedData
{
  public:

    QgsPropertyPrivate() = default;

    QgsPropertyPrivate( const QgsPropertyPrivate &other )
      : QSharedData( other )
      , type( other.type )
      , active( other.active )
      , transformer( other.transformer ? other.transformer->clone() : nullptr )
      , staticValue( other.staticValue )
      , fieldName( other.fieldName )
      , cachedFieldIdx( other.cachedFieldIdx )
      , expressionString( other.expressionString )
      , expressionPrepared( other.expressionPrepared )
      , expressionIsInvalid( other.expressionIsInvalid )
      , expression( other.expression )
      , expressionReferencedCols( other.expressionReferencedCols )
    {}

    ~QgsPropertyPrivate()
    {
      delete transformer;
    }

    int type = 0;

    //! Stores whether the property is currently active
    bool active = true;

    //! Optional transformer
    QgsPropertyTransformer *transformer = nullptr;

    // StaticData
    QVariant staticValue;

    // FieldData
    QString fieldName;
    mutable int cachedFieldIdx = -1;

    // ExpressionData
    QString expressionString;
    mutable bool expressionPrepared = false;
    mutable bool expressionIsInvalid = false;
    mutable QgsExpression expression;
    //! Cached set of referenced columns
    mutable QSet< QString > expressionReferencedCols;

  private:
    QgsPropertyPrivate &operator=( const QgsPropertyPrivate & ) = delete;
};

///@endcond PRIVATE

#endif // QGSPROPERTYPRIVATE_H
