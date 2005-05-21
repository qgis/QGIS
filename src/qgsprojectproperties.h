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
#include "qgis.h"
  
/*!  Dialog to set project level properties

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

  /*! Gets the currently select map units
   */
  QGis::units mapUnits() const;

  /*!
   * Set the map units
   */
  void setMapUnits(QGis::units);

  /*!
     Every project has a title
  */
  QString title() const;
  void title( QString const & title );
  
  /*! Accessor for projection */
  QString projectionWKT();
  /*! Indicates that the projection switch is on */
  bool QgsProjectProperties::isProjected();
public slots:
  /*! 
   * Slot called when a new button (unit) is selected
   * @param int specifying which button was selected. The button ids match the enum
   * values in QGis::units
   */
  void mapUnitChange(int);
  /*!
   * Slot called when apply button is pressed 
   */
  void apply();
  /*!
   * Slot called when ok button pressed (inherits from gui base)
   */
  void accept();
  /*!
   * Slot to show the projections tab when the dialog is opened
   */
  void showProjectionsTab();
  
signals:
  /*! This signal is used to notify all coordinateTransform objects to update
   * their dest SRSID because the project output projection system is changed 
   * @param srsid srs.db tbl_srs pkey value of the newly assigned srs
   */
  void setDestSRSID(long theSRSID);   
  //! Signal used to inform listeners that the mouse display precision may have changed
  void displayPrecisionChanged();
  //! let listening canvases know to refresh
  void refresh();
  //! notification of when on the fly projections are enabled / disabled
  void projectionEnabled(bool);
private:

};
