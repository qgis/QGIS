/***************************************************************************
                         qgsstatusbarmagnifierwidget.h
    begin                : April 2016
    copyright            : (C) 2016 Paul Blottiere, Oslandia
    email                : paul dot blottiere at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTATUSBARMAGNIFIERWIDGET_H
#define QGSSTATUSBARMAGNIFIERWIDGET_H

class QLabel;
class QFont;
class QHBoxLayout;
class QToolButton;
class QgsDoubleSpinBox;

#include <QWidget>
#include "qgis_app.h"

/**
  * A widget which lets the user select the current level of magnification to
  * apply to the canvas.
  */
class APP_EXPORT QgsStatusBarMagnifierWidget : public QWidget
{
    Q_OBJECT

  public:
    /**
     * Constructor
      * \param parent is the parent widget
      */
    QgsStatusBarMagnifierWidget( QWidget *parent = nullptr );

    void setDefaultFactor( double factor );

    /**
     * Set the font of the text
      * \param font the font to use
      */
    void setFont( const QFont &font );


  public slots:
    //! will be triggered from map canvas changes (from mouse wheel, zoom)
    void updateMagnification( double factor );

    /**
     * Will be triggered from map canvas API changes
     * \param locked true if the scale is locked
     * \since QGIS 3.18
     */
    void updateScaleLock( bool locked );

  private slots:
    //! will be triggered form user input in spin box
    void setMagnification( double value );

  signals:
    void magnificationChanged( double factor );

    void scaleLockChanged( bool );

  private:
    QHBoxLayout *mLayout = nullptr;
    QLabel *mLabel = nullptr;
    QgsDoubleSpinBox *mSpinBox = nullptr;
    QToolButton *mLockButton = nullptr;
};

#endif
