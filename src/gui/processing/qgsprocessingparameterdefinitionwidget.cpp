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
#include "qgscolorbutton.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QTabWidget>
#include <QTextEdit>

QgsProcessingAbstractParameterDefinitionWidget::QgsProcessingAbstractParameterDefinitionWidget( QgsProcessingContext &,
    const QgsProcessingParameterWidgetContext &context,
    const QgsProcessingParameterDefinition *,
    const QgsProcessingAlgorithm *, QWidget *parent )
  : QWidget( parent )
  , mWidgetContext( context )
{

}

void QgsProcessingAbstractParameterDefinitionWidget::setWidgetContext( const QgsProcessingParameterWidgetContext &context )
{
  mWidgetContext = context;
}

const QgsProcessingParameterWidgetContext &QgsProcessingAbstractParameterDefinitionWidget::widgetContext() const
{
  return mWidgetContext;
}

void QgsProcessingAbstractParameterDefinitionWidget::registerProcessingContextGenerator( QgsProcessingContextGenerator *generator )
{
  mContextGenerator = generator;
}

QgsExpressionContext QgsProcessingAbstractParameterDefinitionWidget::createExpressionContext() const
{
  return QgsProcessingGuiUtils::createExpressionContext( mContextGenerator, mWidgetContext, nullptr, nullptr );
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
  mDefinitionWidget = QgsGui::processingGuiRegistry()->createParameterDefinitionWidget( type, context, widgetContext, definition, algorithm );

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
  QgsProcessingParameterDefinition::Flags flags = QgsProcessingParameterDefinition::Flags();

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

void QgsProcessingParameterDefinitionWidget::registerProcessingContextGenerator( QgsProcessingContextGenerator *generator )
{
  if ( mDefinitionWidget )
  {
    mDefinitionWidget->registerProcessingContextGenerator( generator );
  }
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
  mTabWidget = new QTabWidget();
  vLayout->addWidget( mTabWidget );

  QVBoxLayout *vLayout2 = new QVBoxLayout();
  mWidget = new QgsProcessingParameterDefinitionWidget( type, context, widgetContext, definition, algorithm );
  vLayout2->addWidget( mWidget );
  QWidget *w = new QWidget();
  w->setLayout( vLayout2 );
  mTabWidget->addTab( w, tr( "Properties" ) );

  QVBoxLayout *commentLayout = new QVBoxLayout();
  mCommentEdit = new QTextEdit();
  mCommentEdit->setAcceptRichText( false );
  commentLayout->addWidget( mCommentEdit, 1 );

  QHBoxLayout *hl = new QHBoxLayout();
  hl->setContentsMargins( 0, 0, 0, 0 );
  hl->addWidget( new QLabel( tr( "Color" ) ) );
  mCommentColorButton = new QgsColorButton();
  mCommentColorButton->setAllowOpacity( true );
  mCommentColorButton->setWindowTitle( tr( "Comment Color" ) );
  mCommentColorButton->setShowNull( true, tr( "Default" ) );
  hl->addWidget( mCommentColorButton );
  commentLayout->addLayout( hl );

  QWidget *w2 = new QWidget();
  w2->setLayout( commentLayout );
  mTabWidget->addTab( w2, tr( "Comments" ) );

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

void QgsProcessingParameterDefinitionDialog::setComments( const QString &comments )
{
  mCommentEdit->setPlainText( comments );
}

QString QgsProcessingParameterDefinitionDialog::comments() const
{
  return mCommentEdit->toPlainText();
}

void QgsProcessingParameterDefinitionDialog::setCommentColor( const QColor &color )
{
  if ( color.isValid() )
    mCommentColorButton->setColor( color );
  else
    mCommentColorButton->setToNull();
}

QColor QgsProcessingParameterDefinitionDialog::commentColor() const
{
  return !mCommentColorButton->isNull() ? mCommentColorButton->color() : QColor();
}

void QgsProcessingParameterDefinitionDialog::switchToCommentTab()
{
  mTabWidget->setCurrentIndex( 1 );
  mCommentEdit->setFocus();
  mCommentEdit->selectAll();
}

void QgsProcessingParameterDefinitionDialog::registerProcessingContextGenerator( QgsProcessingContextGenerator *generator )
{
  if ( mWidget )
  {
    mWidget->registerProcessingContextGenerator( generator );
  }
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
