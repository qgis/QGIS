/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfield.h                                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsField
{
%Docstring
Encapsulate a field in an attribute table or data source.
QgsField stores metadata about an attribute field, including name, type
length, and if applicable, precision.

.. note::

   QgsField objects are implicitly shared.
%End

%TypeHeaderCode
#include "qgsfield.h"
%End
  public:

    QgsField( const QString &name = QString(),
              QVariant::Type type = QVariant::Invalid,
              const QString &typeName = QString(),
              int len = 0,
              int prec = 0,
              const QString &comment = QString(),
              QVariant::Type subType = QVariant::Invalid );
%Docstring
Constructor. Constructs a new QgsField object.

:param name: Field name
:param type: Field variant type, currently supported: String / Int / Double
:param typeName: Field type (e.g., char, varchar, text, int, serial, double).
Field types are usually unique to the source and are stored exactly
as returned from the data store.
:param len: Field length
:param prec: Field precision. Usually decimal places but may also be
used in conjunction with other fields types (e.g., variable character fields)
:param comment: Comment for the field
:param subType: If the field is a collection, its element's type. When
all the elements don't need to have the same type, leave
this to QVariant.Invalid.
%End

    QgsField( const QgsField &other );
%Docstring
Copy constructor
%End


    virtual ~QgsField();

    bool operator==( const QgsField &other ) const;
    bool operator!=( const QgsField &other ) const;

    QString name() const;
%Docstring
Returns the name of the field.

.. seealso:: :py:func:`setName`

.. seealso:: :py:func:`displayName`
%End

    QString displayName() const;
%Docstring
Returns the name to use when displaying this field. This will be the
field alias if set, otherwise the field name.

.. seealso:: :py:func:`name`

.. seealso:: :py:func:`alias`

.. versionadded:: 3.0
%End

    QVariant::Type type() const;
%Docstring
Gets variant type of the field as it will be retrieved from data source
%End

    QVariant::Type subType() const;
%Docstring
If the field is a collection, gets its element's type.
When all the elements don't need to have the same type, this returns
QVariant.Invalid.

.. versionadded:: 3.0
%End

    QString typeName() const;
%Docstring
Gets the field type. Field types vary depending on the data source. Examples
are char, int, double, blob, geometry, etc. The type is stored exactly as
the data store reports it, with no attempt to standardize the value.

:return: QString containing the field type
%End

    int length() const;
%Docstring
Gets the length of the field.

:return: int containing the length of the field
%End

    int precision() const;
%Docstring
Gets the precision of the field. Not all field types have a related precision.

:return: int containing the precision or zero if not applicable to the field type.
%End

    QString comment() const;
%Docstring
Returns the field comment
%End

    bool isNumeric() const;
%Docstring
Returns if this field is numeric. Any integer or floating point type
will return true for this.

.. versionadded:: 2.18
%End

    void setName( const QString &name );
%Docstring
Set the field name.

:param name: Name of the field
%End

    void setType( QVariant::Type type );
%Docstring
Set variant type.
%End

    void setSubType( QVariant::Type subType );
%Docstring
If the field is a collection, set its element's type.
When all the elements don't need to have the same type, set this to
QVariant.Invalid.

.. versionadded:: 3.0
%End

    void setTypeName( const QString &typeName );
%Docstring
Set the field type.

:param typeName: Field type
%End

    void setLength( int len );
%Docstring
Set the field length.

:param len: Length of the field
%End

    void setPrecision( int precision );
%Docstring
Set the field precision.

:param precision: Precision of the field
%End

    void setComment( const QString &comment );
%Docstring
Set the field comment
%End

    QgsDefaultValue defaultValueDefinition() const;
%Docstring
Returns the expression used when calculating the default value for the field.

:return: expression evaluated when calculating default values for field, or an
empty string if no default is set

.. versionadded:: 3.0

.. seealso:: :py:func:`setDefaultValueDefinition`
%End

    void setDefaultValueDefinition( const QgsDefaultValue &defaultValueDefinition );
%Docstring
Sets an expression to use when calculating the default value for the field.

:param defaultValueDefinition: expression to evaluate when calculating default values for field. Pass
a default constructed :py:class:`QgsDefaultValue`() to reset.

.. versionadded:: 3.0

.. seealso:: :py:func:`defaultValueDefinition`
%End

    const QgsFieldConstraints &constraints() const;
%Docstring
Returns constraints which are present for the field.

.. versionadded:: 3.0

.. seealso:: :py:func:`setConstraints`
%End

    void setConstraints( const QgsFieldConstraints &constraints );
%Docstring
Sets constraints which are present for the field.

.. versionadded:: 3.0

.. seealso:: :py:func:`constraints`
%End

    QString alias() const;
%Docstring
Returns the alias for the field (the friendly displayed name of the field ),
or an empty string if there is no alias.

.. seealso:: :py:func:`setAlias`

.. versionadded:: 3.0
%End

    void setAlias( const QString &alias );
%Docstring
Sets the alias for the field (the friendly displayed name of the field ).

:param alias: field alias, or empty string to remove an existing alias

.. seealso:: :py:func:`alias`

.. versionadded:: 3.0
%End

    QString displayString( const QVariant &v ) const;
%Docstring
Formats string for display
%End

    bool convertCompatible( QVariant &v ) const;
%Docstring
Converts the provided variant to a compatible format

:param v:  The value to convert

:return:   True if the conversion was successful
%End
%MethodCode
    PyObject *sipParseErr = NULL;

    {
      QVariant *a0;
      int a0State = 0;
      const QgsField *sipCpp;

      if ( sipParseArgs( &sipParseErr, sipArgs, "BJ1", &sipSelf, sipType_QgsField, &sipCpp, sipType_QVariant, &a0, &a0State ) )
      {
        bool sipRes;

        Py_BEGIN_ALLOW_THREADS
        try
        {
          QgsDebugMsg( a0->toString() );
          sipRes = sipCpp->convertCompatible( *a0 );
          QgsDebugMsg( a0->toString() );
        }
        catch ( ... )
        {
          Py_BLOCK_THREADS

          sipReleaseType( a0, sipType_QVariant, a0State );
          sipRaiseUnknownException();
          return NULL;
        }

        Py_END_ALLOW_THREADS

        PyObject *res = sipConvertFromType( a0, sipType_QVariant, NULL );
        sipReleaseType( a0, sipType_QVariant, a0State );

        if ( !sipRes )
        {
          PyErr_SetString( PyExc_ValueError,
                           QString( "Value %1 (%2) could not be converted to field type %3." ).arg( a0->toString(), a0->typeName() ).arg( sipCpp->type() ).toUtf8().constData() );
          sipError = sipErrorFail;
        }

        return res;
      }
    }

    // Raise an exception if the arguments couldn't be parsed.
    sipNoMethod( sipParseErr, sipName_QgsField, sipName_convertCompatible, doc_QgsField_convertCompatible );

    return nullptr;
%End

    operator QVariant() const;

    void setEditorWidgetSetup( const QgsEditorWidgetSetup &v );
%Docstring
Set the editor widget setup for the field.

:param v:  The value to set
%End

    QgsEditorWidgetSetup editorWidgetSetup() const;
%Docstring
Get the editor widget setup for the field.

Defaults may be set by the provider and can be overridden
by manual field configuration.

:return: the value
%End

}; // class QgsField




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfield.h                                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
