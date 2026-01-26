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
#include "qgis_sip.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"

/**
 * \class QgsFeatureSink
 * \ingroup core
 * \brief An interface for objects which accept features via addFeature(s) methods.
 *
 */
class CORE_EXPORT QgsFeatureSink
{
  public:

    /**
     * Flags that can be set on a QgsFeatureSink. Not all sinks may implement all flags.
     *
     * \since QGIS 3.4
     */
    enum SinkFlag SIP_ENUM_BASETYPE( IntFlag )
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
    enum Flag SIP_ENUM_BASETYPE( IntFlag )
    {

      /**
        * Use faster inserts, at the cost of updating the passed features to reflect changes made at the provider.
        * This includes skipping the update of the passed feature IDs to match the resulting feature IDs for the
        * feature within the data provider.
        * Individual sink subclasses may or may not choose to respect this flag, depending on whether or not
        * skipping this update represents a significant speed boost for the operation.
        */
      FastInsert = 1 << 1,

      /**
       * Roll back the whole transaction if a single add feature operation fails.
       * Individual sink subclasses may choose to ignore this flag and always roll back
       * while other providers will respect the flag and accept partial additions if
       * this flag is not set.
       */
      RollBackOnErrors = 1 << 2,
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    virtual ~QgsFeatureSink() = default;

    /**
     * Finalizes the sink, flushing any buffered features to the destination.
     *
     * \since QGIS 3.42
     */
    virtual void finalize();

    /**
     * Adds a single \a feature to the sink. Feature addition behavior is controlled by the specified \a flags.
     * \see addFeatures()
     * \returns TRUE in case of success and FALSE in case of failure
     */
    virtual bool addFeature( QgsFeature &feature, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() );

    /**
     * Adds a list of \a features to the sink. Feature addition behavior is controlled by the specified \a flags.
     * \see addFeature()
     * \returns TRUE in case of success and FALSE in case of failure
     */
    virtual bool addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) = 0;

    /**
     * Adds all features from the specified \a iterator to the sink. Feature addition behavior is controlled by the specified \a flags.
     * \returns TRUE if all features were added successfully, or FALSE if any feature could not be added
     */
    virtual bool addFeatures( QgsFeatureIterator &iterator, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() );

    /**
     * Flushes any internal buffer which may exist in the sink, causing any buffered features to be added to the sink's destination.
     * \returns FALSE if any buffered features could not be added to the sink.
     */
    virtual bool flushBuffer() { return true; }

    /**
     * Returns the most recent error encountered by the sink, e.g. when a call to addFeatures() returns FALSE.
     *
     * \since QGIS 3.16
     */
    virtual QString lastError() const { return QString(); }
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsFeatureSink::Flags )

Q_DECLARE_METATYPE( QgsFeatureSink * )

#endif // QGSFEATURESINK_H
