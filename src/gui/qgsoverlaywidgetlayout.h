/***************************************************************************
    qgsoverlaywidgetlayout.h
    ---------------------
    begin                : March 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOVERLAYWIDGETLAYOUT_H
#define QGSOVERLAYWIDGETLAYOUT_H

#include "qgis_gui.h"
#include "qgis_sip.h"

#include <QLayout>

/**
 * \ingroup gui
 * \brief A custom layout which can be used to overlay child widgets over a parent widget.
 *
 * \since QGIS 3.38
 */
class GUI_EXPORT QgsOverlayWidgetLayout : public QLayout
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsOverlayWidgetLayout, with the specified \a parent widget.
     */
    QgsOverlayWidgetLayout( QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsOverlayWidgetLayout() override;

    int count() const final;
    void addItem( QLayoutItem *item ) final;
    QLayoutItem *itemAt( int index ) const final;
    QLayoutItem *takeAt( int index ) final;
    QSize sizeHint() const final;
    QSize minimumSize() const final;
    void setGeometry( const QRect &rect ) final;

    /**
     * Adds a \a widget to the layout, which will be bound to the specified \a edge.
     *
     * \note Widgets on the left and right edges will always be positioned first, with
     * top and bottom edge widgets expanding to take the remaining horizontal space.
     */
    void addWidget( QWidget *widget SIP_TRANSFER, Qt::Edge edge );

    /**
     * Sets the spacing between widgets that are laid out side by side.
     *
     * \see horizontalSpacing()
     */
    void setHorizontalSpacing( int spacing );

    /**
     * Returns the spacing between widgets that are laid out side by side.
     *
     * \see setHorizontalSpacing()
     */
    int horizontalSpacing() const { return mHorizontalSpacing; }

    /**
     * Sets the spacing between widgets that are laid out on top of each other.
     *
     * \see verticalSpacing()
     */
    void setVerticalSpacing( int spacing );

    /**
     * Returns the spacing between widgets that are laid out on top of each other.
     *
     * \see setVerticalSpacing()
     */
    int verticalSpacing() const { return mVerticalSpacing; }

  private:

    QList< QLayoutItem *> mLeftItems;
    QList< QLayoutItem *> mRightItems;
    QList< QLayoutItem *> mTopItems;
    QList< QLayoutItem *> mBottomItems;
    int mHorizontalSpacing = 0;
    int mVerticalSpacing = 0;


};

#endif // QGSOVERLAYWIDGETLAYOUT_H
