/***************************************************************************
  qgssettingsenumflageditorwidgetwrapper.h
  --------------------------------------
  Date                 : February 2023
  Copyright            : (C) 2023 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSETTINGSENUMFLAGEDITORWIDGETWRAPPER_H
#define QGSSETTINGSENUMFLAGEDITORWIDGETWRAPPER_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgis_gui.h"
#include "qgssettingseditorwidgetwrapperimpl.h"
#include "qgslogger.h"

#include "qgssettingsentryenumflag.h"

#include <QComboBox>
#include <QStandardItemModel>

/**
 * \ingroup gui
 * \brief This class is a factory of editor for flags settings
 *
 * \since QGIS 3.32
 */
template<class ENUM, class FLAGS>
class GUI_EXPORT QgsSettingsFlagsEditorWidgetWrapper : public QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryEnumFlag<FLAGS>, QComboBox, FLAGS>
{
  public:
    //! Constructor
    QgsSettingsFlagsEditorWidgetWrapper( QObject *parent = nullptr )
      : QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryEnumFlag<FLAGS>, QComboBox, FLAGS>( parent )
    {}

    void enableAutomaticUpdatePrivate() override
    {
      QObject::connect( &mModel, &QStandardItemModel::itemChanged, this, [=]( const QStandardItem *item ) {
        Q_UNUSED( item )
        setSettingFromWidget();
      } );
    }

    QgsSettingsEditorWidgetWrapper *createWrapper( QObject *parent = nullptr ) const override { return new QgsSettingsFlagsEditorWidgetWrapper<ENUM, FLAGS>( parent ); }

    virtual QString id() const override
    {
      return QStringLiteral( "%1-%2" ).arg( sSettingsTypeMetaEnum.valueToKey( static_cast<int>( Qgis::SettingsType::EnumFlag ) ), QMetaEnum::fromType<FLAGS>().name() );
    }

    QVariant variantValueFromWidget() const override
    {
      // enum/flags are stored as text
      return this->mSetting->convertToVariant( valueFromWidget() );
    };

    bool setSettingFromWidget() const override
    {
      if ( this->mEditor )
      {
        this->mSetting->setValue( this->valueFromWidget(), this->mDynamicKeyPartList );
        return true;
      }
      else
      {
        QgsDebugMsgLevel( QStringLiteral( "Settings editor not set for %1" ).arg( this->mSetting->definitionKey() ), 2 );
      }
      return false;
    }

    FLAGS valueFromWidget() const override
    {
      if ( this->mEditor )
      {
        FLAGS value;
        for ( int r = 0; r < mModel.rowCount(); r++ )
        {
          QStandardItem *item = mModel.item( r );
          if ( item->data( Qt::CheckStateRole ) == Qt::Checked )
            value |= item->data().value<ENUM>();
        }
        return value;
      }
      else
      {
        QgsDebugMsgLevel( QString( "editor is not set, returning a non-existing value" ), 2 );
      }
      return FLAGS();
    }

    bool setWidgetValue( const FLAGS &value ) const override
    {
      if ( this->mEditor )
      {
        for ( int r = 0; r < mModel.rowCount(); r++ )
        {
          QStandardItem *item = mModel.item( r );
          bool isChecked = value.testFlag( item->data().value<ENUM>() );
          item->setData( isChecked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
        }
        return true;
      }
      else
      {
        QgsDebugMsgLevel( QStringLiteral( "Settings editor not set for %1" ).arg( this->mSetting->definitionKey() ), 2 );
      }
      return false;
    }

  protected:
    void configureEditorPrivateImplementation() override
    {
      mModel.clear();
      const QMap<ENUM, QString> enumMap = qgsEnumMap<ENUM>();
      for ( auto it = enumMap.constBegin(); it != enumMap.constEnd(); ++it )
      {
        QStandardItem *item = new QStandardItem( it.value() );
        item->setData( QVariant::fromValue( it.key() ) );
        item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
        item->setData( Qt::Unchecked, Qt::CheckStateRole );
        mModel.appendRow( item );
      }
      this->mEditor->setModel( &mModel );
    }

    QStandardItemModel mModel;
};

/**
 * \ingroup gui
 * \brief This class is a factory of editor for enum settings
 *
 * \since QGIS 3.32
 */
template<class ENUM>
class QgsSettingsEnumEditorWidgetWrapper : public QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryEnumFlag<ENUM>, QComboBox, ENUM>
{
  public:
    //! Constructor
    QgsSettingsEnumEditorWidgetWrapper( QObject *parent = nullptr )
      : QgsSettingsEditorWidgetWrapperTemplate<QgsSettingsEntryEnumFlag<ENUM>, QComboBox, ENUM>( parent )
    {}

    void enableAutomaticUpdatePrivate() override
    {
      QObject::connect( this->mEditor, qOverload<int>( &QComboBox::currentIndexChanged ), this, [=]( int index ) {
        Q_UNUSED( index );
        ENUM value = this->mEditor->currentData().template value<ENUM>();
        this->mSetting->setValue( value, this->mDynamicKeyPartList );
      } );
    }

    virtual QString id() const override
    {
      return QStringLiteral( "%1-%2" ).arg( sSettingsTypeMetaEnum.valueToKey( static_cast<int>( Qgis::SettingsType::EnumFlag ) ), QMetaEnum::fromType<ENUM>().name() );
    }

    /**
     * This will set the display strings so they can be readable and translatable.
     * This must be called before calling createEditor or configureEditor.
     * \since QGIS 3.40
     */
    void setDisplayStrings( const QMap<ENUM, QString> &displayStrings ) { mDisplayStrings = displayStrings; }

    QgsSettingsEditorWidgetWrapper *createWrapper( QObject *parent = nullptr ) const override { return new QgsSettingsEnumEditorWidgetWrapper<ENUM>( parent ); }

    QVariant variantValueFromWidget() const override
    {
      // enum/flags are stored as text
      return this->mSetting->convertToVariant( valueFromWidget() );
    };

    bool setSettingFromWidget() const override
    {
      if ( this->mEditor )
      {
        this->mSetting->setValue( this->valueFromWidget(), this->mDynamicKeyPartList );
        return true;
      }
      else
      {
        QgsDebugMsgLevel( QStringLiteral( "Settings editor not set for %1" ).arg( this->mSetting->definitionKey() ), 2 );
      }
      return false;
    }

    ENUM valueFromWidget() const override
    {
      if ( this->mEditor )
      {
        return this->mEditor->currentData().template value<ENUM>();
      }
      else
      {
        QgsDebugMsgLevel( QString( "editor is not set, returning a non-existing value" ), 2 );
      }
      return ENUM();
    }

    bool setWidgetValue( const ENUM &value ) const override
    {
      if ( this->mEditor )
      {
        int i = this->mEditor->findData( QVariant::fromValue( value ) );
        this->mEditor->setCurrentIndex( i );
        return i >= 0;
      }
      else
      {
        QgsDebugMsgLevel( QStringLiteral( "Settings editor not set for %1" ).arg( this->mSetting->definitionKey() ), 2 );
      }
      return false;
    }

  protected:
    void configureEditorPrivateImplementation() override
    {
      const QMap<ENUM, QString> enumMap = qgsEnumMap<ENUM>();
      for ( auto it = enumMap.constBegin(); it != enumMap.constEnd(); ++it )
      {
        const QString displayString = mDisplayStrings.value( it.key(), it.value() );
        this->mEditor->addItem( displayString, QVariant::fromValue( it.key() ) );
      }
    }

  private:
    QMap<ENUM, QString> mDisplayStrings;
};

#endif // QGSSETTINGSENUMFLAGEDITORWIDGETWRAPPER_H
