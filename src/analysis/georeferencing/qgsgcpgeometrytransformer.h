/***************************************************************************
                             qgsgcpgeometrytransformer.h
                             ----------------------
    begin                : February 2021
    copyright            : (C) 2021 by Nyall Dawson
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

#ifndef QGSGCPGEOMETRYTRANSFORMER_H
#define QGSGCPGEOMETRYTRANSFORMER_H

#include "qgis_analysis.h"
#include "qgsgeometrytransformer.h"
#include "qgsgcptransformer.h"
#include "qgsfeedback.h"
#include <memory>

class QgsGeometry;

/**
 * \class QgsGcpGeometryTransformer
 * \ingroup analysis
 * \brief A geometry transformer which uses an underlying Ground Control Points (GCP) based transformation to modify geometries.
 *
 * \since QGIS 3.18
 */
class ANALYSIS_EXPORT QgsGcpGeometryTransformer : public QgsAbstractGeometryTransformer
{
  public:
    /**
     * Constructor for QgsGcpGeometryTransformer, which uses the specified \a gcpTransformer to
     * modify geometries.
     *
     * Ownership of \a gcpTransformer is transferred to the geometry transformer.
     */
    QgsGcpGeometryTransformer( QgsGcpTransformerInterface *gcpTransformer SIP_TRANSFER );

    /**
     * Constructor for QgsGcpGeometryTransformer, which uses the specified transform \a method and
     * list of source and destination coordinates to transform geometries.
     */
    QgsGcpGeometryTransformer( QgsGcpTransformerInterface::TransformMethod method, const QVector<QgsPointXY> &sourceCoordinates, const QVector<QgsPointXY> &destinationCoordinates );

    ~QgsGcpGeometryTransformer() override;

    QgsGcpGeometryTransformer( const QgsGcpGeometryTransformer &other ) = delete;
    QgsGcpGeometryTransformer &operator=( const QgsGcpGeometryTransformer &other ) = delete;

    bool transformPoint( double &x SIP_INOUT, double &y SIP_INOUT, double &z SIP_INOUT, double &m SIP_INOUT ) override;

    /**
     * Transforms the specified input \a geometry using the GCP based transform.
     *
     * \param geometry Input geometry to transform
     * \param ok will be set to TRUE if geometry was successfully transformed, or FALSE if an error occurred
     * \param feedback This optional argument can be used to cancel the transformation before it completes.
     * If this is done, the geometry will be left in a semi-transformed state.
     *
     * \returns transformed geometry
     */
    QgsGeometry transform( const QgsGeometry &geometry, bool &ok SIP_OUT, QgsFeedback *feedback = nullptr );

    /**
     * Returns the underlying GCP transformer used to transform geometries.
     *
     * \see setGcpTransformer()
     */
    QgsGcpTransformerInterface *gcpTransformer() const;

    /**
     * Sets the underlying GCP \a transformer used to transform geometries.
     *
     * Ownership is transferred to this object.
     *
     * \see gcpTransformer()
     */
    void setGcpTransformer( QgsGcpTransformerInterface *transformer SIP_TRANSFER );

  private:
#ifdef SIP_RUN
    QgsGcpGeometryTransformer( const QgsGcpGeometryTransformer &other );
#endif

    std::unique_ptr<QgsGcpTransformerInterface> mGcpTransformer;
};

#endif // QGSGCPGEOMETRYTRANSFORMER_H
