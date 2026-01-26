/***************************************************************************

               ----------------------------------------------------
              date                 : 22.5.2019
              copyright            : (C) 2019 by Matthias Kuhn
              email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROJECTLISTITEMDELEGATE_H
#define QGSPROJECTLISTITEMDELEGATE_H

#include <QStyledItemDelegate>

class QgsProjectPreviewImage
{
  public:
    QgsProjectPreviewImage();
    QgsProjectPreviewImage( const QString &path );
    QgsProjectPreviewImage( const QImage &image );

    void loadImageFromFile( const QString &path );
    void setImage( const QImage &image );
    QPixmap pixmap() const;

    bool isNull() const;

  private:
    QImage mImage;
};

class QgsProjectListItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    enum Role
    {
      TitleRole = Qt::UserRole + 1,
      PathRole = Qt::UserRole + 2,
      NativePathRole = Qt::UserRole + 3,
      CrsRole = Qt::UserRole + 4,
      PinRole = Qt::UserRole + 5,
      AnonymisedNativePathRole = Qt::UserRole + 6
    };

    explicit QgsProjectListItemDelegate( QObject *parent = nullptr );
    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

    bool showPath() const;
    void setShowPath( bool value );

  private:
    int mRoundedRectSizePixels = 5;
    bool mShowPath = true;
    QColor mColor = Qt::white;
};

class QgsNewsItemListItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    explicit QgsNewsItemListItemDelegate( QObject *parent = nullptr );
    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

    /**
     * Returns the area corresponding to the dismiss rectangle.
     */
    QRect dismissRect() const { return mDismissRect; }

    QSize dismissRectSize() const { return mDismissRectSize; }

  private:
    int mRoundedRectSizePixels = 5;
    QColor mColor = Qt::white;
    mutable QRect mDismissRect;
    QSize mDismissRectSize;
};

#endif // QGSPROJECTLISTITEMDELEGATE_H
