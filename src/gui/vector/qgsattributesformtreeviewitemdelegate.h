/***************************************************************************
    qgsattributesformtreeviewitemdelegate.h
    ---------------------
    begin                : June 2025
    copyright            : (C) 2025 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTESFORMTREEVIEWITEMDELEGATE_H
#define QGSATTRIBUTESFORMTREEVIEWITEMDELEGATE_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

/// @cond PRIVATE

#include "qgsproxystyle.h"

class QgsAttributesFormTreeViewIndicator;
class QgsAttributesFormBaseView;

#include <QStyledItemDelegate>

/**
 * Proxy style for field items with indicators.
 *
 * \ingroup gui
 * \since QGIS 4.0
 */
class QgsAttributesFormTreeViewProxyStyle : public QgsProxyStyle
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsAttributesFormTreeViewProxyStyle. Ownership is transferred to the given tree view.
     */
    explicit QgsAttributesFormTreeViewProxyStyle( QgsAttributesFormBaseView *treeView );

    QRect subElementRect( SubElement element, const QStyleOption *option, const QWidget *widget ) const override;

    static const auto SE_AttributesFormTreeItemIndicator = SE_CustomBase + 1;

  private:
    QgsAttributesFormBaseView *mAttributesFormTreeView;
};


/**
 * Item delegate that adds drawing of indicators.
 *
 * \ingroup gui
 * \since QGIS 4.0
 */
class QgsAttributesFormTreeViewItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsAttributesFormTreeViewItemDelegate. Ownership is transferred to the given tree view.
     */
    explicit QgsAttributesFormTreeViewItemDelegate( QgsAttributesFormBaseView *parent );

    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

    bool helpEvent( QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index ) override;

  private:
    QgsAttributesFormBaseView *mAttributesFormTreeView;
};

/// @endcond

#endif // QGSATTRIBUTESFORMTREEVIEWITEMDELEGATE_H
