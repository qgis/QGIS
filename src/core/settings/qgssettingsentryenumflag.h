/***************************************************************************
  qgssettingsentryenumflag.cpp
  --------------------------------------
  Date                 : February 2022
  Copyright            : (C) 2022 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSETTINGSENTRYENUMFLAG_H
#define QGSSETTINGSENTRYENUMFLAG_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgssettingsentry.h"
#include "qgslogger.h"

/**
 * \class QgsSettingsEntryEnumFlag
 * \ingroup core
 *
 * \brief A template class for enum and flag settings entry.
 *
 * \note This template class has a dedicated handling in sipify.py
 * \since QGIS 3.20
 */
template <typename T>
class QgsSettingsEntryEnumFlag : public QgsSettingsEntryBaseTemplate<T>
{
  public:

    /**
     * Constructor for QgsSettingsEntryEnumFlagBase.
     *
     * \param name specifies the name of the setting.
     * \param parent specifies the parent in the tree of settings.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     *
     * \note The enum needs to be declared with Q_ENUM, and flags with Q_FLAG (not Q_FLAGS).
     * \note for Python bindings, a custom implementation is achieved in Python directly
     * \since QGIS 3.30
     */
    QgsSettingsEntryEnumFlag( const QString &name, QgsSettingsTreeNode *parent, T defaultValue, const QString &description = QString(), Qgis::SettingsOptions options = Qgis::SettingsOptions() )
      : QgsSettingsEntryBaseTemplate<T>( name,
                                         parent,
                                         QMetaEnum::fromType<T>().isFlag() ? qgsFlagValueToKeys( defaultValue ) : qgsEnumValueToKey( defaultValue ),
                                         description,
                                         options )
    {
      mMetaEnum = QMetaEnum::fromType<T>();
      Q_ASSERT( mMetaEnum.isValid() );
      if ( !mMetaEnum.isValid() )
        QgsDebugError( QStringLiteral( "Invalid metaenum. Enum/Flag probably misses Q_ENUM/Q_FLAG declaration. Settings key: '%1'" ).arg( this->key() ) );
    }

    /**
     * Constructor for QgsSettingsEntryEnumFlagBase.
     *
     * \param key specifies the final part of the setting key.
     * \param section specifies the section.
     * \param defaultValue specifies the default value for the settings entry.
     * \param description specifies a description for the settings entry.
     * \param options specifies the options for the settings entry.
     *
     * \note The enum needs to be declared with Q_ENUM, and flags with Q_FLAG (not Q_FLAGS).
     * \note for Python bindings, a custom implementation is achieved in Python directly
     */
    QgsSettingsEntryEnumFlag( const QString &key, const QString &section, T defaultValue, const QString &description = QString(), Qgis::SettingsOptions options = Qgis::SettingsOptions() )
      : QgsSettingsEntryBaseTemplate<T>( key,
                                         section,
                                         QMetaEnum::fromType<T>().isFlag() ? qgsFlagValueToKeys( defaultValue ) : qgsEnumValueToKey( defaultValue ),
                                         description,
                                         options )
    {
      mMetaEnum = QMetaEnum::fromType<T>();
      Q_ASSERT( mMetaEnum.isValid() );
      if ( !mMetaEnum.isValid() )
        QgsDebugError( QStringLiteral( "Invalid metaenum. Enum/Flag probably misses Q_ENUM/Q_FLAG declaration. Settings key: '%1'" ).arg( this->key() ) );
    }

    QVariant convertToVariant( const T &value ) const override
    {
      if ( mMetaEnum.isFlag() )
        return qgsFlagValueToKeys( value );
      else
        return qgsEnumValueToKey( value );
    }


    /**
     * Returns settings default value.
     */
    T convertFromVariant( const QVariant &value ) const override
    {
      if ( !mMetaEnum.isValid() )
      {
        QgsDebugError( QStringLiteral( "Invalid metaenum. Enum/Flag probably misses Q_ENUM/Q_FLAG declaration. Settings key: '%1'" ).arg( this->key() ) );
        return T();
      }

      bool ok = false;
      T enumFlagValue;
      if ( !mMetaEnum.isFlag() )
        enumFlagValue = qgsEnumKeyToValue( value.toString(), mDefaultValue, true, &ok );
      else
        enumFlagValue = qgsFlagKeysToValue( value.toString(), mDefaultValue, true, &ok );

      if ( !ok )
      {
        QgsDebugError( QStringLiteral( "Invalid enum/flag key/s '%1' for settings key '%2'" ).arg( value.toString(), this->key() ) );
        return T();
      }

      return enumFlagValue;
    }

    /**
     * Set settings value.
     *
     * The \a value to set.
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     */
    bool setValuePrivate( const T &value, const QStringList &dynamicKeyPartList ) const override
    {
      if ( !mMetaEnum.isValid() )
      {
        QgsDebugError( QStringLiteral( "Invalid metaenum. Enum/Flag probably misses Q_ENUM/Q_FLAG declaration. Settings key: '%1'" ).arg( this->key( dynamicKeyPartList ) ) );
        return false;
      }

      QVariant variantValue;
      bool ok = true;

      if ( this->options().testFlag( Qgis::SettingsOption::SaveEnumFlagAsInt ) )
      {
        variantValue = static_cast<int>( value );
      }
      else
      {
        if ( !mMetaEnum.isFlag() )
          variantValue = qgsEnumValueToKey( value, &ok );
        else
          variantValue = qgsFlagValueToKeys( value, &ok );
      }

      if ( ok )
        return this->setVariantValue( variantValue, dynamicKeyPartList );
      else
        return false;
    }

    virtual Qgis::SettingsType settingsType() const override
    {
      return Qgis::SettingsType::EnumFlag;
    }

    virtual QString typeId() const override
    {
      return QStringLiteral( "%1-%2" ).arg( this->QgsSettingsEntryBase::typeId(), QMetaEnum::fromType<T>().name() );
    }

  private:
    T mDefaultValue;
    QMetaEnum mMetaEnum;

};

#endif // QGSSETTINGSENTRYENUMFLAG_H
