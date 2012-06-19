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

#include "qgsrasterlayer.h" //for DrawingStyle enum
#include <QHash>
#include <QString>

class QDomElement;
//class QgsRasterDataProvider;
class QgsRasterInterface;
class QgsRasterLayer;
class QgsRasterRenderer;
class QgsRasterRendererWidget;

typedef QgsRasterRenderer*( *QgsRasterRendererCreateFunc )( const QDomElement&, QgsRasterInterface* input );
typedef QgsRasterRendererWidget*( *QgsRasterRendererWidgetCreateFunc )( QgsRasterLayer* );

struct CORE_EXPORT QgsRasterRendererRegistryEntry
{
  QgsRasterRendererRegistryEntry( const QString& theName, const QString& theVisibleName, QgsRasterRendererCreateFunc rendererFunction,
                                  QgsRasterRendererWidgetCreateFunc widgetFunction );
  QgsRasterRendererRegistryEntry();
  QString name;
  QString visibleName; //visible (and translatable) name
  QgsRasterRendererCreateFunc rendererCreateFunction; //pointer to create function
  QgsRasterRendererWidgetCreateFunc widgetCreateFunction; //pointer to create function for renderer widget
};

class CORE_EXPORT QgsRasterRendererRegistry
{
  public:
    static QgsRasterRendererRegistry* instance();
    ~QgsRasterRendererRegistry();

    void insert( QgsRasterRendererRegistryEntry entry );
    void insertWidgetFunction( const QString& rendererName, QgsRasterRendererWidgetCreateFunc func );
    bool rendererData( const QString& rendererName, QgsRasterRendererRegistryEntry& data ) const;
    QStringList renderersList() const;
    QList< QgsRasterRendererRegistryEntry > entries() const;

    /**Creates a default renderer for a raster drawing style (considering user options such as default contrast enhancement).
        Caller takes ownership*/
    QgsRasterRenderer* defaultRendererForDrawingStyle( const QgsRasterLayer::DrawingStyle&  theDrawingStyle, QgsRasterDataProvider* provider ) const;

  protected:
    QgsRasterRendererRegistry();

  private:
    static QgsRasterRendererRegistry* mInstance;
    QHash< QString, QgsRasterRendererRegistryEntry > mEntries;

    //read min/max values from
    bool minMaxValuesForBand( int band, QgsRasterDataProvider* provider, double& minValue, double& maxValue ) const;
    static int contrastEnhancementFromString( const QString& contrastEnhancementString );
};

#endif // QGSRASTERRENDERERREGISTRY_H
