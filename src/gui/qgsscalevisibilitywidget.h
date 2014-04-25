/***************************************************************************
   qgsscalevisibilitywidget.h
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

#ifndef QGSSCALEVISIBILITYWIDGET_H
#define QGSSCALEVISIBILITYWIDGET_H

#include "ui_qgsscalevisibilitywidget.h"
#include "qgscollapsiblegroupbox.h"
#include "qgsmaplayer.h"
#include "qgsmapcanvas.h"


class GUI_EXPORT QgsScaleVisibilityWidget : public QWidget, private Ui::QgsScaleVisibilityWidget
{
    Q_OBJECT

  public:
    explicit QgsScaleVisibilityWidget( QWidget *parent = 0 );
    ~QgsScaleVisibilityWidget();

    void showEvent( QShowEvent * );

    //! set the map canvas which will be used for the current scale buttons
    /**
     * @brief setMapCanvas set the map canvas which will be used for the current scale buttons
     * if not set, the buttons are hidden.
     */
    void setMapCanvas( QgsMapCanvas* mapCanvas );

  public slots:
    void setMinimumScale( double scale );
    double minimumScale();

    void setMaximumScale( double scale );
    double maximumScale();

    void setFromLayer( QgsMapLayer* layer );

  private slots:
    void on_mMinimumScaleSetCurrentPushButton_clicked();
    void on_mMaximumScaleSetCurrentPushButton_clicked();

  private:
    //! pointer to the map canvas used for current buttons.
    QgsMapCanvas* mCanvas;
};

#endif // QGSSCALEVISIBILITYWIDGET_H
