/***************************************************************************
                             qgsprojectproperties.h    
       Set various project properties (inherits qgsprojectpropertiesbase)
                              -------------------
  begin                : May 18, 2004
  copyright            : (C) 2004 by Gary E.Sherman
  email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* $Id$ */

#ifdef WIN32
#include <qgsprojectpropertiesbase.h>
#else
#include <qgsprojectpropertiesbase.uic.h>
#endif

#include <qgsscalecalculator.h>

/**  Dialog to set project level properties

  @note actual state is stored in QgsProject singleton instance

 */
class QgsProjectProperties : public QgsProjectPropertiesBase
{
  Q_OBJECT
public:
    //! Constructor
  QgsProjectProperties(QWidget *parent = 0, const char * name = 0);

  //! Destructor
  ~QgsProjectProperties();

  /** Gets the currently select map units
   */
  QgsScaleCalculator::units mapUnits() const;

  /**
   * Set the map units
   */
  void setMapUnits(QgsScaleCalculator::units);

  /**
     Every project has a title
  */
  QString title() const;
  void title( QString const & title );
  
  /** Accessor for projection */
  QString projectionWKT();
  /** Get a short human readable name from a WKT */
  QString getWKTShortName(QString theWKT);
public slots:
  /** 
   * Slot called when a new button (unit) is selected
   * @param int specifying which button was selected. The button ids match the enum
   * values in QgsScaleCalculator::units
   */
  void mapUnitChange(int);
  /**
   * Slot called when apply button is pressed 
   */
  void apply();
  /**
   * Slot called when ok button pressed (inherits from gui base)
   */
  void accept();
  //! Populate the wkts map with projection names...
  void getProjList();
  
  //! Slot called when user selects a different item in the projections tree 
  void coordinateSystemSelected( QListViewItem * );

  
signals:
  /** Used to notify all coordinateTransform objects to update their dest wkt because the project
      output projection system is changed */
  void setDestWKT(QString);   
private:
  typedef QMap<QString,QString> ProjectionWKTMap; //wkt = well known text (see gdal/ogr)
  //stores a list of available projection definitions 
  ProjectionWKTMap mProjectionsMap;
  //XXX List view items for the tree view of projections
  QListViewItem *geoList;
  QListViewItem *projList;
};
