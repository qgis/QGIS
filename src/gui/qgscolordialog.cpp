/***************************************************************************
    qgscolordialog.cpp - color selection dialog

    ---------------------
    begin                : March 19, 2013
    copyright            : (C) 2013 by Larry Shaffer
    email                : larrys at dakcarto dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscolordialog.h"
#include "qgscolorscheme.h"
#include "qgscolorschemeregistry.h"
#include "qgssymbollayerutils.h"
#include "qgscursors.h"
#include "qgsapplication.h"
#include "qgssettings.h"

#include <QPushButton>
#include <QMenu>
#include <QToolButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QInputDialog>

QgsColorDialog::QgsColorDialog( QWidget *parent, Qt::WindowFlags fl, const QColor &color )
  : QDialog( parent, fl )
  , mPreviousColor( color )
{
  setupUi( this );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsColorDialog::mButtonBox_accepted );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsColorDialog::mButtonBox_rejected );
  connect( mButtonBox, &QDialogButtonBox::clicked, this, &QgsColorDialog::mButtonBox_clicked );

  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/ColorDialog/geometry" ) ).toByteArray() );

  if ( mPreviousColor.isValid() )
  {
    QPushButton *resetButton = new QPushButton( tr( "Reset" ) );
    mButtonBox->addButton( resetButton, QDialogButtonBox::ResetRole );
  }

  if ( color.isValid() )
  {
    mColorWidget->setColor( color );
    mColorWidget->setPreviousColor( color );
  }

  mColorWidget->setAllowOpacity( true );

  connect( mColorWidget, &QgsCompoundColorWidget::currentColorChanged, this, &QgsColorDialog::currentColorChanged );
  connect( this, &QDialog::rejected, this, &QgsColorDialog::discardColor );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsColorDialog::showHelp );
}

QColor QgsColorDialog::color() const
{
  return mColorWidget->color();
}

void QgsColorDialog::setTitle( const QString &title )
{
  setWindowTitle( title.isEmpty() ? tr( "Select Color" ) : title );
}

void QgsColorDialog::setAllowOpacity( const bool allowOpacity )
{
  mAllowOpacity = allowOpacity;
  mColorWidget->setAllowOpacity( allowOpacity );
}

QColor QgsColorDialog::getLiveColor( const QColor &initialColor, QObject *updateObject, const char *updateSlot, QWidget *parent, const QString &title, const bool allowOpacity )
{
  QColor returnColor( initialColor );

  QgsSettings settings;

  //using native color dialogs?
  bool useNative = settings.value( QStringLiteral( "qgis/native_color_dialogs" ), false ).toBool();
  if ( useNative )
  {
    QColorDialog *liveDialog = new QColorDialog( initialColor, parent );
    liveDialog->setWindowTitle( title.isEmpty() ? tr( "Select Color" ) : title );
    liveDialog->setOptions( allowOpacity ? QColorDialog::ShowAlphaChannel : ( QColorDialog::ColorDialogOption )0 );

    connect( liveDialog, SIGNAL( currentColorChanged( const QColor & ) ),
             updateObject, updateSlot );

    if ( liveDialog->exec() )
    {
      returnColor = liveDialog->currentColor();
    }
    delete liveDialog;
  }
  else
  {
    QgsColorDialog *liveDialog = new QgsColorDialog( parent, 0, initialColor );
    liveDialog->setWindowTitle( title.isEmpty() ? tr( "Select Color" ) : title );
    if ( !allowOpacity )
    {
      liveDialog->setAllowOpacity( false );
    }

    connect( liveDialog, SIGNAL( currentColorChanged( const QColor & ) ),
             updateObject, updateSlot );

    if ( liveDialog->exec() )
    {
      returnColor = liveDialog->color();
    }
    delete liveDialog;
  }

  return returnColor;
}

QColor QgsColorDialog::getColor( const QColor &initialColor, QWidget *parent, const QString &title, const bool allowOpacity )
{
  QString dialogTitle = title.isEmpty() ? tr( "Select Color" ) : title;

  QgsSettings settings;
  //using native color dialogs?
  bool useNative = settings.value( QStringLiteral( "qgis/native_color_dialogs" ), false ).toBool();
  if ( useNative )
  {
    return QColorDialog::getColor( initialColor, parent, dialogTitle, allowOpacity ? QColorDialog::ShowAlphaChannel : ( QColorDialog::ColorDialogOption )0 );
  }
  else
  {
    QgsColorDialog *dialog = new QgsColorDialog( parent, 0, initialColor );
    dialog->setWindowTitle( dialogTitle );
    dialog->setAllowOpacity( allowOpacity );

    QColor result;
    if ( dialog->exec() )
    {
      result = dialog->color();
    }

    if ( !parent )
    {
      delete dialog;
    }
    return result;
  }
}

void QgsColorDialog::mButtonBox_accepted()
{
  saveSettings();
  accept();
}

void QgsColorDialog::mButtonBox_rejected()
{
  saveSettings();
  reject();
}

void QgsColorDialog::mButtonBox_clicked( QAbstractButton *button )
{
  if ( mButtonBox->buttonRole( button ) == QDialogButtonBox::ResetRole && mPreviousColor.isValid() )
  {
    setColor( mPreviousColor );
  }
}

void QgsColorDialog::discardColor()
{
  mColorWidget->setDiscarded( true );
}

void QgsColorDialog::saveSettings()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/ColorDialog/geometry" ), saveGeometry() );
}

void QgsColorDialog::setColor( const QColor &color )
{
  if ( !color.isValid() )
  {
    return;
  }

  QColor fixedColor = QColor( color );
  if ( !mAllowOpacity )
  {
    //alpha disallowed, so don't permit transparent colors
    fixedColor.setAlpha( 255 );
  }

  mColorWidget->setColor( fixedColor );
  emit currentColorChanged( fixedColor );
}

void QgsColorDialog::closeEvent( QCloseEvent *e )
{
  saveSettings();
  QDialog::closeEvent( e );
}

void QgsColorDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#color-selector" ) );
}
