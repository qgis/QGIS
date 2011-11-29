/***************************************************************************
                          qgslayerorder.h  -  description
                             -------------------
    begin                : 2011-11-09
    copyright            : (C) 2011 by Juergen E. Fischer, norBIT GmbH
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERORDER_H
#define QGSLAYERORDER_H

#include <QListWidget>

class QWidget;
class QgsLegend;
class QgsMapLayer;
class QTreeWidgetItem;

class QgsLayerOrder : public QListWidget
{
    Q_OBJECT
  public:
    /*! Constructor.
    * @param legend link to legend
    * @param canvas link to canvas
    * @param parent An optional parent widget
    * @param name An optional name for the widget
    */
    QgsLayerOrder( QgsLegend *legend, QWidget * parent = 0, const char *name = 0 );

    //! Destructor
    ~QgsLayerOrder();

  protected:
    void mouseMoveEvent( QMouseEvent * e );
    void mousePressEvent( QMouseEvent * e );
    void mouseReleaseEvent( QMouseEvent * e );

  private slots:
    void updateDrawingOrderChecked( bool );
    void itemChanged( QListWidgetItem * );
    void legendItemChanged( QTreeWidgetItem *, int );
    void refreshLayerList();

  private:
    QListWidgetItem *layerItem( QgsMapLayer * ) const;
    void updateLayerOrder();
    void hideLine();

    QgsLegend *mLegend;
    QListWidgetItem *mPressItem;
    QList<QListWidgetItem*> mItemsBeingMoved;

    QWidget *mInsertionLine;
    int mInsertRow;
};
#endif
