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
#include <QSharedDataPointer>

typedef QList<int> QgsAttributeList;

class QgsExpression;
class QgsFieldPrivate;
class QgsFieldsPrivate;

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfield.cpp.
 * See details in QEP #17
 ****************************************************************************/

/** \class QgsField
  * \ingroup core
  * Encapsulate a field in an attribute table or data source.
  * QgsField stores metadata about an attribute field, including name, type
  * length, and if applicable, precision.
  * \note QgsField objects are implicitly shared.
 */

class CORE_EXPORT QgsField
{
  public:
    /** Constructor. Constructs a new QgsField object.
     * @param name Field name
     * @param type Field variant type, currently supported: String / Int / Double
     * @param typeName Field type (eg. char, varchar, text, int, serial, double).
     * Field types are usually unique to the source and are stored exactly
     * as returned from the data store.
     * @param len Field length
     * @param prec Field precision. Usually decimal places but may also be
     * used in conjunction with other fields types (eg. variable character fields)
     * @param comment Comment for the field
     */
    QgsField( const QString& name = QString(),
              QVariant::Type type = QVariant::Invalid,
              const QString& typeName = QString(),
              int len = 0,
              int prec = 0,
              const QString& comment = QString() );

    /** Copy constructor
     */
    QgsField( const QgsField& other );

    /** Assignment operator
     */
    QgsField& operator =( const QgsField &other );

    //! Destructor
    virtual ~QgsField();

    bool operator==( const QgsField& other ) const;
    bool operator!=( const QgsField& other ) const;

    //! Gets the name of the field
    QString name() const;

    //! Gets variant type of the field as it will be retrieved from data source
    QVariant::Type type() const;

    /**
     * Gets the field type. Field types vary depending on the data source. Examples
     * are char, int, double, blob, geometry, etc. The type is stored exactly as
     * the data store reports it, with no attempt to standardize the value.
     * @return QString containing the field type
     */
    QString typeName() const;

    /**
     * Gets the length of the field.
     * @return int containing the length of the field
     */
    int length() const;

    /**
     * Gets the precision of the field. Not all field types have a related precision.
     * @return int containing the precision or zero if not applicable to the field type.
     */
    int precision() const;

    /**
     * Returns the field comment
     */
    QString comment() const;

    /**
     * Set the field name.
     * @param name Name of the field
     */
    void setName( const QString& name );

    /**
     * Set variant type.
     */
    void setType( QVariant::Type type );

    /**
     * Set the field type.
     * @param typeName Field type
     */
    void setTypeName( const QString& typeName );

    /**
     * Set the field length.
     * @param len Length of the field
     */
    void setLength( int len );

    /**
     * Set the field precision.
     * @param precision Precision of the field
     */
    void setPrecision( int precision );

    /**
     * Set the field comment
     */
    void setComment( const QString& comment );

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

    //! Allows direct construction of QVariants from fields.
    operator QVariant() const
    {
      return QVariant::fromValue( *this );
    }

  private:

    QSharedDataPointer<QgsFieldPrivate> d;


}; // class QgsField

Q_DECLARE_METATYPE( QgsField )

/** Writes the field to stream out. QGIS version compatibility is not guaranteed. */
CORE_EXPORT QDataStream& operator<<( QDataStream& out, const QgsField& field );
/** Reads a field from stream in into field. QGIS version compatibility is not guaranteed. */
CORE_EXPORT QDataStream& operator>>( QDataStream& in, QgsField& field );



/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfields.cpp.
 * See details in QEP #17
 ****************************************************************************/

/** \class QgsFields
 * \ingroup core
 * Container of fields for a vector layer.
 *
 * In addition to storing a list of QgsField instances, it also:
 * - allows quick lookups of field names to index in the list
 * - keeps track of where the field definition comes from (vector data provider, joined layer or newly added from an editing operation)
 * \note QgsFields objects are implicitly shared.
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

    /** Constructor for an empty field container
     */
    QgsFields();

    /** Copy constructor
     */
    QgsFields( const QgsFields& other );

    /** Assignment operator
     */
    QgsFields& operator =( const QgsFields &other );

    virtual ~QgsFields();

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
    bool isEmpty() const;
    //! Return number of items
    int count() const;
    //! Return number of items
    int size() const;
    //! Return if a field index is valid
    //! @param i  Index of the field which needs to be checked
    //! @return   True if the field exists
    bool exists( int i ) const;

    //! Get field at particular index (must be in range 0..N-1)
    const QgsField& operator[]( int i ) const;
    //! Get field at particular index (must be in range 0..N-1)
    QgsField& operator[]( int i );
    //! Get field at particular index (must be in range 0..N-1)
    const QgsField& at( int i ) const;
    //! Get field at particular index (must be in range 0..N-1)
    const QgsField& field( int fieldIdx ) const;
    //! Get field at particular index (must be in range 0..N-1)
    const QgsField& field( const QString& name ) const;

    //! Get field's origin (value from an enumeration)
    FieldOrigin fieldOrigin( int fieldIdx ) const;
    //! Get field's origin index (its meaning is specific to each type of origin)
    int fieldOriginIndex( int fieldIdx ) const;

    //! Look up field's index from name. Returns -1 on error
    int indexFromName( const QString& name ) const;

    //! Look up field's index from name
    //! also looks up case-insensitive if there is no match otherwise
    //! @note added in 2.4
    int fieldNameIndex( const QString& fieldName ) const;

    //! Utility function to get list of attribute indexes
    //! @note added in 2.4
    QgsAttributeList allAttributesList() const;

    //! Utility function to return a list of QgsField instances
    QList<QgsField> toList() const;

    //! @note added in 2.6
    bool operator==( const QgsFields& other ) const;
    //! @note added in 2.6
    bool operator!=( const QgsFields& other ) const { return !( *this == other ); }
    /** Returns an icon corresponding to a field index, based on the field's type and source
     * @note added in QGIS 2.14
     */
    QIcon iconForField( int fieldIdx ) const;

    //! Allows direct construction of QVariants from fields.
    operator QVariant() const
    {
      return QVariant::fromValue( *this );
    }

    ///@cond PRIVATE

    class const_iterator;

    class iterator
    {
      public:
        QgsFields::Field* d;
        typedef std::random_access_iterator_tag  iterator_category;
        typedef qptrdiff difference_type;

        inline iterator()
            : d( nullptr )
        {}
        inline iterator( QgsFields::Field *n )
            : d( n )
        {}

        inline QgsField& operator*() const { return d->field; }
        inline QgsField* operator->() const { return &d->field; }
        inline QgsField& operator[]( difference_type j ) const { return d[j].field; }
        inline bool operator==( const iterator &o ) const noexcept { return d == o.d; }
        inline bool operator!=( const iterator &o ) const noexcept { return d != o.d; }
        inline bool operator<( const iterator& other ) const noexcept { return d < other.d; }
        inline bool operator<=( const iterator& other ) const noexcept { return d <= other.d; }
        inline bool operator>( const iterator& other ) const noexcept { return d > other.d; }
        inline bool operator>=( const iterator& other ) const noexcept { return d >= other.d; }

        inline iterator& operator++() { ++d; return *this; }
        inline iterator operator++( int ) { QgsFields::Field* n = d; ++d; return n; }
        inline iterator& operator--() { d--; return *this; }
        inline iterator operator--( int ) { QgsFields::Field* n = d; d--; return n; }
        inline iterator& operator+=( difference_type j ) { d += j; return *this; }
        inline iterator& operator-=( difference_type j ) { d -= j; return *this; }
        inline iterator operator+( difference_type j ) const { return iterator( d + j ); }
        inline iterator operator-( difference_type j ) const { return iterator( d -j ); }
        inline int operator-( iterator j ) const { return int( d - j.d ); }
    };
    friend class iterator;

    class const_iterator
    {
      public:
        const QgsFields::Field* d;

        typedef std::random_access_iterator_tag  iterator_category;
        typedef qptrdiff difference_type;

        inline const_iterator()
            : d( nullptr ) {}
        inline const_iterator( const QgsFields::Field* f )
            : d( f ) {}
        inline const_iterator( const const_iterator &o )
            : d( o.d ) {}
        inline explicit const_iterator( const iterator &o )
            : d( o.d ) {}
        inline const QgsField& operator*() const { return d->field; }
        inline const QgsField* operator->() const { return &d->field; }
        inline const QgsField& operator[]( difference_type j ) const noexcept { return d[j].field; }
        inline bool operator==( const const_iterator &o ) const noexcept { return d == o.d; }
        inline bool operator!=( const const_iterator &o ) const noexcept { return d != o.d; }
        inline bool operator<( const const_iterator& other ) const noexcept { return d < other.d; }
        inline bool operator<=( const const_iterator& other ) const noexcept { return d <= other.d; }
        inline bool operator>( const const_iterator& other ) const noexcept { return d > other.d; }
        inline bool operator>=( const const_iterator& other ) const noexcept { return d >= other.d; }
        inline const_iterator& operator++() { ++d; return *this; }
        inline const_iterator operator++( int ) { const QgsFields::Field* n = d; ++d; return n; }
        inline const_iterator& operator--() { d--; return *this; }
        inline const_iterator operator--( int ) { const QgsFields::Field* n = d; --d; return n; }
        inline const_iterator& operator+=( difference_type j ) { d += j; return *this; }
        inline const_iterator& operator-=( difference_type j ) { d -= j; return *this; }
        inline const_iterator operator+( difference_type j ) const { return const_iterator( d + j ); }
        inline const_iterator operator-( difference_type j ) const { return const_iterator( d -j ); }
        inline int operator-( const_iterator j ) const { return int( d - j.d ); }
    };
    friend class const_iterator;
    ///@endcond


    /**
     * Returns a const STL-style iterator pointing to the first item in the list.
     *
     * @note added in 2.16
     * @note not available in Python bindings
     */
    const_iterator constBegin() const noexcept;

    /**
     * Returns a const STL-style iterator pointing to the imaginary item after the last item in the list.
     *
     * @note added in 2.16
     * @note not available in Python bindings
     */
    const_iterator constEnd() const noexcept;

    /**
     * Returns a const STL-style iterator pointing to the first item in the list.
     *
     * @note added in 2.16
     * @note not available in Python bindings
     */
    const_iterator begin() const noexcept;

    /**
     * Returns a const STL-style iterator pointing to the imaginary item after the last item in the list.
     *
     * @note added in 2.16
     * @note not available in Python bindings
     */
    const_iterator end() const noexcept;

    /**
     * Returns an STL-style iterator pointing to the first item in the list.
     *
     * @note added in 2.16
     * @note not available in Python bindings
     */
    iterator begin();


    /**
     * Returns an STL-style iterator pointing to the imaginary item after the last item in the list.
     *
     * @note added in 2.16
     * @note not available in Python bindings
     */
    iterator end();

  private:

    QSharedDataPointer<QgsFieldsPrivate> d;

};

Q_DECLARE_METATYPE( QgsFields )

/** Writes the fields to stream out. QGIS version compatibility is not guaranteed. */
CORE_EXPORT QDataStream& operator<<( QDataStream& out, const QgsFields& fields );
/** Reads fields from stream in into fields. QGIS version compatibility is not guaranteed. */
CORE_EXPORT QDataStream& operator>>( QDataStream& in, QgsFields& fields );

#endif
