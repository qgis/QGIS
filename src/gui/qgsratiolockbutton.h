/***************************************************************************
    qgsratiolockbutton.cpp - Lock button
     --------------------------------------
    Date                 : July, 2017
    Copyright            : (C) 2017 by Mathieu Pellerin
    Email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLOCKBUTTON_H
#define QGSLOCKBUTTON_H

#include "qgsratiolockbutton.h"

#include <QToolButton>
#include "qgis_gui.h"
#include "qgis.h"

/** \ingroup gui
 * \class QgsRatioLockButton
 * A cross platform button subclass used to represent a locked / unlocked ratio state.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsRatioLockButton : public QToolButton
{
    Q_OBJECT
    Q_PROPERTY( bool locked READ locked WRITE setLocked )

  public:

    /** Construct a new ratio lock button.
     * Use \a parent to attach a parent QWidget to the button.
     */
    QgsRatioLockButton( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /** Sets whether the button state is locked.
     * \param locked locked state
     * \see locked
     */
    void setLocked( const bool locked );

    /** Returns whether the button state is locked.
     * \returns true if the button state is locked.
     * \see setLocked
     */
    bool locked() const { return mLocked; }

  signals:

    /** Emitted whenever the lock state changes.
     */
    void lockChanged( const bool locked );

  protected:

    void changeEvent( QEvent *e ) override;
    void showEvent( QShowEvent *e ) override;
    void resizeEvent( QResizeEvent *event ) override;

  private:

    void drawButton();

    bool mLocked = false;

  private slots:

    void buttonClicked();

};

#endif
