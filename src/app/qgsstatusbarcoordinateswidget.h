/***************************************************************************
   qgsstatusbarcoordinateswidget.h
    --------------------------------------
   Date                 : 05.08.2015
   Copyright            : (C) 2015 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSSTATUSBARCOORDINATESWIDGET_H
#define QGSSTATUSBARCOORDINATESWIDGET_H


class QFont;
class QLabel;
class QLineEdit;
class QTimer;
class QToolButton;
class QValidator;

class QgsMapCanvas;
class QgsPointXY;

#include <QWidget>
#include "qgis_app.h"

class APP_EXPORT QgsStatusBarCoordinatesWidget : public QWidget
{
    Q_OBJECT

    enum CrsMode
    {
      MapCanvas,
      Custom
    };

  public:
    QgsStatusBarCoordinatesWidget( QWidget *parent );

    //! define the map canvas associated to the widget
    void setMapCanvas( QgsMapCanvas *mapCanvas );

    void setFont( const QFont &myFont );

    void setMouseCoordinatesPrecision( unsigned int precision );

  signals:
    void coordinatesChanged();
    void weAreBored();

  private slots:
    void showMouseCoordinates( const QgsPointXY &p );
    void extentsViewToggled( bool flag );
    void validateCoordinates();
    void dizzy();
    void contributors();
    void hackfests();
    void showExtent();
    void ensureCoordinatesVisible();

  private:
    void refreshMapCanvas();

    QLineEdit *mLineEdit = nullptr;
    QToolButton *mToggleExtentsViewButton = nullptr;
    //! Widget that will live on the statusbar to display "Coordinate / Extent"
    QLabel *mLabel = nullptr;

    QValidator *mCoordsEditValidator = nullptr;
    QTimer *mDizzyTimer = nullptr;
    QgsMapCanvas *mMapCanvas = nullptr;
    int mTwoCharSize = 0;
    int mMinimumWidth = 0;

    //! The number of decimal places to use if not automatic
    unsigned int mMousePrecisionDecimalPlaces;

};

#endif // QGSSTATUSBARCOORDINATESWIDGET_H
