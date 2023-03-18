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

#include <QWidget>
#include "qgis_app.h"
#include "qgspointxy.h"

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
    //void showMouseGeoCoordinates(const QgsPointXY& Qp);
    void extentsViewToggled( bool flag );
    void validateCoordinates();
    void dizzy();
    void world();
    void contributors();
    void hackfests();
    void userGroups();
    void showExtent();
    void ensureCoordinatesVisible();
    void updateCoordinateDisplay();
    void updateCoordinateDisplayUpdated(const QgsPointXY& Qp);
    void coordinateDisplaySettingsChanged();

//Mil Grid Function declaration Nihcas added
  inline QString LatLongToMilgridConversion(const QgsPointXY& p);
  inline QString LatLongTopoSheetConversion(const QgsPointXY& p);
  inline QString eveLatLongToMilgridConversion(const QgsPointXY& p);
  inline QString eveLatLongTopoSheetConversion(const QgsPointXY& p);

  char* check_row_2_sides(int r);
  int check_domain_2_sides(float lat1, float long1, float lat2, float long2, double latitude, double longitude);
  char* check_row_3_sides(int r);
  int check_domain_3_sides(float lat1, float long1, float lat2, float long2, double latitude, double longitude);
  char* check_row_4_sides(int r);
  int check_domain_4_sides(float lat1, float long1, float lat2, float long2, double latitude, double longitude);
  int* checkarray(int pe_local, int pn_local);
  //Nihcas added above


  private:
    void refreshMapCanvas();

    QLineEdit *mLineEdit = nullptr;
    QToolButton *mToggleExtentsViewButton = nullptr;
    //! Widget that will live on the statusbar to display "Coordinate / Extent"
    QLabel *mLabel = nullptr;
	  QLabel* mLabeldgr = nullptr;
	  QLabel* mLabeldsheet = nullptr;
	  QLabel* mLabelegr = nullptr;
	  QLabel* mLabelesheet = nullptr;
    QLabel* mLabelgeocord = nullptr;
    QValidator *mCoordsEditValidator = nullptr;
    QTimer *mDizzyTimer = nullptr;
    QgsMapCanvas *mMapCanvas = nullptr;
    int mTwoCharSize = 0;
    int mMinimumWidth = 0;

    //! The number of decimal places to use if not automatic
    unsigned int mMousePrecisionDecimalPlaces;
	
	//Variables for milgrid controls Nihcas added

    QLineEdit* mCoordsGeocord = nullptr;
    QLineEdit* mCoordsEditMgrid = nullptr;
	  QLineEdit* mCoordsEditMgrideve = nullptr;
	  QLineEdit * mCoordsEditMsheet = nullptr;
	  QLineEdit * mCoordsEditMsheeteve = nullptr;
    //! The validator for the mCoordsEdit
	QValidator* mCoordsEditMgridValidator;
	QValidator* mCoordsEditMgrideveValidator;

    QgsPointXY mLastCoordinate;

};

#endif // QGSSTATUSBARCOORDINATESWIDGET_H
