/*******************************************************************
                              qgsgrassutils.cpp
                             -------------------
    begin                : March, 2006
    copyright            : (C) 2006 by Radim Blazek
    email                : radim.blazek@gmail.com
********************************************************************/
/********************************************************************
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
*******************************************************************/

#include "qgsgrassutils.h"
#include "moc_qgsgrassutils.cpp"
#include "qgsgrassselect.h"
#include "qgsgrass.h"

#include "qgisinterface.h"
#include "qgsapplication.h"
#include "qgslogger.h"

#include <QFileInfo>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

QString QgsGrassUtils::vectorLayerName( QString map, QString layer, int nLayers )
{
  QString name = map;
  if ( nLayers > 1 )
    name += " " + layer;
  return name;
}

void QgsGrassUtils::addVectorLayers( QgisInterface *iface, QString gisbase, QString location, QString mapset, QString map )
{
  QStringList layers;
  try
  {
    layers = QgsGrass::vectorLayers( gisbase, location, mapset, map );
  }
  catch ( QgsGrass::Exception &e )
  {
    QgsDebugError( e.what() );
    return;
  }

  for ( int i = 0; i < layers.count(); i++ )
  {
    QString name = QgsGrassUtils::vectorLayerName( map, layers[i], layers.size() );

    QString uri = gisbase + "/" + location + "/"
                  + mapset + "/" + map + "/" + layers[i];

    QgsDebugMsgLevel( QString( "layer = %1" ).arg( layers[i].toLocal8Bit().constData() ), 3 );
    QgsDebugMsgLevel( QString( "uri = %1" ).arg( uri.toLocal8Bit().constData() ), 3 );
    QgsDebugMsgLevel( QString( "name = %1" ).arg( name.toLocal8Bit().constData() ), 3 );

    iface->addVectorLayer( uri, name, QStringLiteral( "grass" ) );
  }
}

bool QgsGrassUtils::itemExists( QString element, QString item )
{
  QString path = QgsGrass::getDefaultGisdbase() + "/"
                 + QgsGrass::getDefaultLocation() + "/"
                 + QgsGrass::getDefaultMapset() + "/"
                 + "/" + element + "/" + item;

  QFileInfo fi( path );
  return fi.exists();
}


QString QgsGrassUtils::htmlBrowserPath()
{
  return QgsApplication::libexecPath() + "grass/bin/qgis.g.browser" + QString::number( QgsGrass::versionMajor() );
}

QgsGrassElementDialog::QgsGrassElementDialog( QWidget *parent )
  : mParent( parent )
{
}

QString QgsGrassElementDialog::getItem( QString element, QString title, QString label, QString text, QString source, bool *ok )
{
  if ( ok )
    *ok = false;
  mElement = element;
  mSource = source;
  mDialog = new QDialog( mParent );
  mDialog->setWindowTitle( title );
  QVBoxLayout *layout = new QVBoxLayout( mDialog );
  QHBoxLayout *buttonLayout = new QHBoxLayout();

  mLabel = new QLabel( label );
  layout->addWidget( mLabel );

  mLineEdit = new QLineEdit( text );
  QRegularExpression rx;
  if ( element == QLatin1String( "vector" ) )
  {
    rx.setPattern( QStringLiteral( "[A-Za-z_][A-Za-z0-9_]+" ) );
  }
  else
  {
    rx.setPattern( QStringLiteral( "[A-Za-z0-9_.]+" ) );
  }
  QRegularExpressionValidator *val = new QRegularExpressionValidator( rx, this );
  mLineEdit->setValidator( val );

  layout->addWidget( mLineEdit );

  mErrorLabel = new QLabel( QStringLiteral( "X" ) );
  layout->addWidget( mErrorLabel );
  // Intention: keep fixed size - but it does not help
  mErrorLabel->adjustSize();
  mErrorLabel->setMinimumHeight( mErrorLabel->height() + 5 );

  mOkButton = new QPushButton();
  mCancelButton = new QPushButton( tr( "Cancel" ) );

  layout->insertLayout( -1, buttonLayout );
  buttonLayout->addWidget( mOkButton );
  buttonLayout->addWidget( mCancelButton );

  connect( mLineEdit, &QLineEdit::textChanged, this, &QgsGrassElementDialog::textChanged );
  connect( mOkButton, &QAbstractButton::clicked, mDialog, &QDialog::accept );
  connect( mCancelButton, &QAbstractButton::clicked, mDialog, &QDialog::reject );

  textChanged();
  if ( ok && mDialog->exec() == QDialog::Accepted )
  {
    *ok = true;
  }

  QString name = mLineEdit->text();
  delete mDialog;

  return name;
}

void QgsGrassElementDialog::textChanged()
{
  QString text = mLineEdit->text().trimmed();

  mErrorLabel->setText( QStringLiteral( "   " ) );
  mOkButton->setText( tr( "OK" ) );
  mOkButton->setEnabled( true );

  if ( text.length() == 0 )
  {
    mErrorLabel->setText( tr( "<font color='red'>Enter a name!</font>" ) );
    mOkButton->setEnabled( false );
    return;
  }

#ifdef Q_OS_WIN
  if ( !mSource.isNull() && text.toLower() == mSource.toLower() )
#else
  if ( !mSource.isNull() && text == mSource )
#endif
  {
    mErrorLabel->setText( tr( "<font color='red'>This is name of the source!</font>" ) );
    mOkButton->setEnabled( false );
    return;
  }
  if ( QgsGrassUtils::itemExists( mElement, text ) )
  {
    mErrorLabel->setText( tr( "<font color='red'>Exists!</font>" ) );
    mOkButton->setText( tr( "Overwrite" ) );
    return;
  }
}
