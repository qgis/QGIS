/***************************************************************************
    qgsopacitywidget.h
     -----------------
    Date                 : May 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall.dawson@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOPACITYWIDGET_H
#define QGSOPACITYWIDGET_H

#include <QWidget>
#include "qgis_sip.h"
#include "qgis_gui.h"

class QgsDoubleSpinBox;
class QSlider;

/**
 * \class QgsOpacityWidget
 * \ingroup gui
 * \brief A widget for setting an opacity value.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsOpacityWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( double opacity READ opacity WRITE setOpacity NOTIFY opacityChanged )

  public:

    /**
     * Constructor for QgsOpacityWidget.
     */
    explicit QgsOpacityWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the current opacity selected in the widget, where opacity ranges from 0.0 (transparent)
     * to 1.0 (opaque).
     * \see setOpacity()
     * \see opacityChanged()
     */
    double opacity() const;

  public slots:

    /**
     * Sets the current \a opacity to show in the widget, where \a opacity ranges from 0.0 (transparent)
     * to 1.0 (opaque).
     * \see opacity()
     * \see opacityChanged()
     */
    void setOpacity( double opacity );

  signals:

    /**
     * Emitted when the \a opacity is changed in the widget, where \a opacity ranges from 0.0 (transparent)
     * to 1.0 (opaque).
     * \see setOpacity()
     * \see opacity()
     */
    void opacityChanged( double opacity );

  private slots:

    void spinChanged( double value );

  private:

    QgsDoubleSpinBox *mSpinBox = nullptr;
    QSlider *mSlider = nullptr;

};

#endif // QGSOPACITYWIDGET_H
