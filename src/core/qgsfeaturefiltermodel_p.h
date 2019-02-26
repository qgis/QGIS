/***************************************************************************
  qgsfeaturefiltermodel_p - QgsFieldExpressionValuesGatherer

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
#ifndef QGSFEATUREFILTERMODEL_P_H
#define QGSFEATUREFILTERMODEL_P_H

#include <QThread>
#include "qgsfeaturefiltermodel.h"
#include "qgslogger.h"
#include "qgsvectorlayerfeatureiterator.h"

#define SIP_NO_FILE

// just internal guff - definitely not for exposing to public API!
///@cond PRIVATE

/**
 * \class QgsFieldExpressionValuesGatherer
 * Gathers features with substring matching on an expression.
 *
 * \since QGIS 3.0
 */
class QgsFieldExpressionValuesGatherer: public QThread
{
    Q_OBJECT

  public:
    QgsFieldExpressionValuesGatherer( QgsVectorLayer *layer, const QString &displayExpression, const QString &identifierField, const QgsFeatureRequest &request = QgsFeatureRequest() )
      : mSource( new QgsVectorLayerFeatureSource( layer ) )
      , mDisplayExpression( displayExpression )
      , mRequest( request )
      , mIdentifierField( identifierField )
    {
    }

    void run() override
    {
      mWasCanceled = false;

      mIterator = mSource->getFeatures( mRequest );

      mDisplayExpression.prepare( &mExpressionContext );

      QgsFeature feat;
      const int attribute = mSource->fields().indexOf( mIdentifierField );

      while ( mIterator.nextFeature( feat ) )
      {
        mExpressionContext.setFeature( feat );
        mEntries.append( QgsFeatureFilterModel::Entry( feat.attribute( attribute ), mDisplayExpression.evaluate( &mExpressionContext ).toString(), feat ) );

        if ( mWasCanceled )
          return;
      }

      emit collectedValues();
    }

    //! Informs the gatherer to immediately stop collecting values
    void stop()
    {
      mWasCanceled = true;
    }

    //! Returns TRUE if collection was canceled before completion
    bool wasCanceled() const { return mWasCanceled; }

    QVector<QgsFeatureFilterModel::Entry> entries() const
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

  signals:

    /**
     * Emitted when values have been collected
     * \param values list of unique matching string values
     */
    void collectedValues();

  private:

    std::unique_ptr<QgsVectorLayerFeatureSource> mSource;
    QgsExpression mDisplayExpression;
    QgsExpressionContext mExpressionContext;
    QgsFeatureRequest mRequest;
    QgsFeatureIterator mIterator;
    bool mWasCanceled = false;
    QVector<QgsFeatureFilterModel::Entry> mEntries;
    QString mIdentifierField;
    QVariant mData;
};

///@endcond


#endif // QGSFEATUREFILTERMODEL_P_H
