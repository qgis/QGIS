/** \class QgsField
  * \ingroup core
  * Encapsulate a field in an attribute table or data source.
  * QgsField stores metadata about an attribute field, including name, type
  * length, and if applicable, precision.
  * \note QgsField objects are implicitly shared.
 */

class QgsField
{

%TypeHeaderCode
#include <qgsfield.h>
%End

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

    /** Copy constructor
     */
    QgsField( const QgsField& other );

    //! Destructor
    virtual ~QgsField();

    bool operator==( const QgsField& other ) const;
    bool operator!=( const QgsField& other ) const;

    //! Gets the name of the field
    const QString& name() const;

    //! Gets variant type of the field as it will be retrieved from data source
    QVariant::Type type() const;

    /**
      Gets the field type. Field types vary depending on the data source. Examples
      are char, int, double, blob, geometry, etc. The type is stored exactly as
      the data store reports it, with no attempt to standardize the value.
      @return QString containing the field type
     */
    const QString& typeName() const;

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
    const QString& comment() const;

    /**
      Set the field name.
      @param name Name of the field
     */
    void setName( const QString& name );

    /**
      Set variant type.
     */
    void setType( QVariant::Type type );

    /**
      Set the field type.
      @param typeName Field type
     */
    void setTypeName( const QString& typeName );

    /**
      Set the field length.
      @param len Length of the field
     */
    void setLength( int len );

    /**
      Set the field precision.
      @param precision Precision of the field
     */
    void setPrecision( int precision );

    /**
      Set the field comment
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
    QVariant convertCompatible( QVariant& v ) const;
%MethodCode
    PyObject *sipParseErr = NULL;

    {
      QVariant * a0;
      int a0State = 0;
      const QgsField *sipCpp;

      if (sipParseArgs(&sipParseErr, sipArgs, "BJ1", &sipSelf, sipType_QgsField, &sipCpp, sipType_QVariant,&a0, &a0State))
      {
        bool sipRes;

        Py_BEGIN_ALLOW_THREADS
        try
        {
          QgsDebugMsg( a0->toString() );
          sipRes = sipCpp->convertCompatible(*a0);
          QgsDebugMsg( a0->toString() );
        }
        catch (...)
        {
          Py_BLOCK_THREADS

          sipReleaseType(a0,sipType_QVariant,a0State);
            sipRaiseUnknownException();
            return NULL;
        }

        Py_END_ALLOW_THREADS

        PyObject* res = sipConvertFromType( a0, sipType_QVariant, NULL );
        sipReleaseType(a0,sipType_QVariant,a0State);

        if ( !sipRes )
        {
          PyErr_SetString(PyExc_ValueError,
            QString( "Value %1 (%2) could not be converted to field type %3." ).arg( a0->toString() ).arg ( a0->typeName() ).arg( sipCpp->type() ).toUtf8().constData() );
            sipError = sipErrorFail;
        }

        return res;
      }
    }

    /* Raise an exception if the arguments couldn't be parsed. */
    sipNoMethod(sipParseErr, sipName_QgsField, sipName_convertCompatible, doc_QgsField_convertCompatible);

    return NULL;
%End
}; // class QgsField


/** \class QgsFields
 * \ingroup core
 * Container of fields for a vector layer.
 *
 * In addition to storing a list of QgsField instances, it also:
 * - allows quick lookups of field names to index in the list
 *- keeps track of where the field definition comes from (vector data provider, joined layer or newly added from an editing operation)
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
    bool operator==( const QgsFields& other ) const;
    //! @note added in 2.6
    bool operator!=( const QgsFields& other ) const;

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
