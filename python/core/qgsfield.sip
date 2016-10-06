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
     * Field types are usually unique to the source and are stored exactly
     * as returned from the data store.
     * @param len Field length
     * @param prec Field precision. Usually decimal places but may also be
     * used in conjunction with other fields types (eg. variable character fields)
     * @param comment Comment for the field
     * @param subType If the field is a collection, its element's type. When
     *                all the elements don't need to have the same type, leave
     *                this to QVariant::Invalid.
     */
    QgsField( const QString& name = QString(),
              QVariant::Type type = QVariant::Invalid,
              const QString& typeName = QString(),
              int len = 0,
              int prec = 0,
              const QString& comment = QString(),
              QVariant::Type subType = QVariant::Invalid );

    /** Copy constructor
     */
    QgsField( const QgsField& other );

    //! Destructor
    virtual ~QgsField();

    bool operator==( const QgsField& other ) const;
    bool operator!=( const QgsField& other ) const;

    /** Returns the name of the field.
     * @see setName()
     * @see displayName()
     */
    QString name() const;

    /** Returns the name to use when displaying this field. This will be the
     * field alias if set, otherwise the field name.
     * @see name()
     * @see alias()
     * @note added in QGIS 3.0
     */
    QString displayName() const;

    //! Gets variant type of the field as it will be retrieved from data source
    QVariant::Type type() const;

    /**
     * If the field is a collection, gets its element's type.
     * When all the elements don't need to have the same type, this returns
     * QVariant::Invalid.
     * @note added in QGIS 3.0
     */
    QVariant::Type subType() const;

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
     * Returns if this field is numeric. Any integer or floating point type
     * will return true for this.
     *
     * @note added in QGIS 2.18
     */
    bool isNumeric() const;

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
     * If the field is a collection, set its element's type.
     * When all the elements don't need to have the same type, set this to
     * QVariant::Invalid.
     * @note added in QGIS 3.0
     */
    void setSubType( QVariant::Type subType );

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

    /** Returns the expression used when calculating the default value for the field.
     * @returns expression evaluated when calculating default values for field, or an
     * empty string if no default is set
     * @note added in QGIS 3.0
     * @see setDefaultValueExpression()
     */
    QString defaultValueExpression() const;

    /** Sets an expression to use when calculating the default value for the field.
     * @param expression expression to evaluate when calculating default values for field. Pass
     * an empty expression to clear the default.
     * @note added in QGIS 3.0
     * @see defaultValueExpression()
     */
    void setDefaultValueExpression( const QString& expression );

    /** Returns the alias for the field (the friendly displayed name of the field ),
     * or an empty string if there is no alias.
     * @see setAlias()
     * @note added in QGIS 3.0
     */
    QString alias() const;

    /** Sets the alias for the field (the friendly displayed name of the field ).
     * @param alias field alias, or empty string to remove an existing alias
     * @see alias()
     * @note added in QGIS 3.0
     */
    void setAlias( const QString& alias );

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
            QString( "Value %1 (%2) could not be converted to field type %3." ).arg( a0->toString(), a0->typeName() ).arg( sipCpp->type() ).toUtf8().constData() );
            sipError = sipErrorFail;
        }

        return res;
      }
    }

    /* Raise an exception if the arguments couldn't be parsed. */
    sipNoMethod(sipParseErr, sipName_QgsField, sipName_convertCompatible, doc_QgsField_convertCompatible);

    return nullptr;
%End

    //! Allows direct construction of QVariants from fields.
    operator QVariant() const;

    /**
     * Set the editor widget setup for the field.
     *
     * @param v  The value to set
     */
    void setEditorWidgetSetup( const QgsEditorWidgetSetup& v );

    /**
     * Get the editor widget setup for the field.
     *
     * @return the value
     */
    const QgsEditorWidgetSetup& editorWidgetSetup() const;
}; // class QgsField
