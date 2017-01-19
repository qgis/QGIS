/***************************************************************************
     qgsproperty_p.h
     ---------------
    Date                 : January 2017
    Copyright            : (C) 2017 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
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

#include "qgis_core.h"
#include <QSharedData>
#include <QVariant>
#include "qgsexpression.h"
#include "qgspropertytransformer.h"

class QgsPropertyPrivate : public QSharedData
{
  public:

    QgsPropertyPrivate() = default;

    QgsPropertyPrivate( const QgsPropertyPrivate& other )
        : QSharedData( other )
        , type( other.type )
        , active( other.active )
        , transformer( other.transformer ? other.transformer->clone() : nullptr )
        , staticData( other.staticData )
        , fieldData( other.fieldData )
        , expressionData( other.expressionData )
    {}

    ~QgsPropertyPrivate()
    {
      delete transformer;
    }

    int type = 0;

    //! Stores whether the property is currently active
    bool active = true;

    //! Optional transfomer
    QgsPropertyTransformer* transformer = nullptr;

    struct StaticData
    {
      QVariant value;
    };
    struct FieldData
    {
      QString fieldName;
      mutable int cachedFieldIdx = -1;
    };
    struct ExpressionData
    {
      QString expressionString;
      mutable bool prepared = false;
      mutable QgsExpression expression;
      //! Cached set of referenced columns
      mutable QSet< QString > referencedCols;
    };

    StaticData staticData;
    FieldData fieldData;
    ExpressionData expressionData;

};

///@endcond PRIVATE

#endif // QGSPROPERTYPRIVATE_H
