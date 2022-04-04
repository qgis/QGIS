/***************************************************************************
    qgsfielddomain.h
    ------------------
    Date                 : January 2022
    Copyright            : (C) 2022 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFIELDDOMAIN_H
#define QGSFIELDDOMAIN_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"

/**
 * \ingroup core
 * \class QgsFieldDomain
 *
 * \brief Base class for field domains.
 *
 * A field domain is a set of constraints that apply to one or several fields.
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsFieldDomain
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( sipCpp->type() == Qgis::FieldDomainType::Coded )
    {
      sipType = sipType_QgsCodedFieldDomain;
    }
    else if ( sipCpp->type() == Qgis::FieldDomainType::Range )
    {
      sipType = sipType_QgsRangeFieldDomain;
    }
    else if ( sipCpp->type() == Qgis::FieldDomainType::Glob )
    {
      sipType = sipType_QgsGlobFieldDomain;
    }
    else
    {
      sipType = 0;
    }
    SIP_END
#endif

  public:

    /**
     * Constructor for QgsFieldDomain, with the specified \a name, \a description and \a fieldType.
     */
    QgsFieldDomain( const QString &name,
                    const QString &description,
                    QVariant::Type fieldType );

    virtual ~QgsFieldDomain();

    /**
     * Clones the field domain.
     */
    virtual QgsFieldDomain *clone() const = 0 SIP_FACTORY;

    /**
     * Returns the type of field domain.
     */
    virtual Qgis::FieldDomainType type() const = 0;

    /**
     * Returns a translated name of the field domain type.
     */
    virtual QString typeName() const = 0;

    /**
     * Returns the name of the field domain.
     *
     * \see setName()
     */
    QString name() const { return mName; }

    /**
     * Sets the \a name of the field domain.
     *
     * \see name()
     */
    void setName( const QString &name ) { mName = name; }

    /**
     * Returns the description of the field domain.
     *
     * \see setDescription()
     */
    QString description() const { return mDescription; }

    /**
     * Sets the \a description of the field domain.
     *
     * \see description()
     */
    void setDescription( const QString &description ) { mDescription = description; }

    /**
     * Returns the associated field type.
     *
     * \see setFieldType()
     */
    QVariant::Type fieldType() const { return mFieldType; }

    /**
     * Sets the associated field \a type.
     *
     * \see fieldType()
     */
    void setFieldType( QVariant::Type type ) { mFieldType = type; }

    /**
     * Returns the split policy.
     *
     * \see setSplitPolicy()
     */
    Qgis::FieldDomainSplitPolicy splitPolicy() const { return mSplitPolicy; }

    /**
     * Sets the split \a policy.
     *
     * \see splitPolicy()
     */
    void setSplitPolicy( Qgis::FieldDomainSplitPolicy policy ) { mSplitPolicy = policy; }

    /**
     * Returns the merge policy.
     *
     * \see setMergePolicy()
     */
    Qgis::FieldDomainMergePolicy mergePolicy() const { return mMergePolicy; }

    /**
     * Sets the merge \a policy.
     *
     * \see mergePolicy()
     */
    void setMergePolicy( Qgis::FieldDomainMergePolicy policy ) { mMergePolicy = policy; }

  protected:

    QString mName;
    QString mDescription;

    QVariant::Type mFieldType = QVariant::Type::String;
    Qgis::FieldDomainSplitPolicy mSplitPolicy = Qgis::FieldDomainSplitPolicy::DefaultValue;
    Qgis::FieldDomainMergePolicy mMergePolicy = Qgis::FieldDomainMergePolicy::DefaultValue;

};

/**
 * \ingroup core
 * \brief Associates a code and a value.
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsCodedValue
{
  public:

    /**
     * Constructor for QgsCodedValue, with the associated \a code and \a value.
     *
     * The \a code is the underlying value stored in feature attributes, while the
     * \a value is the user-facing string representation.
     */
    QgsCodedValue( const QVariant &code, const QString &value )
      : mCode( code )
      , mValue( value )
    {}

    /**
     * Returns the associated code, which is the underlying
     * value stored in fields.
     */
    QVariant code() const { return mCode; }

    /**
     * Returns the associated value, which is the user-friendly
     * string representation.
     */
    QString value() const { return mValue; }

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsCodedValue: %1 (%2)>" ).arg( sipCpp->code().toString(), sipCpp->value() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    bool operator==( const QgsCodedValue &other ) const;
    bool operator!=( const QgsCodedValue &other ) const;

  private:

    QVariant mCode;
    QString mValue;
};


/**
 * \ingroup core
 * \brief Definition of a coded / enumerated field domain.
 *
 * A code field domain is a domain for which only a limited set of codes,
 * associated with their expanded value, are allowed.
 * The type of the code should be the one of the field domain.
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsCodedFieldDomain : public QgsFieldDomain
{

  public:

    /**
     * Constructor for QgsCodedFieldDomain, with the associated \a name, \a description and \a fieldType.
     *
     * The \a values list details the coded field values as QgsCodedValue objects. Each code should
     * appear only once, but it is the responsibility of the user to check this.
     */
    QgsCodedFieldDomain( const QString &name,
                         const QString &description,
                         QVariant::Type fieldType,
                         const QList<QgsCodedValue> &values );

#ifndef SIP_RUN
    //! QgsCodedFieldDomain cannot be copied - use clone() instead
    QgsCodedFieldDomain( const QgsCodedFieldDomain & ) = delete;
    //! QgsCodedFieldDomain cannot be copied - use clone() instead
    QgsCodedFieldDomain &operator= ( const QgsCodedFieldDomain & ) = delete;
#endif

    Qgis::FieldDomainType type() const override;
    QString typeName() const override;
    QgsCodedFieldDomain *clone() const override SIP_FACTORY;

    /**
     * Returns the enumeration as QgsCodedValue values.
     *
     * \see setValues()
     */
    QList< QgsCodedValue> values() const { return mValues; }

    /**
     * Sets the enumeration as QgsCodedValue \a values.
     *
     * \see values()
     */
    void setValues( const QList< QgsCodedValue> &values ) { mValues = values; }

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsCodedFieldDomain: %1>" ).arg( sipCpp->name() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  private:
    QList< QgsCodedValue> mValues;

#ifdef SIP_RUN
    QgsCodedFieldDomain( const QgsCodedFieldDomain & );
#endif
};


/**
 * \ingroup core
 * \brief Definition of a numeric field domain with a range of validity for values.
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsRangeFieldDomain : public QgsFieldDomain
{

  public:

    /**
     * Constructor for QgsRangeFieldDomain, with the specified \a name, \a description and \a fieldType.
     *
     * Set an invalid QVariant for \a minimum or \a maximum respectively if no minimum or maximum value is desired.
     */
    QgsRangeFieldDomain( const QString &name,
                         const QString &description,
                         QVariant::Type fieldType,
                         const QVariant &minimum,
                         bool minimumIsInclusive,
                         const QVariant &maximum,
                         bool maximumIsInclusive );

#ifndef SIP_RUN
    //! QgsRangeFieldDomain cannot be copied - use clone() instead
    QgsRangeFieldDomain( const QgsRangeFieldDomain & ) = delete;
    //! QgsRangeFieldDomain cannot be copied - use clone() instead
    QgsRangeFieldDomain &operator= ( const QgsRangeFieldDomain & ) = delete;
#endif

    Qgis::FieldDomainType type() const override;
    QString typeName() const override;
    QgsRangeFieldDomain *clone() const override SIP_FACTORY;

    /**
     * Returns the minimum value.
     *
     * If no minimum value is set then an invalid variant will be returned.
     *
     * \see minimumIsInclusive()
     * \see setMinimum()
     */
    QVariant minimum() const { return mMin; }

    /**
     * Sets the \a minimum allowed value.
     *
     * If no minimum value is desired then an invalid variant should be set.
     *
     * \see setMinimumIsInclusive()
     * \see minimum()
     */
    void setMinimum( const QVariant &minimum ) { mMin = minimum; }

    /**
     * Returns TRUE if the minimum value is inclusive.
     *
     * \see minimum()
     * \see setMinimumIsInclusive()
     */
    bool minimumIsInclusive() const { return mMinIsInclusive; }

    /**
     * Sets whether the minimum value is \a inclusive.
     *
     * \see setMinimum()
     * \see minimumIsInclusive()
     */
    void setMinimumIsInclusive( bool inclusive ) { mMinIsInclusive = inclusive; }

    /**
     * Returns the maximum value.
     *
     * If no maximum value is set then an invalid variant will be returned.
     *
     * \see maximumIsInclusive()
     * \see setMaximum()
     */
    QVariant maximum() const { return mMax; }

    /**
     * Sets the \a maximum allowed value.
     *
     * If no maximum value is desired then an invalid variant should be set.
     *
     * \see setMaximumIsInclusive()
     * \see maximum()
     */
    void setMaximum( const QVariant &maximum ) { mMax = maximum; }

    /**
     * Returns TRUE if the maximum value is inclusive.
     *
     * \see maximum()
     * \see setMaximumIsInclusive()
     */
    bool maximumIsInclusive() const { return mMaxIsInclusive; }

    /**
     * Sets whether the maximum value is \a inclusive.
     *
     * \see setMaximum()
     * \see maximumIsInclusive()
     */
    void setMaximumIsInclusive( bool inclusive ) { mMaxIsInclusive = inclusive; }

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsRangeFieldDomain: %1 %2%3, %4%5>" ).arg( sipCpp->name(),
                  sipCpp->minimumIsInclusive() ? QStringLiteral( "[" ) : QStringLiteral( "(" ),
                  sipCpp->minimum().toString(),
                  sipCpp->maximum().toString(),
                  sipCpp->maximumIsInclusive() ? QStringLiteral( "]" ) : QStringLiteral( ")" ) );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  private:
    QVariant mMin;
    QVariant mMax;
    bool mMinIsInclusive = false;
    bool mMaxIsInclusive = false;

#ifdef SIP_RUN
    QgsRangeFieldDomain( const QgsRangeFieldDomain & );
#endif
};


/**
 * \ingroup core
 * \brief Definition of a field domain for field content validated by a glob.
 *
 * Globs are matching expression like "*[a-z][0-1]?"
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsGlobFieldDomain : public QgsFieldDomain
{

  public:

    /**
     * Constructor for QgsGlobFieldDomain, with the specified \a name, \a description and \a fieldType.
     *
     * The \a glob argument specifies the content validation glob, e.g. "*[a-z][0-1]?".
     */
    QgsGlobFieldDomain( const QString &name,
                        const QString &description,
                        QVariant::Type fieldType,
                        const QString &glob );

#ifndef SIP_RUN
    //! QgsGlobFieldDomain cannot be copied - use clone() instead
    QgsGlobFieldDomain( const QgsGlobFieldDomain & ) = delete;
    //! QgsGlobFieldDomain cannot be copied - use clone() instead
    QgsGlobFieldDomain &operator= ( const QgsGlobFieldDomain & ) = delete;
#endif

    Qgis::FieldDomainType type() const override;
    QString typeName() const override;
    QgsGlobFieldDomain *clone() const override SIP_FACTORY;

    /**
     * Returns the glob expression.
     *
     * Globs are matching expression like "*[a-z][0-1]?"
     *
     * \see setGlob()
     */
    QString glob() const { return mGlob; }

    /**
     * Sets the \a glob expression.
     *
     * Globs are matching expression like "*[a-z][0-1]?"
     *
     * \see glob()
     */
    void setGlob( const QString &glob ) { mGlob = glob; }

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsGlobFieldDomain: %1 '%2'>" ).arg( sipCpp->name(), sipCpp->glob() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  private:
    QString mGlob;

#ifdef SIP_RUN
    QgsGlobFieldDomain( const QgsGlobFieldDomain & );
#endif

};

#endif // QGSFIELDDOMAIN_H
