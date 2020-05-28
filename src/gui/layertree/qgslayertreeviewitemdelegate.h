/***************************************************************************
  qgslayertreeviewitemdelegate.h
  --------------------------------------
  Date                 : January 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREEVIEWITEMDELEGATE_H
#define QGSLAYERTREEVIEWITEMDELEGATE_H

#include "qgis_sip.h"

SIP_NO_FILE

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

class QgsLayerTreeView;

#include "qgsproxystyle.h"
#include <QStyledItemDelegate>

/**
 * Proxy style for layer items with indicators
 */
class QgsLayerTreeViewProxyStyle : public QgsProxyStyle
{
    Q_OBJECT

  public:
    explicit QgsLayerTreeViewProxyStyle( QgsLayerTreeView *treeView );

    QRect subElementRect( SubElement element, const QStyleOption *option, const QWidget *widget ) const override;

    static const auto SE_LayerTreeItemIndicator = SE_CustomBase + 1;

  private:
    QgsLayerTreeView *mLayerTreeView;
};


/**
 * Item delegate that adds drawing of indicators
 */
class QgsLayerTreeViewItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
  public:
    explicit QgsLayerTreeViewItemDelegate( QgsLayerTreeView *parent );

    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

    bool helpEvent( QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index ) override;

  private slots:
    void onClicked( const QModelIndex &index );

  private:
    QgsLayerTreeView *mLayerTreeView;
};

/// @endcond

#endif // QGSLAYERTREEVIEWITEMDELEGATE_H
