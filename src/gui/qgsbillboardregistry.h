/***************************************************************************
                              qgsbillboardregistry.h
                              ----------------------
  begin                : February 2016
  copyright            : (C) 2016 by Sandro Mani
  email                : smani@sourcepole.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSBILLBOARDREGISTRY_H
#define QGSBILLBOARDREGISTRY_H

#include <QObject>
#include <QImage>
#include "qgspoint.h"

class QgsMapCanvasItem;

class GUI_EXPORT QgsBillBoardItem
{
  public:
    QImage image;
    QgsPoint worldPos;
    QString layerId;
};

class GUI_EXPORT QgsBillBoardRegistry : public QObject
{
    Q_OBJECT
  public:
    static QgsBillBoardRegistry* instance();
    void addItem( void* parent, const QImage& image, const QgsPoint& worldPos, const QString& layerId = QString() );
    void removeItem( void* parent );
    QList<QgsBillBoardItem*> items() const;

  signals:
    void itemAdded( QgsBillBoardItem* item );
    void itemRemoved( QgsBillBoardItem* item );

  private:
    QgsBillBoardRegistry( QObject* parent = 0 ) : QObject( parent ) {}

    QMap<void*, QgsBillBoardItem*> mItems;
};

#endif // QGSBILLBOARDREGISTRY_H
