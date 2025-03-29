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

class QgsLayerTreeView;

#include "qgsproxystyle.h"
#include <QStyledItemDelegate>

#ifndef SIP_RUN
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

#endif


/**
 * \ingroup gui
 * \class QgsLayerTreeViewItemDelegate
 * \brief Item delegate that adds drawing of layer status indicators to the
 * Layers panel (legend).
 *
 * Plug-ins that customize the drawing of the legend entries should use this
 * class as their base class (rather than QStyledItemDelegate); this ensures
 * that the status icons are displayed.
 *
 * (Note that to add status icons to a legend layer one should use not subclass
 * this class but rather use QgsLayerTreeView.addIndicator.)
 *
 * This class was not exported in earlier versions of QGIS.  To ensure backwards
 * compatibility, plug-in code should test whether QgsLayerTreeViewItemDelegate
 * is available and if it is not should fall back to using QStyledItemDelegate
 * as the base class.  This can be done as follows:
 *
 * Import the best available class
 * -------------------------------
 *
 *     try:
 *         from qgis.gui import QgsLayerTreeViewItemDelegate
 *         has_QgsLayerTreeViewItemDelegate = True
 *     except:
 *         # QgsLayerTreeViewItemDelegate isn't exported
 *         # so fall back to using QStyledItemDelegate
 *         from PyQt5.QtWidgets import QStyledItemDelegate
 *         has_QgsLayerTreeViewItemDelegate = False
 *
 * Access the class
 * ----------------
 *
 * The best way to access the imported class depends on the context.  Here are
 * some examples of how it can be done in a backwards-compatible manner:
 *
 *  **Saving the existing delegate**
 *
 *     if has_QgsLayerTreeViewItemDelegate:
 *         original_delegate = QgsLayerTreeViewItemDelegate( self.iface.layerTreeView() )
 *     else:
 *         original_delegate = QStyledItemDelegate( self.iface.layerTreeView() )
 *
 * **Passing a reference to the delegate class**
 *
 *     if has_QgsLayerTreeViewItemDelegate:
 *         delegateClass = QgsLayerTreeViewItemDelegate
 *     else:
 *         delegateClass = QStyledItemDelegate
 *
 *     class CusomLayerItemDelegate( delegateClass ):
 *         # …
 */
class GUI_EXPORT QgsLayerTreeViewItemDelegate : public QStyledItemDelegate
{
#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( sipCpp->inherits( "QgsLayerTreeViewItemDelegate" ) )
      sipType = QgsLayerTreeViewItemDelegate;
    else
      sipType = 0;
    SIP_END
#endif

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


#endif // QGSLAYERTREEVIEWITEMDELEGATE_H
