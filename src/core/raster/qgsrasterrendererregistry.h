/***************************************************************************
                         qgsrasterrendererregistry.h
                         ---------------------------
    begin                : January 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERRENDERERREGISTRY_H
#define QGSRASTERRENDERERREGISTRY_H


#define SIP_NO_FILE



#include "qgis_core.h"
#include "qgis.h"
#include <QHash>
#include <QString>

class QDomElement;
class QgsRasterInterface;
class QgsRasterLayer;
class QgsRasterRenderer;
class QgsRasterRendererWidget;
class QgsRasterDataProvider;
class QgsRectangle;

typedef QgsRasterRenderer *( *QgsRasterRendererCreateFunc )( const QDomElement &, QgsRasterInterface *input );
typedef QgsRasterRendererWidget *( *QgsRasterRendererWidgetCreateFunc )( QgsRasterLayer *, const QgsRectangle &extent );

/**
 * \ingroup core
  * \brief Registry for raster renderer entries.
  */
struct CORE_EXPORT QgsRasterRendererRegistryEntry
{
  QgsRasterRendererRegistryEntry( const QString &name, const QString &visibleName, QgsRasterRendererCreateFunc rendererFunction,
                                  QgsRasterRendererWidgetCreateFunc widgetFunction );

  /**
   * Constructor for QgsRasterRendererRegistryEntry.
   */
  QgsRasterRendererRegistryEntry() = default;
  QString name;
  QString visibleName; //visible (and translatable) name
  QIcon icon();
  QgsRasterRendererCreateFunc rendererCreateFunction = nullptr ; //pointer to create function
  QgsRasterRendererWidgetCreateFunc widgetCreateFunction = nullptr ; //pointer to create function for renderer widget
};

/**
 * \ingroup core
  * \brief Registry for raster renderers.
  *
  * QgsRasterRendererRegistry is not usually directly created, but rather accessed through
  * QgsApplication::rasterRendererRegistry().
  *
  * \note not available in Python bindings
  */
class CORE_EXPORT QgsRasterRendererRegistry
{
  public:

    QgsRasterRendererRegistry();

    void insert( const QgsRasterRendererRegistryEntry &entry );
    void insertWidgetFunction( const QString &rendererName, QgsRasterRendererWidgetCreateFunc func );
    bool rendererData( const QString &rendererName, QgsRasterRendererRegistryEntry &data ) const;
    QStringList renderersList() const;
    QList< QgsRasterRendererRegistryEntry > entries() const;

    /**
     * Creates a default renderer for a raster drawing style (considering user options such as default contrast enhancement).
     * Caller takes ownership.
    */
    QgsRasterRenderer *defaultRendererForDrawingStyle( Qgis::RasterDrawingStyle drawingStyle, QgsRasterDataProvider *provider ) const;

  private:
    QHash< QString, QgsRasterRendererRegistryEntry > mEntries;
    QStringList mSortedEntries;

    //read min/max values from
    bool minMaxValuesForBand( int band, QgsRasterDataProvider *provider, double &minValue, double &maxValue ) const;
};

#endif // QGSRASTERRENDERERREGISTRY_H
