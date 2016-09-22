/** \class QgsFields
 * \ingroup core
 * Container of fields for a vector layer.
 *
 * In addition to storing a list of QgsField instances, it also:
 * - allows quick lookups of field names to index in the list
 * - keeps track of where the field definition comes from (vector data provider, joined layer or newly added from an editing operation)
 * \note QgsFields objects are implicitly shared.
 */

class QgsFields
{
%TypeHeaderCode
#include <qgsfield.h>
%End
  public:

    enum FieldOrigin
    {
      OriginUnknown,   //!< it has not been specified where the field comes from
      OriginProvider,  //!< field comes from the underlying data provider of the vector layer  (originIndex = index in provider's fields)
      OriginJoin,      //!< field comes from a joined layer   (originIndex / 1000 = index of the join, originIndex % 1000 = index within the join)
      OriginEdit,      //!< field has been temporarily added in editing mode (originIndex = index in the list of added attributes)
      OriginExpression //!< field is calculated from an expression
    };

    /** Constructor for an empty field container
     */
    QgsFields();

    /** Copy constructor
     */
    QgsFields( const QgsFields& other );

    virtual ~QgsFields();

    //! Remove all fields
    void clear();
    //! Append a field. The field must have unique name, otherwise it is rejected (returns false)
    bool append( const QgsField& field, FieldOrigin origin = OriginProvider, int originIndex = -1 );
    //! Append an expression field. The field must have unique name, otherwise it is rejected (returns false)
    bool appendExpressionField( const QgsField& field, int originIndex );
    //! Remove a field with the given index
    void remove( int fieldIdx );
%MethodCode
  if ( a0 < 0 || a0 >= sipCpp->count() )
  {
    PyErr_SetString(PyExc_KeyError, QByteArray::number(a0));
    sipIsErr = 1;
  }
  else
  {
    sipCpp->remove( a0 );
  }
%End

    //! Extend with fields from another QgsFields container
    void extend( const QgsFields& other );

    //! Check whether the container is empty
    bool isEmpty() const;
    //! Return number of items
    int count() const;
    // __len__ annotation since sip 4.10.3
    //int count() const /__len__/;
    int __len__() const;
%MethodCode
  sipRes = sipCpp->count();
%End
    //! Return number of items
    int size() const;
    //! Return if a field index is valid
    //! @param i  Index of the field which needs to be checked
    //! @return   True if the field exists
    bool exists( int i ) const;

    //! Get field at particular index (must be in range 0..N-1)
    // const QgsField& operator[]( int i ) const;
    QgsField& operator[](int i) /Factory/;
%MethodCode
  SIP_SSIZE_T idx = sipConvertFromSequenceIndex(a0, sipCpp->count());
  if (idx < 0)
    sipIsErr = 1;
  else
    sipRes = new QgsField(sipCpp->operator[](idx));
%End

    //! Get field at particular index (must be in range 0..N-1)
    QgsField at( int i ) const /Factory/;
%MethodCode
  if ( a0 < 0 || a0 >= sipCpp->count() )
  {
    PyErr_SetString(PyExc_KeyError, QByteArray::number(a0));
    sipIsErr = 1;
  }
  else
  {
    sipRes = new QgsField( sipCpp->at( a0 ) );
  }
%End

    //! Get field at particular index (must be in range 0..N-1)
    QgsField field( int fieldIdx ) const /Factory/;
%MethodCode
  if ( a0 < 0 || a0 >= sipCpp->count() )
  {
    PyErr_SetString(PyExc_KeyError, QByteArray::number(a0));
    sipIsErr = 1;
  }
  else
  {
    sipRes = new QgsField( sipCpp->field( a0 ) );
  }
%End

    //! Get field at particular index (must be in range 0..N-1)
    QgsField field( const QString& name ) const /Factory/;
%MethodCode
  int fieldIdx = sipCpp->indexFromName(*a0);
  if (fieldIdx == -1)
  {
    PyErr_SetString(PyExc_KeyError, a0->toAscii());
    sipIsErr = 1;
  }
  else
  {
    sipRes = new QgsField( sipCpp->field( *a0 ) );
  }
%End

    //! Get field's origin (value from an enumeration)
    FieldOrigin fieldOrigin( int fieldIdx ) const;
%MethodCode
  if ( a0 < 0 || a0 >= sipCpp->count() )
  {
    PyErr_SetString(PyExc_KeyError, QByteArray::number(a0));
    sipIsErr = 1;
  }
  else
  {
    sipRes = sipCpp->fieldOrigin( a0 );
  }
%End

    //! Get field's origin index (its meaning is specific to each type of origin)
    int fieldOriginIndex( int fieldIdx ) const;
%MethodCode
  if ( a0 < 0 || a0 >= sipCpp->count() )
  {
    PyErr_SetString(PyExc_KeyError, QByteArray::number(a0));
    sipIsErr = 1;
  }
  else
  {
    sipRes = sipCpp->fieldOriginIndex( a0 );
  }
%End

    /**
     * Get the field index from the field name.
     * This method takes is case sensitive and only matches the data source
     * name of the field.
     * Alias for indexOf
     *
     * @param fieldName The name of the field.
     *
     * @return The field index if found or -1 in case it cannot be found.
     * @see lookupField For a more tolerant alternative.
     */
    int indexFromName( const QString& fieldName ) const;

    /**
     * Get the field index from the field name.
     * This method takes is case sensitive and only matches the data source
     * name of the field.
     *
     * @param fieldName The name of the field.
     *
     * @return The field index if found or -1 in case it cannot be found.
     * @see lookupField For a more tolerant alternative.
     * @note Added in QGIS 3.0
     */
    int indexOf( const QString& fieldName ) const;

    /**
     * Look up field's index from the field name.
     * This method matches in the following order:
     *
     *  1. The exact field name taking case sensitivity into account
     *  2. Looks for the field name by case insensitive comparison
     *  3. The field alias (case insensitive)
     *
     * @param fieldName The name to look for.
     *
     * @return The field index if found or -1 in case it cannot be found.
     * @see indexFromName For a more performant and precise but less tolerant alternative.
     * @note added in 2.4
     */
    int lookupField( const QString& fieldName ) const;

    //! Utility function to get list of attribute indexes
    //! @note added in 2.4
    QgsAttributeList allAttributesList() const;

    //! Utility function to return a list of QgsField instances
    QList<QgsField> toList() const;

    //! @note added in 2.6
    bool operator==( const QgsFields& other ) const;
    //! @note added in 2.6
    bool operator!=( const QgsFields& other ) const;

    /** Returns an icon corresponding to a field index, based on the field's type and source
     * @note added in QGIS 2.14
     */
    QIcon iconForField( int fieldIdx ) const /Factory/;
    %MethodCode
      if ( a0 < 0 || a0 >= sipCpp->count() )
      {
        PyErr_SetString(PyExc_KeyError, QByteArray::number(a0));
        sipIsErr = 1;
      }
      else
      {
        sipRes = new QIcon( sipCpp->iconForField( a0 ) );
      }
    %End

    //! Allows direct construction of QVariants from fields.
    operator QVariant() const;

/*  SIP_PYOBJECT __getitem__(int key);
%MethodCode
  if (a0 = sipConvertFromSequenceIndex(a0, sipCpp->count()) < 0)
    sipIsErr = 1;
  else
  {
    qDebug("__getitem__ %d", a0);
    QgsField* fld = new QgsField(sipCpp->at(a0));
    sipRes = sipConvertFromType(fld, sipType_QgsField, Py_None);
  }
%End*/

void __setitem__(int key, const QgsField& field);
%MethodCode
  int idx = (int)sipConvertFromSequenceIndex(a0, sipCpp->count());
  if (idx < 0)
    sipIsErr = 1;
  else
    (*sipCpp)[idx] = *a1;
%End

};
