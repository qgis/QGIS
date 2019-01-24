/***************************************************************************
    qgsscrollarea.h
    ---------------
    begin                : March 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSCROLLAREA_H
#define QGSSCROLLAREA_H

#include <QScrollArea>
#include "qgis_sip.h"
#include "qgis_gui.h"
#include <QTimer>
class ScrollAreaFilter;

/**
 * \class QgsScrollArea
 * \ingroup gui
 * A QScrollArea subclass with improved scrolling behavior.
 *
 * QgsScrollArea should be used instead of QScrollArea widgets.
 * In most cases the use is identical, however QgsScrollArea
 * has extra logic to avoid wheel events changing child widget
 * values when the mouse cursor is temporarily located over
 * a child widget during a scroll event.
 *
 * All QGIS code and plugins should use QgsScrollArea in place
 * of QScrollArea.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsScrollArea : public QScrollArea
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsScrollArea.
     */
    explicit QgsScrollArea( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Should be called when a scroll occurs on with the
     * QScrollArea itself or its child viewport().
     */
    void scrollOccurred();

    /**
     * Returns true if a scroll recently occurred within
     * the QScrollArea or its child viewport()
     */
    bool hasScrolled() const;

  protected:
    void wheelEvent( QWheelEvent *event ) override;

  private:
    QTimer mTimer;
    ScrollAreaFilter *mFilter = nullptr;
};

#ifndef SIP_RUN

///@cond PRIVATE

/**
 * \class ScrollAreaFilter
 * Swallows wheel events for QScrollArea children for a short period
 * following a scroll.
 */
class ScrollAreaFilter : public QObject
{
    Q_OBJECT
  public:

    ScrollAreaFilter( QgsScrollArea *parent = nullptr,
                      QWidget *viewPort = nullptr );

  protected:
    bool eventFilter( QObject *obj, QEvent *event ) override;

  private:
    QgsScrollArea *mScrollAreaWidget = nullptr;
    QWidget *mViewPort = nullptr;

    void addChild( QObject *child );
    void removeChild( QObject *child );

};

///@endcond PRIVATE

#endif

#endif // QGSSCROLLAREA_H
