#ifndef QGSATTRIBUTESFORMTREEVIEWITEMDELEGATE_H
#define QGSATTRIBUTESFORMTREEVIEWITEMDELEGATE_H

#include "qgsattributesformview.h"
#include "qgsproxystyle.h"

#include <QStyledItemDelegate>


/**
 * Proxy style for field items with indicators
 */
class QgsAttributesFormTreeViewProxyStyle : public QgsProxyStyle
{
    Q_OBJECT

  public:
    explicit QgsAttributesFormTreeViewProxyStyle( QgsAttributesFormBaseView *treeView );

    QRect subElementRect( SubElement element, const QStyleOption *option, const QWidget *widget ) const override;

    static const auto SE_AttributesFormTreeItemIndicator = SE_CustomBase + 1;

  private:
    QgsAttributesFormBaseView *mAttributesFormTreeView;
};


/**
 * Item delegate that adds drawing of indicators
 */
class QgsAttributesFormTreeViewItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
  public:
    explicit QgsAttributesFormTreeViewItemDelegate( QgsAttributesFormBaseView *parent );

    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

    bool helpEvent( QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index ) override;

  private:
    QgsAttributesFormBaseView *mAttributesFormTreeView;
};

#endif // QGSATTRIBUTESFORMTREEVIEWITEMDELEGATE_H
