/***************************************************************************
    qgsgrassselect.h  -  Select GRASS layer dialog
                             -------------------
    begin                : March, 2004
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
#ifndef QGSGRASSSELECT_H
#define QGSGRASSSELECT_H
#include "ui_qgsgrassselectbase.h"

/*! \class QgsGrassSelect
 * \brief Dialog to select GRASS layer.
 *
 */
class QgsGrassSelect: public QDialog, private Ui::QgsGrassSelectBase
{
    Q_OBJECT

  public:
    //! Constructor
    //QgsGrassSelect(QWidget *parent = 0, int type = VECTOR );
    QgsGrassSelect( int type = VECTOR );
    //! Destructor
    ~QgsGrassSelect();

    enum TYPE
    {
      MAPSET,
      VECTOR,
      RASTER,
      GROUP, // group of rasters, used in selectedType
      MAPCALC // file in $MAPSET/mapcalc directory (used by QgsGrassMapcalc)
    };

    QString  gisdbase;
    QString  location;
    QString  mapset;
    QString  map;
    QString  layer;
    int      selectedType;  // RASTER or GROUP

  public slots:
    //! OK
    void on_ok_clicked();

    //! Cancel
    void on_cancel_clicked();

    //! Open dialog for Gisdbase
    void on_GisdbaseBrowse_clicked();

    //! Reset combobox of locations for current Gisdbase
    void on_egisdbase_textChanged() { setLocations(); }
    void setLocations();

    //! Reset combobox of mapsets for current Location
    void on_elocation_activated() { setMapsets(); }
    void setMapsets();

    //! Reset combobox of maps for current Gisdbase + Location
    void on_emapset_activated() { setMaps(); }
    void setMaps();

    //! Reset combobox of layers for current Gisdbase + Location + Map
    void on_emap_activated() { setLayers(); }
    void setLayers();


  private:
    int type; // map type (mapset element)
    static bool first; // called first time
    static QString lastGisdbase; // Last selected values
    static QString lastLocation;
    static QString lastMapset;
    static QString lastVectorMap;
    static QString lastRasterMap;
    static QString lastLayer; // vector layer
    static QString lastMapcalc;
};


#endif // QGSGRASSSELECT_H
