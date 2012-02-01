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

class QgsRasterDataProvider;
class QgsRasterRenderer;

typedef QgsRasterRenderer*(*QgsRasterRendererCreateFunc)(const QDomElement&);

struct QgsRasterRendererRegistryEntry
{
  QString name;
  QgsRasterRendererCreateFunc rendererCreateFunction; //pointer to create function
  //pointer to create function for renderer widget
};

class QgsRasterRendererRegistry
{
  public:
    static QgsRasterRendererRegistry* instance();
    ~QgsRasterRendererRegistry();

    void insert( QgsRasterRendererRegistryEntry entry );
    bool rendererData( const QString& rendererName, QgsRasterRendererRegistryEntry& data ) const;

  protected:
    QgsRasterRendererRegistry();

  private:
    static QgsRasterRendererRegistry* mInstance;
    QHash< QString, QgsRasterRendererRegistryEntry > mEntries;
};

#endif // QGSRASTERRENDERERREGISTRY_H
