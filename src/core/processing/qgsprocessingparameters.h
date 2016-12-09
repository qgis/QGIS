/***************************************************************************
                         qgsprocessingparameters.h
                         -----------------------
    begin                : December 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGPARAMETERS_H
#define QGSPROCESSINGPARAMETERS_H

#include <QString>
#include <QVariant>

class CORE_EXPORT QgsProcessingParameter
{
  public:

    enum Flag
    {
      FlagAdvanced = 1 << 1, //!< Parameter is an advanced parameter which should be hidden from users by default
      FlagHidden = 1 << 2, //!< Parameter is hidden and should not be shown to users
      FlagOptional = 1 << 3, //!< Parameter is optional
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    QgsProcessingParameter( const QString& name, const QString& description = QString(), const QVariant& defaultValue = QVariant(),
                            bool optional = false );

    virtual QString type() const = 0;

    /**
     * Returns the name of the parameter. This is the internal identifier by which
     * algorithms access this parameter.
     * @see setName()
     */
    QString name() const { return mName; }

    /**
     * Sets the name of the parameter. This is the internal identifier by which
     * algorithms access this parameter.
     * @see name()
     */
    void setName( const QString& name );

    /**
     * Returns the description for the parameter. This is the user-visible string
     * used to identify this parameter.
     * @see setDescription()
     */
    QString description() const { return mDescription; }

    /**
     * Sets the description for the parameter. This is the user-visible string
     * used to identify this parameter.
     * @see description()
     */
    void setDescription( const QString& description );

    /**
     * Returns the default value for the parameter.
     * @see setDefaultValue()
     */
    virtual QVariant defaultValue() const { return mDefault; }

    /**
     * Sets the default value for the parameter. Returns true if default value was successfully set,
     * or false if value is not acceptable for the parameter.
     * @see defaultValue()
     */
    virtual bool setDefaultValue( const QVariant& value );

    /**
     * Returns any flags associated with the parameter.
     * @see setFlags()
     */
    Flags flags() const { return mFlags; }

    /**
     * Sets the flags associated with the parameter.
     * @see flags()
     */
    void setFlags( const Flags& flags );

    /**
     * Returns true if the specified value is acceptable for the parameter.
     */
    virtual bool acceptsValue( const QVariant& value ) const = 0;

    virtual QVariant parseValue( const QVariant& value ) const { return value; }

    /**
     * Returns the value of this parameter as it should have been entered in the console if calling
     * an algorithm manually.
     */
    virtual QString valueAsCommandLineParameter( const QVariant& value ) const;

    virtual QString asScriptCode() const;

  protected:

    //! Parameter name
    QString mName;

    //! Parameter description
    QString mDescription;

    //! Default value for parameter
    QVariant mDefault;

    //! Parameter flags
    Flags mFlags;


};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsProcessingParameter::Flags )


class CORE_EXPORT QgsProcessingParameterBoolean : public QgsProcessingParameter
{
  public:

    QgsProcessingParameterBoolean( const QString& name, const QString& description = QString(), const QVariant& defaultValue = QVariant(),
                                   bool optional = false );

    QString type() const override { return QStringLiteral( "boolean" ); }

    bool acceptsValue( const QVariant& value ) const override;

    QVariant parseValue( const QVariant& value ) const override;

    static QgsProcessingParameter* createFromScriptCode( const QString& name, const QString& description, bool isOptional, const QString& definition );

  private:

    static QVariant convertToBool( const QVariant& value );

};


#endif // QGSPROCESSINGPARAMETERS_H


