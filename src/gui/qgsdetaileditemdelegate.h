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
#include <QString>

class QCheckBox;
class QgsDetailedItemWidget;
class QgsDetailedItemData;
class QFontMetrics;
class QFont;

/** \ingroup gui
 * A custom model/view delegate that can display an icon, heading
 * and detail sections.
 * @see also QgsDetailedItemData
 */
class GUI_EXPORT QgsDetailedItemDelegate :
      public QAbstractItemDelegate
{
    Q_OBJECT
  public:
    QgsDetailedItemDelegate( QObject * parent = 0 );
    ~QgsDetailedItemDelegate();
    /** Reimplement for parent class */
    void paint( QPainter * thePainter,
                const QStyleOptionViewItem & theOption,
                const QModelIndex & theIndex ) const override;
    /** Reimplement for parent class */
    QSize sizeHint( const QStyleOptionViewItem & theOption,
                    const QModelIndex & theIndex ) const override;

    void setVerticalSpacing( int theValue );

    int verticalSpacing() const;

    void setHorizontalSpacing( int theValue );

    int horizontalSpacing() const;

  private:
    QFont detailFont( const QStyleOptionViewItem &theOption ) const;
    QFont categoryFont( const QStyleOptionViewItem &theOption ) const;
    QFont titleFont( const QStyleOptionViewItem &theOption ) const;
    void drawHighlight( const QStyleOptionViewItem &theOption,
                        QPainter * thepPainter,
                        int theHeight ) const;

    QStringList wordWrap( QString theString,
                          QFontMetrics theMetrics,
                          int theWidth ) const;
    void paintManually( QPainter *thePainter,
                        const QStyleOptionViewItem &theOption,
                        const QgsDetailedItemData &theData ) const;
    void paintAsWidget( QPainter *thePainter,
                        const QStyleOptionViewItem &theOption,
                        const QgsDetailedItemData &theData ) const;
    int height( const QStyleOptionViewItem &theOption,
                const QgsDetailedItemData &theData ) const;
    QgsDetailedItemWidget * mpWidget;
    QCheckBox * mpCheckBox;
    int mVerticalSpacing;
    int mHorizontalSpacing;
};

#endif //QGSDETAILEDITEMDELEGATE_H
