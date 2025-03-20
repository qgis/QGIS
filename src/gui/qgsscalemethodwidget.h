/***************************************************************************
                              qgsscalemethodwidget.h
                              ------------------------
  begin                : March 2025
  copyright            : (C) 2025 by Nyall Dawson
  email                : nyall.dawson@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSCALEMETHODWIDGET_H
#define QGSSCALEMETHODWIDGET_H

#include <QComboBox>
#include "qgis_sip.h"
#include <QPainter> // For QPainter::CompositionMode enum
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \brief A widget which which lets the user select from scale calculation methods.
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsScaleMethodWidget : public QWidget
{
    Q_OBJECT
  public:
    //! Constructor for QgsScaleMethodWidget, with the specified \a parent widget
    QgsScaleMethodWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the selected scale method.
     * \see setBlendMode()
     */
    Qgis::ScaleCalculationMethod scaleMethod() const;

    /**
     * Sets the selected blend mode.
     * \see blendMode()
     */
    void setScaleMethod( Qgis::ScaleCalculationMethod method );

  signals:

    /**
     * Emitted when the selected method is changed.
     */
    void methodChanged();

  private:
    QComboBox *mCombo = nullptr;
};

#endif // QGSSCALEMETHODWIDGET_H
