/***************************************************************************
    qgsbinarywidgetwrapper.cpp
     -------------------------
    Date                 : November 2018
    Copyright            : (C) 2018 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsbinarywidgetwrapper.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsfileutils.h"
#include "qgsfocuskeeper.h"
#include "qgssettings.h"
#include "qgsmessagebar.h"
#include "qgsapplication.h"
#include <QHBoxLayout>
#include <QFileDialog>
#include <QLabel>
#include <QToolButton>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QUrl>

QgsBinaryWidgetWrapper::QgsBinaryWidgetWrapper( QgsVectorLayer *layer, int fieldIdx, QWidget *editor, QWidget *parent, QgsMessageBar *messageBar )
  : QgsEditorWidgetWrapper( layer, fieldIdx, editor, parent )
  , mMessageBar( messageBar )
{
}


QVariant QgsBinaryWidgetWrapper::value() const
{
  return mValue.isEmpty() || mValue.isNull() ? QVariant( QVariant::ByteArray ) : mValue;
}

void QgsBinaryWidgetWrapper::showIndeterminateState()
{
  if ( mLabel )
    mLabel->clear();
}

void QgsBinaryWidgetWrapper::setEnabled( bool enabled )
{
  if ( mSetAction )
    mSetAction->setEnabled( enabled );
  if ( mClearAction )
    mClearAction->setEnabled( enabled && !mValue.isEmpty() );
}

QWidget *QgsBinaryWidgetWrapper::createWidget( QWidget *parent )
{
  QWidget *container = new QWidget( parent );
  QHBoxLayout *layout = new QHBoxLayout();
  container->setLayout( layout );
  layout->setContentsMargins( 0, 0, 0, 0 );

  QLabel *label = new QLabel();
  layout->addWidget( label, 1 );

  QToolButton *button = new QToolButton();
  button->setText( QChar( 0x2026 ) );
  layout->addWidget( button, 0 );

  container->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Minimum );
  return container;
}

void QgsBinaryWidgetWrapper::initWidget( QWidget *editor )
{
  mLabel = editor->findChild<QLabel *>();
  mButton = editor->findChild<QToolButton *>();

  if ( mLabel )
  {
    QFont f = mLabel->font();
    f.setItalic( true );
    mLabel->setFont( f );
  }

  if ( mButton )
  {
    mButton->setPopupMode( QToolButton::InstantPopup );

    mSetAction = new QAction( tr( "Embed File…" ), mButton );
    connect( mSetAction, &QAction::triggered, this, &QgsBinaryWidgetWrapper::setContent );
    mClearAction = new QAction( tr( "Clear Contents…" ), mButton );
    connect( mClearAction, &QAction::triggered, this, &QgsBinaryWidgetWrapper::clear );
    mSaveAction = new QAction( tr( "Save Contents to File…" ), mButton );
    connect( mSaveAction, &QAction::triggered, this, &QgsBinaryWidgetWrapper::saveContent );
    QMenu *menu = new QMenu( mButton );
    menu->addAction( mSetAction );
    menu->addAction( mClearAction );
    menu->addSeparator();
    menu->addAction( mSaveAction );
    mButton->setMenu( menu );
  }
}

bool QgsBinaryWidgetWrapper::valid() const
{
  return mLabel && mButton;
}

void QgsBinaryWidgetWrapper::updateValues( const QVariant &value, const QVariantList & )
{
  mValue = value.isValid() && !QgsVariantUtils::isNull( value ) && value.canConvert< QByteArray >() ? value.toByteArray() : QByteArray();
  if ( mValue.length() == 0 )
    mValue = QByteArray();

  if ( mLabel )
  {
    if ( !mValue.isEmpty() )
    {
      mLabel->setText( tr( "Binary (%1)" ).arg( QgsFileUtils::representFileSize( mValue.size() ) ) );
    }
    else
    {
      mLabel->setText( QgsApplication::nullRepresentation() );
    }
  }
  if ( mSaveAction )
    mSaveAction->setEnabled( !mValue.isEmpty() );
  if ( mClearAction )
    mClearAction->setEnabled( !mValue.isEmpty() );
}

void QgsBinaryWidgetWrapper::saveContent()
{
  QgsSettings s;

  QString file;
  {
    const QgsFocusKeeper focusKeeper;

    file = QFileDialog::getSaveFileName( nullptr,
                                         tr( "Save Contents to File" ),
                                         defaultPath(),
                                         tr( "All files" ) + " (*.*)" );
  }
  if ( file.isEmpty() )
  {
    return;
  }

  const QFileInfo fi( file );
  s.setValue( QStringLiteral( "/UI/lastBinaryDir" ), fi.absolutePath() );

  QFile fileOut( file );
  fileOut.open( QIODevice::WriteOnly );
  fileOut.write( mValue );
  fileOut.close();

  if ( mMessageBar )
    mMessageBar->pushSuccess( QString(), tr( "Saved content to <a href=\"%1\">%2</a>" ).arg(
                                QUrl::fromLocalFile( file ).toString(), QDir::toNativeSeparators( file ) ) );
}

void QgsBinaryWidgetWrapper::setContent()
{
  QgsSettings s;

  QString file;
  {
    const QgsFocusKeeper focusKeeper;

    file = QFileDialog::getOpenFileName( nullptr,
                                         tr( "Embed File" ),
                                         defaultPath(),
                                         tr( "All files" ) + " (*.*)" );
  }

  const QFileInfo fi( file );
  if ( file.isEmpty() || !fi.exists() )
  {
    return;
  }

  s.setValue( QStringLiteral( "/UI/lastBinaryDir" ), fi.absolutePath() );

  QFile fileSource( file );
  if ( !fileSource.open( QIODevice::ReadOnly ) )
  {
    return;
  }

  updateValues( fileSource.readAll() );
  emitValueChanged();
}

void QgsBinaryWidgetWrapper::clear()
{
  {
    const QgsFocusKeeper focusKeeper;
    if ( QMessageBox::question( nullptr, tr( "Clear Contents" ), tr( "Are you sure you want the clear this field's content?" ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) != QMessageBox::Yes )
      return;
  }

  updateValues( QByteArray() );
  emitValueChanged();
}

QString QgsBinaryWidgetWrapper::defaultPath()
{
  return QgsSettings().value( QStringLiteral( "/UI/lastBinaryDir" ), QDir::homePath() ).toString();
}

