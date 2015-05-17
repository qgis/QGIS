/***************************************************************************
    qgsnewnamedialog.cpp
                             -------------------
    begin                : May, 2015
    copyright            : (C) 2015 Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegExpValidator>
#include <QSizePolicy>

#include "qgslogger.h"
#include "qgsnewnamedialog.h"

QgsNewNameDialog::QgsNewNameDialog( const QString& source, const QString& initial,
                                    const QStringList& extensions, const QStringList& existing,
                                    const QRegExp& regexp, Qt::CaseSensitivity cs,
                                    QWidget *parent, Qt::WindowFlags flags )
    : QgsDialog( parent, flags, QDialogButtonBox::Ok | QDialogButtonBox::Cancel )
    , mExiting( existing )
    , mExtensions( extensions )
    , mCaseSensitivity( cs )
    , mNamesLabel( 0 )
{
  setWindowTitle( tr( "New name" ) );
  QDialog::layout()->setSizeConstraint( QLayout::SetMinimumSize );
  layout()->setSizeConstraint( QLayout::SetMinimumSize );
  mOkString = buttonBox()->button( QDialogButtonBox::Ok )->text();
  QString hintString;
  QString nameDesc = mExtensions.isEmpty() ? tr( "name" ) : tr( "base name" );
  if ( source.isEmpty() )
  {
    hintString = tr( "Enter new %1" ).arg( nameDesc );
  }
  else
  {
    hintString = tr( "Enter new %1 for %2" ).arg( nameDesc ).arg( source );
  }
  QLabel* hintLabel = new QLabel( hintString, this );
  layout()->addWidget( hintLabel );

  mLineEdit = new QLineEdit( initial, this );
  if ( !regexp.isEmpty() )
  {
    QRegExpValidator *validator = new QRegExpValidator( regexp, this );
    mLineEdit->setValidator( validator );
  }
  connect( mLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( nameChanged() ) );
  layout()->addWidget( mLineEdit );

  mNamesLabel = new QLabel( " ", this );
  mNamesLabel->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  if ( !mExtensions.isEmpty() )
  {
    mNamesLabel->setWordWrap( true );
    layout()->addWidget( mNamesLabel );
  }

  mErrorLabel = new QLabel( " ", this );
  mErrorLabel->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  mErrorLabel->setWordWrap( true );
  layout()->addWidget( mErrorLabel );

  nameChanged();
}

QString QgsNewNameDialog::highlightText( const QString& text )
{
  return "<b>" + text + "</b>";
}

void QgsNewNameDialog::nameChanged()
{
  QgsDebugMsg( "entered" );

  QString namesString = tr( "Full names" ) + ": ";
  if ( !mExtensions.isEmpty() )
  {
    mNamesLabel->setText( namesString );
  }
  mErrorLabel->setText( " " ); // space to keep vertical space
  QPushButton* okButton = buttonBox()->button( QDialogButtonBox::Ok );
  okButton->setText( mOkString );
  okButton->setEnabled( true );

  QString newName = name();

  if ( newName.length() == 0 )
  {
    //mErrorLabel->setText( highlightText( tr( "Enter new name" ) );
    okButton->setEnabled( false );
    return;
  }

  QStringList newNames = fullNames( newName, mExtensions );
  if ( !mExtensions.isEmpty() )
  {
    namesString += " " + newNames.join( ", " );
    mNamesLabel->setText( namesString );
  }

  QStringList conflicts = matching( newNames, mExiting, mCaseSensitivity );

  if ( !conflicts.isEmpty() )
  {
    mErrorLabel->setText( highlightText( tr( "Name(s) %1 exists", 0, conflicts.size() ).arg( conflicts.join( ", " ) ) ) );
    okButton->setText( tr( "Overwrite" ) );
    return;
  }
}

QString QgsNewNameDialog::name() const
{
  return mLineEdit->text().trimmed();
}

QStringList QgsNewNameDialog::fullNames( const QString& name, const QStringList& extensions )
{
  QStringList list;
  foreach ( QString ext, extensions )
  {
    list << name + ext;

  }
  if ( list.isEmpty() )
  {
    list << name;
  }
  return list;
}

QStringList QgsNewNameDialog::matching( const QStringList& newNames, const QStringList& existingNames,
                                        Qt::CaseSensitivity cs )
{
  QStringList list;

  foreach ( QString newName, newNames )
  {
    foreach ( QString existingName, existingNames )
    {
      if ( existingName.compare( newName, cs ) == 0 )
      {
        list << existingName;
      }
    }
  }
  return list;
}

bool QgsNewNameDialog::exists( const QString& name, const QStringList& extensions,
                               const QStringList& existing, Qt::CaseSensitivity cs )
{
  QStringList newNames = fullNames( name, extensions );
  QStringList conflicts = matching( newNames, existing, cs );
  return conflicts.size() > 0;
}
