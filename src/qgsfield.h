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
class QString;
/**
  \class QgsField
  \brief Class to encapsulate a field in an attribute table or data source.

  QgsField stores metadata about an attribute field, including name, type
  length, and if applicable, precision.
 */

class QgsField{
  public:
    /** Constructor. Constructs a new QgsField object.
     * @param nam Field name
     * @param typ Field type (eg. char, varchar, text, int, serial, double). 
     Field types are usually unique to the source and are stored exactly
     as returned from the data store.
     * @param len Field length
     * @param prec Field precision. Usually decimal places but may also be
     * used in conjunction with other fields types (eg. variable character fields)
     */
    QgsField(QString nam=0, QString typ=0, int len=0, int prec=0);
    //! Destructor
    ~QgsField();
    //! Gets the name of the field
    QString name();
    /** 
      Gets the field type. Field types vary depending on the data source. Examples
      are char, int, double, blob, geometry, etc. The type is stored exactly as
      the data store reports it, with no attenpt to standardize the value.
      @return QString containing the field type
     */
    QString type();
    /**
      Gets the length of the field.
      @return int containing the length of the field
     */
    int length();
    /**
      Gets the precision of the field. Not all field types have a related precision.
      @return int containing the precision or zero if not applicable to the field type.
     */
    int precision();
    /**
      Set the field name.
      @param nam Name of the field
     */
    void setName(QString nam);
    /**
      Set the field type.
      @param typ Field type
     */
    void setType(QString typ);
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
  private:
    //! Name
    QString mName;
    //! Type
    QString mType;
    //! Length
    int mLength;
    //! Precision
    int mPrecision;
};
