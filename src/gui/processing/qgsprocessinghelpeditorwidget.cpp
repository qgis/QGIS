/***************************************************************************
                             qgsprocessinghelpeditorwidget.h
                             ------------------------
    Date                 : February 2022
    Copyright            : (C) 2022 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessinghelpeditorwidget.h"
#include "qgsprocessingmodelalgorithm.h"
#include "qgsgui.h"
#include <QTreeWidgetItem>
#include <QDialogButtonBox>
#include <QVBoxLayout>

///@cond NOT_STABLE

const QString QgsProcessingHelpEditorWidget::ALGORITHM_DESCRIPTION = QStringLiteral( "ALG_DESC" );
const QString QgsProcessingHelpEditorWidget::ALGORITHM_CREATOR = QStringLiteral( "ALG_CREATOR" );
const QString QgsProcessingHelpEditorWidget::ALGORITHM_HELP_CREATOR = QStringLiteral( "ALG_HELP_CREATOR" );
const QString QgsProcessingHelpEditorWidget::ALGORITHM_VERSION = QStringLiteral( "ALG_VERSION" );
const QString QgsProcessingHelpEditorWidget::ALGORITHM_SHORT_DESCRIPTION = QStringLiteral( "SHORT_DESCRIPTION" );
const QString QgsProcessingHelpEditorWidget::ALGORITHM_HELP_URL = QStringLiteral( "HELP_URL" );
const QString QgsProcessingHelpEditorWidget::ALGORITHM_EXAMPLES = QStringLiteral( "EXAMPLES" );


class QgsProcessingHelpEditorTreeItem : public QTreeWidgetItem
{
  public:

    QgsProcessingHelpEditorTreeItem( const QString &name, const QString &description )
      : QTreeWidgetItem()
      , name( name )
      , description( description )
    {
      setText( 0, description );
    }

    QString name;
    QString description;

};


QgsProcessingHelpEditorWidget::QgsProcessingHelpEditorWidget( QWidget *parent )
  : QWidget( parent )
  , mCurrentName( ALGORITHM_DESCRIPTION )
{
  setupUi( this );

  connect( mElementTree, &QTreeWidget::currentItemChanged, this, &QgsProcessingHelpEditorWidget::changeItem );
  connect( mTextEdit, &QTextEdit::textChanged, this, [ = ]
  {
    if ( !mCurrentName.isEmpty() )
    {
      mHelpContent[ mCurrentName] = mTextEdit->toPlainText();
      updateHtmlView();
    }
  } );
}

QgsProcessingHelpEditorWidget::~QgsProcessingHelpEditorWidget() = default;

void QgsProcessingHelpEditorWidget::setAlgorithm( const QgsProcessingAlgorithm *algorithm )
{
  if ( !algorithm )
    return;

  mAlgorithm.reset( algorithm->create() );

  if ( const QgsProcessingModelAlgorithm *model = dynamic_cast< const QgsProcessingModelAlgorithm *>( mAlgorithm.get() ) )
  {
    mHelpContent = model->helpContent();
  }

  if ( mHelpContent.contains( ALGORITHM_DESCRIPTION ) )
  {
    mTextEdit->setText( mHelpContent.value( ALGORITHM_CREATOR ).toString() );
  }

  mElementTree->addTopLevelItem( new QgsProcessingHelpEditorTreeItem( ALGORITHM_DESCRIPTION, tr( "Algorithm description" ) ) );
  mElementTree->addTopLevelItem( new QgsProcessingHelpEditorTreeItem( ALGORITHM_SHORT_DESCRIPTION, tr( "Short description" ) ) );

  QgsProcessingHelpEditorTreeItem *parametersItem = new QgsProcessingHelpEditorTreeItem( QString(), tr( "Input parameters" ) );
  mElementTree->addTopLevelItem( parametersItem );

  const QList< const QgsProcessingParameterDefinition * > definitions = mAlgorithm->parameterDefinitions();
  for ( const QgsProcessingParameterDefinition *definition : definitions )
  {
    if ( definition->flags() & QgsProcessingParameterDefinition::FlagHidden || definition->isDestination() )
      continue;

    parametersItem->addChild( new QgsProcessingHelpEditorTreeItem( definition->name(), definition->description() ) );
  }

  QgsProcessingHelpEditorTreeItem *outputsItem = new QgsProcessingHelpEditorTreeItem( QString(), tr( "Outputs" ) );
  mElementTree->addTopLevelItem( outputsItem );
  const QList< const QgsProcessingOutputDefinition * > outputs = mAlgorithm->outputDefinitions();
  for ( const QgsProcessingOutputDefinition *output : outputs )
  {
    outputsItem->addChild( new QgsProcessingHelpEditorTreeItem( output->name(), output->description() ) );
  }

  mElementTree->addTopLevelItem( new QgsProcessingHelpEditorTreeItem( ALGORITHM_EXAMPLES, tr( "Examples" ) ) );

  mElementTree->addTopLevelItem( new QgsProcessingHelpEditorTreeItem( ALGORITHM_CREATOR, tr( "Algorithm author" ) ) );
  mElementTree->addTopLevelItem( new QgsProcessingHelpEditorTreeItem( ALGORITHM_HELP_CREATOR, tr( "Help author" ) ) );
  mElementTree->addTopLevelItem( new QgsProcessingHelpEditorTreeItem( ALGORITHM_VERSION, tr( "Algorithm version" ) ) );
  mElementTree->addTopLevelItem( new QgsProcessingHelpEditorTreeItem( ALGORITHM_HELP_URL, tr( "Documentation help URL (for help button)" ) ) );

  updateHtmlView();
}

QVariantMap QgsProcessingHelpEditorWidget::helpContent()
{
  if ( !mCurrentName.isEmpty() )
    mHelpContent[ mCurrentName] = mTextEdit->toPlainText();

  return mHelpContent;
}

void QgsProcessingHelpEditorWidget::updateHtmlView()
{
  mTextPreview->setHtml( formattedHelp() );
}

void QgsProcessingHelpEditorWidget::changeItem( QTreeWidgetItem *, QTreeWidgetItem * )
{
  if ( QgsProcessingHelpEditorTreeItem *item = dynamic_cast< QgsProcessingHelpEditorTreeItem *>( mElementTree->currentItem() ) )
  {
    if ( !mCurrentName.isEmpty() )
    {
      mHelpContent[ mCurrentName] = mTextEdit->toPlainText();
    }

    const QString name = item->name;
    if ( !name.isEmpty() )
    {
      mTextEdit->setEnabled( true );
      updateHtmlView();
      mCurrentName = name;
      if ( mHelpContent.contains( name ) )
        mTextEdit->setText( mHelpContent[name].toString() );
      else
        mTextEdit->clear();
    }
    else
    {
      mCurrentName.clear();
      mTextEdit->clear();
      mTextEdit->setEnabled( false );
      updateHtmlView();
    }
  }
}

QString QgsProcessingHelpEditorWidget::formattedHelp() const
{
  if ( !mAlgorithm )
    return QString();

  return QgsProcessingUtils::formatHelpMapAsHtml( mHelpContent, mAlgorithm.get() );
}

QString QgsProcessingHelpEditorWidget::helpComponent( const QString &name ) const
{
  if ( mHelpContent.contains( name ) )
  {
    QString component = mHelpContent.value( name ).toString();
    component.replace( '\n', QStringLiteral( "<br>" ) );
    return component;
  }
  else
  {
    return QString();
  }
}

QgsProcessingHelpEditorDialog::QgsProcessingHelpEditorDialog( QWidget *parent, Qt::WindowFlags flags )
  : QDialog( parent, flags )
{
  setObjectName( QStringLiteral( "QgsProcessingHelpEditorDialog" ) );

  QVBoxLayout *vLayout = new QVBoxLayout();
  mWidget = new QgsProcessingHelpEditorWidget();
  vLayout->addWidget( mWidget, 1 );

  QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  vLayout->addWidget( buttonBox );
  setLayout( vLayout );

  QgsGui::enableAutoGeometryRestore( this );
}

void QgsProcessingHelpEditorDialog::setAlgorithm( const QgsProcessingAlgorithm *algorithm )
{
  mWidget->setAlgorithm( algorithm );
}

QVariantMap QgsProcessingHelpEditorDialog::helpContent()
{
  return mWidget->helpContent();
}


///@endcond

