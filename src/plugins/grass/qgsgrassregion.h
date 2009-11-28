/***************************************************************************
    qgsgrassregion.h  -  Edit region
                             -------------------
    begin                : August, 2004
    copyright            : (C) 2004 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGRASSREGION_H
#define QGSGRASSREGION_H

#include "ui_qgsgrassregionbase.h"

class QgsGrassPlugin;
class QgsGrassRegionEdit;

class QgisInterface;
class QgsMapCanvas;
//class QgsPoint;

class QButtonGroup;

extern "C"
{
#include <grass/gis.h>
}

/*! \class QgsGrassRegion
 *  \brief GRASS attributes.
 *
 */
class QgsGrassRegion: public QDialog, private Ui::QgsGrassRegionBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsGrassRegion( QgsGrassPlugin *plugin, QgisInterface *iface,
    QWidget * parent = 0, Qt::WFlags f = 0 );

    //! Destructor
    ~QgsGrassRegion();

  public slots:
    //! OK
    void on_acceptButton_clicked() { accept(); }
    void accept( void );

    //! Close
    void on_rejectButton_clicked() { reject(); }
    void reject( void );

    //! Called when rendering is finished
    void postRender( QPainter * );

    //! Mouse event receiver
    //void mouseEventReceiverMove ( QgsPoint & );

    //! Mouse event receiver
    //void mouseEventReceiverClick ( QgsPoint & );

    //! Calculate region, called if any value is changed
    void adjust( void );

    //! Value in GUI was changed
    void northChanged( const QString &str );
    void southChanged( const QString &str );
    void eastChanged( const QString &str );
    void westChanged( const QString &str );
    void NSResChanged( const QString &str );
    void EWResChanged( const QString &str );
    void rowsChanged( const QString &str );
    void colsChanged( const QString &str );

    void radioChanged( void ) ;

    void changeColor( void ) ;
    void changeWidth( void ) ;

    void restorePosition( void );

  private:
    //! Pointer to plugin
    QgsGrassPlugin *mPlugin;

    //! Pointer to QGIS interface
    QgisInterface *mInterface;

    //! Pointer to canvas
    QgsMapCanvas *mCanvas;

    QButtonGroup *mNSRadioGroup;
    QButtonGroup *mEWRadioGroup;

    //! Current new region
    struct Cell_head mWindow;

    //! Display current state of new region in XOR mode
    void displayRegion( void );

    //! Region was displayed
    bool mDisplayed;

    //! Draw region
    void draw( double x1, double y1, double x2, double y2 );

    //! First corner coordinates
    double mX;
    double mY;

    //! Currently updating GUI, don't run *Changed methods
    bool mUpdatingGui;

    // Set region values in GUI from mWindow
    void setGuiValues( bool north = true, bool south = true, bool east = true, bool west = true,
                       bool nsres = true, bool ewres = true, bool rows = true, bool cols = true );


    void saveWindowLocation( void );

    // Format N, S, E, W value
    QString formatEdge( double v );

    QgsGrassRegionEdit* mRegionEdit;

    friend class QgsGrassRegionEdit;
};

#endif // QGSGRASSREGION_H
