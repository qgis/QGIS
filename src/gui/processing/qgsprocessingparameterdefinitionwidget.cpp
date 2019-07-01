/***************************************************************************
                         qgsprocessingparameterdefinitionwidget.cpp
                         ------------------------------------------
    begin                : July 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsprocessingparameterdefinitionwidget.h"
#include "qgsgui.h"
#include "qgsprocessingguiregistry.h"
#include "qgsapplication.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingparametertype.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMessageBox>

QgsProcessingAbstractParameterDefinitionWidget::QgsProcessingAbstractParameterDefinitionWidget( QgsProcessingContext &,
    const QgsProcessingParameterWidgetContext &,
    const QgsProcessingParameterDefinition *,
    const QgsProcessingAlgorithm *, QWidget *parent )
  : QWidget( parent )
{

}

//
// QgsProcessingParameterDefinitionWidget
//

QgsProcessingParameterDefinitionWidget::QgsProcessingParameterDefinitionWidget( const QString &type,
    QgsProcessingContext &context,
    const QgsProcessingParameterWidgetContext &widgetContext,
    const QgsProcessingParameterDefinition *definition,
    const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QWidget( parent )
  , mType( type )
{
  mDefinitionWidget = QgsGui::instance()->processingGuiRegistry()->createParameterDefinitionWidget( type, context, widgetContext, definition, algorithm );

  QVBoxLayout *vlayout = new QVBoxLayout();

  QLabel *label = new QLabel( tr( "Description" ) );
  vlayout->addWidget( label );
  mDescriptionLineEdit = new QLineEdit();
  vlayout->addWidget( mDescriptionLineEdit );

  if ( definition )
  {
    mDescriptionLineEdit->setText( definition->description() );
  }

  if ( mDefinitionWidget )
    vlayout->addWidget( mDefinitionWidget );

  vlayout->addSpacing( 20 );
  mRequiredCheckBox = new QCheckBox( tr( "Mandatory" ) );
  if ( definition )
    mRequiredCheckBox->setChecked( !( definition->flags() & QgsProcessingParameterDefinition::FlagOptional ) );
  else
    mRequiredCheckBox->setChecked( true );
  vlayout->addWidget( mRequiredCheckBox );

  mAdvancedCheckBox = new QCheckBox( tr( "Advanced" ) );
  if ( definition )
    mAdvancedCheckBox->setChecked( definition->flags() & QgsProcessingParameterDefinition::FlagAdvanced );
  else
    mAdvancedCheckBox->setChecked( false );
  vlayout->addWidget( mAdvancedCheckBox );

  vlayout->addStretch();
  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingParameterDefinitionWidget::createParameter( const QString &name ) const
{
  std::unique_ptr< QgsProcessingParameterDefinition > param;
  QgsProcessingParameterDefinition::Flags flags = nullptr;

  if ( !mRequiredCheckBox->isChecked() )
    flags |= QgsProcessingParameterDefinition::FlagOptional;
  if ( mAdvancedCheckBox->isChecked() )
    flags |= QgsProcessingParameterDefinition::FlagAdvanced;

  if ( mDefinitionWidget )
  {
    // if a specific definition widget exists, get it to create the parameter (since it will know
    // how to set all the additional properties of that parameter, which we don't)
    param.reset( mDefinitionWidget->createParameter( name, mDescriptionLineEdit->text(), flags ) );
  }
  else if ( QgsApplication::processingRegistry()->parameterType( mType ) )
  {
    // otherwise, just create a default version of the parameter
    param.reset( QgsApplication::processingRegistry()->parameterType( mType )->create( name ) );
    if ( param )
    {
      param->setDescription( mDescriptionLineEdit->text() );
      param->setFlags( flags );
    }
  }

  return param.release();
}

//
// QgsProcessingParameterDefinitionDialog
//

QgsProcessingParameterDefinitionDialog::QgsProcessingParameterDefinitionDialog( const QString &type,
    QgsProcessingContext &context,
    const QgsProcessingParameterWidgetContext &widgetContext,
    const QgsProcessingParameterDefinition *definition,
    const QgsProcessingAlgorithm *algorithm,
    QWidget *parent )
  : QDialog( parent )
{
  QVBoxLayout *vLayout = new QVBoxLayout();
  mWidget = new QgsProcessingParameterDefinitionWidget( type, context, widgetContext, definition, algorithm );
  vLayout->addWidget( mWidget );
  QDialogButtonBox *bbox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Ok );
  connect( bbox, &QDialogButtonBox::accepted, this, &QgsProcessingParameterDefinitionDialog::accept );
  connect( bbox, &QDialogButtonBox::rejected, this, &QgsProcessingParameterDefinitionDialog::reject );
  vLayout->addWidget( bbox );
  setLayout( vLayout );
  setWindowTitle( definition ? tr( "%1 Parameter Definition" ).arg( definition->description() )
                  : QgsApplication::processingRegistry()->parameterType( type ) ? tr( "%1 Parameter Definition" ).arg( QgsApplication::processingRegistry()->parameterType( type )->name() ) :
                  tr( "Parameter Definition" ) );
  setObjectName( QStringLiteral( "QgsProcessingParameterDefinitionDialog" ) );
  QgsGui::enableAutoGeometryRestore( this );
}

QgsProcessingParameterDefinition *QgsProcessingParameterDefinitionDialog::createParameter( const QString &name ) const
{
  return mWidget->createParameter( name );
}

void QgsProcessingParameterDefinitionDialog::accept()
{
  if ( mWidget->mDescriptionLineEdit->text().isEmpty() )
  {
    QMessageBox::warning( this, tr( "Unable to define parameter" ),
                          tr( "Invalid parameter name" ) );
    return;
  }
  QDialog::accept();
}
