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
#include <QToolButton>


#include "qgscollapsiblegroupbox.h"
#include "qgsmaplayer.h"
#include "qgsmapcanvas.h"
#include "qgsscalecombobox.h"


class GUI_EXPORT QgsScaleRangeWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit QgsScaleRangeWidget( QWidget *parent = 0 );
    ~QgsScaleRangeWidget();

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
    void setMaxScaleFromCanvas();
    void setMinScaleFromCanvas();

  private:
    //! pointer to the map canvas used for current buttons.
    QgsMapCanvas* mCanvas;

    // ui
    QGridLayout* mLayout;
    QLabel* mMaximumScaleIconLabel;
    QLabel* mMinimumScaleIconLabel;
    QToolButton* mMaximumScaleSetCurrentPushButton;
    QToolButton* mMinimumScaleSetCurrentPushButton;
    QgsScaleComboBox* mMaximumScaleComboBox;
    QgsScaleComboBox* mMinimumScaleComboBox;
};

#endif // QGSSCALERANGEWIDGET_H
