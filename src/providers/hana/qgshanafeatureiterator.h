/***************************************************************************
   qgshanafeatureiterator.h
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maksim Rylov
 ***************************************************************************/

/***************************************************************************
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
***************************************************************************/
#ifndef QGSHANAFEATUREITERATOR_H
#define QGSHANAFEATUREITERATOR_H

#include "qgsfeatureiterator.h"
#include "qgshanaconnection.h"
#include "qgshanaprovider.h"

#include "odbc/Forwards.h"

class QgsHanaFeatureSource : public QgsAbstractFeatureSource
{
  public:
    explicit QgsHanaFeatureSource( const QgsHanaProvider *p );
    ~QgsHanaFeatureSource() override;

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;

  private:
    bool isSpatial() const { return !mGeometryColumn.isEmpty() || mGeometryType != QgsWkbTypes::Unknown; }

  private:
    QgsDataSourceUri mUri;
    QString mSchemaName;
    QString mTableName;
    QString mFidColumn;
    QgsFields mFields;
    QVector<FieldInfo> mFieldInfos;
    QString mGeometryColumn;
    QgsWkbTypes::Type mGeometryType;
    int mSrid;
    QgsRectangle mSrsExtent;
    QgsCoordinateReferenceSystem mCrs;
    QString mQueryWhereClause;

    friend class QgsHanaFeatureIterator;
    friend class QgsHanaExpressionCompiler;
};

class QgsHanaFeatureIterator : public QgsAbstractFeatureIteratorFromSource<QgsHanaFeatureSource>
{
  public:
    QgsHanaFeatureIterator(
      QgsHanaFeatureSource *source,
      bool ownSource,
      const QgsFeatureRequest &request );

    ~QgsHanaFeatureIterator() override;

    bool rewind() override;
    bool close() override;

  protected:
    bool fetchFeature( QgsFeature &feature ) override;
    void fetchFeatureAttribute( int attrIndex, unsigned short paramIndex, QgsFeature &feature );
    void fetchFeatureGeometry( unsigned short paramIndex, QgsFeature &feature );
    bool nextFeatureFilterExpression( QgsFeature &feature ) override;

  private:
    void buildStatement( const QgsFeatureRequest &request );
    void ensureBufferCapacity( std::size_t capacity );
    QString getBBOXFilter( const QgsRectangle &bbox, const QVersionNumber &dbVersion ) const;

  private:
    QgsHanaConnectionRef mConnRef;
    odbc::PreparedStatementRef mStatement;
    odbc::ResultSetRef mResultSet;
    QgsCoordinateTransform mTransform;
    QgsRectangle mFilterRect;
    QgsRectangle mSrsExtent;
    QgsAttributeList mAttributesToFetch;
    std::vector<unsigned char> mBuffer;
    bool mHasFidColumn;
    bool mHasAttributes;
    bool mHasGeometryColumn;
    bool mExpressionCompiled = false;
};

#endif // QGSHANAFEATUREITERATOR_H
