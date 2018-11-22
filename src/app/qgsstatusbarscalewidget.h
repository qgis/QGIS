/***************************************************************************
                         qgsstatusbarscalewidget.h
    begin                : May 2016
    copyright            : (C) 2016 Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTATUSBARSCALEWIDGET_H
#define QGSSTATUSBARSCALEWIDGET_H

class QFont;
class QHBoxLayout;
class QLabel;
class QValidator;

class QgsMapCanvas;
class QgsScaleComboBox;


#include <QWidget>
#include "qgis_app.h"

/**
  * Widget to define scale of the map canvas.
  * \since QGIS 2.16
  */
class APP_EXPORT QgsStatusBarScaleWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit QgsStatusBarScaleWidget( QgsMapCanvas *canvas, QWidget *parent = nullptr );

    /**
     * Set the selected \a scale from double.
     * The \a scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     */
    void setScale( double scale );

    /**
     * Lock the scale widget.
     * \param state the lock state
     * \since QGIS 3.4
     */
    void setLocked( bool state );

    /**
     * \brief isLocked check if the scale should be locked to use magnifier instead of scale to zoom in/out
     * \returns True if the scale shall be locked
     */
    bool isLocked() const;

    /**
     * Set the font of the text
      * \param font the font to use
      */
    void setFont( const QFont &font );

  public slots:
    void updateScales( const QStringList &scales = QStringList() );

  private slots:
    void userScale() const;

  private:
    QgsMapCanvas *mMapCanvas = nullptr;
    QHBoxLayout *mLayout = nullptr;

    //! Widget that will live on the statusbar to display "scale 1:"
    QLabel *mLabel = nullptr;
    //! Widget that will live on the statusbar to display scale value
    QgsScaleComboBox *mScale = nullptr;
};

#endif // QGSSTATUSBARSCALEWIDGET_H
