/***************************************************************************
                          qgsfeatureattribute.h  -  description
                             -------------------
    begin                : Mon Sep 01 2003
    copyright            : (C) 2003 by Gary E.Sherman
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
#ifndef QGSFEATUREATTRIBUTE_H
#define QGSFEATUREATTRIBUTE_H

#include <qstring.h>

/** \class QgsFeatureAttribute - Feature attribute class.
 * \brief Encapsulates a single feature attribute.
  *@author Gary E.Sherman
  */

class QgsFeatureAttribute
{
public:

  //! Constructor
  QgsFeatureAttribute(QString const & field = 0, QString const & value = 0);

  //! Destructor
  ~QgsFeatureAttribute();

  /**
	* Get the field name for this feature attribute
	* @return Field name
	*/
  QString const & fieldName() const;


  /**
	* Get the field value for this feature attribute
	* @return Field value
	*/
  QString const & fieldValue() const;

private:

  //! attribute field name
  QString field;

  //! attribute field value
  QString value;

}; // class QgsFeatureAttribute

#endif
