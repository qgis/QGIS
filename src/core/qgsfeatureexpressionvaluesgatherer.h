/***************************************************************************
  qgsfeatureexpressionvaluesgatherer - QgsFeatureExpressionValuesGatherer
 ---------------------
 begin                : 10.3.2017
 copyright            : (C) 2017 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFEATUREEXPRESSIONVALUESGATHERER_H
#define QGSFEATUREEXPRESSIONVALUESGATHERER_H

#include <QThread>
#include <QMutex>
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerfeatureiterator.h"

#define SIP_NO_FILE

// just internal guff - definitely not for exposing to public API!
///@cond PRIVATE

/**
 * \class QgsFieldExpressionValuesGatherer
 * Gathers features with substring matching on an expression.
 *
 * \since QGIS 3.14
 */
class QgsFeatureExpressionValuesGatherer: public QThread
{
    Q_OBJECT

  public:

    /**
       * Constructor
       * \param layer the vector layer
       * \param displayExpression if empty, the display expression is taken from the layer definition
       * \param request the request to perform
       * \param identifierFields an optional list of fields name to be save in a variant list for an easier reuse
       */
    QgsFeatureExpressionValuesGatherer( QgsVectorLayer *layer,
                                        const QString &displayExpression = QString(),
                                        const QgsFeatureRequest &request = QgsFeatureRequest(),
                                        const QStringList &identifierFields = QStringList() )
      : mSource( new QgsVectorLayerFeatureSource( layer ) )
      , mDisplayExpression( displayExpression.isEmpty() ? layer->displayExpression() : displayExpression )
      , mRequest( request )
      , mIdentifierFields( identifierFields )
    {
    }

    struct Entry
    {
      Entry() = default;

      Entry( const QVariantList &_identifierFields, const QString &_value, const QgsFeature &_feature )
        : identifierFields( _identifierFields )
        , featureId( _feature.isValid() ? _feature.id() : FID_NULL )
        , value( _value )
        , feature( _feature )
      {}

      Entry( const QgsFeatureId &_featureId, const QString &_value, const QgsVectorLayer *layer )
        : featureId( _featureId )
        , value( _value )
        , feature( QgsFeature( layer ? layer->fields() : QgsFields() ) )
      {}

      QVariantList identifierFields;
      QgsFeatureId featureId;
      QString value;
      QgsFeature feature;

      bool operator()( const Entry &lhs, const Entry &rhs ) const;
    };

    static Entry nullEntry( QgsVectorLayer *layer )
    {
      return Entry( QVariantList(), QgsApplication::nullRepresentation(), QgsFeature( layer->fields() ) );
    }

    void run() override
    {
      mWasCanceled = false;

      QgsFeatureIterator iterator = mSource->getFeatures( mRequest );

      mDisplayExpression.prepare( &mExpressionContext );

      QgsFeature feature;
      QList<int> attributeIndexes;
      for ( auto it = mIdentifierFields.constBegin(); it != mIdentifierFields.constEnd(); ++it )
        attributeIndexes << mSource->fields().indexOf( *it );

      while ( iterator.nextFeature( feature ) )
      {
        mExpressionContext.setFeature( feature );
        QVariantList attributes;
        for ( const int idx : attributeIndexes )
          attributes << feature.attribute( idx );

        const QString expressionValue = mDisplayExpression.evaluate( &mExpressionContext ).toString();

        mEntries.append( Entry( attributes, expressionValue, feature ) );

        const QMutexLocker locker( &mCancelMutex );
        if ( mWasCanceled )
          return;
      }
    }

    //! Informs the gatherer to immediately stop collecting values
    void stop()
    {
      const QMutexLocker locker( &mCancelMutex );
      mWasCanceled = true;
    }

    //! Returns TRUE if collection was canceled before completion
    bool wasCanceled() const
    {
      const QMutexLocker locker( &mCancelMutex );
      return mWasCanceled;
    }

    QVector<Entry> entries() const
    {
      return mEntries;
    }

    QgsFeatureRequest request() const
    {
      return mRequest;
    }

    /**
     * Internal data, use for whatever you want.
     */
    QVariant data() const
    {
      return mData;
    }

    /**
     * Internal data, use for whatever you want.
     */
    void setData( const QVariant &data )
    {
      mData = data;
    }

  protected:
    QVector<Entry> mEntries;

  private:
    std::unique_ptr<QgsVectorLayerFeatureSource> mSource;
    QgsExpression mDisplayExpression;
    QgsExpressionContext mExpressionContext;
    QgsFeatureRequest mRequest;
    bool mWasCanceled = false;
    mutable QMutex mCancelMutex;
    QStringList mIdentifierFields;
    QVariant mData;
};

///@endcond


#endif // QGSFEATUREEXPRESSIONVALUESGATHERER_H
