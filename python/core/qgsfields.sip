/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfields.h                                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsFields
{
%Docstring
 Container of fields for a vector layer.

 In addition to storing a list of QgsField instances, it also:
 - allows quick lookups of field names to index in the list
 - keeps track of where the field definition comes from (vector data provider, joined layer or newly added from an editing operation)
.. note::

   QgsFields objects are implicitly shared.
%End

%TypeHeaderCode
#include "qgsfields.h"
%End
  public:

    enum FieldOrigin
    {
      OriginUnknown,
      OriginProvider,
      OriginJoin,
      OriginEdit,
      OriginExpression
    };


    QgsFields();
%Docstring
 Constructor for an empty field container
%End

    QgsFields( const QgsFields &other );
%Docstring
 Copy constructor
%End


    virtual ~QgsFields();

    void clear();
%Docstring
Remove all fields
%End

    bool append( const QgsField &field, FieldOrigin origin = OriginProvider, int originIndex = -1 );
%Docstring
Append a field. The field must have unique name, otherwise it is rejected (returns false)
 :rtype: bool
%End

    bool appendExpressionField( const QgsField &field, int originIndex );
%Docstring
Append an expression field. The field must have unique name, otherwise it is rejected (returns false)
 :rtype: bool
%End

    void remove( int fieldIdx );
%Docstring
Remove a field with the given index
%End
%MethodCode
    if ( a0 < 0 || a0 >= sipCpp->count() )
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      sipCpp->remove( a0 );
    }
%End

    void extend( const QgsFields &other );
%Docstring
Extend with fields from another QgsFields container
%End

    bool isEmpty() const;
%Docstring
Check whether the container is empty
 :rtype: bool
%End

    int count() const;
%Docstring
Return number of items
 :rtype: int
%End

    int __len__() const;
%Docstring
 :rtype: int
%End
%MethodCode
    sipRes = sipCpp->count();
%End

    int size() const;
%Docstring
Return number of items
 :rtype: int
%End

    QStringList names() const;
%Docstring
 Returns a list with field names
.. versionadded:: 3.0
 :rtype: list of str
%End

    bool exists( int i ) const;
%Docstring
 Return if a field index is valid
 \param i  Index of the field which needs to be checked
 :return:   True if the field exists
 :rtype: bool
%End


    QgsField &operator[]( int i ) /Factory/;
%MethodCode
    SIP_SSIZE_T idx = sipConvertFromSequenceIndex( a0, sipCpp->count() );
    if ( idx < 0 )
      sipIsErr = 1;
    else
      sipRes = new QgsField( sipCpp->operator[]( idx ) );
%End

    QgsField at( int i ) const /Factory/;
%Docstring
Get field at particular index (must be in range 0..N-1)
 :rtype: QgsField
%End
%MethodCode
    if ( a0 < 0 || a0 >= sipCpp->count() )
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      sipRes = new QgsField( sipCpp->at( a0 ) );
    }
%End

    QgsField field( int fieldIdx ) const /Factory/;
%Docstring
Get field at particular index (must be in range 0..N-1)
 :rtype: QgsField
%End
%MethodCode
    if ( a0 < 0 || a0 >= sipCpp->count() )
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      sipRes = new QgsField( sipCpp->field( a0 ) );
    }
%End

    QgsField field( const QString &name ) const /Factory/;
%Docstring
Get field with matching name
 :rtype: QgsField
%End
%MethodCode
    int fieldIdx = sipCpp->indexFromName( *a0 );
    if ( fieldIdx == -1 )
    {
      PyErr_SetString( PyExc_KeyError, a0->toAscii() );
      sipIsErr = 1;
    }
    else
    {
      sipRes = new QgsField( sipCpp->field( *a0 ) );
    }
%End

    FieldOrigin fieldOrigin( int fieldIdx ) const;
%Docstring
Get field's origin (value from an enumeration)
 :rtype: FieldOrigin
%End
%MethodCode
    if ( a0 < 0 || a0 >= sipCpp->count() )
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      sipRes = sipCpp->fieldOrigin( a0 );
    }
%End

    int fieldOriginIndex( int fieldIdx ) const;
%Docstring
Get field's origin index (its meaning is specific to each type of origin)
 :rtype: int
%End
%MethodCode
    if ( a0 < 0 || a0 >= sipCpp->count() )
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      sipRes = sipCpp->fieldOriginIndex( a0 );
    }
%End

    int indexFromName( const QString &fieldName ) const;
%Docstring
 Get the field index from the field name.
 This method is case sensitive and only matches the data source
 name of the field.
 Alias for indexOf

 \param fieldName The name of the field.

 :return: The field index if found or -1 in case it cannot be found.
.. seealso:: lookupField For a more tolerant alternative.
 :rtype: int
%End

    int indexOf( const QString &fieldName ) const;
%Docstring
 Get the field index from the field name.
 This method is case sensitive and only matches the data source
 name of the field.

 \param fieldName The name of the field.

 :return: The field index if found or -1 in case it cannot be found.
.. seealso:: lookupField For a more tolerant alternative.
.. versionadded:: 3.0
 :rtype: int
%End

    int lookupField( const QString &fieldName ) const;
%Docstring
 Look up field's index from the field name.
 This method matches in the following order:

  1. The exact field name taking case sensitivity into account
  2. Looks for the field name by case insensitive comparison
  3. The field alias (case insensitive)

 \param fieldName The name to look for.

 :return: The field index if found or -1 in case it cannot be found.
.. seealso:: indexFromName For a more performant and precise but less tolerant alternative.
.. versionadded:: 2.4
 :rtype: int
%End

    QgsAttributeList allAttributesList() const;
%Docstring
 Utility function to get list of attribute indexes
.. versionadded:: 2.4
 :rtype: QgsAttributeList
%End

    QList<QgsField> toList() const;
%Docstring
Utility function to return a list of QgsField instances
 :rtype: list of QgsField
%End

    bool operator==( const QgsFields &other ) const;
    bool operator!=( const QgsFields &other ) const;
%Docstring
.. versionadded:: 2.6
 :rtype: bool
%End

    QIcon iconForField( int fieldIdx ) const /Factory/;
%Docstring
 Returns an icon corresponding to a field index, based on the field's type and source
.. versionadded:: 2.14
 :rtype: QIcon
%End
%MethodCode
    if ( a0 < 0 || a0 >= sipCpp->count() )
    {
      PyErr_SetString( PyExc_KeyError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      sipRes = new QIcon( sipCpp->iconForField( a0 ) );
    }
%End

    operator QVariant() const;
%Docstring
Allows direct construction of QVariants from fields.
%End


    void __setitem__( int key, const QgsField &field );
%MethodCode
    int idx = ( int )sipConvertFromSequenceIndex( a0, sipCpp->count() );
    if ( idx < 0 )
      sipIsErr = 1;
    else
      ( *sipCpp )[idx] = *a1;
%End



};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfields.h                                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
