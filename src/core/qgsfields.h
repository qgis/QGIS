/***************************************************************************
  qgsfields.h - QgsFields

 ---------------------
 begin                : 22.9.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFIELDS_H
#define QGSFIELDS_H


#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgis.h"
#include "qgsfield.h"

class QgsFieldsPrivate;

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfields.cpp.
 * See details in QEP #17
 ****************************************************************************/

/**
 * \class QgsFields
 * \ingroup core
 * \brief Container of fields for a vector layer.
 *
 * In addition to storing a list of QgsField instances, it also:
 *
 * - allows quick lookups of field names to index in the list
 * - keeps track of where the field definition comes from (vector data provider, joined layer or newly added from an editing operation)
 *
 * \note QgsFields objects are implicitly shared.
 */
class CORE_EXPORT QgsFields
{
    Q_GADGET

    Q_PROPERTY( bool isEmpty READ isEmpty )
    Q_PROPERTY( int count READ count )
    Q_PROPERTY( QStringList names READ names )

  public:

#ifndef SIP_RUN

    typedef struct Field
    {
      Field()
      {}

      Field( const QgsField &f, Qgis::FieldOrigin o, int oi )
        : field( f )
        , origin( o )
        , originIndex( oi )
      {}

      // TODO c++20 - replace with = default

      bool operator==( const Field &other ) const { return field == other.field && origin == other.origin && originIndex == other.originIndex; }
      bool operator!=( const Field &other ) const { return !( *this == other ); }

      QgsField field;      //!< Field
      Qgis::FieldOrigin origin = Qgis::FieldOrigin::Unknown ;  //!< Origin of the field
      int originIndex = -1 ;     //!< Index specific to the origin
    } Field;

#endif

    /**
     * Constructor for an empty field container
     */
    QgsFields() SIP_HOLDGIL;

    QgsFields( const QgsFields &other ) SIP_HOLDGIL;
    QgsFields &operator =( const QgsFields &other ) SIP_SKIP;

    /**
     * Construct QgsFields from a list of \a fields.
     *
     * \since QGIS 3.40
     */
    QgsFields( const QList< QgsField > &fields ) SIP_HOLDGIL;

    virtual ~QgsFields();

    //! Removes all fields
    void clear() SIP_HOLDGIL;

    /**
     * Appends a \a field.
     *
     * The field must have a unique name, otherwise it is rejected (returns FALSE).
     *
     * The \a originIndex argument must be set to a value corresponding to the \a origin type:
     *
     * - Qgis::FieldOrigin::Provider: The field's originIndex is the index in provider's fields.
     * - Qgis::FieldOrigin::Join: The field's originIndex / 1000 = index of the join, originIndex % 1000 = index within the join
     * - Qgis::FieldOrigin::Edit: The originIndex is the index in the list of added attributes
     */
    bool append( const QgsField &field, Qgis::FieldOrigin origin = Qgis::FieldOrigin::Provider, int originIndex = -1 ) SIP_HOLDGIL;

    /**
     * Appends a list of \a fields.
     *
     * The fields must have unique names, otherwise it is rejected (returns FALSE).
     *
     * \since QGIS 3.40
     */
    bool append( const QList< QgsField > &fields, Qgis::FieldOrigin origin = Qgis::FieldOrigin::Provider ) SIP_HOLDGIL;

    /**
     * Appends another set of \a fields to these fields.
     *
     * The fields must have unique names, otherwise it is rejected (returns FALSE).
     *
     * \since QGIS 3.40
     */
    bool append( const QgsFields &fields ) SIP_HOLDGIL;

    /**
     * Renames a name of field. The field must have unique name, otherwise change is rejected (returns FALSE)
     * \since QGIS 3.6
     */
    bool rename( int fieldIdx, const QString &name ) SIP_HOLDGIL;

    //! Appends an expression field. The field must have unique name, otherwise it is rejected (returns FALSE)
    bool appendExpressionField( const QgsField &field, int originIndex ) SIP_HOLDGIL;

#ifndef SIP_RUN

    /**
     * Removes the field with the given index.
     */
    void remove( int fieldIdx );
#else

    /**
     * Removes the field with the given index.
     *
     * \throws KeyError if no field with the specified index exists
     */
    void remove( int fieldIdx ) SIP_HOLDGIL;
    % MethodCode
    if ( a0 < 0 || a0 >= sipCpp->count() )
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      sipCpp->remove( a0 );
    }
    % End
#endif

    //! Extends with fields from another QgsFields container
    void extend( const QgsFields &other ) SIP_HOLDGIL;

    //! Checks whether the container is empty
    bool isEmpty() const SIP_HOLDGIL;

    //! Returns number of items
    int count() const SIP_HOLDGIL;

#ifdef SIP_RUN
    int __len__() const SIP_HOLDGIL;
    % MethodCode
    sipRes = sipCpp->count();
    % End

    //! Ensures that bool(obj) returns TRUE (otherwise __len__() would be used)
    int __bool__() const SIP_HOLDGIL;
    % MethodCode
    sipRes = true;
    % End
#endif

    //! Returns number of items
    int size() const SIP_HOLDGIL;

    /**
     * Returns a list with field names
     */
    QStringList names() const SIP_HOLDGIL;

    /**
     * Returns if a field index is valid
     * \param i  Index of the field which needs to be checked
     * \returns   TRUE if the field exists
     */
    Q_INVOKABLE bool exists( int i ) const SIP_HOLDGIL;

#ifndef SIP_RUN
    //! Gets field at particular index (must be in range 0..N-1)
    QgsField operator[]( int i ) const;
#endif

    //! Gets field at particular index (must be in range 0..N-1)
    QgsField &operator[]( int i ) SIP_HOLDGIL SIP_FACTORY;
#ifdef SIP_RUN
    % MethodCode
    SIP_SSIZE_T idx = sipConvertFromSequenceIndex( a0, sipCpp->count() );
    if ( idx < 0 )
      sipIsErr = 1;
    else
      sipRes = new QgsField( sipCpp->operator[]( idx ) );
    % End
#endif

#ifdef SIP_RUN
    SIP_PYOBJECT __getitem__( const QString &name ) const SIP_HOLDGIL SIP_TYPEHINT( QgsField );
    % MethodCode
    const int fieldIdx = sipCpp->lookupField( *a0 );
    if ( fieldIdx == -1 )
    {
      PyErr_SetString( PyExc_KeyError, a0->toLatin1() );
      sipIsErr = 1;
    }
    else
    {
      sipRes = sipConvertFromType( new QgsField( sipCpp->at( fieldIdx ) ), sipType_QgsField, Py_None );
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Returns the field at particular index (must be in range 0..N-1).
     */
    QgsField at( int i ) const SIP_FACTORY;
#else

    /**
     * Returns the field at particular index (must be in range 0..N-1).
     * \throws KeyError if no field exists at the specified index
     */
    QgsField at( int i ) const SIP_HOLDGIL SIP_FACTORY;
    % MethodCode
    if ( a0 < 0 || a0 >= sipCpp->count() )
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      sipRes = new QgsField( sipCpp->at( a0 ) );
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Returns the field at particular index (must be in range 0..N-1).
     */
    QgsField field( int fieldIdx ) const SIP_FACTORY;
#else

    /**
     * Returns the field at particular index (must be in range 0..N-1).
     * \throws KeyError if no field exists at the specified index
     */
    QgsField field( int fieldIdx ) const SIP_HOLDGIL SIP_FACTORY;
    % MethodCode
    if ( a0 < 0 || a0 >= sipCpp->count() )
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      sipRes = new QgsField( sipCpp->field( a0 ) );
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Returns the field with matching name.
     */
    QgsField field( const QString &name ) const SIP_FACTORY;
#else

    /**
     * Returns the field with matching name.
     * \throws KeyError if no matching field was found.
     */
    QgsField field( const QString &name ) const SIP_HOLDGIL SIP_FACTORY;
    % MethodCode
    int fieldIdx = sipCpp->indexFromName( *a0 );
    if ( fieldIdx == -1 )
    {
      PyErr_SetString( PyExc_KeyError, a0->toLatin1() );
      sipIsErr = 1;
    }
    else
    {
      sipRes = new QgsField( sipCpp->field( *a0 ) );
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Returns the field's origin (value from an enumeration).
     */
    Qgis::FieldOrigin fieldOrigin( int fieldIdx ) const;
#else

    /**
     * Returns the field's origin (value from an enumeration).
     *
     * \throws KeyError if no field exists at the specified index
     */
    Qgis::FieldOrigin fieldOrigin( int fieldIdx ) const SIP_HOLDGIL;
    % MethodCode
    if ( a0 < 0 || a0 >= sipCpp->count() )
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      sipRes = sipCpp->fieldOrigin( a0 );
    }
    % End
#endif

#ifndef SIP_RUN

    /**
     * Returns the field's origin index (its meaning is specific to each type of origin).
     *
     * - Qgis::FieldOrigin::Provider: The field's originIndex is the index in provider's fields.
     * - Qgis::FieldOrigin::Join: The field's originIndex / 1000 = index of the join, originIndex % 1000 = index within the join
     * - Qgis::FieldOrigin::Edit: The originIndex is the index in the list of added attributes
     */
    int fieldOriginIndex( int fieldIdx ) const;
#else

    /**
     * Returns the field's origin index (its meaning is specific to each type of origin).
     *
     * - Qgis::FieldOrigin::Provider: The field's originIndex is the index in provider's fields.
     * - Qgis::FieldOrigin::Join: The field's originIndex / 1000 = index of the join, originIndex % 1000 = index within the join
     * - Qgis::FieldOrigin::Edit: The originIndex is the index in the list of added attributes
     *
     * \throws KeyError if no field exists at the specified index
     */
    int fieldOriginIndex( int fieldIdx ) const SIP_HOLDGIL;
    % MethodCode
    if ( a0 < 0 || a0 >= sipCpp->count() )
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      sipRes = sipCpp->fieldOriginIndex( a0 );
    }
    % End
#endif

    /**
     * Gets the field index from the field name.
     *
     * This method is case sensitive and only matches the data source
     * name of the field.
     *
     * Alias for indexOf
     *
     * \param fieldName The name of the field.
     *
     * \returns The field index if found or -1 in case it cannot be found.
     * \see lookupField For a more tolerant alternative.
     */
    Q_INVOKABLE int indexFromName( const QString &fieldName ) const SIP_HOLDGIL;

    /**
     * Gets the field index from the field name.
     *
     * This method is case sensitive and only matches the data source
     * name of the field.
     *
     * \param fieldName The name of the field.
     *
     * \returns The field index if found or -1 in case it cannot be found.
     * \see lookupField For a more tolerant alternative.
     */
    Q_INVOKABLE int indexOf( const QString &fieldName ) const SIP_HOLDGIL;

    /**
     * Looks up field's index from the field name.
     * This method matches in the following order:
     *
     * 1. The exact field name taking case sensitivity into account
     * 2. Looks for the field name by case insensitive comparison
     * 3. The field alias (case insensitive)
     *
     * \param fieldName The name to look for.
     *
     * \returns The field index if found or -1 in case it cannot be found.
     * \see indexFromName For a more performant and precise but less tolerant alternative.
     */
    Q_INVOKABLE  int lookupField( const QString &fieldName ) const SIP_HOLDGIL;

    /**
     * Utility function to get list of attribute indexes
     */
    QgsAttributeList allAttributesList() const SIP_HOLDGIL;

    //! Utility function to return a list of QgsField instances
    QList<QgsField> toList() const SIP_HOLDGIL;

    bool operator==( const QgsFields &other ) const SIP_HOLDGIL;
    bool operator!=( const QgsFields &other ) const  SIP_HOLDGIL { return !( *this == other ); }

#ifndef SIP_RUN

    /**
     * Returns an icon corresponding to a field index, based on the field's type and source
     * \param fieldIdx the field index
     * \param considerOrigin if TRUE the icon will the origin of the field
     */
    QIcon iconForField( int fieldIdx, bool considerOrigin = false ) const SIP_FACTORY;
#else

    /**
     * Returns an icon corresponding to a field index, based on the field's type and source
     * \param fieldIdx the field index
     * \param considerOrigin if TRUE the icon will the origin of the field
     * \throws KeyError if no field exists at the specified index
     */
    QIcon iconForField( int fieldIdx, bool considerOrigin = false ) const SIP_FACTORY;
    % MethodCode
    if ( a0 < 0 || a0 >= sipCpp->count() )
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      sipRes = new QIcon( sipCpp->iconForField( a0 ) );
    }
    % End
#endif

    /**
     * Returns an icon corresponding to a field \a type
     *
     * Since QGIS 3.24, the optional \a subType argument can be used to specify the type of variant list or map values.
     * Since QGIS 3.30, the optional \a typeString argument can be used to specify field type strings for handling user field types.
     *
     * \since QGIS 3.16
     */
    static QIcon iconForFieldType( QMetaType::Type type, QMetaType::Type subType = QMetaType::Type::UnknownType, const QString &typeString = QString() );

    /**
     * Returns an icon corresponding to a field \a type
     *
     * Since QGIS 3.24, the optional \a subType argument can be used to specify the type of variant list or map values.
     * Since QGIS 3.30, the optional \a typeString argument can be used to specify field type strings for handling user field types.
     *
     * \since QGIS 3.16
     * \deprecated QGIS 3.38. Use the method with a QMetaType::Type argument instead.
     */
    Q_DECL_DEPRECATED static QIcon iconForFieldType( QVariant::Type type, QVariant::Type subType, const QString &typeString = QString() ) SIP_DEPRECATED;

    //! Allows direct construction of QVariants from fields.
    operator QVariant() const
    {
      return QVariant::fromValue( *this );
    }

#ifdef SIP_RUN

    void __setitem__( int key, const QgsField &field ) SIP_HOLDGIL;
    % MethodCode
    int idx = ( int )sipConvertFromSequenceIndex( a0, sipCpp->count() );
    if ( idx < 0 )
      sipIsErr = 1;
    else
      ( *sipCpp )[idx] = *a1;
    % End

#endif

#ifndef SIP_RUN

    ///@cond PRIVATE

    class const_iterator;

    class iterator
    {
      public:
        QgsFields::Field *d = nullptr;
        typedef std::random_access_iterator_tag  iterator_category;
        typedef qptrdiff difference_type;

        inline iterator()
        {}

        inline iterator( QgsFields::Field *n )
          : d( n )
        {}

        inline QgsField &operator*() const { return d->field; }
        inline QgsField *operator->() const { return &d->field; }
        inline QgsField &operator[]( difference_type j ) const { return d[j].field; }
        inline bool operator==( const iterator &o ) const noexcept { return d == o.d; } // clazy:exclude=function-args-by-value
        inline bool operator!=( const iterator &o ) const noexcept { return d != o.d; } // clazy:exclude=function-args-by-value
        inline bool operator<( const iterator &other ) const noexcept { return d < other.d; } // clazy:exclude=function-args-by-value
        inline bool operator<=( const iterator &other ) const noexcept { return d <= other.d; } // clazy:exclude=function-args-by-value
        inline bool operator>( const iterator &other ) const noexcept { return d > other.d; } // clazy:exclude=function-args-by-value
        inline bool operator>=( const iterator &other ) const noexcept { return d >= other.d; } // clazy:exclude=function-args-by-value

        inline iterator &operator++() { ++d; return *this; }
        inline iterator operator++( int ) { QgsFields::Field *n = d; ++d; return n; }
        inline iterator &operator--() { d--; return *this; }
        inline iterator operator--( int ) { QgsFields::Field *n = d; d--; return n; }
        inline iterator &operator+=( difference_type j ) { d += j; return *this; }
        inline iterator &operator-=( difference_type j ) { d -= j; return *this; }
        inline iterator operator+( difference_type j ) const { return iterator( d + j ); }
        inline iterator operator-( difference_type j ) const { return iterator( d - j ); }
        inline int operator-( iterator j ) const { return int( d - j.d ); }
    };
    friend class iterator;

    class const_iterator // clazy:exclude=rule-of-three
    {
      public:
        const QgsFields::Field *d = nullptr;

        typedef std::random_access_iterator_tag  iterator_category;
        typedef qptrdiff difference_type;

        inline const_iterator()
        {}

        inline const_iterator( const QgsFields::Field *f )
          : d( f ) {}
        inline const_iterator( const const_iterator &o )
          : d( o.d ) {}
        inline explicit const_iterator( const iterator &o ) // clazy:exclude=function-args-by-value
          : d( o.d ) {}
        inline const QgsField &operator*() const { return d->field; }
        inline const QgsField *operator->() const { return &d->field; }
        inline const QgsField &operator[]( difference_type j ) const noexcept { return d[j].field; }
        inline bool operator==( const const_iterator &o ) const noexcept { return d == o.d; }
        inline bool operator!=( const const_iterator &o ) const noexcept { return d != o.d; }
        inline bool operator<( const const_iterator &other ) const noexcept { return d < other.d; }
        inline bool operator<=( const const_iterator &other ) const noexcept { return d <= other.d; }
        inline bool operator>( const const_iterator &other ) const noexcept { return d > other.d; }
        inline bool operator>=( const const_iterator &other ) const noexcept { return d >= other.d; }
        inline const_iterator &operator++() { ++d; return *this; }
        inline const_iterator operator++( int ) { const QgsFields::Field *n = d; ++d; return n; }
        inline const_iterator &operator--() { d--; return *this; }
        inline const_iterator operator--( int ) { const QgsFields::Field *n = d; --d; return n; }
        inline const_iterator &operator+=( difference_type j ) { d += j; return *this; }
        inline const_iterator &operator-=( difference_type j ) { d -= j; return *this; }
        inline const_iterator operator+( difference_type j ) const { return const_iterator( d + j ); }
        inline const_iterator operator-( difference_type j ) const { return const_iterator( d - j ); }
        inline int operator-( const_iterator j ) const { return int( d - j.d ); } // clazy:exclude=function-args-by-ref
      private:
        const_iterator &operator= ( const const_iterator & ) = delete;
    };
    friend class const_iterator;
    ///@endcond


    /**
     * Returns a const STL-style iterator pointing to the first item in the list.
     *
     * \note not available in Python bindings
     */
    const_iterator constBegin() const noexcept;

    /**
     * Returns a const STL-style iterator pointing to the imaginary item after the last item in the list.
     *
     * \note not available in Python bindings
     */
    const_iterator constEnd() const noexcept;

    /**
     * Returns a const STL-style iterator pointing to the first item in the list.
     *
     * \note not available in Python bindings
     */
    const_iterator begin() const noexcept;

    /**
     * Returns a const STL-style iterator pointing to the imaginary item after the last item in the list.
     *
     * \note not available in Python bindings
     */
    const_iterator end() const noexcept;

    /**
     * Returns an STL-style iterator pointing to the first item in the list.
     *
     * \note not available in Python bindings
     */
    iterator begin();


    /**
     * Returns an STL-style iterator pointing to the imaginary item after the last item in the list.
     *
     * \note not available in Python bindings
     */
    iterator end();

#endif

  private:

    QSharedDataPointer<QgsFieldsPrivate> d;

};

Q_DECLARE_METATYPE( QgsFields )

//! Writes the fields to stream out. QGIS version compatibility is not guaranteed.
CORE_EXPORT QDataStream &operator<<( QDataStream &out, const QgsFields &fields );
//! Reads fields from stream in into fields. QGIS version compatibility is not guaranteed.
CORE_EXPORT QDataStream &operator>>( QDataStream &in, QgsFields &fields );

#endif // QGSFIELDS_H
