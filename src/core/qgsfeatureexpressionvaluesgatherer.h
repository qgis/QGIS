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
 * \since QGIS 3.0
 */
template <class T>
class QgsFeatureExpressionValuesGatherer: public QThread
{
    Q_OBJECT

  public:

    /**
       * Constructor
       * \param layer the vector layer
       * \param displayExpression if empty, the display expression is taken from the layer definition
       * \param request the reqeust to perform
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

    //! The type to identify a feature (QgsFeatureId, QVariantList, …)
    using IdentifierType = T;

    struct Entry
    {
      Entry( const T &_identifier, const QString &_value, const QgsFeature &_feature )
        : identifier( _identifier )
        , value( _value )
        , feature( _feature )
      {}

      T identifier;
      QString value;
      QgsFeature feature;

      bool operator()( const Entry &lhs, const Entry &rhs ) const;
    };

    void run() override
    {
      mWasCanceled = false;

      mIterator = mSource->getFeatures( mRequest );

      mDisplayExpression.prepare( &mExpressionContext );

      QgsFeature feat;
      QList<int> attributeIndexes;
      for ( const QString &fieldName : qgis::as_const( mIdentifierFields ) )
        attributeIndexes << mSource->fields().indexOf( fieldName );

      while ( mIterator.nextFeature( feat ) )
      {
        mExpressionContext.setFeature( feat );
        QVariantList attributes;
        for ( const int idx : attributeIndexes )
          attributes << feat.attribute( idx );

        const QString expressionValue = mDisplayExpression.evaluate( &mExpressionContext ).toString();

        addEntry( feat, attributes, expressionValue );

        QMutexLocker locker( &mCancelMutex );
        if ( mWasCanceled )
          return;
      }
    }

    //! Add an entry to the list
    virtual void addEntry( const QgsFeature &feature, const QVariantList &attributes, const QString &expressionValue ) = 0;

    //! Returns TRUE if the 2 entries refers to the same feature
    virtual bool compareEntries( const Entry &a, const Entry &b ) {return a.identifier == b.identifier;}

    //! Returns TRUE if the entry is null
    virtual bool identifierIsNull( const T &identifier ) = 0;

    //! Returns a human reading representation of the identifier
    virtual QString identifierToString( const T &identifier ) = 0;

    //! Informs the gatherer to immediately stop collecting values
    void stop()
    {
      QMutexLocker locker( &mCancelMutex );
      mWasCanceled = true;
    }

    //! Returns TRUE if collection was canceled before completion
    bool wasCanceled() const
    {
      QMutexLocker locker( &mCancelMutex );
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
    QgsFeatureIterator mIterator;
    bool mWasCanceled = false;
    mutable QMutex mCancelMutex;
    QStringList mIdentifierFields;
    QVariant mData;
};



class QgsFeatureByIdExpressionValuesGatherer : public QgsFeatureExpressionValuesGatherer<QgsFeatureId>
{
  public:
    QgsFeatureByIdExpressionValuesGatherer( QgsVectorLayer *layer,
                                            const QString &displayExpression = QString(),
                                            const QgsFeatureRequest &request = QgsFeatureRequest() )
      : QgsFeatureExpressionValuesGatherer( layer, displayExpression, request )
    {}

    static Entry nullEntry()
    {
      return Entry( QgsFeatureId( FID_NULL ), QgsApplication::nullRepresentation(), QgsFeature() );
    }

    void addEntry( const QgsFeature &feature, const QVariantList &attributes, const QString &expressionValue ) override
    {
      Q_UNUSED( attributes )
      mEntries.append( Entry( feature.id(), expressionValue, feature ) );
    }

    bool identifierIsNull( const QgsFeatureId &identifier ) override
    {
      return identifier == FID_NULL;
    }

    QString identifierToString( const QgsFeatureId &identifier ) override
    {
      return QStringLiteral( "(%1)" ).arg( identifier );
    }
};


class QgsFeatureByIdentifierFieldsExpressionValuesGatherer : public QgsFeatureExpressionValuesGatherer<QVariantList>
{
  public:
    QgsFeatureByIdentifierFieldsExpressionValuesGatherer( QgsVectorLayer *layer,
        const QString &displayExpression = QString(),
        const QgsFeatureRequest &request = QgsFeatureRequest(),
        const QStringList &identifierFields = QStringList() )
      : QgsFeatureExpressionValuesGatherer( layer, displayExpression, request, identifierFields )
    {}

    bool compareEntries( const Entry &a, const Entry &b ) override {return qVariantListCompare( a.identifier, b.identifier );}

    static Entry nullEntry()
    {
      return Entry( QVariantList(), QgsApplication::nullRepresentation(), QgsFeature() );
    }

    void addEntry( const QgsFeature &feature, const QVariantList &attributes, const QString &expressionValue ) override
    {
      mEntries.append( Entry( attributes, expressionValue, feature ) );
    }

    bool identifierIsNull( const QList<QVariant> &identifier ) override
    {
      return identifier.isEmpty();
    }

    QString identifierToString( const QList<QVariant> &identifier ) override
    {
      QStringList values;
      for ( const QVariant &v : qgis::as_const( identifier ) )
        values << QStringLiteral( "(%1)" ).arg( v.toString() );
      return values.join( QStringLiteral( " " ) );
    }

  private:
    bool qVariantListCompare( const QVariantList &a, const QVariantList &b )
    {
      if ( a.size() != b.size() )
        return false;

      for ( int i = 0; i < a.size(); ++i )
      {
        if ( !qgsVariantEqual( a.at( i ), b.at( i ) ) )
          return false;
      }
      return true;
    }
};
///@endcond


#endif // QGSFEATUREEXPRESSIONVALUESGATHERER_H
