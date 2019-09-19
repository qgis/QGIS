/***************************************************************************
     qgsdetaileditemdelegate.h  -  A rich QItemDelegate subclass
                             -------------------
    begin                : Sat May 17 2008
    copyright            : (C) 2008 Tim Sutton
    email                : tim@linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDETAILEDITEMDELEGATE_H
#define QGSDETAILEDITEMDELEGATE_H

#include <QAbstractItemDelegate>
#include "qgis_sip.h"
#include <QString>
#include "qgis_gui.h"

class QCheckBox;
class QgsDetailedItemWidget;
class QgsDetailedItemData;
class QFontMetrics;
class QFont;

/**
 * \ingroup gui
 * A custom model/view delegate that can display an icon, heading
 * and detail sections.
 * \see also QgsDetailedItemData
 */
class GUI_EXPORT QgsDetailedItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
  public:

    //! Constructor for QgsDetailedItemDelegate
    QgsDetailedItemDelegate( QObject *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsDetailedItemDelegate() override;
    //! Reimplement for parent class
    void paint( QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index ) const override;
    //! Reimplement for parent class
    QSize sizeHint( const QStyleOptionViewItem &option,
                    const QModelIndex &index ) const override;

    void setVerticalSpacing( int value );

    int verticalSpacing() const;

    void setHorizontalSpacing( int value );

    int horizontalSpacing() const;

  private:
    QFont detailFont( const QStyleOptionViewItem &option ) const;
    QFont categoryFont( const QStyleOptionViewItem &option ) const;
    QFont titleFont( const QStyleOptionViewItem &option ) const;
    void drawHighlight( const QStyleOptionViewItem &option,
                        QPainter *thepPainter,
                        int height ) const;

    QStringList wordWrap( const QString &string,
                          const QFontMetrics &metrics,
                          int width ) const;
    void paintManually( QPainter *painter,
                        const QStyleOptionViewItem &option,
                        const QgsDetailedItemData &data ) const;
    void paintAsWidget( QPainter *painter,
                        const QStyleOptionViewItem &option,
                        const QgsDetailedItemData &data ) const;
    int height( const QStyleOptionViewItem &option,
                const QgsDetailedItemData &data ) const;
    QgsDetailedItemWidget *mpWidget = nullptr;
    QCheckBox *mpCheckBox = nullptr;
    int mVerticalSpacing;
    int mHorizontalSpacing;
};

#endif //QGSDETAILEDITEMDELEGATE_H
