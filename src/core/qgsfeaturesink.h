/***************************************************************************
                         qgsfeaturesink.h
                         ----------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFEATURESINK_H
#define QGSFEATURESINK_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"

/**
 * \class QgsFeatureSink
 * \ingroup core
 * An interface for objects which accept features via addFeature(s) methods.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsFeatureSink
{
  public:

    /**
     * Flags that can be set on a QgsFeatureSink. Not all sinks may implement all flags.
     *
     * \since QGIS 3.4
     */
    enum SinkFlag
    {

      /**
       * This flag indicates, that a primary key field cannot be guaranteed to be unique and
       * the sink should ignore it if somehow possible.
       * This should for example be set for a geopackage file if the field "fid" has a risk
       * to contain duplicate entries. In this case sinks like QgsVectorFileWriter or
       * QgsVectorLayerExporter will prefer to regenerate the fid instead of trying to reuse
       * the fids provided in addFeature calls.
       *
       * \since QGIS 3.4
       */
      RegeneratePrimaryKey = 1 << 1,
    };
    Q_DECLARE_FLAGS( SinkFlags, SinkFlag )

    //! Flags controlling how features are added to a sink.
    enum Flag
    {

      /**
        * Use faster inserts, at the cost of updating the passed features to reflect changes made at the provider.
        * This includes skipping the update of the passed feature IDs to match the resulting feature IDs for the
        * feature within the data provider.
        * Individual sink subclasses may or may not choose to respect this flag, depending on whether or not
        * skipping this update represents a significant speed boost for the operation.
        */
      FastInsert = 1 << 1,
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    virtual ~QgsFeatureSink() = default;

    /**
     * Adds a single \a feature to the sink. Feature addition behavior is controlled by the specified \a flags.
     * \see addFeatures()
     * \returns true in case of success and false in case of failure
     */
    virtual bool addFeature( QgsFeature &feature, QgsFeatureSink::Flags flags = nullptr );

    /**
     * Adds a list of \a features to the sink. Feature addition behavior is controlled by the specified \a flags.
     * \see addFeature()
     * \returns true in case of success and false in case of failure
     */
    virtual bool addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags = nullptr ) = 0;

    /**
     * Adds all features from the specified \a iterator to the sink. Feature addition behavior is controlled by the specified \a flags.
     * \returns true if all features were added successfully, or false if any feature could not be added
     */
    virtual bool addFeatures( QgsFeatureIterator &iterator, QgsFeatureSink::Flags flags = nullptr );

    /**
     * Flushes any internal buffer which may exist in the sink, causing any buffered features to be added to the sink's destination.
     * \returns false if any buffered features could not be added to the sink.
     */
    virtual bool flushBuffer() { return true; }
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsFeatureSink::Flags )


/**
 * \class QgsProxyFeatureSink
 * \ingroup core
 * A simple feature sink which proxies feature addition on to another feature sink.
 *
 * This class is designed to allow factory methods which always return new QgsFeatureSink
 * objects. Since it is not always possible to create an entirely new QgsFeatureSink
 * (e.g. if the feature sink is a layer's data provider), a new QgsProxyFeatureSink
 * can instead be returned which forwards features on to the destination sink. The
 * proxy sink can be safely deleted without affecting the destination sink.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsProxyFeatureSink : public QgsFeatureSink
{
  public:

    /**
     * Constructs a new QgsProxyFeatureSink which forwards features onto a destination \a sink.
     */
    QgsProxyFeatureSink( QgsFeatureSink *sink );
    bool addFeature( QgsFeature &feature, QgsFeatureSink::Flags flags = nullptr ) override { return mSink->addFeature( feature, flags ); }
    bool addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags = nullptr ) override { return mSink->addFeatures( features, flags ); }
    bool addFeatures( QgsFeatureIterator &iterator, QgsFeatureSink::Flags flags = nullptr ) override { return mSink->addFeatures( iterator, flags ); }

    /**
     * Returns the destination QgsFeatureSink which the proxy will forward features to.
     */
    QgsFeatureSink *destinationSink() { return mSink; }

  private:

    QgsFeatureSink *mSink = nullptr;
};

Q_DECLARE_METATYPE( QgsFeatureSink * )

#endif // QGSFEATURESINK_H
