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
#include "qgsprojectpropertiesbase.h"
#else
#include "qgsprojectpropertiesbase.uic.h"
#endif
/**
 * \class QgsProjectProperties
 * \brief Dialog to set project level properties
 */
class QgsProjectProperties : public QgsProjectPropertiesBase
{
  Q_OBJECT
public:
    //! Constructor
  QgsProjectProperties(QWidget *parent=0, const char *name=0);
  //! Destructor
  ~QgsProjectProperties();
  /**
   * Gets the currently select map units
   * @return int which matches a value in the units enum in QgsScaleCalculator::units
   */
  int mapUnits();
  /**
   * Set the map units
   * @param int specifying units (matches a value in the units enum in QgsScaleCalculator::units)
   */
  void setMapUnits(int);

public slots:
  /** 
   * Slot called when a new button (unit) is selected
   * @param int specifying which button was selected. The button ids match the enum
   * values in QgsScaleCalculator::units
   */
  void mapUnitChange(int);
private:
  //! private member to hold the currently selected map units (so we can fetch them later)
  int mMapUnits;
  
};
