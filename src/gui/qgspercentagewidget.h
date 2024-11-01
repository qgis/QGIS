/***************************************************************************
    qgspercentagewidget.h
     -----------------
    Date                 : January 2024
    Copyright            : (C) 2024 Nyall Dawson
    Email                : nyall.dawson@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPERCENTAGEWIDGET_H
#define QGSPERCENTAGEWIDGET_H

#include <QWidget>
#include "qgis_sip.h"
#include "qgis_gui.h"

class QgsDoubleSpinBox;
class QSlider;

/**
 * \class QgsPercentageWidget
 * \ingroup gui
 * \brief A widget for setting a percentage value.
 * \since QGIS 3.36
 */
class GUI_EXPORT QgsPercentageWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( double value READ value WRITE setValue NOTIFY valueChanged )

  public:
    /**
     * Constructor for QgsPercentageWidget.
     */
    explicit QgsPercentageWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the current percentage selected in the widget, as a factor from 0.0 to 1.0.
     * \see setValue()
     * \see valueChanged()
     */
    double value() const;

  public slots:

    /**
     * Sets the current \a value to show in the widget, where \a value is a factor which ranges from 0.0 to 1.0.
     *
     * \see value()
     * \see valueChanged()
     */
    void setValue( double value );

  signals:

    /**
     * Emitted when the \a value is changed in the widget, where \a value is a factor which ranges from 0.0 to 1.0.
     *
     * \see setValue()
     * \see value()
     */
    void valueChanged( double value );

  private slots:

    void spinChanged( double value );

  private:
    QgsDoubleSpinBox *mSpinBox = nullptr;
    QSlider *mSlider = nullptr;
};

#endif // QGSPERCENTAGEWIDGET_H
