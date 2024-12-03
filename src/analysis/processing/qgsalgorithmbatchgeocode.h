/***************************************************************************
                         qgsalgorithmbatchgeocode.h
                         ------------------
    begin                : August 2020
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

#ifndef QGSALGORITHMBATCHGEOCODE_H
#define QGSALGORITHMBATCHGEOCODE_H

#include "qgis_sip.h"
#include "qgis_analysis.h"
#include "qgsprocessingalgorithm.h"

class QgsGeocoderInterface;

/**
 * \ingroup analysis
 *
 * \brief A base class for batch geocoder algorithms, which takes a QgsGeocoderInterface object and exposes it as
 * a Processing algorithm for batch geocoding operations.
 *
 * ### Example
 *
 * \code{.py}
 *   # create a class which implements the QgsGeocoderInterface interface:
 *   class MyGeocoder(QgsGeocoderInterface):
 *
 *      def geocodeString(self, string, context, feedback):
 *         # calculate and return results...
 *
 *   my_geocoder = MyGeocoder()
 *
 *   # create an algorithm which allows for batch geocoding operations using the custom geocoder interface
 *   # and implement the few required pure virtual methods
 *   class MyGeocoderAlgorithm(QgsBatchGeocodeAlgorithm):
 *
 *       def __init__(self):
 *           super().__init__(my_geocoder)
 *
 *       def displayName(self):
 *           return "My Geocoder"
 *
 *       def name(self):
 *           return "my_geocoder_alg"
 *
 *       def createInstance(self):
 *           return MyGeocoderAlgorithm()
 *
 *       # optionally, the group(), groupId(), tags(), shortHelpString() and other metadata style methods can be overridden and customized:
 *       def tags(self):
 *           return 'geocode,my service,batch'
 *
 * \endcode
 *
 * \since QGIS 3.18
 */
class ANALYSIS_EXPORT QgsBatchGeocodeAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    /**
     * Constructor for QgsBatchGeocodeAlgorithm.
     *
     * The \a geocoder must specify an instance of a class which implements the QgsGeocoderInterface
     * interface. Ownership of \a geocoder is not transferred, and the caller must ensure that \a geocoder
     * exists for the lifetime of this algorithm.
     */
    QgsBatchGeocodeAlgorithm( QgsGeocoderInterface *geocoder );

    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QList<int> inputLayerTypes() const override;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;

  protected:
    QString outputName() const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback *feedback ) override;
    QgsCoordinateReferenceSystem outputCrs( const QgsCoordinateReferenceSystem &inputCrs ) const override;
    QgsFields outputFields( const QgsFields &inputFields ) const override;
    Qgis::WkbType outputWkbType( Qgis::WkbType inputWkbType ) const override;

  private:
    bool mIsInPlace = false;
    QString mAddressField;
    QgsGeocoderInterface *mGeocoder = nullptr;
    QgsStringMap mInPlaceFieldMap;
    mutable QgsCoordinateReferenceSystem mOutputCrs;
    mutable QStringList mAdditionalFields;
};


#endif // QGSALGORITHMBATCHGEOCODE_H
