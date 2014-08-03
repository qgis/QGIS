/***************************************************************************
                         qgsattributetypedialog.cpp  -  description
                             -------------------
    begin                : June 2009
    copyright            : (C) 2000 by Richard Kostecky
    email                : cSf.Kostej@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributetypedialog.h"
#include "qgsattributetypeloaddialog.h"
#include "qgsvectordataprovider.h"
#include "qgsmaplayerregistry.h"
#include "qgsmapcanvas.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgisapp.h"
#include "qgsproject.h"
#include "qgslogger.h"
#include "qgseditorwidgetfactory.h"
#include "qgseditorwidgetregistry.h"

#include <QTableWidgetItem>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QScrollBar>

#include <climits>
#include <cfloat>

QgsAttributeTypeDialog::QgsAttributeTypeDialog( QgsVectorLayer *vl , int fieldIdx )
    : QDialog()
    , mLayer( vl )
    , mFieldIdx( fieldIdx )
{
  setupUi( this );
  connect( selectionListWidget, SIGNAL( currentRowChanged( int ) ), this, SLOT( setStackPage( int ) ) );

  QMapIterator<QString, QgsEditorWidgetFactory*> it( QgsEditorWidgetRegistry::instance()->factories() );
  while ( it.hasNext() )
  {
    it.next();
    QListWidgetItem* item = new QListWidgetItem( selectionListWidget );
    item->setText( it.value()->name() );
    item->setData( Qt::UserRole, it.key() );
    if ( !it.value()->supportsField( vl, fieldIdx ) )
      item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
    selectionListWidget->addItem( item );
  }

  // Set required list width based on content + twice the border width
  selectionListWidget->setMinimumWidth( selectionListWidget->sizeHintForColumn( 0 )
                                        + 2 );
  selectionListWidget->setMaximumWidth( selectionListWidget->sizeHintForColumn( 0 )
                                        + 2 );

  if ( vl->pendingFields().fieldOrigin( fieldIdx ) == QgsFields::OriginJoin ||
       vl->pendingFields().fieldOrigin( fieldIdx ) == QgsFields::OriginExpression )
  {
    isFieldEditableCheckBox->setEnabled( false );
  }

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/QgsAttributeTypeDialog/geometry" ).toByteArray() );
}

QgsAttributeTypeDialog::~QgsAttributeTypeDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/QgsAttributeTypeDialog/geometry", saveGeometry() );

  qDeleteAll( mEditorConfigWidgets.values() );
}

const QString QgsAttributeTypeDialog::editorWidgetV2Type()
{
  QListWidgetItem* item = selectionListWidget->currentItem();
  if ( item )
  {
    return item->data( Qt::UserRole ).toString();
  }
  else
  {
    return QString();
  }
}

const QString QgsAttributeTypeDialog::editorWidgetV2Text()
{
  QListWidgetItem* item = selectionListWidget->currentItem();
  if ( item )
  {
    return item->text();
  }
  else
  {
    return QString();
  }
}

const QgsEditorWidgetConfig QgsAttributeTypeDialog::editorWidgetV2Config()
{
  QListWidgetItem* item = selectionListWidget->currentItem();
  if ( item )
  {
    QString widgetType = item->data( Qt::UserRole ).toString();
    QgsEditorConfigWidget* cfgWdg = mEditorConfigWidgets[ widgetType ];
    if ( cfgWdg )
    {
      return cfgWdg->config();
    }
  }

  return QgsEditorWidgetConfig();
}

void QgsAttributeTypeDialog::setWidgetV2Type( const QString& type )
{
  for ( int i = 0; i < selectionListWidget->count(); i++ )
  {
    QListWidgetItem* item = selectionListWidget->item( i );
    if ( item->data( Qt::UserRole ).toString() == type )
    {
      selectionListWidget->setCurrentItem( item );
      break;
    }
  }

  if ( mEditorConfigWidgets.contains( type ) )
  {
    stackedWidget->setCurrentWidget( mEditorConfigWidgets[type] );
  }
  else
  {
    QgsEditorConfigWidget* cfgWdg = QgsEditorWidgetRegistry::instance()->createConfigWidget( type, mLayer, mFieldIdx, this );

    if ( cfgWdg )
    {
      cfgWdg->setConfig( mWidgetV2Config );

      stackedWidget->addWidget( cfgWdg );
      stackedWidget->setCurrentWidget( cfgWdg );
      mEditorConfigWidgets.insert( type, cfgWdg );
    }
    else
    {
      QgsDebugMsg( "Oops, couldn't create editor widget config dialog..." );
    }
  }
}

void QgsAttributeTypeDialog::setWidgetV2Config( const QgsEditorWidgetConfig& config )
{
  mWidgetV2Config = config;
}

bool QgsAttributeTypeDialog::fieldEditable()
{
  return isFieldEditableCheckBox->isChecked();
}

bool QgsAttributeTypeDialog::labelOnTop()
{
  return labelOnTopCheckBox->isChecked();
}

void QgsAttributeTypeDialog::setFieldEditable( bool editable )
{
  isFieldEditableCheckBox->setChecked( editable );
}

void QgsAttributeTypeDialog::setLabelOnTop( bool onTop )
{
  labelOnTopCheckBox->setChecked( onTop );
}

void QgsAttributeTypeDialog::on_selectionListWidget_currentRowChanged( int index )
{
  const QString editType = selectionListWidget->item( index )->data( Qt::UserRole ).toString();

  setWidgetV2Type( editType );
}
