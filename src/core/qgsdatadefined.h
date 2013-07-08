/***************************************************************************
    qgsdatadefined.h - Data defined container class
     --------------------------------------
    Date                 : 9-May-2013
    Copyright            : (C) 2013 by Larry Shaffer
    Email                : larrys at dakcarto dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDATADEFINED_H
#define QGSDATADEFINED_H

#include "qgsfield.h"
#include "qgsvectorlayer.h"

#include <QStringList>

class QgsExpression;


/** \ingroup core
 * \class QgsDataDefined
 * A container class for data source field mapping or expression.
 * @note added in QGIS 1.9
 */

class CORE_EXPORT QgsDataDefined
{
  public:
    /**
     * Construct a new data defined object
     *
     * @param active Whether the current data defined is active
     * @param useexpr Whether to use expression instead of field
     * @param expr Expression string
     * @param field Field name string
     */
    QgsDataDefined( bool active = false,
                    bool useexpr = false,
                    const QString& expr = QString(),
                    const QString& field = QString() );

    ~QgsDataDefined();

    bool isActive() const { return mActive; }
    void setActive( bool active ) { mActive = active; }

    bool useExpression() const { return mUseExpression; }
    void setUseExpression( bool use ) { mUseExpression = use; }

    QString expressionString() const { return mExpressionString; }
    void setExpressionString( const QString& expr ) { mExpressionString = expr; }

    // @note not available in python bindings
    QMap<QString, QVariant> expressionParams() const { return mExpressionParams; }
    // @note not available in python bindings
    void setExpressionParams( QMap<QString, QVariant> params ) { mExpressionParams = params; }
    void insertExpressionParam( QString key, QVariant param );

    bool prepareExpression( QgsVectorLayer* layer );
    bool expressionIsPrepared() const { return mExpressionPrepared; }

    QgsExpression* expression() { return mExpression; }
    QStringList referencedColumns( QgsVectorLayer* layer );

    QString field() const { return mField; }
    void setField( const QString& field ) { mField = field; }

    // @note not available in python bindings
    QMap< QString, QString > toMap();

  private:
    QgsExpression* mExpression;

    bool mActive;
    bool mUseExpression;
    QString mExpressionString;
    QString mField;

    QMap<QString, QVariant> mExpressionParams;
    bool mExpressionPrepared;
    QStringList mExprRefColmuns;
};

#endif // QGSDATADEFINED_H
