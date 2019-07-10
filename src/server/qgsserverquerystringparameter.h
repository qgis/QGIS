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

class QgsServerApiBadRequestError;



/**
 * The QgsServerQueryStringParameter class holds the information regarding a query string input parameter
 * and its validation.
 *
 * The class is extendable through custom validators and/or by subclassing and overriding the value() method.
 * Custom validators return
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
      String = QVariant::String,   //! parameter is a string
      Int = QVariant::LongLong,    //! parameter is an integer
      Double = QVariant::Double,   //! parameter is a double
      Bool = QVariant::Bool,       //! parameter is a boolean
      List = QVariant::StringList, //! parameter is a list of strings, the handler will perform any further required conversion of the list values
    };
    Q_ENUM( Type )


    QgsServerQueryStringParameter( const QString name,
                                   bool required = false,
                                   Type type = Type::String,
                                   const QString &description = QString() );

    virtual ~QgsServerQueryStringParameter();

    /**
     * Extracts the value from the request \a context by validating the parameter
     * value and converting it to its proper Type.
     *
     * Validation steps:
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
#endif

    /**
     * Returns parameter description
     */
    QString description() const;

    /**
     * Returns the name of the \a type
     */
    static QString typeName( const Type type );

  private:

    QString mName;
    bool mRequired = false;
    Type mType = Type::String;
    customValidator mCustomValidator = nullptr;
    QString mDescription;

    friend class TestQgsServerQueryStringParameter;

};

#endif // QGSSERVERQUERYSTRINGPARAMETER_H
