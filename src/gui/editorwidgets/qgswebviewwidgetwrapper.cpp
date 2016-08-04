/***************************************************************************
    qgswebviewwidgetwrapper.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswebviewwidgetwrapper.h"

#include "qgsfilterlineedit.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsproject.h"

#include <QGridLayout>
#include <QFileDialog>
#include <QSettings>

QgsWebViewWidgetWrapper::QgsWebViewWidgetWrapper( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent )
    : QgsEditorWidgetWrapper( vl, fieldIdx, editor, parent )
    , mWebView( nullptr )
    , mLineEdit( nullptr )
    , mButton( nullptr )
{
}

void QgsWebViewWidgetWrapper::loadUrl( const QString &url )
{
  QString path = url;

  if ( QUrl( url ).isRelative() )
    path = QDir( QgsProject::instance()->fileInfo().absolutePath() ).filePath( url );

  if ( mWebView )
    mWebView->load( path );
}

QVariant QgsWebViewWidgetWrapper::value() const
{
  QVariant v;

  if ( mLineEdit )
  {
    if ( mLineEdit->text() == QSettings().value( "qgis/nullValue", "NULL" ).toString() )
      v = QVariant( QVariant::String );
    else
      v = mLineEdit->text();
  }

  return v;
}

void QgsWebViewWidgetWrapper::showIndeterminateState()
{
  if ( mLineEdit )
  {
    whileBlocking( mLineEdit )->clear();
  }

  if ( mWebView )
    mWebView->load( QString() );
}

QWidget* QgsWebViewWidgetWrapper::createWidget( QWidget* parent )
{
  QWidget* container = new QWidget( parent );
  QGridLayout* layout = new QGridLayout();
  QgsFilterLineEdit* le = new QgsFilterLineEdit();
  QWebView* webView = new QWebView();
  webView->setObjectName( "EditorWebView" );
  QPushButton* pb = new QPushButton( tr( "..." ) );
  pb->setObjectName( "FileChooserButton" );

  layout->addWidget( webView, 0, 0, 1, 2 );
  layout->addWidget( le, 1, 0 );
  layout->addWidget( pb, 1, 1 );
  layout->setMargin( 0 );
  layout->setContentsMargins( 0, 0, 0, 0 );

  container->setLayout( layout );

  return container;
}

void QgsWebViewWidgetWrapper::initWidget( QWidget* editor )
{
  QWidget* container;

  mLineEdit = qobject_cast<QLineEdit*>( editor );

  if ( mLineEdit )
  {

    QgsFilterLineEdit* fle = qobject_cast<QgsFilterLineEdit*>( mLineEdit );
    if ( fle )
    {
      fle->setNullValue( QSettings().value( "qgis/nullValue", "NULL" ).toString() );
    }

    container = qobject_cast<QWidget*>( mLineEdit->parent() );
  }
  else
  {
    container = editor;
    mLineEdit = container->findChild<QLineEdit*>();
  }

  mButton = container->findChild<QPushButton*>( "FileChooserButton" );
  if ( !mButton )
    mButton = container->findChild<QPushButton*>();

  mWebView = container->findChild<QWebView*>( "EditorWebView" );
  if ( !mWebView )
    mWebView = container->findChild<QWebView*>();

  if ( mWebView )
  {
    mWebView->page()->setNetworkAccessManager( QgsNetworkAccessManager::instance() );
    mWebView->settings()->setAttribute( QWebSettings::LocalContentCanAccessRemoteUrls, true );
    mWebView->settings()->setAttribute( QWebSettings::JavascriptCanOpenWindows, true );
#ifdef QGISDEBUG
    mWebView->settings()->setAttribute( QWebSettings::DeveloperExtrasEnabled, true );
#endif
  }

  if ( mButton )
    connect( mButton, SIGNAL( clicked() ), this, SLOT( selectFileName() ) );

  if ( mLineEdit )
  {
    connect( mLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( loadUrl( QString ) ) );
    connect( mLineEdit, SIGNAL( textChanged( QString ) ), this, SLOT( valueChanged( QString ) ) );
  }
}

bool QgsWebViewWidgetWrapper::valid() const
{
  return mWebView || mButton || mLineEdit;
}

void QgsWebViewWidgetWrapper::setValue( const QVariant& value )
{
  if ( mLineEdit )
  {
    if ( value.isNull() )
      mLineEdit->setText( QSettings().value( "qgis/nullValue", "NULL" ).toString() );
    else
      mLineEdit->setText( value.toString() );
  }

  loadUrl( value.toString() );
}

void QgsWebViewWidgetWrapper::setEnabled( bool enabled )
{
  if ( mLineEdit )
    mLineEdit->setEnabled( enabled );

  if ( mButton )
    mButton->setEnabled( enabled );
}

void QgsWebViewWidgetWrapper::selectFileName()
{
  QString text;

  if ( mLineEdit )
    text = mLineEdit->text();

  QString fileName = QFileDialog::getOpenFileName( mLineEdit, tr( "Select a file" ), QFileInfo( text ).absolutePath() );

  if ( fileName.isNull() )
    return;

  QString projPath = QDir::toNativeSeparators( QDir::cleanPath( QgsProject::instance()->fileInfo().absolutePath() ) );
  QString filePath = QDir::toNativeSeparators( QDir::cleanPath( QFileInfo( fileName ).absoluteFilePath() ) );

  if ( filePath.startsWith( projPath ) )
    filePath = QDir( projPath ).relativeFilePath( filePath );

  if ( mLineEdit )
    mLineEdit->setText( filePath );
}

void QgsWebViewWidgetWrapper::updateConstraintWidgetStatus( bool constraintValid )
{
  if ( mLineEdit )
  {
    if ( constraintValid )
      mLineEdit->setStyleSheet( QString() );
    else
    {
      mLineEdit->setStyleSheet( "QgsFilterLineEdit { background-color: #dd7777; }" );
    }
  }
}
