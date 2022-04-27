/***************************************************************************
                         qgsremappingproxyfeaturesink.h
                         ----------------------
    begin                : April 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#ifndef QGSREMAPPINGPROXYFEATURESINK_H
#define QGSREMAPPINGPROXYFEATURESINK_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsfeaturesink.h"
#include "qgsproperty.h"
#include "qgscoordinatetransform.h"

/**
 * \class QgsRemappingSinkDefinition
 * \ingroup core
 * \brief Defines the parameters used to remap features when creating a QgsRemappingProxyFeatureSink.
 *
 * The definition includes parameters required to correctly map incoming features to the structure
 * of the destination sink, e.g. information about how to create output field values and how to transform
 * geometries to match the destination CRS.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsRemappingSinkDefinition
{
  public:

    /**
     * Returns the field mapping, which defines how to map the values from incoming features to destination
     * field values.
     *
     * Field values are mapped using a QgsProperty source object, which allows either direct field value to field value
     * mapping or use of QgsExpression expressions to transform values to the destination field.
     *
     * \see setFieldMap()
     * \see addMappedField()
     */
    QMap< QString, QgsProperty > fieldMap() const { return mFieldMap; }

    /**
     * Sets the field mapping, which defines how to map the values from incoming features to destination
     * field values.
     *
     * Field values are mapped using a QgsProperty source object, which allows either direct field value to field value
     * mapping or use of QgsExpression expressions to transform values to the destination field.
     *
     * \see fieldMap()
     * \see addMappedField()
     */
    void setFieldMap( const QMap< QString, QgsProperty > &map ) { mFieldMap = map; }

    /**
     * Adds a mapping for a destination field.
     *
     * Field values are mapped using a QgsProperty source object, which allows either direct field value to field value
     * mapping or use of QgsExpression expressions to transform values to the destination field.
     *
     * \see setFieldMap()
     * \see fieldMap()
     */
    void addMappedField( const QString &destinationField, const QgsProperty &property ) { mFieldMap.insert( destinationField, property ); }

    /**
     * Returns the source CRS used for reprojecting incoming features to the sink's destination CRS.
     *
     * \see setSourceCrs()
     */
    QgsCoordinateReferenceSystem sourceCrs() const { return mSourceCrs; }

    /**
     * Sets the \a source crs used for reprojecting incoming features to the sink's destination CRS.
     *
     * \see sourceCrs()
     */
    void setSourceCrs( const QgsCoordinateReferenceSystem &source ) { mSourceCrs = source; }

    /**
     * Returns the destination CRS used for reprojecting incoming features to the sink's destination CRS.
     *
     * \see setDestinationCrs()
     */
    QgsCoordinateReferenceSystem destinationCrs() const { return mDestinationCrs; }

    /**
     * Sets the \a destination crs used for reprojecting incoming features to the sink's destination CRS.
     *
     * \see destinationCrs()
     */
    void setDestinationCrs( const QgsCoordinateReferenceSystem &destination ) { mDestinationCrs = destination; }

    /**
     * Returns the WKB geometry type for the destination.
     *
     * \see setDestinationWkbType()
     */
    QgsWkbTypes::Type destinationWkbType() const { return mDestinationWkbType; }

    /**
     * Sets the WKB geometry \a type for the destination.
     *
     * \see setDestinationWkbType()
     */
    void setDestinationWkbType( QgsWkbTypes::Type type ) { mDestinationWkbType = type; }

    /**
     * Returns the fields for the destination sink.
     *
     * \see setDestinationFields()
     */
    QgsFields destinationFields() const { return mDestinationFields; }

    /**
     * Sets the \a fields for the destination sink.
     *
     * \see destinationFields()
     */
    void setDestinationFields( const QgsFields &fields ) { mDestinationFields = fields; }

    /**
     * Saves this remapping definition to a QVariantMap, wrapped in a QVariant.
     * You can use QgsXmlUtils::writeVariant to save it to an XML document.
     * \see loadVariant()
     */
    QVariant toVariant() const;

    /**
     * Loads this remapping definition from a QVariantMap, wrapped in a QVariant.
     * You can use QgsXmlUtils::readVariant to load it from an XML document.
     * \see toVariant()
     */
    bool loadVariant( const QVariantMap &map );

    // TODO c++20 - replace with = default
    bool operator==( const QgsRemappingSinkDefinition &other ) const;
    bool operator!=( const QgsRemappingSinkDefinition &other ) const;

  private:

    QMap< QString, QgsProperty > mFieldMap;

    QgsCoordinateReferenceSystem mSourceCrs;
    QgsCoordinateReferenceSystem mDestinationCrs;

    QgsWkbTypes::Type mDestinationWkbType = QgsWkbTypes::Unknown;

    QgsFields mDestinationFields;

};

Q_DECLARE_METATYPE( QgsRemappingSinkDefinition )



/**
 * \class QgsRemappingProxyFeatureSink
 * \ingroup core
 * \brief A QgsFeatureSink which proxies incoming features to a destination feature sink, after applying
 * transformations and field value mappings.
 *
 * This sink allows for transformation of incoming features to match the requirements of storing
 * in an existing destination layer, e.g. by reprojecting the features to the destination's CRS
 * and by coercing geometries to the format required by the destination sink.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsRemappingProxyFeatureSink : public QgsFeatureSink
{
  public:

#ifndef SIP_RUN

    /**
     * Constructor for QgsRemappingProxyFeatureSink, using the specified \a mappingDefinition
     * to manipulate features before sending them to the destination \a sink.
     *
     * Ownership of \a sink is dictated by \a ownsSink. If \a ownsSink is FALSE,
     * ownership is not transferred, and callers must ensure that \a sink exists for the lifetime of this object.
     * If \a ownsSink is TRUE, then this object will take ownership of \a sink.
     */
    QgsRemappingProxyFeatureSink( const QgsRemappingSinkDefinition &mappingDefinition, QgsFeatureSink *sink, bool ownsSink = false );
#else

    /**
     * Constructor for QgsRemappingProxyFeatureSink, using the specified \a mappingDefinition
     * to manipulate features before sending them to the destination \a sink.
     */
    QgsRemappingProxyFeatureSink( const QgsRemappingSinkDefinition &mappingDefinition, QgsFeatureSink *sink );
#endif

    ~QgsRemappingProxyFeatureSink() override;

    /**
     * Sets the expression \a context to use when evaluating mapped field values.
     */
    void setExpressionContext( const QgsExpressionContext &context ) const;

    /**
     * Sets the transform \a context to use when reprojecting features.
     */
    void setTransformContext( const QgsCoordinateTransformContext &context );

    /**
     * Remaps a \a feature to a set of features compatible with the destination sink.
     */
    QgsFeatureList remapFeature( const QgsFeature &feature ) const;

    bool addFeature( QgsFeature &feature, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;
    bool addFeatures( QgsFeatureList &features, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;
    bool addFeatures( QgsFeatureIterator &iterator, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;
    QString lastError() const override;

    /**
     * Returns the destination QgsFeatureSink which the proxy will forward features to.
     */
    QgsFeatureSink *destinationSink() { return mSink; }

  private:

    QgsRemappingSinkDefinition mDefinition;
    QgsCoordinateTransform mTransform;
    QgsFeatureSink *mSink = nullptr;
    mutable QgsExpressionContext mContext;
    bool mOwnsSink = false;
};

#endif // QGSREMAPPINGPROXYFEATURESINK_H




