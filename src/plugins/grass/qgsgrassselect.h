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

/**
 * \class QgsGrassSelect
 * \brief Dialog to select GRASS layer.
 *
 */
class QgsGrassSelect: public QDialog, private Ui::QgsGrassSelectBase
{
    Q_OBJECT

  public:
    //! Constructor
    //QgsGrassSelect(QWidget *parent = 0, int type = VECTOR );
    QgsGrassSelect( QWidget *parent, int type = Vector );

    enum Type
    {
      MapSet,
      Vector,
      Raster,
      Group, // group of rasters, used in selectedType
      MapCalc // file in $MAPSET/mapcalc directory (used by QgsGrassMapcalc)
    };

    QString  gisdbase;
    QString  location;
    QString  mapset;
    QString  map;
    QString  layer;
    int      selectedType;  // RASTER or GROUP

  public slots:
    void accept() override;

    //! Open dialog for Gisdbase
    void GisdbaseBrowse_clicked();

    //! Reset combobox of locations for current Gisdbase
    void egisdbase_textChanged() { setLocations(); }
    void setLocations();

    //! Reset combobox of mapsets for current Location
    void elocation_activated() { setMapsets(); }
    void setMapsets();

    //! Reset combobox of maps for current Gisdbase + Location
    void emapset_activated() { setMaps(); }
    void setMaps();

    //! Reset combobox of layers for current Gisdbase + Location + Map
    void emap_activated() { setLayers(); }
    void setLayers();

  private:
    int type; // map type (mapset element)
    static bool sFirst; // called first time
    static QString sLastGisdbase; // Last selected values
    static QString sLastLocation;
    static QString sLastMapset;
    static QString sLastVectorMap;
    static QString sLastRasterMap;
    static QString sLastLayer; // vector layer
    static QString sLastMapcalc;
};


#endif // QGSGRASSSELECT_H
