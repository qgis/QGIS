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
#include <QRegularExpressionValidator>
#include <QSizePolicy>

#include "qgslogger.h"
#include "qgsnewnamedialog.h"

QgsNewNameDialog::QgsNewNameDialog( const QString &source,
                                    const QString &initial,
                                    const QStringList &extensions,
                                    const QStringList &existing,
                                    Qt::CaseSensitivity cs,
                                    QWidget *parent,
                                    Qt::WindowFlags flags )
  : QgsDialog( parent, flags, QDialogButtonBox::Ok | QDialogButtonBox::Cancel )
  , mExiting( existing )
  , mExtensions( extensions )
  , mCaseSensitivity( cs )
{
  setWindowTitle( tr( "New Name" ) );
  QgsDialog::layout()->setSizeConstraint( QLayout::SetMinimumSize );
  layout()->setSizeConstraint( QLayout::SetMinimumSize );
  layout()->setSpacing( 6 );
  mOkString = buttonBox()->button( QDialogButtonBox::Ok )->text();
  QString hintString;
  const QString nameDesc = mExtensions.isEmpty() ? tr( "name" ) : tr( "base name" );
  if ( source.isEmpty() )
  {
    hintString = tr( "Enter new %1" ).arg( nameDesc );
  }
  else
  {
    hintString = tr( "Enter new %1 for %2" ).arg( nameDesc, source );
  }
  mHintLabel = new QLabel( hintString, this );
  layout()->addWidget( mHintLabel );

  mLineEdit = new QLineEdit( initial, this );
  mLineEdit->setMinimumWidth( mLineEdit->fontMetrics().horizontalAdvance( 'x' ) * 44 );

  connect( mLineEdit, &QLineEdit::textChanged, this, &QgsNewNameDialog::nameChanged );
  connect( mLineEdit, &QLineEdit::textChanged, this, &QgsNewNameDialog::newNameChanged );
  layout()->addWidget( mLineEdit );

  mNamesLabel = new QLabel( QStringLiteral( " " ), this );
  mNamesLabel->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  if ( !mExtensions.isEmpty() )
  {
    mNamesLabel->setWordWrap( true );
    layout()->addWidget( mNamesLabel );
  }

  mErrorLabel = new QLabel( QStringLiteral( " " ), this );
  mErrorLabel->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  mErrorLabel->setWordWrap( true );
  layout()->addWidget( mErrorLabel );

  mLineEdit->setFocus();
  mLineEdit->selectAll();

  nameChanged();
}

void QgsNewNameDialog::setHintString( const QString &hintString )
{
  mHintLabel->setText( hintString );
}

QString QgsNewNameDialog::hintString() const
{
  return mHintLabel->text();
}

void QgsNewNameDialog::setOverwriteEnabled( bool enabled )
{
  mOverwriteEnabled = enabled;
  nameChanged(); //update UI
}

void QgsNewNameDialog::setAllowEmptyName( bool allowed )
{
  mAllowEmptyName = allowed;
  nameChanged(); //update UI
}

void QgsNewNameDialog::setConflictingNameWarning( const QString &string )
{
  mConflictingNameWarning = string;
  nameChanged(); //update UI
}

void QgsNewNameDialog::setRegularExpression( const QString &expression )
{
  if ( !expression.isEmpty() )
  {
    mRegularExpression = QRegularExpression( QRegularExpression::anchoredPattern( expression ) );
    QRegularExpressionValidator *validator = new QRegularExpressionValidator( mRegularExpression, this );
    mLineEdit->setValidator( validator );
  }
  else
  {
    mRegularExpression = QRegularExpression();
    mLineEdit->setValidator( nullptr );
  }
  nameChanged();
}

QString QgsNewNameDialog::highlightText( const QString &text )
{
  return "<b>" + text + "</b>";
}

void QgsNewNameDialog::nameChanged()
{
  QString namesString = tr( "Full names" ) + ": ";
  if ( !mExtensions.isEmpty() )
  {
    mNamesLabel->setText( namesString );
  }
  mErrorLabel->setText( QStringLiteral( " " ) ); // space to keep vertical space
  QPushButton *okButton = buttonBox()->button( QDialogButtonBox::Ok );
  okButton->setText( mOkString );
  okButton->setEnabled( true );

  const QString newName = name();

  if ( newName.length() == 0 || ( !mRegularExpression.pattern().isEmpty() && !mRegularExpression.match( newName ).hasMatch() ) )
  {
    //mErrorLabel->setText( highlightText( tr( "Enter new name" ) );
    okButton->setEnabled( mAllowEmptyName );
    return;
  }

  const QStringList newNames = fullNames( newName, mExtensions );
  if ( !mExtensions.isEmpty() )
  {
    namesString += ' ' + newNames.join( QLatin1String( ", " ) );
    mNamesLabel->setText( namesString );
  }

  const QStringList conflicts = matching( newNames, mExiting, mCaseSensitivity );

  if ( !conflicts.isEmpty() )
  {
    const QString warning = !mConflictingNameWarning.isEmpty() ? mConflictingNameWarning
                            : tr( "%n Name(s) %1 exists", nullptr, conflicts.size() ).arg( conflicts.join( QLatin1String( ", " ) ) );
    mErrorLabel->setText( highlightText( warning ) );
    if ( mOverwriteEnabled )
    {
      okButton->setText( tr( "Overwrite" ) );
    }
    else
    {
      okButton->setEnabled( false );
    }
    return;
  }
}

QString QgsNewNameDialog::name() const
{
  return mLineEdit->text().trimmed();
}

QStringList QgsNewNameDialog::fullNames( const QString &name, const QStringList &extensions )
{
  QStringList list;
  const auto constExtensions = extensions;
  for ( const QString &ext : constExtensions )
  {
    list << name + ext;

  }
  if ( list.isEmpty() )
  {
    list << name;
  }
  return list;
}

QStringList QgsNewNameDialog::matching( const QStringList &newNames, const QStringList &existingNames,
                                        Qt::CaseSensitivity cs )
{
  QStringList list;

  const auto constNewNames = newNames;
  for ( const QString &newName : constNewNames )
  {
    const auto constExistingNames = existingNames;
    for ( const QString &existingName : constExistingNames )
    {
      if ( existingName.compare( newName, cs ) == 0 )
      {
        list << existingName;
      }
    }
  }
  return list;
}

bool QgsNewNameDialog::exists( const QString &name, const QStringList &extensions,
                               const QStringList &existing, Qt::CaseSensitivity cs )
{
  const QStringList newNames = fullNames( name, extensions );
  const QStringList conflicts = matching( newNames, existing, cs );
  return !conflicts.isEmpty();
}
