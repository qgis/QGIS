/***************************************************************************
    qgsvectorwarper.h
     --------------------------------------
    Date                 : February 2022
    Copyright            : (C) 2022 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORWARPER_H
#define QGSVECTORWARPER_H

#include "qgspoint.h"
#include "qgis_analysis.h"
#include "qgis_sip.h"
#include "qgsgcptransformer.h"
#include "qgsgcppoint.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturesource.h"
#include "qgstaskmanager.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayerfeatureiterator.h"

class QgsFeatureSink;

/**
 * \ingroup analysis
 * \brief Vector layer warper which warps vector layers based on a list of source and destination GCPs.
 *
 * \since QGIS 3.26
*/
class ANALYSIS_EXPORT QgsVectorWarper
{
  public:

    /**
     * Constructor for QgsVectorWarper.
     *
     * \param method specifies the transformation method
     * \param points list of GCP points to use for the transformation
     * \param destinationCrs target CRS for transformed features
     */
    explicit QgsVectorWarper( QgsGcpTransformerInterface::TransformMethod method, const QList < QgsGcpPoint > &points,
                              const QgsCoordinateReferenceSystem &destinationCrs );

    /**
     * Transforms the features from \a iterator and adds the results to the specified \a sink.
     *
     * The optional \a feedback argument can be used for progress reporting and cancellation.
     *
     * \returns TRUE if all features were successfully transformed
     */
    bool transformFeatures( QgsFeatureIterator &iterator, QgsFeatureSink *sink,
                            const QgsCoordinateTransformContext &context,
                            QgsFeedback *feedback = nullptr ) const;

    /**
     * Returns the last error obtained during transformation.
     */
    QString error() const { return mError; }

  private:
    QgsGcpTransformerInterface::TransformMethod mMethod = QgsGcpTransformerInterface::TransformMethod::Linear;
    QList < QgsGcpPoint > mPoints;
    QgsCoordinateReferenceSystem mDestinationCrs;

    mutable QString mError;

};


/**
 * \ingroup analysis
 * \brief A task for warping a vector layer in a background thread.
 *
 * \since QGIS 3.26
*/
class ANALYSIS_EXPORT QgsVectorWarperTask : public QgsTask
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsVectorWarperTask.
     *
     * \param method transformation method
     * \param points GCP point list
     * \param destinationCrs destination layer CRS
     * \param layer source layer
     * \param fileName filename for destination layer
     */
    QgsVectorWarperTask( QgsGcpTransformerInterface::TransformMethod method, const QList < QgsGcpPoint > &points,
                         const QgsCoordinateReferenceSystem &destinationCrs,
                         QgsVectorLayer *layer,
                         const QString &fileName );

    void cancel() override;

    //! Task results
    enum class Result
    {
      Success, //!< Warping completed successfully
      Canceled, //!< Task was canceled before completion
      Error, //!< An error occurred while warping
    };

    /**
     * Returns the result of running the task.
     */
    Result result() const { return mResult; }

    /**
     * Returns the descriptive error message, if an error occurred.
     */
    QString errorMessage() const { return mErrorMessage; }

  protected:

    bool run() override;

  private:

    QgsGcpTransformerInterface::TransformMethod mMethod = QgsGcpTransformerInterface::TransformMethod::Linear;
    long long mFeatureCount = 0;
    std::unique_ptr< QgsVectorLayerFeatureSource > mSource;
    QList < QgsGcpPoint > mPoints;
    QgsCoordinateReferenceSystem mDestinationCrs;

    QString mDestFileName;

    std::unique_ptr< QgsFeedback > mFeedback;

    QgsCoordinateTransformContext mTransformContext;
    QgsFields mFields;
    QgsWkbTypes::Type mWkbType = QgsWkbTypes::Unknown;

    Result mResult = Result::Success;
    QString mErrorMessage;
    double mLastProgress = 0;
};


#endif //QGSVECTORWARPER_H
