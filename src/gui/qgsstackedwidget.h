/***************************************************************************
    qgsstackedwidget.h
    ------------------
    begin                : January 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTACKEDWIDGET_H
#define QGSSTACKEDWIDGET_H

#define SIP_NO_FILE

#include <QStackedWidget>
#include "qgis_gui.h"

class QSize;

/**
 * \class QgsStackedWidget
 * \ingroup gui
 * \brief A QStackedWidget that can be shrunk to its current widget's size.
 *
 * A regular QStackedWidget can be shrunk down the size of its largest page widget.
 * A QgsStackedWidget can be set to only consider the current page widget's sizeHint
 * and minimumSizeHint when resizing.
 *
 * \since QGIS 3.36
 */
class GUI_EXPORT QgsStackedWidget : public QStackedWidget
{
    Q_OBJECT

  public:
    /**
     * Possible modes for calculating a QgsStackedWidget's size
     */
    enum class SizeMode
    {
      //! The sizes of all pages are considered when calculating the stacked widget size
      ConsiderAllPages, //#spellok
      CurrentPageOnly,  //!< Only the size of the current page is considered when calculating the stacked widget size
    };

    /**
     * Constructor for QgsStackedWidget.
     * SizeMode defaults to Consider All Pages, same as QStackedWidget
     */
    explicit QgsStackedWidget( QWidget *parent = nullptr );

    /**
     * Returns the SizeMode for this QgsStackedWidget.
     * See QgsStackedWidget::SizeMode for interpretation
     * \see setSizeMode()
     */
    SizeMode sizeMode() const { return mSizeMode; }

    /**
     * Sets the \a mode for this QgsStackedWidget.
     * See QgsStackedWidget::SizeMode for interpretation
     * \see sizeMode()
     */
    void setSizeMode( SizeMode mode ) { mSizeMode = mode; }

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

  private:
    SizeMode mSizeMode;
};

#endif // QGSSTACKEDWIDGET_H
