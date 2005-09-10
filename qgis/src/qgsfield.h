/***************************************************************************
               qgsfield.h - Describes a field in a layer or table
                     --------------------------------------
               Date                 : 01-Jan-2004
               Copyright            : (C) 2004 by Gary E.Sherman
               email                : sherman at mrcc.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSFIELD_H
#define QGSFIELD_H

#include <qstring.h>

/**
  \class QgsField
  \brief Class to encapsulate a field in an attribute table or data source.

  QgsField stores metadata about an attribute field, including name, type
  length, and if applicable, precision.
 */

class QgsField
{
public:
    /** Constructor. Constructs a new QgsField object.
     * @param nam Field name
     * @param typ Field type (eg. char, varchar, text, int, serial, double). 
     Field types are usually unique to the source and are stored exactly
     as returned from the data store.
     * @param len Field length
     * @param prec Field precision. Usually decimal places but may also be
     * used in conjunction with other fields types (eg. variable character fields)
     * @param num Has to be true if field contains numeric values.
     */
  QgsField(QString nam = "", QString typ = "", int len = 0, int prec = 0, bool num = false);

  //! Destructor
   ~QgsField();

   bool operator==(const QgsField other) const;
   bool operator!=(const QgsField other) const;

  //! Gets the name of the field
  QString const & name() const;


    /** 
      Gets the field type. Field types vary depending on the data source. Examples
      are char, int, double, blob, geometry, etc. The type is stored exactly as
      the data store reports it, with no attenpt to standardize the value.
      @return QString containing the field type
     */
  QString const & type() const;


    /**
      Gets the length of the field.
      @return int containing the length of the field
     */
  int length() const;


    /**
      Gets the precision of the field. Not all field types have a related precision.
      @return int containing the precision or zero if not applicable to the field type.
     */
  int precision() const;

    /**
     Returns true if field contains numeric values. This information is set by provider.
     */
  bool isNumeric() const;


    /**
      Set the field name.
      @param nam Name of the field
     */
  void setName(QString const & nam);

    /**
      Set the field type.
      @param typ Field type
     */
  void setType(QString const & typ);

    /**
      Set the field length.
      @param len Length of the field
     */
  void setLength(int len);

    /**
      Set the field precision.
      @param prec Precision of the field
     */
  void setPrecision(int prec);

    /**
      Set whether field is numeric
      */
  void setNumeric(bool num);

private:

  //! Name
  QString mName;

  //! Type
  QString mType;

  //! Length
  int mLength;

  //! Precision
  int mPrecision;

  //! Numeric
  bool mNumeric;

}; // class QgsField

#endif
