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
#include "qgis_sip.h"

#include <QPointer>
class QDoubleSpinBox;

/**
 * \ingroup gui
 * \class QgsRatioLockButton
 * A cross platform button subclass used to represent a locked / unlocked ratio state.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsRatioLockButton : public QToolButton
{
    Q_OBJECT
    Q_PROPERTY( bool locked READ locked WRITE setLocked )

  public:

    /**
     * Construct a new ratio lock button.
     * Use \a parent to attach a parent QWidget to the button.
     */
    QgsRatioLockButton( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets whether the button state is locked.
     * \param locked locked state
     * \see locked
     */
    void setLocked( bool locked );

    /**
     * Returns whether the button state is locked.
     * \returns true if the button state is locked.
     * \see setLocked
     */
    bool locked() const { return mLocked; }

    /**
     * Registers a spin box \a widget as the linked "width" spin box.
     *
     * If both a width and height spin box are linked to the button, they will automatically
     * have their values updates when if the other spin box value is changed. I.e. changing the
     * width spin box will automatically update the height spin box to a value which keeps the
     * same locked ratio.
     *
     * \see setHeightSpinBox()
     */
    void setWidthSpinBox( QDoubleSpinBox *widget );

    /**
     * Registers a spin box \a widget as the linked "height" spin box.
     *
     * If both a width and height spin box are linked to the button, they will automatically
     * have their values updates when if the other spin box value is changed. I.e. changing the
     * width spin box will automatically update the height spin box to a value which keeps the
     * same locked ratio.
     *
     * \see setWidthSpinBox()
     */
    void setHeightSpinBox( QDoubleSpinBox *widget );

    /**
     * Resets the current width/height ratio, taking the width and height
     * from the current values of the width and height spin boxes.
     */
    void resetRatio();

  signals:

    /**
     * Emitted whenever the lock state changes.
     */
    void lockChanged( bool locked );

  protected:

    void changeEvent( QEvent *e ) override;
    void showEvent( QShowEvent *e ) override;
    void resizeEvent( QResizeEvent *event ) override;

  private:

    void drawButton();

    bool mLocked = false;

    QPointer< QDoubleSpinBox > mWidthSpinBox;
    double mPrevWidth = 0;
    QPointer< QDoubleSpinBox > mHeightSpinBox;
    double mPrevHeight = 0;
    bool mUpdatingRatio = false;

  private slots:

    void buttonClicked();

    void widthSpinBoxChanged( double value );
    void heightSpinBoxChanged( double value );

};

#endif
