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

#include "qgsgui.h"
#include "qgsprocessingmodelalgorithm.h"

#include <QDialogButtonBox>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include "moc_qgsprocessinghelpeditorwidget.cpp"

///@cond NOT_STABLE

const QString QgsProcessingHelpEditorWidget::ALGORITHM_DESCRIPTION = u"ALG_DESC"_s;
const QString QgsProcessingHelpEditorWidget::ALGORITHM_CREATOR = u"ALG_CREATOR"_s;
const QString QgsProcessingHelpEditorWidget::ALGORITHM_HELP_CREATOR = u"ALG_HELP_CREATOR"_s;
const QString QgsProcessingHelpEditorWidget::ALGORITHM_VERSION = u"ALG_VERSION"_s;
const QString QgsProcessingHelpEditorWidget::ALGORITHM_SHORT_DESCRIPTION = u"SHORT_DESCRIPTION"_s;
const QString QgsProcessingHelpEditorWidget::ALGORITHM_HELP_URL = u"HELP_URL"_s;
const QString QgsProcessingHelpEditorWidget::ALGORITHM_EXAMPLES = u"EXAMPLES"_s;


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
  connect( mTextEdit, &QTextEdit::textChanged, this, [this] {
    if ( !mCurrentName.isEmpty() )
    {
      if ( mEditStackedWidget->currentWidget() == mPagePlainText )
      {
        mHelpContent[mCurrentName] = mTextEdit->toPlainText();
        updateHtmlView();
      }
    }
  } );

  connect( mRichTextEdit, &QgsRichTextEditor::textChanged, this, [this] {
    if ( !mCurrentName.isEmpty() )
    {
      if ( mEditStackedWidget->currentWidget() == mPageRichEdit )
      {
        mHelpContent[mCurrentName] = mRichTextEdit->toHtml();
        updateHtmlView();
      }
    }
  } );
}

QgsProcessingHelpEditorWidget::~QgsProcessingHelpEditorWidget() = default;

void QgsProcessingHelpEditorWidget::setAlgorithm( const QgsProcessingAlgorithm *algorithm )
{
  if ( !algorithm )
    return;

  mAlgorithm.reset( algorithm->create() );

  if ( const QgsProcessingModelAlgorithm *model = dynamic_cast<const QgsProcessingModelAlgorithm *>( mAlgorithm.get() ) )
  {
    mHelpContent = model->helpContent();
  }

  mEditStackedWidget->setCurrentWidget( mPageRichEdit );
  if ( mHelpContent.contains( ALGORITHM_DESCRIPTION ) )
  {
    mRichTextEdit->setText( mHelpContent.value( ALGORITHM_DESCRIPTION ).toString() );
  }

  mElementTree->addTopLevelItem( new QgsProcessingHelpEditorTreeItem( ALGORITHM_DESCRIPTION, tr( "Algorithm description" ) ) );
  mElementTree->addTopLevelItem( new QgsProcessingHelpEditorTreeItem( ALGORITHM_SHORT_DESCRIPTION, tr( "Short description" ) ) );

  QgsProcessingHelpEditorTreeItem *parametersItem = new QgsProcessingHelpEditorTreeItem( QString(), tr( "Input parameters" ) );
  mElementTree->addTopLevelItem( parametersItem );

  const QList<const QgsProcessingParameterDefinition *> definitions = mAlgorithm->parameterDefinitions();
  for ( const QgsProcessingParameterDefinition *definition : definitions )
  {
    if ( definition->flags() & Qgis::ProcessingParameterFlag::Hidden || definition->isDestination() )
      continue;

    parametersItem->addChild( new QgsProcessingHelpEditorTreeItem( definition->name(), definition->description() ) );
  }

  QgsProcessingHelpEditorTreeItem *outputsItem = new QgsProcessingHelpEditorTreeItem( QString(), tr( "Outputs" ) );
  mElementTree->addTopLevelItem( outputsItem );
  const QList<const QgsProcessingOutputDefinition *> outputs = mAlgorithm->outputDefinitions();
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
  storeCurrentValue();
  return mHelpContent;
}

void QgsProcessingHelpEditorWidget::updateHtmlView()
{
  mTextPreview->setHtml( formattedHelp() );
}

void QgsProcessingHelpEditorWidget::changeItem( QTreeWidgetItem *, QTreeWidgetItem * )
{
  if ( QgsProcessingHelpEditorTreeItem *item = dynamic_cast<QgsProcessingHelpEditorTreeItem *>( mElementTree->currentItem() ) )
  {
    storeCurrentValue();

    const QString name = item->name;
    if ( !name.isEmpty() )
    {
      mTextEdit->setEnabled( true );
      mRichTextEdit->setEnabled( true );

      updateHtmlView();
      mCurrentName = name;

      const bool useRichTextEdit = name == ALGORITHM_EXAMPLES || name == ALGORITHM_DESCRIPTION;
      if ( useRichTextEdit )
      {
        mEditStackedWidget->setCurrentWidget( mPageRichEdit );
        if ( mHelpContent.contains( name ) )
          mRichTextEdit->setText( mHelpContent.value( name ).toString() );
        else
          mRichTextEdit->setText( QString() );
      }
      else
      {
        mEditStackedWidget->setCurrentWidget( mPagePlainText );
        if ( mHelpContent.contains( name ) )
          mTextEdit->setText( mHelpContent.value( name ).toString() );
        else
          mTextEdit->clear();
      }
    }
    else
    {
      mCurrentName.clear();
      mTextEdit->clear();
      mRichTextEdit->setText( QString() );
      mTextEdit->setEnabled( false );
      mRichTextEdit->setEnabled( false );
      mEditStackedWidget->setCurrentWidget( mPagePlainText );
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

void QgsProcessingHelpEditorWidget::storeCurrentValue()
{
  if ( !mCurrentName.isEmpty() )
  {
    if ( mEditStackedWidget->currentWidget() == mPagePlainText )
      mHelpContent[mCurrentName] = mTextEdit->toPlainText();
    else
      mHelpContent[mCurrentName] = mRichTextEdit->toHtml();
  }
}

QgsProcessingHelpEditorDialog::QgsProcessingHelpEditorDialog( QWidget *parent, Qt::WindowFlags flags )
  : QDialog( parent, flags )
{
  setObjectName( u"QgsProcessingHelpEditorDialog"_s );

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
