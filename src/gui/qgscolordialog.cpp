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
#include "qgssymbollayerv2utils.h"
#include "qgscursors.h"
#include "qgsapplication.h"
#include <QSettings>
#include <QPushButton>
#include <QMenu>
#include <QToolButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QInputDialog>

QgsColorDialog::QgsColorDialog()
{
}

QgsColorDialog::~QgsColorDialog()
{
}

QColor QgsColorDialog::getLiveColor( const QColor& initialColor, QObject* updateObject, const char* updateSlot,
                                     QWidget* parent,
                                     const QString& title,
                                     const QColorDialog::ColorDialogOptions& options )
{
  QColor returnColor( initialColor );
  QColorDialog* liveDialog = new QColorDialog( initialColor, parent );
  liveDialog->setWindowTitle( title.isEmpty() ? tr( "Select Color" ) : title );
  liveDialog->setOptions( options );

  connect( liveDialog, SIGNAL( currentColorChanged( const QColor& ) ),
           updateObject, updateSlot );

  if ( liveDialog->exec() )
  {
    returnColor = liveDialog->currentColor();
  }
  delete liveDialog;
  liveDialog = nullptr;

  return returnColor;
}


//
// QgsColorDialogV2
//

QgsColorDialogV2::QgsColorDialogV2( QWidget *parent, const Qt::WindowFlags& fl, const QColor& color )
    : QDialog( parent, fl )
    , mPreviousColor( color )
    , mAllowAlpha( true )
{
  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/ColorDialog/geometry" ).toByteArray() );

  if ( mPreviousColor.isValid() )
  {
    QPushButton* resetButton = new QPushButton( tr( "Reset" ) );
    mButtonBox->addButton( resetButton, QDialogButtonBox::ResetRole );
  }

  if ( color.isValid() )
  {
    mColorWidget->setColor( color );
    mColorWidget->setPreviousColor( color );
  }

  mColorWidget->setAllowAlpha( true );

  connect( mColorWidget, SIGNAL( currentColorChanged( QColor ) ), this, SIGNAL( currentColorChanged( QColor ) ) );
}

QgsColorDialogV2::~QgsColorDialogV2()
{

}

QColor QgsColorDialogV2::color() const
{
  return mColorWidget->color();
}

void QgsColorDialogV2::setTitle( const QString& title )
{
  setWindowTitle( title.isEmpty() ? tr( "Select Color" ) : title );
}

void QgsColorDialogV2::setAllowAlpha( const bool allowAlpha )
{
  mAllowAlpha = allowAlpha;
  mColorWidget->setAllowAlpha( allowAlpha );
}

QColor QgsColorDialogV2::getLiveColor( const QColor &initialColor, QObject *updateObject, const char *updateSlot, QWidget *parent, const QString &title, const bool allowAlpha )
{
  QColor returnColor( initialColor );
  QgsColorDialogV2* liveDialog = new QgsColorDialogV2( parent, nullptr, initialColor );
  liveDialog->setWindowTitle( title.isEmpty() ? tr( "Select Color" ) : title );
  if ( !allowAlpha )
  {
    liveDialog->setAllowAlpha( false );
  }

  connect( liveDialog, SIGNAL( currentColorChanged( const QColor& ) ),
           updateObject, updateSlot );

  if ( liveDialog->exec() )
  {
    returnColor = liveDialog->color();
  }
  delete liveDialog;
  liveDialog = nullptr;

  return returnColor;
}

QColor QgsColorDialogV2::getColor( const QColor &initialColor, QWidget *parent, const QString &title, const bool allowAlpha )
{
  QString dialogTitle = title.isEmpty() ? tr( "Select Color" ) : title;

  QSettings settings;
  //using native color dialogs?
  bool useNative = settings.value( "/qgis/native_color_dialogs", false ).toBool();
  if ( useNative )
  {
    return QColorDialog::getColor( initialColor, parent, dialogTitle, allowAlpha ? QColorDialog::ShowAlphaChannel : ( QColorDialog::ColorDialogOption )0 );
  }
  else
  {
    QgsColorDialogV2* dialog = new QgsColorDialogV2( parent, nullptr, initialColor );
    dialog->setWindowTitle( dialogTitle );
    dialog->setAllowAlpha( allowAlpha );

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

void QgsColorDialogV2::on_mButtonBox_accepted()
{
  saveSettings();
  accept();
}

void QgsColorDialogV2::on_mButtonBox_rejected()
{
  saveSettings();
  reject();
}

void QgsColorDialogV2::on_mButtonBox_clicked( QAbstractButton * button )
{
  if ( mButtonBox->buttonRole( button ) == QDialogButtonBox::ResetRole && mPreviousColor.isValid() )
  {
    setColor( mPreviousColor );
  }
}

void QgsColorDialogV2::saveSettings()
{
  QSettings settings;
  settings.setValue( "/Windows/ColorDialog/geometry", saveGeometry() );
}

void QgsColorDialogV2::setColor( const QColor &color )
{
  if ( !color.isValid() )
  {
    return;
  }

  QColor fixedColor = QColor( color );
  if ( !mAllowAlpha )
  {
    //alpha disallowed, so don't permit transparent colors
    fixedColor.setAlpha( 255 );
  }

  mColorWidget->setColor( fixedColor );
  emit currentColorChanged( fixedColor );
}

void QgsColorDialogV2::closeEvent( QCloseEvent *e )
{
  saveSettings();
  QDialog::closeEvent( e );
}
