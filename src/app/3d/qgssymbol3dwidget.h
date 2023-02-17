/***************************************************************************
  qgssymbol3dwidget.h
  --------------------------------------
  Date                 : January 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSYMBOL3DWIDGET_H
#define QGSSYMBOL3DWIDGET_H

#include <QWidget>
#include <memory>
#include "qgsstyle.h"

class QLabel;
class QStackedWidget;

class QgsAbstract3DSymbol;
class QgsLine3DSymbolWidget;
class QgsPoint3DSymbolWidget;
class QgsPolygon3DSymbolWidget;
class QgsStyleItemsListWidget;

class QgsVectorLayer;

/**
 * Widget for selection of 3D symbol
 */
class QgsSymbol3DWidget : public QWidget
{
    Q_OBJECT

  public:
    QgsSymbol3DWidget( QgsVectorLayer *layer, QWidget *parent = nullptr );

    //! Returns a new symbol instance or NULLPTR
    std::unique_ptr< QgsAbstract3DSymbol > symbol();

    //! Sets symbol (does not take ownership)
    void setSymbol( const QgsAbstract3DSymbol *symbol, QgsVectorLayer *vlayer );

  signals:
    void widgetChanged();

  private slots:

    void setSymbolFromStyle( const QString &name, QgsStyle::StyleEntity entity, const QString &stylePath );
    void saveSymbol();

  private:

    void updateSymbolWidget( const QgsAbstract3DSymbol *newSymbol );

    QStackedWidget *widgetStack = nullptr;
    QgsLine3DSymbolWidget *widgetLine = nullptr;
    QgsPoint3DSymbolWidget *widgetPoint = nullptr;
    QgsPolygon3DSymbolWidget *widgetPolygon = nullptr;
    QLabel *widgetUnsupported = nullptr;

    QgsStyleItemsListWidget *mStyleWidget = nullptr;

    QgsVectorLayer *mLayer = nullptr;

};


#endif // QGSSYMBOL3DWIDGET_H
