/***************************************************************************
  qgsappscreenshots.h
  --------------------------------------
  Date                 : September 2018
  Copyright            : (C) 2018 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSAPPSCREENSHOTS_H
#define QGSAPPSCREENSHOTS_H

#include <QObject>
#include <QRect>

class QScreen;
class QgsVectorLayer;

class QgsAppScreenShots
{
    Q_GADGET
  public:
    enum GrabMode
    {
      GrabWidget,
      GrabWidgetAndFrame,
      GrabWholeWindow
    };
    Q_ENUM( GrabMode )

    enum Reference
    {
      Widget,
      QgisApp,
      Screen
    };

    //! Not part of the API to avoid cluttering
    enum Category
    {
      All = 0,
      GlobalOptions = 1 << 1,
      Symbol25D = 1 << 2,
      VectorLayerProperties = 1 << 3,
    };
    Q_ENUM( Category )
    Q_DECLARE_FLAGS( Categories, Category )
    Q_FLAG( Categories )

    QgsAppScreenShots( const QString &saveDirectory );

    //! if categories is null, then takes all categories
    void takePicturesOf( Categories categories = nullptr );

    //! set gradient size
    void setGradientSize( int size );

  private:
    QScreen *screen( QWidget *widget = nullptr );
    void moveWidgetTo( QWidget *widget, Qt::Corner corner, Reference reference = Screen );
    //! take and directly save screenshot
    void takeScreenshot( const QString &name, const QString &folder, QWidget *widget = nullptr, GrabMode mode = GrabWidgetAndFrame );
    //! take screenshot and return pixmap
    QPixmap takeScreenshot( QWidget *widget = nullptr, GrabMode mode = GrabWidgetAndFrame, QRect crop = QRect(), bool gradient = false );

    /**
     * save screenshot from pixmap
     * @param pixmap
     * @param name
     * @param crop the crop can have only one dimension (empty but not null rect)
     * @param gradient
     */
    void saveScreenshot( QPixmap &pixmap, const QString &name, const QString &folder );

    void takeVectorLayerProperties();
    void take25dSymbol();
    void takeGlobalOptions();

    QString mSaveDirectory;
    int mGradientSize = 200;
    QgsVectorLayer *mLineLayer = nullptr;
    QgsVectorLayer *mPolygonLayer = nullptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsAppScreenShots::Categories )

#endif // QGSAPPSCREENSHOTS_H
