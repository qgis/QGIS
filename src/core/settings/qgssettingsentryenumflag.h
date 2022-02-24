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

#include "qgssettingsentry.h"

/**
 * \class QgsSettingsEntryEnumFlag
 * \ingroup core
 *
 * \brief A template class for enum and flag settings entry.
 *
 * \note This template class has a dedicated handling in sipify.pl
 * \since QGIS 3.20
 */
template <typename T>
class CORE_EXPORT QgsSettingsEntryEnumFlag : public QgsSettingsEntryBase
{
  public:

    /**
     * Constructor for QgsSettingsEntryEnumFlagBase.
     *
     * The \a key argument specifies the final part of the settings key.
     * The \a section argument specifies the section.
     * The \a defaultValue argument specifies the default value for the settings entry.
     * The \a description argument specifies a description for the settings entry.
     *
     * \note The enum needs to be declared with Q_ENUM, and flags with Q_FLAG (not Q_FLAGS).
     * \note for Python bindings, a custom implementation is achieved in Python directly
     */
    QgsSettingsEntryEnumFlag( const QString &key, QgsSettings::Section section, const T &defaultValue, const QString &description = QString() )
      : QgsSettingsEntryBase( key, section, QMetaEnum::fromType<T>().isFlag() ? QVariant( QMetaEnum::fromType<T>().valueToKeys( static_cast <int >( defaultValue ) ) ) : QVariant( QMetaEnum::fromType<T>().valueToKey( static_cast< int >( defaultValue ) ) ), description )
    {
      mMetaEnum = QMetaEnum::fromType<T>();
      Q_ASSERT( mMetaEnum.isValid() );
      if ( !mMetaEnum.isValid() )
        QgsDebugMsg( QStringLiteral( "Invalid metaenum. Enum/Flag probably misses Q_ENUM/Q_FLAG declaration. Settings key: '%1'" ).arg( QgsSettingsEntryBase::key() ) );
    }

    /**
     * Returns settings value.
     *
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     * The \a useDefaultValueOverride argument specifies if defaultValueOverride should be used.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     */
    T value( const QString &dynamicKeyPart = QString(), bool useDefaultValueOverride = false, const T &defaultValueOverride = T() ) const
    {
      QStringList dynamicKeyPartList;
      if ( !dynamicKeyPart.isEmpty() )
        dynamicKeyPartList.append( dynamicKeyPart );

      return value( dynamicKeyPartList, useDefaultValueOverride, defaultValueOverride );
    }

    /**
     * Returns settings value.
     *
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     * The \a useDefaultValueOverride argument specifies if defaultValueOverride should be used.
     * The \a defaultValueOverride argument if valid is used instead of the normal default value.
     */
    T value( const QStringList &dynamicKeyPartList, bool useDefaultValueOverride = false, const T &defaultValueOverride = T() ) const
    {
      T defaultVal = defaultValue();
      if ( useDefaultValueOverride )
        defaultVal = defaultValueOverride;

      if ( !mMetaEnum.isFlag() )
        return QgsSettings().enumValue( key( dynamicKeyPartList ),
                                        defaultVal,
                                        section() );
      else
        return QgsSettings().flagValue( key( dynamicKeyPartList ),
                                        defaultVal,
                                        section() );
    }

    /**
     * Returns settings default value.
     */
    T defaultValue() const
    {
      if ( !mMetaEnum.isValid() )
      {
        QgsDebugMsg( QStringLiteral( "Invalid metaenum. Enum/Flag probably misses Q_ENUM/Q_FLAG declaration. Settings key: '%1'" ).arg( key() ) );
        return T();
      }

      bool ok = false;
      T defaultValue;
      if ( !mMetaEnum.isFlag() )
        defaultValue = static_cast<T>( mMetaEnum.keyToValue( defaultValueAsVariant().toByteArray(), &ok ) );
      else
        defaultValue = static_cast<T>( mMetaEnum.keysToValue( defaultValueAsVariant().toByteArray(), &ok ) );

      if ( !ok )
      {
        QgsDebugMsg( QStringLiteral( "Invalid enum/flag key/s '%1' for settings key '%2'" ).arg( defaultValueAsVariant().toString(), key() ) );
        return T();
      }

      return defaultValue;
    }

    /**
     * Set settings value.
     *
     * The \a value to set.
     * The \a dynamicKeyPart argument specifies the dynamic part of the settings key.
     */
    bool setValue( const T &value, const QString &dynamicKeyPart = QString() ) const
    {
      QStringList dynamicKeyPartList;
      if ( !dynamicKeyPart.isEmpty() )
        dynamicKeyPartList.append( dynamicKeyPart );

      return setValue( value, dynamicKeyPartList );
    }

    /**
     * Set settings value.
     *
     * The \a value to set.
     * The \a dynamicKeyParts argument specifies the list of dynamic parts of the settings key.
     */
    bool setValue( const T &value, const QStringList &dynamicKeyPartList ) const
    {
      if ( !mMetaEnum.isValid() )
      {
        QgsDebugMsg( QStringLiteral( "Invalid metaenum. Enum/Flag probably misses Q_ENUM/Q_FLAG declaration. Settings key: '%1'" ).arg( key( dynamicKeyPartList ) ) );
        return false;
      }

      if ( !mMetaEnum.isFlag() )
      {
        const char *enumKey = mMetaEnum.valueToKey( static_cast< int >( value ) );
        if ( enumKey == nullptr )
        {
          QgsDebugMsg( QStringLiteral( "Invalid enum value '%1'." ).arg( static_cast< int >( value ) ) );
          return false;
        }

        return QgsSettingsEntryBase::setVariantValue( enumKey, dynamicKeyPartList );
      }
      else
      {
        const QByteArray flagKeys = mMetaEnum.valueToKeys( static_cast< int >( value ) );
        if ( flagKeys.isEmpty() )
        {
          QgsDebugMsg( QStringLiteral( "Invalid flag value '%1'." ).arg( static_cast< int >( value ) ) );
          return false;
        }
        return QgsSettingsEntryBase::setVariantValue( flagKeys, dynamicKeyPartList );
      }
    }

    virtual Qgis::SettingsType settingsType() const override
    {
      return Qgis::SettingsType::EnumFlag;
    }

  private:

    QMetaEnum mMetaEnum;

};

#endif // QGSSETTINGSENTRYENUMFLAG_H
