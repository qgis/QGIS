/***************************************************************************
    qgsselectbyformdialog.cpp
     ------------------------
    Date                 : June 2016
    Copyright            : (C) 2016 nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsselectbyformdialog.h"
#include "qgsattributeform.h"
#include <QLayout>
#include <QSettings>

QgsSelectByFormDialog::QgsSelectByFormDialog( QgsVectorLayer* layer, const QgsAttributeEditorContext& context, QWidget* parent, Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
  QgsAttributeEditorContext dlgContext = context;
  dlgContext.setFormMode( QgsAttributeEditorContext::StandaloneDialog );
  dlgContext.setAllowCustomUi( false );

  mForm = new QgsAttributeForm( layer, QgsFeature(), dlgContext, this );
  mForm->setMode( QgsAttributeForm::SearchMode );

  QVBoxLayout* vLayout = new QVBoxLayout();
  vLayout->setMargin( 0 );
  vLayout->setContentsMargins( 0, 0, 0, 0 );
  setLayout( vLayout );

  vLayout->addWidget( mForm );

  connect( mForm, SIGNAL( closed() ), this, SLOT( close() ) );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/SelectByForm/geometry" ).toByteArray() );

  setWindowTitle( tr( "Select features by value" ) );
}

QgsSelectByFormDialog::~QgsSelectByFormDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/SelectByForm/geometry", saveGeometry() );
}

void QgsSelectByFormDialog::setMessageBar( QgsMessageBar* messageBar )
{
  mForm->setMessageBar( messageBar );
}
