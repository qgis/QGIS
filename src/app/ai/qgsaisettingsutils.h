/***************************************************************************
    qgsaisettingsutils.h
    ---------------------
    begin                : July 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAISETTINGSUTILS_H
#define QGSAISETTINGSUTILS_H

#include <QComboBox>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>
#include <QVariant>
#include <QWidget>

using namespace Qt::StringLiterals;

/**
 * Small helpers shared between the AI chat dock and the AI settings dialog.
 * Header-only so both translation units can use them without a new library
 * dependency.
 */
namespace QgsAiSettingsUtils
{
  inline QString humanBytes( qint64 bytes )
  {
    if ( bytes < 1024 )
      return u"%1 B"_s.arg( bytes );
    const double kb = bytes / 1024.0;
    if ( kb < 1024 )
      return u"%1 KiB"_s.arg( kb, 0, 'f', 1 );
    const double mb = kb / 1024.0;
    if ( mb < 1024 )
      return u"%1 MiB"_s.arg( mb, 0, 'f', 1 );
    return u"%1 GiB"_s.arg( mb / 1024.0, 0, 'f', 2 );
  }

  //! Reads \a key falling back to \a legacyKeys so renamed settings keep their stored value.
  template<typename Settings> QVariant settingValueWithLegacy( Settings &settings, const QString &key, const QStringList &legacyKeys, const QVariant &defaultValue )
  {
    if ( settings.contains( key ) )
      return settings.value( key, defaultValue );
    for ( const QString &legacyKey : legacyKeys )
    {
      if ( settings.contains( legacyKey ) )
        return settings.value( legacyKey, defaultValue );
    }
    return defaultValue;
  }

  //! Bold group header label; styled via the aiRole=sectionHeader dynamic property.
  inline QLabel *sectionHeader( const QString &title, QWidget *parent = nullptr )
  {
    QLabel *label = new QLabel( title, parent );
    label->setProperty( "aiRole", u"sectionHeader"_s );
    QFont font = label->font();
    font.setBold( true );
    label->setFont( font );
    return label;
  }

  /**
   * Settings row in the Cursor style: bold title with an optional gray
   * description on the left, \a control right-aligned. Long checkbox texts
   * belong in \a description; the control itself should stay terse.
   */
  inline QWidget *settingRow( const QString &title, const QString &description, QWidget *control, QWidget *parent = nullptr )
  {
    QWidget *row = new QWidget( parent );
    QHBoxLayout *rowLayout = new QHBoxLayout( row );
    rowLayout->setContentsMargins( 0, 4, 0, 4 );
    rowLayout->setSpacing( 16 );

    QVBoxLayout *textLayout = new QVBoxLayout();
    textLayout->setContentsMargins( 0, 0, 0, 0 );
    textLayout->setSpacing( 2 );
    QLabel *titleLabel = new QLabel( title, row );
    titleLabel->setProperty( "aiRole", u"rowTitle"_s );
    QFont titleFont = titleLabel->font();
    titleFont.setBold( true );
    titleLabel->setFont( titleFont );
    textLayout->addWidget( titleLabel );
    if ( !description.isEmpty() )
    {
      QLabel *descriptionLabel = new QLabel( description, row );
      descriptionLabel->setProperty( "aiRole", u"rowDescription"_s );
      descriptionLabel->setWordWrap( true );
      textLayout->addWidget( descriptionLabel );
    }
    rowLayout->addLayout( textLayout, 1 );

    if ( control )
    {
      control->setAccessibleName( title );
      if ( qobject_cast<QLineEdit *>( control ) || qobject_cast<QComboBox *>( control ) )
        control->setMinimumWidth( 260 );
      rowLayout->addWidget( control, 0, Qt::AlignVCenter | Qt::AlignRight );
    }
    return row;
  }

  //! Full-width variant for tall controls (text edits): title + description stacked above the control.
  inline QWidget *settingRowFullWidth( const QString &title, const QString &description, QWidget *control, QWidget *parent = nullptr )
  {
    QWidget *row = new QWidget( parent );
    QVBoxLayout *rowLayout = new QVBoxLayout( row );
    rowLayout->setContentsMargins( 0, 4, 0, 4 );
    rowLayout->setSpacing( 4 );

    QLabel *titleLabel = new QLabel( title, row );
    titleLabel->setProperty( "aiRole", u"rowTitle"_s );
    QFont titleFont = titleLabel->font();
    titleFont.setBold( true );
    titleLabel->setFont( titleFont );
    rowLayout->addWidget( titleLabel );
    if ( !description.isEmpty() )
    {
      QLabel *descriptionLabel = new QLabel( description, row );
      descriptionLabel->setProperty( "aiRole", u"rowDescription"_s );
      descriptionLabel->setWordWrap( true );
      rowLayout->addWidget( descriptionLabel );
    }
    if ( control )
    {
      control->setAccessibleName( title );
      rowLayout->addWidget( control );
    }
    return row;
  }
} // namespace QgsAiSettingsUtils

#endif // QGSAISETTINGSUTILS_H
