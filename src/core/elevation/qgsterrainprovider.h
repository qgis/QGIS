/***************************************************************************
                         qgsterrainprovider.h
                         ---------------
    begin                : February 2022
    copyright            : (C) 2022 by Nyall Dawson
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
#ifndef QGSTERRAINPROVIDER_H
#define QGSTERRAINPROVIDER_H

#include "qgis_core.h"
#include "qgsrange.h"
#include "qgsunittypes.h"
#include "qgsmaplayerref.h"
#include "qgsrasterlayer.h"
#include "qgsmeshlayer.h"

#include <QObject>

class QDomElement;
class QgsReadWriteContext;
class QDomDocument;

/**
 * \brief Abstract base class for terrain providers
 *
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsAbstractTerrainProvider
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( sipCpp->type().compare( QLatin1String( "flat" ) ) == 0 )
    {
      sipType = sipType_QgsFlatTerrainProvider;
    }
    else if ( sipCpp->type().compare( QLatin1String( "raster" ) ) == 0 )
    {
      sipType = sipType_QgsRasterDemTerrainProvider;
    }
    else if ( sipCpp->type().compare( QLatin1String( "mesh" ) ) == 0 )
    {
      sipType = sipType_QgsMeshTerrainProvider;
    }
    else
    {
      sipType = 0;
    }
    SIP_END
#endif

  public:

    virtual ~QgsAbstractTerrainProvider();

    /**
     * QgsAbstractTerrainProvider cannot be assigned.
     */
    QgsAbstractTerrainProvider &operator=( const QgsAbstractTerrainProvider &other ) = delete;

    /**
     * Resolves reference to layers from stored layer ID (if it has not been resolved already)
     */
    virtual void resolveReferences( const QgsProject *project );

    /**
     * Reads the terrain provider state from a DOM \a element.
     * \see writeXml()
     */
    virtual bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) = 0;

    /**
     * Returns a DOM element representing the state of the terrain provider.
     * \see readXml()
     */
    virtual QDomElement writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const = 0;

    /**
     * Returns the unique type ID string for the provider.
     */
    virtual QString type() const = 0;

    /**
     * Creates a clone of the provider and returns the new object.
     *
     * Ownership is transferred to the caller.
     */
    virtual QgsAbstractTerrainProvider *clone() const = 0 SIP_FACTORY;

    /**
     * Returns the native coordinate reference system of the terrain provider.
     */
    virtual QgsCoordinateReferenceSystem crs() const = 0;

    /**
     * Returns the height at the point (x,y) in the terrain provider's native crs().
     *
     * Returns NaN if the height could not be obtained at the specified point.
     */
    virtual double heightAt( double x, double y ) const = 0;

    /**
     * Returns the vertical scale factor, which can be used to exaggerate vertical heights.
     *
     * \see setScale()
     * \see offset()
    */
    double scale() const { return mScale; }

    /**
     * Sets the vertical \a scale factor, which can be used to exaggerate vertical heights.
     *
     * \see scale()
     * \see setOffset()
    */
    void setScale( double scale ) { mScale = scale; }

    /**
     * Returns the vertical offset value, used for adjusting the heights from the terrain provider.
     *
     * \see setOffset()
     * \see scale()
    */
    double offset() const { return mOffset; }

    /**
     * Returns the vertical \a offset value, used for adjusting the heights from the terrain provider
     *
     * \see offset()
     * \see setScale()
    */
    void setOffset( double offset ) { mOffset = offset; }

  protected:

    /**
     * Constructor for QgsAbstractTerrainProvider.
     */
    QgsAbstractTerrainProvider() = default;

    /**
     * Copy constructor
     */
    QgsAbstractTerrainProvider( const QgsAbstractTerrainProvider &other );

    /**
     * Writes common properties to a DOM \a element.
     */
    void writeCommonProperties( QDomElement &element, const QgsReadWriteContext &context ) const;

    /**
     * Reads common properties from a DOM \a element.
     */
    void readCommonProperties( const QDomElement &element, const QgsReadWriteContext &context );

    //! Scale factor
    double mScale = 1.0;

    //! Offset amount
    double mOffset = 0.0;

};


/**
 * \brief A terrain provider where the terrain is a simple flat surface.
 *
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsFlatTerrainProvider : public QgsAbstractTerrainProvider
{
  public:

    /**
     * Constructor for QgsFlatTerrainProvider.
     */
    QgsFlatTerrainProvider() = default;

    QString type() const override;
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    QDomElement writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const override;
    QgsCoordinateReferenceSystem crs() const override;
    double heightAt( double x, double y ) const override;
    QgsFlatTerrainProvider *clone() const override SIP_FACTORY;
};

/**
 * \brief A terrain provider where the terrain source is a raster DEM layer.
 *
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsRasterDemTerrainProvider : public QgsAbstractTerrainProvider
{
  public:

    /**
     * Constructor for QgsRasterDemTerrainProvider.
     */
    QgsRasterDemTerrainProvider() = default;

    QString type() const override;
    void resolveReferences( const QgsProject *project ) override;
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    QDomElement writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const override;
    QgsCoordinateReferenceSystem crs() const override;
    double heightAt( double x, double y ) const override;
    QgsRasterDemTerrainProvider *clone() const override SIP_FACTORY;

    /**
     * Sets the raster \a layer with elevation model to be used as the terrain source.
     *
     * \see layer()
     */
    void setLayer( QgsRasterLayer *layer );

    /**
     * Returns the raster layer with elevation model to be used as the terrain source.
     *
     * \see layer()
     */
    QgsRasterLayer *layer() const;


  private:

    _LayerRef<QgsRasterLayer> mRasterLayer;

};


/**
 * \brief A terrain provider that uses the Z values of a mesh layer to build a terrain surface.
 *
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsMeshTerrainProvider : public QgsAbstractTerrainProvider
{
  public:

    /**
     * Constructor for QgsMeshTerrainProvider.
     */
    QgsMeshTerrainProvider() = default;

    QString type() const override;
    void resolveReferences( const QgsProject *project ) override;
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    QDomElement writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const override;
    QgsCoordinateReferenceSystem crs() const override;
    double heightAt( double x, double y ) const override;
    QgsMeshTerrainProvider *clone() const override SIP_FACTORY;

    /**
     * Sets the mesh \a layer to be used as the terrain source.
     *
     * \see layer()
     */
    void setLayer( QgsMeshLayer *layer );

    /**
     * Returns the mesh layer to be used as the terrain source.
     *
     * \see layer()
     */
    QgsMeshLayer *layer() const;

  private:

    _LayerRef<QgsMeshLayer> mMeshLayer;

};

#endif // QGSTERRAINPROVIDER_H
