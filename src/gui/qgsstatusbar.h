/***************************************************************************
                         qgsstatusbar.h
                         --------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTATUSBAR_H
#define QGSSTATUSBAR_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include <QWidget>

class QHBoxLayout;
class QLineEdit;

/**
 * \class QgsStatusBar
 * \ingroup gui
 * A proxy widget for QStatusBar.
 *
 * Unlike QStatusBar, QgsStatusBar allows finer control of widget placement, including
 * the option to locate permanent widgets on the left side of the bar.
 *
 * QgsStatusBar is designed to be embedded into an existing
 * window's QStatusBar, as a permanent widget. This allows reuse of the special QStatusBar handling
 * for resize grips and other platform specific status bar tweaks.
 *
 * Instead of adding child widgets and showing messages directly in the window's status bar,
 * these widgets (and messages) should instead be added into the embedded QgsStatusBar.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsStatusBar : public QWidget
{
    Q_OBJECT

  public:

    //! Placement anchor for widgets
    enum Anchor
    {
      AnchorLeft = 0, //!< Anchor widget to left of status bar
      AnchorRight, //!< Anchor widget to right of status bar
    };

    /**
     * Constructor for QgsStatusBar.
     */
    QgsStatusBar( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Adds the given \a widget permanently to this status bar, reparenting the widget if it isn't already a child
     * of this object.
     *
     * The \a stretch parameter is used to compute a suitable size for the given widget as the status bar
     * grows and shrinks. The default stretch factor is 0, i.e giving the widget a minimum of space.
     *
     * The \a anchor parameter controls which side of the status bar the widget should be anchored to.
     */
    void addPermanentWidget( QWidget *widget SIP_TRANSFER, int stretch = 0, Anchor anchor = AnchorRight );

    /**
     * Removes a \a widget from the status bar. Ownership of the widget remains unchanged, and the
     * widget itself is not deleted.
     */
    void removeWidget( QWidget *widget );

    /**
     * Returns the current message shown in the status bar.
     * \see showMessage()
     */
    QString currentMessage() const;

  public slots:

    /**
     * Displays the given \a message for the specified number of milli-seconds (\a timeout).
     * If \a timeout is 0 (default), the message remains displayed until the clearMessage()
     * slot is called or until the showMessage() slot is called again to change the message.
     * \see clearMessage()
     * \see currentMessage()
     */
    void showMessage( const QString &message, int timeout = 0 );

    /**
     * Removes any temporary message being shown.
     * \see showMessage()
     */
    void clearMessage();


  protected:

    void changeEvent( QEvent *event ) override;

  private:

    QHBoxLayout *mLayout = nullptr;
    QLineEdit *mLineEdit = nullptr;
    QTimer *mTempMessageTimer = nullptr;

};

#endif // QGSSTATUSBAR_H


