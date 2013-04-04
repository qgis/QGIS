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

#ifndef QGSFIELD_H
#define QGSFIELD_H

#include <QString>
#include <QVariant>
#include <QVector>

/** \ingroup core
  * Encapsulate a field in an attribute table or data source.
  * QgsField stores metadata about an attribute field, including name, type
  * length, and if applicable, precision.
 */

class CORE_EXPORT QgsField
{
  public:
    /** Constructor. Constructs a new QgsField object.
     * @param name Field name
     * @param type Field variant type, currently supported: String / Int / Double
     * @param typeName Field type (eg. char, varchar, text, int, serial, double).
     Field types are usually unique to the source and are stored exactly
     as returned from the data store.
     * @param len Field length
     * @param prec Field precision. Usually decimal places but may also be
     * used in conjunction with other fields types (eg. variable character fields)
     * @param comment Comment for the field
     */

    QgsField( QString name = QString(),
              QVariant::Type type = QVariant::Invalid,
              QString typeName = QString(),
              int len = 0,
              int prec = 0,
              QString comment = QString() );

    //! Destructor
    ~QgsField();

    bool operator==( const QgsField& other ) const;
    bool operator!=( const QgsField& other ) const;

    //! Gets the name of the field
    const QString & name() const;

    //! Gets variant type of the field as it will be retrieved from data source
    QVariant::Type type() const;

    /**
      Gets the field type. Field types vary depending on the data source. Examples
      are char, int, double, blob, geometry, etc. The type is stored exactly as
      the data store reports it, with no attenpt to standardize the value.
      @return QString containing the field type
     */
    const QString & typeName() const;


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
    Returns the field comment
    */
    const QString & comment() const;

    /**
      Set the field name.
      @param nam Name of the field
     */
    void setName( const QString & nam );

    /**
      Set variant type.
     */
    void setType( QVariant::Type type );

    /**
      Set the field type.
      @param typ Field type
     */
    void setTypeName( const QString & typ );

    /**
      Set the field length.
      @param len Length of the field
     */
    void setLength( int len );

    /**
      Set the field precision.
      @param prec Precision of the field
     */
    void setPrecision( int prec );


    /**
      Set the field comment
      */
    void setComment( const QString & comment );

    /**Formats string for display*/
    QString displayString( const QVariant& v ) const;

  private:

    //! Name
    QString mName;

    //! Variant type
    QVariant::Type mType;

    //! Type name from provider
    QString mTypeName;

    //! Length
    int mLength;

    //! Precision
    int mPrecision;

    //! Comment
    QString mComment;

}; // class QgsField

// key = field index, value=field data
typedef QMap<int, QgsField> QgsFieldMap;



class CORE_EXPORT QgsFields
{
  public:

    enum FieldOrigin { OriginUnknown, OriginProvider, OriginJoin, OriginEdit };
    typedef struct Field
    {
      Field(): origin( OriginUnknown ), originIndex( -1 ) {}
      Field( const QgsField& f, FieldOrigin o, int oi ): field( f ), origin( o ), originIndex( oi ) {}

      QgsField field;
      FieldOrigin origin; // TODO[MD]: originIndex or QVariant originID?
      int originIndex;
    } Field;

    void clear() { mFields.clear(); mNameToIndex.clear(); }
    /** append: fields must have unique names! */
    bool append( const QgsField& field, FieldOrigin origin = OriginProvider, int originIndex = -1 )
    {
      if ( mNameToIndex.contains( field.name() ) )
        return false;

      if ( originIndex == -1 && origin == OriginProvider )
        originIndex = mFields.count();
      mFields.append( Field( field, origin, originIndex ) );

      mNameToIndex.insert( field.name(), mFields.count() - 1 );
      return true;
    }
    void remove( int fieldIdx ) { mNameToIndex.remove( mFields[fieldIdx].field.name() ); mFields.remove( fieldIdx ); }

    inline bool isEmpty() const { return mFields.isEmpty(); }
    inline int count() const { return mFields.count(); }
    inline int size() const { return mFields.count(); } // TODO[MD]: delete?
    inline const QgsField& operator[]( int i ) const { return mFields[i].field; }
    inline QgsField& operator[]( int i ) { return mFields[i].field; }
    const QgsField& at( int i ) const { return mFields[i].field; }
    QList<QgsField> toList() const { QList<QgsField> lst; for ( int i = 0; i < mFields.count(); ++i ) lst.append( mFields[i].field ); return lst; } // TODO[MD]: delete?

    const QgsField& field( int fieldIdx ) const { return mFields[fieldIdx].field; }
    const QgsField& field( const QString& name ) const { return mFields[ indexFromName( name )].field; }
    FieldOrigin fieldOrigin( int fieldIdx ) const { return mFields[fieldIdx].origin; }
    int fieldOriginIndex( int fieldIdx ) const { return mFields[fieldIdx].originIndex; }

    int indexFromName( const QString& name ) const { return mNameToIndex.value( name, -1 ); }

  protected:
    QVector<Field> mFields;

    //! map for quick resolution of name to index
    QHash<QString, int> mNameToIndex;
};


#endif
