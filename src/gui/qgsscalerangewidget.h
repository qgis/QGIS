/***************************************************************************
   qgsscalerangewidget.h
    --------------------------------------
   Date                 : 25.04.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSSCALERANGEWIDGET_H
#define QGSSCALERANGEWIDGET_H

#include <QGridLayout>
#include <QLabel>

#include "qgscollapsiblegroupbox.h"
#include "qgsmaplayer.h"
#include "qgsmapcanvas.h"
#include "qgsscalewidget.h"

/** \ingroup gui
 * \class QgsScaleRangeWidget
 */
class GUI_EXPORT QgsScaleRangeWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit QgsScaleRangeWidget( QWidget *parent = nullptr );
    ~QgsScaleRangeWidget();

    //! set the map canvas which will be used for the current scale buttons
    /**
     * @brief setMapCanvas set the map canvas which will be used for the current scale buttons
     * if not set, the buttons are hidden.
     */
    void setMapCanvas( QgsMapCanvas* mapCanvas );

    //! return the minimum scale
    double minimumScale();

    //! return the maximum scale
    double maximumScale();

    //! return the minimum scale denominator ( = 1 / maximum scale )
    double minimumScaleDenom();

    //! return the maximum scale denominator ( = 1 / minimum scale )
    double maximumScaleDenom();

    //! call to reload the project scales and apply them to the 2 scales combo boxes
    void reloadProjectScales();

  public slots:
    void setMinimumScale( double scale );

    void setMaximumScale( double scale );

    void setScaleRange( double min, double max );

  signals:

    /** Emitted when the scale range set in the widget is changed.
     * @param min minimum scale
     * @param max maximum scale
     * @note added in QGIS 2.16
     */
    void rangeChanged( double min, double max );

  private slots:

    void emitRangeChanged();

  private:
    //! pointer to the map canvas used for current buttons.
    QgsMapCanvas* mCanvas;

    // ui
    QGridLayout* mLayout;
    QLabel* mMaximumScaleIconLabel;
    QLabel* mMinimumScaleIconLabel;
    QgsScaleWidget* mMaximumScaleWidget;
    QgsScaleWidget* mMinimumScaleWidget;
};

#endif // QGSSCALERANGEWIDGET_H
