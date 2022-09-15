/***************************************************************************
  qgsserverquerystringparameter.h - QgsServerQueryStringParameter

 ---------------------
 begin                : 10.7.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSERVERQUERYSTRINGPARAMETER_H
#define QGSSERVERQUERYSTRINGPARAMETER_H

#include "qgsserverapicontext.h"
#include "qgis_server.h"
#include "qgis_sip.h"
#include <QString>
#include <QVariant>
#include <QObject>


#include "nlohmann/json_fwd.hpp"

#ifndef SIP_RUN
using namespace nlohmann;
#endif


class QgsServerApiBadRequestException;


/**
 * The QgsServerQueryStringParameter class holds the information regarding
 * a query string input parameter and its validation.
 *
 * The class is extendable through custom validators (C++ only) and/or by
 * subclassing and overriding the value() method.
 *
 * \ingroup server
 * \since QGIS 3.10
 */
class SERVER_EXPORT QgsServerQueryStringParameter
{

    Q_GADGET

#ifndef SIP_RUN
    typedef  std::function< bool ( const QgsServerApiContext &, QVariant & ) > customValidator;
#endif
  public:

    /**
     * The Type enum represents the parameter type
     */
    enum class Type
    {
      String = QVariant::String,    //! parameter is a string
      Integer = QVariant::LongLong, //! parameter is an integer
      Double = QVariant::Double,    //! parameter is a double
      Boolean = QVariant::Bool,     //! parameter is a boolean
      List = QVariant::StringList,  //! parameter is a (comma separated) list of strings, the handler will perform any further required conversion of the list values
    };
    Q_ENUM( Type )


    /**
     * Constructs a QgsServerQueryStringParameter object.
     *
     * \param name parameter name
     * \param required
     * \param type the parameter type
     * \param description parameter description
     * \param defaultValue default value, it is ignored if the parameter is required
     */
    QgsServerQueryStringParameter( const QString name,
                                   bool required = false,
                                   Type type = QgsServerQueryStringParameter::Type::String,
                                   const QString &description = QString(),
                                   const QVariant &defaultValue = QVariant() );

    virtual ~QgsServerQueryStringParameter();

    /**
     * Extracts the value from the request \a context by validating the parameter
     * value and converting it to its proper Type.
     * If the value is not set and a default was not provided an invalid QVariant is returned.
     *
     * Validation steps:
     *
     * - required
     * - can convert to proper Type
     * - custom validator (if set - not available in Python bindings)
     *
     * \see setCustomValidator() (not available in Python bindings)
     * \returns the parameter value or an invalid QVariant if not found (and not required)
     * \throws QgsServerApiBadRequestError if validation fails
     */
    virtual QVariant value( const QgsServerApiContext &context ) const;

#ifndef SIP_RUN

    /**
     * Sets the custom validation function to \a customValidator.
     * Validator function signature is:
     * bool ( const QgsServerApiContext &context, QVariant &value )
     * \note a validator can change the value if needed and must return TRUE if the validation passed
     * \note not available in Python bindings
     */
    void setCustomValidator( const customValidator &customValidator );

    /**
     * Returns the handler information as a JSON object.
     */
    json data( ) const;

#endif

    /**
     * Returns parameter description
     */
    QString description() const;

    /**
     * Returns the name of the \a type
     */
    static QString typeName( const Type type );

    /**
     * Returns the name of the parameter
     */
    QString name() const;

    /**
     * Sets validator \a description
     */
    void setDescription( const QString &description );

    /**
     * Returns TRUE if the parameter is hidden from the schema.
     *
     * Hidden params can be useful to implement legacy parameters or
     * parameters that can be accepted without being advertised.
     *
     * \since QGIS 3.28
     */
    bool hidden() const;

    /**
     * Set the parameter's \a hidden status, parameters are not hidden by default.
     *
     * \since QGIS 3.28
     */
    void setHidden( bool hidden );

  private:

    QString mName;
    bool mRequired = false;
    Type mType = Type::String;
    customValidator mCustomValidator = nullptr;
    QString mDescription;
    QVariant mDefaultValue;
    bool mHidden = false;

    friend class TestQgsServerQueryStringParameter;

};

#endif // QGSSERVERQUERYSTRINGPARAMETER_H
