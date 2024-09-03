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
#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"

typedef QList<int> QgsAttributeList SIP_SKIP;

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsfield.cpp.
 * See details in QEP #17
 ****************************************************************************/

#include "qgseditorwidgetsetup.h"
#include "qgsfieldconstraints.h"
#include "qgsdefaultvalue.h"
#include "qgis.h"

class QgsFieldPrivate;

/**
 * \class QgsField
  * \ingroup core
  * \brief Encapsulate a field in an attribute table or data source.
  *
  * QgsField stores metadata about an attribute field, including name, type
  * length, and if applicable, precision.
  * \note QgsField objects are implicitly shared.
 */

class CORE_EXPORT QgsField
{
    Q_GADGET

    Q_PROPERTY( bool isNumeric READ isNumeric )
    Q_PROPERTY( bool isDateOrTime READ isDateOrTime )
    Q_PROPERTY( int length READ length WRITE setLength )
    Q_PROPERTY( int precision READ precision WRITE setPrecision )
    Q_PROPERTY( QMetaType::Type type READ type WRITE setType )
    Q_PROPERTY( QString comment READ comment WRITE setComment )
    Q_PROPERTY( QString name READ name WRITE setName )
    Q_PROPERTY( QString alias READ alias WRITE setAlias )
    Q_PROPERTY( QgsDefaultValue defaultValueDefinition READ defaultValueDefinition WRITE setDefaultValueDefinition )
    Q_PROPERTY( QgsFieldConstraints constraints READ constraints WRITE setConstraints )
    Q_PROPERTY( Qgis::FieldConfigurationFlags configurationFlags READ configurationFlags WRITE setConfigurationFlags )
    Q_PROPERTY( bool isReadOnly READ isReadOnly WRITE setReadOnly )

  public:

    /**
     * Constructor. Constructs a new QgsField object.
     * \param name Field name
     * \param type Field variant type, currently supported: String / Int / Double
     * \param typeName Field type (e.g., char, varchar, text, int, serial, double).
     * Field types are usually unique to the source and are stored exactly
     * as returned from the data store.
     * \param len Field length
     * \param prec Field precision. Usually decimal places but may also be
     * used in conjunction with other fields types (e.g., variable character fields)
     * \param comment Comment for the field
     * \param subType If the field is a collection, its element's type. When
     *                all the elements don't need to have the same type, leave
     *                this to QVariant::Invalid.
     */
    QgsField( const QString &name = QString(),
              QMetaType::Type type = QMetaType::Type::UnknownType,
              const QString &typeName = QString(),
              int len = 0,
              int prec = 0,
              const QString &comment = QString(),
              QMetaType::Type subType = QMetaType::Type::UnknownType ) SIP_HOLDGIL;


    /**
     * Constructor. Constructs a new QgsField object.
     * \param name Field name
     * \param type Field variant type, currently supported: String / Int / Double
     * \param typeName Field type (e.g., char, varchar, text, int, serial, double).
     * Field types are usually unique to the source and are stored exactly
     * as returned from the data store.
     * \param len Field length
     * \param prec Field precision. Usually decimal places but may also be
     * used in conjunction with other fields types (e.g., variable character fields)
     * \param comment Comment for the field
     * \param subType If the field is a collection, its element's type. When
     *                all the elements don't need to have the same type, leave
     *                this to QVariant::Invalid.
     * \deprecated QGIS 3.38. Use the method with a QMetaType::Type argument instead.
     */
    Q_DECL_DEPRECATED QgsField( const QString &name,
                                QVariant::Type type,
                                const QString &typeName = QString(),
                                int len = 0,
                                int prec = 0,
                                const QString &comment = QString(),
                                QVariant::Type subType = QVariant::Invalid ) SIP_HOLDGIL SIP_DEPRECATED;

    QgsField( const QgsField &other ) SIP_HOLDGIL;
    QgsField &operator =( const QgsField &other ) SIP_SKIP;

    virtual ~QgsField();

    bool operator==( const QgsField &other ) const SIP_HOLDGIL;
    bool operator!=( const QgsField &other ) const SIP_HOLDGIL;

    /**
     * Returns the name of the field.
     * \see setName()
     * \see displayName()
     */
    QString name() const SIP_HOLDGIL;

    /**
     * Returns the name to use when displaying this field. This will be the
     * field alias if set, otherwise the field name.
     * \see name()
     * \see alias()
     */
    QString displayName() const SIP_HOLDGIL;

    /**
     * Returns the name to use when displaying this field and adds the alias in parenthesis if it is defined.
     *
     * This will be used when working close to the data structure (i.e. building expressions and queries),
     * when the real field name must be shown but the alias is also useful to understand what the field
     * represents.
     *
     * \see name()
     * \see alias()
     * \since QGIS 3.12
     */
    QString displayNameWithAlias() const SIP_HOLDGIL;

    /**
     * Returns the type to use when displaying this field, including the length and precision of the datatype if applicable.
     *
     * This will be used when the full datatype with details has to displayed to the user.
     *
     * \see type()
     * \see friendlyTypeString()
     *
     * \since QGIS 3.14
     */
    QString displayType( bool showConstraints = false ) const SIP_HOLDGIL;

    /**
     * Returns a user friendly, translated representation of the field type.
     *
     * Unlike displayType(), this method only returns strings representing the field type and does not include
     * length, precision or constraint information.
     *
     * \see type()
     * \see displayType()
     * \since QGIS 3.14
     */
    QString friendlyTypeString() const SIP_HOLDGIL;

    //! Gets variant type of the field as it will be retrieved from data source
    QMetaType::Type type() const SIP_HOLDGIL;

    /**
     * If the field is a collection, gets its element's type.
     * When all the elements don't need to have the same type, this returns
     * QVariant::Invalid.
     */
    QMetaType::Type subType() const SIP_HOLDGIL;

    /**
     * Gets the field type. Field types vary depending on the data source. Examples
     * are char, int, double, blob, geometry, etc. The type is stored exactly as
     * the data store reports it, with no attempt to standardize the value.
     * \returns QString containing the field type
     */
    QString typeName() const SIP_HOLDGIL;

    /**
     * Gets the length of the field.
     * \returns int containing the length of the field
     */
    int length() const SIP_HOLDGIL;

    /**
     * Gets the precision of the field. Not all field types have a related precision.
     * \returns int containing the precision or zero if not applicable to the field type.
     */
    int precision() const SIP_HOLDGIL;

    /**
     * Returns the field comment
     */
    QString comment() const SIP_HOLDGIL;

    /**
     * Returns the map of field metadata.
     *
     * Map keys should match values from the Qgis::FieldMetadataProperty enum.
     *
     * \see setMetadata()
     * \since QGIS 3.32
     */
    QMap< int, QVariant > metadata() const SIP_HOLDGIL;

    /**
     * Returns a specific metadata \a property.
     *
     * \see setMetadata()
     * \since QGIS 3.32
     */
    QVariant metadata( Qgis::FieldMetadataProperty property ) const SIP_SKIP;

    /**
     * Returns a specific metadata \a property.
     *
     * \see setMetadata()
     * \since QGIS 3.32
     */
    QVariant metadata( int property ) const SIP_HOLDGIL;

    /**
     * Sets the map of field \a metadata.
     *
     * Map keys should match values from the Qgis::FieldMetadataProperty enum.
     *
     * \see metadata()
     * \since QGIS 3.32
     */
    void setMetadata( const QMap< int, QVariant > metadata ) SIP_HOLDGIL;

    /**
     * Sets a metadata \a property to \a value.
     *
     * \see metadata()
     * \since QGIS 3.32
     */
    void setMetadata( Qgis::FieldMetadataProperty property, const QVariant &value ) SIP_SKIP;

    /**
     * Sets a metadata \a property to \a value.
     *
     * \see metadata()
     * \since QGIS 3.32
     */
    void setMetadata( int property, const QVariant &value ) SIP_HOLDGIL;

    /**
     * Returns if this field is numeric. Any integer or floating point type
     * will return TRUE for this.
     */
    bool isNumeric() const SIP_HOLDGIL;

    /**
     * Returns if this field is a date and/or time type.
     *
     * \since QGIS 3.6
     */
    bool isDateOrTime() const SIP_HOLDGIL;

    /**
     * Set the field name.
     * \param name Name of the field
     */
    void setName( const QString &name ) SIP_HOLDGIL;

    /**
     * Set variant \a type.
     */
    void setType( QMetaType::Type type ) SIP_HOLDGIL;

    /**
     * Set variant \a type.
     * \deprecated QGIS 3.38. Use the method with a QMetaType::Type argument instead.
     */
    Q_DECL_DEPRECATED void setType( QVariant::Type type ) SIP_HOLDGIL SIP_DEPRECATED;

    /**
     * If the field is a collection, set its element's type.
     * When all the elements don't need to have the same type, set this to
     * QVariant::Invalid.
     */
    void setSubType( QMetaType::Type subType ) SIP_HOLDGIL;

    /**
     * If the field is a collection, set its element's type.
     * When all the elements don't need to have the same type, set this to
     * QVariant::Invalid.
     * \deprecated QGIS 3.38. Use the method with a QMetaType::Type argument instead.
     */
    Q_DECL_DEPRECATED void setSubType( QVariant::Type subType ) SIP_HOLDGIL  SIP_DEPRECATED;

    /**
     * Set the field type.
     * \param typeName Field type
     */
    void setTypeName( const QString &typeName ) SIP_HOLDGIL;

    /**
     * Set the field length.
     * \param len Length of the field
     */
    void setLength( int len ) SIP_HOLDGIL;

    /**
     * Set the field precision.
     * \param precision Precision of the field
     */
    void setPrecision( int precision ) SIP_HOLDGIL;

    /**
     * Set the field comment
     */
    void setComment( const QString &comment ) SIP_HOLDGIL;

    /**
     * Returns the expression used when calculating the default value for the field.
     * \returns expression evaluated when calculating default values for field, or an
     * empty string if no default is set
     * \see setDefaultValueDefinition()
     */
    QgsDefaultValue defaultValueDefinition() const SIP_HOLDGIL;

    /**
     * Sets an expression to use when calculating the default value for the field.
     * \param defaultValueDefinition expression to evaluate when calculating default values for field. Pass
     * a default constructed QgsDefaultValue to reset.
     * \see defaultValueDefinition()
     */
    void setDefaultValueDefinition( const QgsDefaultValue &defaultValueDefinition ) SIP_HOLDGIL;

    /**
     * Returns constraints which are present for the field.
     * \see setConstraints()
     */
    const QgsFieldConstraints &constraints() const SIP_HOLDGIL;

    /**
     * Sets constraints which are present for the field.
     * \see constraints()
     */
    void setConstraints( const QgsFieldConstraints &constraints ) SIP_HOLDGIL;

    /**
     * Returns the alias for the field (the friendly displayed name of the field ),
     * or an empty string if there is no alias.
     * \see setAlias()
     */
    QString alias() const SIP_HOLDGIL;

    /**
     * Sets the alias for the field (the friendly displayed name of the field ).
     * \param alias field alias, or empty string to remove an existing alias
     * \see alias()
     */
    void setAlias( const QString &alias ) SIP_HOLDGIL;

    /**
     * Returns the Flags for the field (searchable, …).
     * \see setConfigurationFlags()
     * \since QGIS 3.34
     */
    Qgis::FieldConfigurationFlags configurationFlags() const SIP_HOLDGIL;

    /**
     * Sets the Flags for the field (searchable, …).
     * \see configurationFlags()
     * \since QGIS 3.34
     */
    void setConfigurationFlags( Qgis::FieldConfigurationFlags flags ) SIP_HOLDGIL;

    //! Formats string for display
    QString displayString( const QVariant &v ) const;

    /**
     * Returns the readable and translated value of the configuration flag
     * \since QGIS 3.16
     */
    static QString readableConfigurationFlag( Qgis::FieldConfigurationFlag flag ) SIP_HOLDGIL;

#ifndef SIP_RUN

    /**
     * Converts the provided variant to a compatible format
     *
     * \param v  The value to convert
     * \param errorMessage if specified, will be set to a descriptive error when a conversion failure occurs
     *
     * \returns   TRUE if the conversion was successful
     */
    bool convertCompatible( QVariant &v, QString *errorMessage = nullptr ) const;
#else

    /**
     * Converts the provided variant to a compatible format
     *
     * \param v  The value to convert
     *
     * \throws ValueError if the value could not be converted to a compatible format
     */
    bool convertCompatible( QVariant &v ) const;
    % MethodCode
    PyObject *sipParseErr = NULL;

    {
      QVariant *a0;
      int a0State = 0;
      const QgsField *sipCpp;

      if ( sipParseArgs( &sipParseErr, sipArgs, "BJ1", &sipSelf, sipType_QgsField, &sipCpp, sipType_QVariant, &a0, &a0State ) )
      {
        bool sipRes;
        QString errorMessage;

        Py_BEGIN_ALLOW_THREADS
        try
        {
          sipRes = sipCpp->convertCompatible( *a0, &errorMessage );
        }
        catch ( ... )
        {
          Py_BLOCK_THREADS

          sipReleaseType( a0, sipType_QVariant, a0State );
          sipRaiseUnknownException();
          return NULL;
        }

        Py_END_ALLOW_THREADS

        if ( !sipRes )
        {
          PyErr_SetString( PyExc_ValueError,
                           QString( "Value could not be converted to field type %1: %2" ).arg( QMetaType::typeName( sipCpp->type() ), errorMessage ).toUtf8().constData() );
          sipIsErr = 1;
        }
        else
        {
          PyObject *res = sipConvertFromType( a0, sipType_QVariant, NULL );
          sipReleaseType( a0, sipType_QVariant, a0State );
          return res;
        }
      }
      else
      {
        // Raise an exception if the arguments couldn't be parsed.
        sipNoMethod( sipParseErr, sipName_QgsField, sipName_convertCompatible, doc_QgsField_convertCompatible );

        return nullptr;
      }
    }

    % End
#endif

    //! Allows direct construction of QVariants from fields.
    operator QVariant() const;

    /**
     * Set the editor widget setup for the field.
     *
     * \param v  The value to set
     */
    void setEditorWidgetSetup( const QgsEditorWidgetSetup &v ) SIP_HOLDGIL;

    /**
     * Gets the editor widget setup for the field.
     *
     * Defaults may be set by the provider and can be overridden
     * by manual field configuration.
     *
     * \returns the value
     */
    QgsEditorWidgetSetup editorWidgetSetup() const SIP_HOLDGIL;

    /**
     * Make field read-only if \a readOnly is set to true. This is the case for
     * providers which support generated fields for instance.
     * \since QGIS 3.18
     */
    void setReadOnly( bool readOnly ) SIP_HOLDGIL;

    /**
     * Returns TRUE if this field is a read-only field. This is the case for
     * providers which support generated fields for instance.
     * \since QGIS 3.18
     */
    bool isReadOnly() const SIP_HOLDGIL;

    /**
     * Returns the field's split policy, which indicates how field values should
     * be handled during a split operation.
     *
     * \see setSplitPolicy()
     *
     * \since QGIS 3.30
     */
    Qgis::FieldDomainSplitPolicy splitPolicy() const SIP_HOLDGIL;

    /**
     * Sets the field's split \a policy, which indicates how field values should
     * be handled during a split operation.
     *
     * \see splitPolicy()
     *
     * \since QGIS 3.30
     */
    void setSplitPolicy( Qgis::FieldDomainSplitPolicy policy ) SIP_HOLDGIL;

    /**
     * Returns the field's duplicate policy, which indicates how field values should
     * be handled during a duplicate operation.
     *
     * \see setDuplicatePolicy()
     *
     * \since QGIS 3.38
     */
    Qgis::FieldDuplicatePolicy duplicatePolicy() const SIP_HOLDGIL;

    /**
     * Sets the field's duplicate \a policy, which indicates how field values should
     * be handled during a duplicate operation.
     *
     * \see duplicatePolicy()
     *
     * \since QGIS 3.38
     */
    void setDuplicatePolicy( Qgis::FieldDuplicatePolicy policy ) SIP_HOLDGIL;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsField: %1 (%2)>" ).arg( sipCpp->name() ).arg( sipCpp->typeName() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

#ifndef SIP_RUN
    static constexpr int MAX_WKT_LENGTH = 999;
#endif

  private:

    QSharedDataPointer<QgsFieldPrivate> d;


}; // class QgsField

Q_DECLARE_METATYPE( QgsField )

//! Writes the field to stream out. QGIS version compatibility is not guaranteed.
CORE_EXPORT QDataStream &operator<<( QDataStream &out, const QgsField &field );
//! Reads a field from stream in into field. QGIS version compatibility is not guaranteed.
CORE_EXPORT QDataStream &operator>>( QDataStream &in, QgsField &field );


#endif
