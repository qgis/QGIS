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

#include <QHash>
#include <QString>

class QDomElement;
class QgsRasterDataProvider;
class QgsRasterLayer;
class QgsRasterRenderer;
class QgsRasterRendererWidget;

typedef QgsRasterRenderer*( *QgsRasterRendererCreateFunc )( const QDomElement&, QgsRasterDataProvider* provider );
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

  protected:
    QgsRasterRendererRegistry();

  private:
    static QgsRasterRendererRegistry* mInstance;
    QHash< QString, QgsRasterRendererRegistryEntry > mEntries;
};

#endif // QGSRASTERRENDERERREGISTRY_H
