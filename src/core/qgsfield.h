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

typedef QList<int> QgsAttributeList;

class QgsExpression;

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
      the data store reports it, with no attempt to standardize the value.
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

    /** Formats string for display*/
    QString displayString( const QVariant& v ) const;

    /**
     * Converts the provided variant to a compatible format
     *
     * @param v  The value to convert
     *
     * @return   True if the conversion was successful
     */
    bool convertCompatible( QVariant& v ) const;

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


/**
 \ingroup core
 Container of fields for a vector layer.

 In addition to storing a list of QgsField instances, it also:
 - allows quick lookups of field names to index in the list
 - keeps track of where the field definition comes from (vector data provider, joined layer or newly added from an editing operation)
 */
class CORE_EXPORT QgsFields
{
  public:

    enum FieldOrigin
    {
      OriginUnknown,   //!< it has not been specified where the field comes from
      OriginProvider,  //!< field comes from the underlying data provider of the vector layer  (originIndex = index in provider's fields)
      OriginJoin,      //!< field comes from a joined layer   (originIndex / 1000 = index of the join, originIndex % 1000 = index within the join)
      OriginEdit,      //!< field has been temporarily added in editing mode (originIndex = index in the list of added attributes)
      OriginExpression //!< field is calculated from an expression
    };

    typedef struct Field
    {
      Field(): origin( OriginUnknown ), originIndex( -1 ) {}
      Field( const QgsField& f, FieldOrigin o, int oi ): field( f ), origin( o ), originIndex( oi ) {}

      //! @note added in 2.6
      bool operator==( const Field& other ) const { return field == other.field && origin == other.origin && originIndex == other.originIndex; }
      //! @note added in 2.6
      bool operator!=( const Field& other ) const { return !( *this == other ); }

      QgsField field;      //!< field
      FieldOrigin origin;  //!< origin of the field
      int originIndex;     //!< index specific to the origin
    } Field;

    //! Remove all fields
    void clear();
    //! Append a field. The field must have unique name, otherwise it is rejected (returns false)
    bool append( const QgsField& field, FieldOrigin origin = OriginProvider, int originIndex = -1 );
    //! Append an expression field. The field must have unique name, otherwise it is rejected (returns false)
    bool appendExpressionField( const QgsField& field, int originIndex );
    //! Remove a field with the given index
    void remove( int fieldIdx );
    //! Extend with fields from another QgsFields container
    void extend( const QgsFields& other );

    //! Check whether the container is empty
    inline bool isEmpty() const { return mFields.isEmpty(); }
    //! Return number of items
    inline int count() const { return mFields.count(); }
    //! Return number of items
    inline int size() const { return mFields.count(); }
    //! Return if a field index is valid
    //! @param i  Index of the field which needs to be checked
    //! @return   True if the field exists
    inline bool exists( int i ) const { return i >= 0 && i < mFields.count(); }

    //! Get field at particular index (must be in range 0..N-1)
    inline const QgsField& operator[]( int i ) const { return mFields[i].field; }
    //! Get field at particular index (must be in range 0..N-1)
    inline QgsField& operator[]( int i ) { return mFields[i].field; }
    //! Get field at particular index (must be in range 0..N-1)
    const QgsField& at( int i ) const { return mFields[i].field; }
    //! Get field at particular index (must be in range 0..N-1)
    const QgsField& field( int fieldIdx ) const { return mFields[fieldIdx].field; }
    //! Get field at particular index (must be in range 0..N-1)
    const QgsField& field( const QString& name ) const { return mFields[ indexFromName( name )].field; }

    //! Get field's origin (value from an enumeration)
    FieldOrigin fieldOrigin( int fieldIdx ) const;
    //! Get field's origin index (its meaning is specific to each type of origin)
    int fieldOriginIndex( int fieldIdx ) const { return mFields[fieldIdx].originIndex; }

    //! Look up field's index from name. Returns -1 on error
    int indexFromName( const QString& name ) const { return mNameToIndex.value( name, -1 ); }

    //! Look up field's index from name - case insensitive
    //! TODO: sort out case sensitive (indexFromName()) vs insensitive (fieldNameIndex()) calls
    //! @note added in 2.4
    int fieldNameIndex( const QString& fieldName ) const;

    //! Utility function to get list of attribute indexes
    //! @note added in 2.4
    QgsAttributeList allAttributesList() const;

    //! Utility function to return a list of QgsField instances
    QList<QgsField> toList() const;

    //! @note added in 2.6
    bool operator==( const QgsFields& other ) const { return mFields == other.mFields; }
    //! @note added in 2.6
    bool operator!=( const QgsFields& other ) const { return !( *this == other ); }

  protected:
    //! internal storage of the container
    QVector<Field> mFields;

    //! map for quick resolution of name to index
    QHash<QString, int> mNameToIndex;

};




#endif
