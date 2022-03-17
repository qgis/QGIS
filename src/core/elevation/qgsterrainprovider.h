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
     * Returns the native coordinate reference system of the terrain provider.
     */
    virtual QgsCoordinateReferenceSystem crs() const = 0;

    /**
     * Returns the height at the point (x,y) in the terrain provider's native crs().
     */
    virtual double heightAt( double x, double y ) const = 0;

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
    QString type() const override;
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    QDomElement writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const override;
    QgsCoordinateReferenceSystem crs() const override;
    double heightAt( double x, double y ) const override;
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

    QString type() const override;
    void resolveReferences( const QgsProject *project ) override;
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    QDomElement writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const override;
    QgsCoordinateReferenceSystem crs() const override;
    double heightAt( double x, double y ) const override;

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

    QString type() const override;
    void resolveReferences( const QgsProject *project ) override;
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    QDomElement writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const override;
    QgsCoordinateReferenceSystem crs() const override;
    double heightAt( double x, double y ) const override;

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
