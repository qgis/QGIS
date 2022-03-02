/***************************************************************************
    qgisexpressionbuilderwidget.cpp - A generic expression string builder widget.
     --------------------------------------
    Date                 :  29-May-2011
    Copyright            : (C) 2011 by Nathan Woodrow
    Email                : woodrow.nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QInputDialog>
#include <QComboBox>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QMessageBox>
#include <QVersionNumber>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileDialog>
#include <QMenu>

#include "qgsexpressionbuilderwidget.h"
#include "qgslogger.h"
#include "qgsexpression.h"
#include "qgsexpressionfunction.h"
#include "qgsexpressionnodeimpl.h"
#include "qgsapplication.h"
#include "qgspythonrunner.h"
#include "qgsgeometry.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsvectorlayer.h"
#include "qgssettings.h"
#include "qgsproject.h"
#include "qgsrelation.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfieldformatterregistry.h"
#include "qgsfieldformatter.h"
#include "qgsexpressionstoredialog.h"
#include "qgsexpressiontreeview.h"



bool formatterCanProvideAvailableValues( QgsVectorLayer *layer, const QString &fieldName )
{
  if ( layer )
  {
    const QgsFields fields = layer->fields();
    int fieldIndex = fields.lookupField( fieldName );
    if ( fieldIndex != -1 )
    {
      const QgsEditorWidgetSetup setup = fields.at( fieldIndex ).editorWidgetSetup();
      const QgsFieldFormatter *formatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( setup.type() );

      return ( formatter->flags() & QgsFieldFormatter::CanProvideAvailableValues );
    }
  }
  return false;
}


QgsExpressionBuilderWidget::QgsExpressionBuilderWidget( QWidget *parent )
  : QWidget( parent )
  , mProject( QgsProject::instance() )
{
  setupUi( this );

  connect( btnRun, &QToolButton::pressed, this, &QgsExpressionBuilderWidget::btnRun_pressed );
  connect( btnNewFile, &QPushButton::clicked, this, &QgsExpressionBuilderWidget::btnNewFile_pressed );
  connect( btnRemoveFile, &QPushButton::clicked, this, &QgsExpressionBuilderWidget::btnRemoveFile_pressed );
  connect( cmbFileNames, &QListWidget::currentItemChanged, this, &QgsExpressionBuilderWidget::cmbFileNames_currentItemChanged );
  connect( txtExpressionString, &QgsCodeEditorExpression::textChanged, this, &QgsExpressionBuilderWidget::txtExpressionString_textChanged );
  connect( txtPython, &QgsCodeEditorPython::textChanged, this, &QgsExpressionBuilderWidget::txtPython_textChanged );
  connect( txtSearchEditValues, &QgsFilterLineEdit::textChanged, this, &QgsExpressionBuilderWidget::txtSearchEditValues_textChanged );
  connect( mValuesListView, &QListView::doubleClicked, this, &QgsExpressionBuilderWidget::mValuesListView_doubleClicked );
  connect( btnSaveExpression, &QToolButton::clicked, this, &QgsExpressionBuilderWidget::storeCurrentUserExpression );
  connect( btnEditExpression, &QToolButton::clicked, this, &QgsExpressionBuilderWidget::editSelectedUserExpression );
  connect( btnRemoveExpression, &QToolButton::clicked, this, &QgsExpressionBuilderWidget::removeSelectedUserExpression );
  connect( btnImportExpressions, &QToolButton::clicked, this, &QgsExpressionBuilderWidget::importUserExpressions_pressed );
  connect( btnExportExpressions, &QToolButton::clicked, this, &QgsExpressionBuilderWidget::exportUserExpressions_pressed );
  connect( btnClearEditor, &QToolButton::clicked, txtExpressionString, &QgsCodeEditorExpression::clear );
  connect( txtSearchEdit, &QgsFilterLineEdit::textChanged, mExpressionTreeView, &QgsExpressionTreeView::setSearchText );

  connect( mExpressionPreviewWidget, &QgsExpressionPreviewWidget::toolTipChanged, txtExpressionString, &QgsCodeEditorExpression::setToolTip );
  connect( mExpressionPreviewWidget, &QgsExpressionPreviewWidget::expressionParsed, this, &QgsExpressionBuilderWidget::onExpressionParsed );
  connect( mExpressionPreviewWidget, &QgsExpressionPreviewWidget::expressionParsed, btnSaveExpression, &QToolButton::setEnabled );
  connect( mExpressionPreviewWidget, &QgsExpressionPreviewWidget::expressionParsed, this, &QgsExpressionBuilderWidget::expressionParsed ); // signal-to-signal
  connect( mExpressionPreviewWidget, &QgsExpressionPreviewWidget::parserErrorChanged, this, &QgsExpressionBuilderWidget::parserErrorChanged ); // signal-to-signal
  connect( mExpressionPreviewWidget, &QgsExpressionPreviewWidget::evalErrorChanged, this, &QgsExpressionBuilderWidget::evalErrorChanged ); // signal-to-signal

  connect( mExpressionTreeView, &QgsExpressionTreeView::expressionItemDoubleClicked, this, &QgsExpressionBuilderWidget::insertExpressionText );
  connect( mExpressionTreeView, &QgsExpressionTreeView::currentExpressionItemChanged, this, &QgsExpressionBuilderWidget::expressionTreeItemChanged );

  mExpressionTreeMenuProvider = new ExpressionTreeMenuProvider( this );
  mExpressionTreeView->setMenuProvider( mExpressionTreeMenuProvider );

  txtHelpText->setOpenExternalLinks( true );
  mValueGroupBox->hide();
  // highlighter = new QgsExpressionHighlighter( txtExpressionString->document() );

  // Note: must be in sync with the json help file for UserGroup
  mUserExpressionsGroupName = QgsExpression::group( QStringLiteral( "UserGroup" ) );

  // Set icons for tool buttons
  btnSaveExpression->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionFileSave.svg" ) ) );
  btnEditExpression->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "symbologyEdit.svg" ) ) );
  btnRemoveExpression->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionDeleteSelected.svg" ) ) );
  btnExportExpressions->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionSharingExport.svg" ) ) );
  btnImportExpressions->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionSharingImport.svg" ) ) );
  btnClearEditor->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionFileNew.svg" ) ) );

  connect( btnLoadAll, &QAbstractButton::clicked, this, &QgsExpressionBuilderWidget::loadAllValues );
  connect( btnLoadSample, &QAbstractButton::clicked, this, &QgsExpressionBuilderWidget::loadSampleValues );

  const auto pushButtons { mOperatorsGroupBox->findChildren<QPushButton *>() };
  for ( QPushButton *button : pushButtons )
  {
    connect( button, &QAbstractButton::clicked, this, &QgsExpressionBuilderWidget::operatorButtonClicked );
  }

  txtSearchEdit->setShowSearchIcon( true );
  txtSearchEdit->setPlaceholderText( tr( "Search…" ) );

  mValuesModel = std::make_unique<QStandardItemModel>();
  mProxyValues = std::make_unique<QSortFilterProxyModel>();
  mProxyValues->setSourceModel( mValuesModel.get() );
  mValuesListView->setModel( mProxyValues.get() );
  txtSearchEditValues->setShowSearchIcon( true );
  txtSearchEditValues->setPlaceholderText( tr( "Search…" ) );

  editorSplit->setSizes( QList<int>( {175, 300} ) );

  functionsplit->setCollapsible( 0, false );
  connect( mShowHelpButton, &QPushButton::clicked, this, [ = ]()
  {
    functionsplit->setSizes( QList<int>( {mOperationListGroup->width() - mHelpAndValuesWidget->minimumWidth(),
                                          mHelpAndValuesWidget->minimumWidth()
                                         } ) );
    mShowHelpButton->setEnabled( false );
  } );
  connect( functionsplit, &QSplitter::splitterMoved, this, [ = ]( int, int )
  {
    mShowHelpButton->setEnabled( functionsplit->sizes().at( 1 ) == 0 );
  } );

  QgsSettings settings;
  splitter->restoreState( settings.value( QStringLiteral( "Windows/QgsExpressionBuilderWidget/splitter" ) ).toByteArray() );
  editorSplit->restoreState( settings.value( QStringLiteral( "Windows/QgsExpressionBuilderWidget/editorsplitter" ) ).toByteArray() );
  functionsplit->restoreState( settings.value( QStringLiteral( "Windows/QgsExpressionBuilderWidget/functionsplitter" ) ).toByteArray() );
  mShowHelpButton->setEnabled( functionsplit->sizes().at( 1 ) == 0 );

  if ( QgsPythonRunner::isValid() )
  {
    QgsPythonRunner::eval( QStringLiteral( "qgis.user.expressionspath" ), mFunctionsPath );
    updateFunctionFileList( mFunctionsPath );
    btnRemoveFile->setEnabled( cmbFileNames->count() > 0 );
  }
  else
  {
    tab_2->hide();
  }

  txtExpressionString->setWrapMode( QsciScintilla::WrapWord );
  lblAutoSave->clear();

  // Note: If you add a indicator here you should add it to clearErrors method if you need to clear it on text parse.
  txtExpressionString->indicatorDefine( QgsCodeEditor::SquiggleIndicator, QgsExpression::ParserError::FunctionUnknown );
  txtExpressionString->indicatorDefine( QgsCodeEditor::SquiggleIndicator, QgsExpression::ParserError::FunctionWrongArgs );
  txtExpressionString->indicatorDefine( QgsCodeEditor::SquiggleIndicator, QgsExpression::ParserError::FunctionInvalidParams );
  txtExpressionString->indicatorDefine( QgsCodeEditor::SquiggleIndicator, QgsExpression::ParserError::FunctionNamedArgsError );
#if defined(QSCINTILLA_VERSION) && QSCINTILLA_VERSION >= 0x20a00
  txtExpressionString->indicatorDefine( QgsCodeEditor::TriangleIndicator, QgsExpression::ParserError::Unknown );
#else
  txtExpressionString->indicatorDefine( QgsCodeEditor::SquiggleIndicator, QgsExpression::ParserError::Unknown );
#endif

  // Set all the error markers as red. -1 is all.
  txtExpressionString->setIndicatorForegroundColor( QColor( Qt::red ), -1 );
  txtExpressionString->setIndicatorHoverForegroundColor( QColor( Qt::red ), -1 );
  txtExpressionString->setIndicatorOutlineColor( QColor( Qt::red ), -1 );

  // Hidden function markers.
  txtExpressionString->indicatorDefine( QgsCodeEditor::HiddenIndicator, FUNCTION_MARKER_ID );
  txtExpressionString->setIndicatorForegroundColor( QColor( Qt::blue ), FUNCTION_MARKER_ID );
  txtExpressionString->setIndicatorHoverForegroundColor( QColor( Qt::blue ), FUNCTION_MARKER_ID );
  txtExpressionString->setIndicatorHoverStyle( QgsCodeEditor::DotsIndicator, FUNCTION_MARKER_ID );

  connect( txtExpressionString, &QgsCodeEditorExpression::indicatorClicked, this, &QgsExpressionBuilderWidget::indicatorClicked );
  txtExpressionString->setAutoCompletionCaseSensitivity( false );
  txtExpressionString->setAutoCompletionSource( QsciScintilla::AcsAPIs );
  txtExpressionString->setCallTipsVisible( 0 );

  setExpectedOutputFormat( QString() );
  mFunctionBuilderHelp->setLineNumbersVisible( false );
  mFunctionBuilderHelp->setFoldingVisible( false );
  mFunctionBuilderHelp->setEdgeMode( QsciScintilla::EdgeNone );
  mFunctionBuilderHelp->setEdgeColumn( 0 );
  mFunctionBuilderHelp->setReadOnly( true );
  mFunctionBuilderHelp->setText( tr( "\"\"\"Define a new function using the @qgsfunction decorator.\n\
\n\
 The function accepts the following parameters\n\
\n\
 : param [any]: Define any parameters you want to pass to your function before\n\
 the following arguments.\n\
 : param feature: The current feature\n\
 : param parent: The QgsExpression object\n\
 : param context: If there is an argument called ``context`` found at the last\n\
                   position, this variable will contain a ``QgsExpressionContext``\n\
                   object, that gives access to various additional information like\n\
                   expression variables. E.g. ``context.variable( 'layer_id' )``\n\
 : returns: The result of the expression.\n\
\n\
\n\
\n\
 The @qgsfunction decorator accepts the following arguments:\n\
\n\
\n\
 : param args: Defines the number of arguments. With ``args = 'auto'`` the number of\n\
               arguments will automatically be extracted from the signature.\n\
               With ``args = -1``, any number of arguments are accepted.\n\
 : param group: The name of the group under which this expression function will\n\
                be listed.\n\
 : param handlesnull: Set this to True if your function has custom handling for NULL values.\n\
                     If False, the result will always be NULL as soon as any parameter is NULL.\n\
                     Defaults to False.\n\
 : param usesgeometry : Set this to True if your function requires access to\n\
                        feature.geometry(). Defaults to False.\n\
 : param referenced_columns: An array of attribute names that are required to run\n\
                             this function. Defaults to [QgsFeatureRequest.ALL_ATTRIBUTES].\n\
     \"\"\"" ) );
}


QgsExpressionBuilderWidget::~QgsExpressionBuilderWidget()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/QgsExpressionBuilderWidget/splitter" ), splitter->saveState() );
  settings.setValue( QStringLiteral( "Windows/QgsExpressionBuilderWidget/editorsplitter" ), editorSplit->saveState() );
  settings.setValue( QStringLiteral( "Windows/QgsExpressionBuilderWidget/functionsplitter" ), functionsplit->saveState() );
  delete mExpressionTreeMenuProvider;
}

void QgsExpressionBuilderWidget::init( const QgsExpressionContext &context, const QString &recentCollection, QgsExpressionBuilderWidget::Flags flags )
{
  setExpressionContext( context );

  if ( flags.testFlag( LoadRecent ) )
    mExpressionTreeView->loadRecent( recentCollection );

  if ( flags.testFlag( LoadUserExpressions ) )
    mExpressionTreeView->loadUserExpressions();
}

void QgsExpressionBuilderWidget::initWithLayer( QgsVectorLayer *layer, const QgsExpressionContext &context, const QString &recentCollection, QgsExpressionBuilderWidget::Flags flags )
{
  init( context, recentCollection, flags );
  setLayer( layer );
}

void QgsExpressionBuilderWidget::initWithFields( const QgsFields &fields, const QgsExpressionContext &context, const QString &recentCollection, QgsExpressionBuilderWidget::Flags flags )
{
  init( context, recentCollection, flags );
  mExpressionTreeView->loadFieldNames( fields );
}


void QgsExpressionBuilderWidget::setLayer( QgsVectorLayer *layer )
{
  mLayer = layer;
  mExpressionTreeView->setLayer( mLayer );
  mExpressionPreviewWidget->setLayer( mLayer );

  //TODO - remove existing layer scope from context

  if ( mLayer )
  {
    mExpressionContext << QgsExpressionContextUtils::layerScope( mLayer );
    expressionContextUpdated();
    txtExpressionString->setFields( mLayer->fields() );
  }
}

void QgsExpressionBuilderWidget::expressionContextUpdated()
{
  txtExpressionString->setExpressionContext( mExpressionContext );
  mExpressionTreeView->setExpressionContext( mExpressionContext );
  mExpressionPreviewWidget->setExpressionContext( mExpressionContext );
}

QgsVectorLayer *QgsExpressionBuilderWidget::layer() const
{
  return mLayer;
}

void QgsExpressionBuilderWidget::expressionTreeItemChanged( QgsExpressionItem *item )
{
  txtSearchEditValues->clear();

  if ( !item )
    return;

  bool isField = mLayer && item->getItemType() == QgsExpressionItem::Field;
  if ( isField )
  {
    mValuesModel->clear();

    cbxValuesInUse->setVisible( formatterCanProvideAvailableValues( mLayer, item->data( QgsExpressionItem::ITEM_NAME_ROLE ).toString() ) );
    cbxValuesInUse->setChecked( false );
  }
  mValueGroupBox->setVisible( isField );

  mShowHelpButton->setText( isField ? tr( "Show Values" ) : tr( "Show Help" ) );

  // Show the help for the current item.
  QString help = loadFunctionHelp( item );
  txtHelpText->setText( help );

  bool isUserExpression = item->parent() && item->parent()->text() == mUserExpressionsGroupName;

  btnRemoveExpression->setEnabled( isUserExpression );
  btnEditExpression->setEnabled( isUserExpression );
}

void QgsExpressionBuilderWidget::btnRun_pressed()
{
  if ( !cmbFileNames->currentItem() )
    return;

  QString file = cmbFileNames->currentItem()->text();
  saveFunctionFile( file );
  runPythonCode( txtPython->text() );
}

void QgsExpressionBuilderWidget::runPythonCode( const QString &code )
{
  if ( QgsPythonRunner::isValid() )
  {
    QString pythontext = code;
    QgsPythonRunner::run( pythontext );
  }
  mExpressionTreeView->refresh();
}

QgsVectorLayer *QgsExpressionBuilderWidget::contextLayer( const QgsExpressionItem *item ) const
{
  QgsVectorLayer *layer = nullptr;
  if ( ! item->data( QgsExpressionItem::LAYER_ID_ROLE ).isNull() )
  {
    layer = qobject_cast<QgsVectorLayer *>( QgsProject::instance()->mapLayer( item->data( QgsExpressionItem::LAYER_ID_ROLE ).toString() ) );
  }
  else
  {
    layer = mLayer;
  }
  return layer;
}


void QgsExpressionBuilderWidget::saveFunctionFile( QString fileName )
{
  QDir myDir( mFunctionsPath );
  if ( !myDir.exists() )
  {
    myDir.mkpath( mFunctionsPath );
  }

  if ( !fileName.endsWith( QLatin1String( ".py" ) ) )
  {
    fileName.append( ".py" );
  }

  fileName = mFunctionsPath + QDir::separator() + fileName;
  QFile myFile( fileName );
  if ( myFile.open( QIODevice::WriteOnly | QFile::Truncate ) )
  {
    QTextStream myFileStream( &myFile );
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    myFileStream << txtPython->text() << endl;
#else
    myFileStream << txtPython->text() << Qt::endl;
#endif
    myFile.close();
  }
}

void QgsExpressionBuilderWidget::updateFunctionFileList( const QString &path )
{
  mFunctionsPath = path;
  QDir dir( path );
  dir.setNameFilters( QStringList() << QStringLiteral( "*.py" ) );
  QStringList files = dir.entryList( QDir::Files );
  cmbFileNames->clear();
  const auto constFiles = files;
  for ( const QString &name : constFiles )
  {
    QFileInfo info( mFunctionsPath + QDir::separator() + name );
    if ( info.baseName() == QLatin1String( "__init__" ) ) continue;
    QListWidgetItem *item = new QListWidgetItem( QgsApplication::getThemeIcon( QStringLiteral( "console/iconTabEditorConsole.svg" ) ), info.baseName() );
    cmbFileNames->addItem( item );
  }
  if ( !cmbFileNames->currentItem() )
  {
    cmbFileNames->setCurrentRow( 0 );
  }

  if ( cmbFileNames->count() == 0 )
  {
    // Create default sample entry.
    newFunctionFile( QStringLiteral( "default" ) );
    txtPython->setText( QStringLiteral( "'''\n#Sample custom function file\n"
                                        "#(uncomment to use and customize or Add button to create a new file) \n%1 \n '''" ).arg( txtPython->text() ) );
    saveFunctionFile( QStringLiteral( "default" ) );
  }
}

void QgsExpressionBuilderWidget::newFunctionFile( const QString &fileName )
{
  QList<QListWidgetItem *> items = cmbFileNames->findItems( fileName, Qt::MatchExactly );
  if ( !items.isEmpty() )
    return;

  QListWidgetItem *item = new QListWidgetItem( QgsApplication::getThemeIcon( QStringLiteral( "console/iconTabEditorConsole.svg" ) ), fileName );
  cmbFileNames->insertItem( 0, item );
  cmbFileNames->setCurrentRow( 0 );

  QString templatetxt;
  QgsPythonRunner::eval( QStringLiteral( "qgis.user.default_expression_template" ), templatetxt );
  txtPython->setText( templatetxt );
  saveFunctionFile( fileName );
}

void QgsExpressionBuilderWidget::btnNewFile_pressed()
{
  bool ok;
  QString text = QInputDialog::getText( this, tr( "New File" ),
                                        tr( "New file name:" ), QLineEdit::Normal,
                                        QString(), &ok );
  if ( ok && !text.isEmpty() )
  {
    newFunctionFile( text );
  }
}

void QgsExpressionBuilderWidget::btnRemoveFile_pressed()
{
  if ( QMessageBox::question( this, tr( "Remove File" ),
                              tr( "Are you sure you want to remove current functions file?" ),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) == QMessageBox::No )
    return;

  int currentRow = cmbFileNames->currentRow();
  QString fileName = cmbFileNames->currentItem()->text();
  if ( QFile::remove( mFunctionsPath + QDir::separator() + fileName.append( ".py" ) ) )
  {
    {
      QListWidgetItem *itemToRemove = whileBlocking( cmbFileNames )->takeItem( currentRow );
      delete itemToRemove;
    }

    if ( cmbFileNames->count() > 0 )
    {
      cmbFileNames->setCurrentRow( currentRow > 0 ? currentRow - 1 : 0 );
      loadCodeFromFile( mFunctionsPath + QDir::separator() + cmbFileNames->currentItem()->text() );
    }
    else
    {
      btnRemoveFile->setEnabled( false );
      txtPython->clear();
    }
  }
  else
  {
    QMessageBox::warning( this, tr( "Remove file" ), tr( "Failed to remove function file '%1'." ).arg( fileName ) );
  }
}

void QgsExpressionBuilderWidget::cmbFileNames_currentItemChanged( QListWidgetItem *item, QListWidgetItem *lastitem )
{
  if ( lastitem )
  {
    QString filename = lastitem->text();
    saveFunctionFile( filename );
  }
  QString path = mFunctionsPath + QDir::separator() + item->text();
  loadCodeFromFile( path );
}

void QgsExpressionBuilderWidget::loadCodeFromFile( QString path )
{
  if ( !path.endsWith( QLatin1String( ".py" ) ) )
    path.append( ".py" );

  txtPython->loadScript( path );
}

void QgsExpressionBuilderWidget::loadFunctionCode( const QString &code )
{
  txtPython->setText( code );
}

void QgsExpressionBuilderWidget::insertExpressionText( const QString &text )
{
  // Insert the expression text or replace selected text
  txtExpressionString->insertText( text );
  txtExpressionString->setFocus();
}

void QgsExpressionBuilderWidget::loadFieldsAndValues( const QMap<QString, QStringList> &fieldValues )
{
  Q_UNUSED( fieldValues )
  // This is not maintained and setLayer() should be used instead.
}

void QgsExpressionBuilderWidget::fillFieldValues( const QString &fieldName, QgsVectorLayer *layer, int countLimit, bool forceUsedValues )
{
  // TODO We should really return a error the user of the widget that
  // the there is no layer set.
  if ( !layer )
    return;

  // TODO We should thread this so that we don't hold the user up if the layer is massive.

  const QgsFields fields = layer->fields();
  int fieldIndex = fields.lookupField( fieldName );

  if ( fieldIndex < 0 )
    return;

  const QgsEditorWidgetSetup setup = fields.at( fieldIndex ).editorWidgetSetup();
  const QgsFieldFormatter *formatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( setup.type() );

  QVariantList values;
  if ( cbxValuesInUse->isVisible() && !cbxValuesInUse->isChecked() && !forceUsedValues )
  {
    QgsFieldFormatterContext fieldFormatterContext;
    fieldFormatterContext.setProject( mProject );
    values = formatter->availableValues( setup.config(), countLimit, fieldFormatterContext );
  }
  else
  {
    values = qgis::setToList( layer->uniqueValues( fieldIndex, countLimit ) );
  }
  std::sort( values.begin(), values.end() );

  mValuesModel->clear();
  for ( const QVariant &value : std::as_const( values ) )
  {
    QString strValue;
    bool forceRepresentedValue = false;
    if ( value.isNull() )
      strValue = QStringLiteral( "NULL" );
    else if ( value.type() == QVariant::Int || value.type() == QVariant::Double || value.type() == QVariant::LongLong )
      strValue = value.toString();
    else if ( value.type() == QVariant::StringList )
    {
      QString result;
      const QStringList strList = value.toStringList();
      for ( QString str : strList )
      {
        if ( !result.isEmpty() )
          result.append( QStringLiteral( ", " ) );

        result.append( '\'' + str.replace( '\'', QLatin1String( "''" ) ) + '\'' );
      }
      strValue = QStringLiteral( "array(%1)" ).arg( result );
      forceRepresentedValue = true;
    }
    else if ( value.type() == QVariant::List )
    {
      QString result;
      const QList list = value.toList();
      for ( const QVariant &item : list )
      {
        if ( !result.isEmpty() )
          result.append( QStringLiteral( ", " ) );

        result.append( item.toString() );
      }
      strValue = QStringLiteral( "array(%1)" ).arg( result );
      forceRepresentedValue = true;
    }
    else
      strValue = '\'' + value.toString().replace( '\'', QLatin1String( "''" ) ) + '\'';

    QString representedValue = formatter->representValue( layer, fieldIndex, setup.config(), QVariant(), value );
    if ( forceRepresentedValue || representedValue != value.toString() )
      representedValue = representedValue + QStringLiteral( " [" ) + strValue + ']';

    QStandardItem *item = new QStandardItem( representedValue );
    item->setData( strValue );
    mValuesModel->appendRow( item );
  }
}

QString QgsExpressionBuilderWidget::getFunctionHelp( QgsExpressionFunction *function )
{
  if ( !function )
    return QString();

  QString helpContents = QgsExpression::helpText( function->name() );

  return QStringLiteral( "<head><style>" ) + helpStylesheet() + QStringLiteral( "</style></head><body>" ) + helpContents + QStringLiteral( "</body>" );

}



bool QgsExpressionBuilderWidget::isExpressionValid()
{
  return mExpressionValid;
}

void QgsExpressionBuilderWidget::saveToRecent( const QString &collection )
{
  mExpressionTreeView->saveToRecent( expressionText(), collection );
}

void QgsExpressionBuilderWidget::loadRecent( const QString &collection )
{
  mExpressionTreeView->loadRecent( collection );
}

QgsExpressionTreeView *QgsExpressionBuilderWidget::expressionTree() const
{
  return mExpressionTreeView;
}

// this is potentially very slow if there are thousands of user expressions, every time entire cleanup and load
void QgsExpressionBuilderWidget::loadUserExpressions( )
{
  mExpressionTreeView->loadUserExpressions();
}

void QgsExpressionBuilderWidget::saveToUserExpressions( const QString &label, const QString &expression, const QString &helpText )
{
  mExpressionTreeView->saveToUserExpressions( label, expression, helpText );
}

void QgsExpressionBuilderWidget::removeFromUserExpressions( const QString &label )
{
  mExpressionTreeView->removeFromUserExpressions( label );
}


void QgsExpressionBuilderWidget::setGeomCalculator( const QgsDistanceArea &da )
{
  mExpressionPreviewWidget->setGeomCalculator( da );
}

QString QgsExpressionBuilderWidget::expressionText()
{
  return txtExpressionString->text();
}

void QgsExpressionBuilderWidget::setExpressionText( const QString &expression )
{
  txtExpressionString->setText( expression );
}

QString QgsExpressionBuilderWidget::expectedOutputFormat()
{
  return lblExpected->text();
}

void QgsExpressionBuilderWidget::setExpectedOutputFormat( const QString &expected )
{
  lblExpected->setText( expected );
  mExpectedOutputFrame->setVisible( !expected.isNull() );
}

void QgsExpressionBuilderWidget::setExpressionContext( const QgsExpressionContext &context )
{
  mExpressionContext = context;
  expressionContextUpdated();
}

void QgsExpressionBuilderWidget::txtExpressionString_textChanged()
{
  QString text = expressionText();

  btnClearEditor->setEnabled( ! txtExpressionString->text().isEmpty() );
  btnSaveExpression->setEnabled( false );

  mExpressionPreviewWidget->setExpressionText( text );
}

bool QgsExpressionBuilderWidget::parserError() const
{
  return mExpressionPreviewWidget->parserError();
}

void QgsExpressionBuilderWidget::setExpressionPreviewVisible( bool isVisible )
{
  mExpressionPreviewWidget->setVisible( isVisible );
}

bool QgsExpressionBuilderWidget::evalError() const
{
  return mExpressionPreviewWidget->evalError();
}

QStandardItemModel *QgsExpressionBuilderWidget::model()
{
  Q_NOWARN_DEPRECATED_PUSH
  return mExpressionTreeView->model();
  Q_NOWARN_DEPRECATED_POP
}

QgsProject *QgsExpressionBuilderWidget::project()
{
  return mProject;
}

void QgsExpressionBuilderWidget::setProject( QgsProject *project )
{
  mProject = project;
  mExpressionTreeView->setProject( project );
}

void QgsExpressionBuilderWidget::showEvent( QShowEvent *e )
{
  QWidget::showEvent( e );
  txtExpressionString->setFocus();
}

void QgsExpressionBuilderWidget::createErrorMarkers( const QList<QgsExpression::ParserError> &errors )
{
  clearErrors();
  for ( const QgsExpression::ParserError &error : errors )
  {
    int errorFirstLine = error.firstLine - 1 ;
    int errorFirstColumn = error.firstColumn - 1;
    int errorLastColumn = error.lastColumn - 1;
    int errorLastLine = error.lastLine - 1;

    // If we have a unknown error we just mark the point that hit the error for now
    // until we can handle others more.
    if ( error.errorType == QgsExpression::ParserError::Unknown )
    {
      errorFirstLine = errorLastLine;
      errorFirstColumn = errorLastColumn - 1;
    }
    txtExpressionString->fillIndicatorRange( errorFirstLine,
        errorFirstColumn,
        errorLastLine,
        errorLastColumn, error.errorType );
  }
}

void QgsExpressionBuilderWidget::createMarkers( const QgsExpressionNode *inNode )
{
  switch ( inNode->nodeType() )
  {
    case QgsExpressionNode::NodeType::ntFunction:
    {
      const QgsExpressionNodeFunction *node = static_cast<const QgsExpressionNodeFunction *>( inNode );
      txtExpressionString->SendScintilla( QsciScintilla::SCI_SETINDICATORCURRENT, FUNCTION_MARKER_ID );
      txtExpressionString->SendScintilla( QsciScintilla::SCI_SETINDICATORVALUE, node->fnIndex() );
      int start = inNode->parserFirstColumn - 1;
      int end = inNode->parserLastColumn - 1;
      int start_pos = txtExpressionString->positionFromLineIndex( inNode->parserFirstLine - 1, start );
      txtExpressionString->SendScintilla( QsciScintilla::SCI_INDICATORFILLRANGE, start_pos, end - start );
      if ( node->args() )
      {
        const QList< QgsExpressionNode * > nodeList = node->args()->list();
        for ( QgsExpressionNode *n : nodeList )
        {
          createMarkers( n );
        }
      }
      break;
    }
    case QgsExpressionNode::NodeType::ntLiteral:
    {
      break;
    }
    case QgsExpressionNode::NodeType::ntUnaryOperator:
    {
      const QgsExpressionNodeUnaryOperator *node = static_cast<const QgsExpressionNodeUnaryOperator *>( inNode );
      createMarkers( node->operand() );
      break;
    }
    case QgsExpressionNode::NodeType::ntBinaryOperator:
    {
      const QgsExpressionNodeBinaryOperator *node = static_cast<const QgsExpressionNodeBinaryOperator *>( inNode );
      createMarkers( node->opLeft() );
      createMarkers( node->opRight() );
      break;
    }
    case QgsExpressionNode::NodeType::ntColumnRef:
    {
      break;
    }
    case QgsExpressionNode::NodeType::ntInOperator:
    {
      const QgsExpressionNodeInOperator *node = static_cast<const QgsExpressionNodeInOperator *>( inNode );
      if ( node->list() )
      {
        const QList< QgsExpressionNode * > nodeList = node->list()->list();
        for ( QgsExpressionNode *n : nodeList )
        {
          createMarkers( n );
        }
      }
      break;
    }
    case QgsExpressionNode::NodeType::ntCondition:
    {
      const QgsExpressionNodeCondition *node = static_cast<const QgsExpressionNodeCondition *>( inNode );
      const QList<QgsExpressionNodeCondition::WhenThen *> conditions = node->conditions();
      for ( QgsExpressionNodeCondition::WhenThen *cond : conditions )
      {
        createMarkers( cond->whenExp() );
        createMarkers( cond->thenExp() );
      }
      if ( node->elseExp() )
      {
        createMarkers( node->elseExp() );
      }
      break;
    }
    case QgsExpressionNode::NodeType::ntIndexOperator:
    {
      break;
    }
  }
}

void QgsExpressionBuilderWidget::clearFunctionMarkers()
{
  int lastLine = txtExpressionString->lines() - 1;
  txtExpressionString->clearIndicatorRange( 0, 0, lastLine, txtExpressionString->text( lastLine ).length() - 1, FUNCTION_MARKER_ID );
}

void QgsExpressionBuilderWidget::clearErrors()
{
  int lastLine = txtExpressionString->lines() - 1;
  // Note: -1 here doesn't seem to do the clear all like the other functions.  Will need to make this a bit smarter.
  txtExpressionString->clearIndicatorRange( 0, 0, lastLine, txtExpressionString->text( lastLine ).length(), QgsExpression::ParserError::Unknown );
  txtExpressionString->clearIndicatorRange( 0, 0, lastLine, txtExpressionString->text( lastLine ).length(), QgsExpression::ParserError::FunctionInvalidParams );
  txtExpressionString->clearIndicatorRange( 0, 0, lastLine, txtExpressionString->text( lastLine ).length(), QgsExpression::ParserError::FunctionUnknown );
  txtExpressionString->clearIndicatorRange( 0, 0, lastLine, txtExpressionString->text( lastLine ).length(), QgsExpression::ParserError::FunctionWrongArgs );
  txtExpressionString->clearIndicatorRange( 0, 0, lastLine, txtExpressionString->text( lastLine ).length(), QgsExpression::ParserError::FunctionNamedArgsError );
}

void QgsExpressionBuilderWidget::txtSearchEditValues_textChanged()
{
  mProxyValues->setFilterCaseSensitivity( Qt::CaseInsensitive );
  mProxyValues->setFilterWildcard( txtSearchEditValues->text() );
}

void QgsExpressionBuilderWidget::mValuesListView_doubleClicked( const QModelIndex &index )
{
  // Insert the item text or replace selected text
  txtExpressionString->insertText( ' ' + index.data( Qt::UserRole + 1 ).toString() + ' ' );
  txtExpressionString->setFocus();
}

void QgsExpressionBuilderWidget::operatorButtonClicked()
{
  QPushButton *button = qobject_cast<QPushButton *>( sender() );

  // Insert the button text or replace selected text
  txtExpressionString->insertText( ' ' + button->text() + ' ' );
  txtExpressionString->setFocus();
}

void QgsExpressionBuilderWidget::loadSampleValues()
{
  QgsExpressionItem *item = mExpressionTreeView->currentItem();
  if ( ! item )
  {
    return;
  }

  QgsVectorLayer *layer { contextLayer( item ) };
  // TODO We should really return a error the user of the widget that
  // the there is no layer set.
  if ( !layer )
  {
    return;
  }

  mValueGroupBox->show();
  fillFieldValues( item->data( QgsExpressionItem::ITEM_NAME_ROLE ).toString(), layer, 10 );
}

void QgsExpressionBuilderWidget::loadAllValues()
{
  QgsExpressionItem *item = mExpressionTreeView->currentItem();
  if ( ! item )
  {
    return;
  }

  QgsVectorLayer *layer { contextLayer( item ) };
  // TODO We should really return a error the user of the widget that
  // the there is no layer set.
  if ( !layer )
  {
    return;
  }

  mValueGroupBox->show();
  fillFieldValues( item->data( QgsExpressionItem::ITEM_NAME_ROLE ).toString(), layer, -1 );
}

void QgsExpressionBuilderWidget::loadSampleUsedValues()
{
  QgsExpressionItem *item = mExpressionTreeView->currentItem();
  if ( ! item )
  {
    return;
  }

  QgsVectorLayer *layer { contextLayer( item ) };
  // TODO We should really return a error the user of the widget that
  // the there is no layer set.
  if ( !layer )
  {
    return;
  }

  mValueGroupBox->show();
  fillFieldValues( item->data( QgsExpressionItem::ITEM_NAME_ROLE ).toString(), layer, 10, true );
}

void QgsExpressionBuilderWidget::loadAllUsedValues()
{
  QgsExpressionItem *item = mExpressionTreeView->currentItem();
  if ( ! item )
  {
    return;
  }

  QgsVectorLayer *layer { contextLayer( item ) };
  // TODO We should really return a error the user of the widget that
  // the there is no layer set.
  if ( !layer )
  {
    return;
  }

  mValueGroupBox->show();
  fillFieldValues( item->data( QgsExpressionItem::ITEM_NAME_ROLE ).toString(), layer, -1, true );
}

void QgsExpressionBuilderWidget::txtPython_textChanged()
{
  lblAutoSave->setText( tr( "Saving…" ) );
  if ( mAutoSave )
  {
    autosave();
  }
}

void QgsExpressionBuilderWidget::autosave()
{
  // Don't auto save if not on function editor that would be silly.
  if ( tabWidget->currentIndex() != 1 )
    return;

  QListWidgetItem *item = cmbFileNames->currentItem();
  if ( !item )
    return;

  QString file = item->text();
  saveFunctionFile( file );
  lblAutoSave->setText( QStringLiteral( "Saved" ) );
  QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect();
  lblAutoSave->setGraphicsEffect( effect );
  QPropertyAnimation *anim = new QPropertyAnimation( effect, "opacity" );
  anim->setDuration( 2000 );
  anim->setStartValue( 1.0 );
  anim->setEndValue( 0.0 );
  anim->setEasingCurve( QEasingCurve::OutQuad );
  anim->start( QAbstractAnimation::DeleteWhenStopped );
}

void QgsExpressionBuilderWidget::storeCurrentUserExpression()
{
  const QString expression { this->expressionText() };
  QgsExpressionStoreDialog dlg { expression, expression, QString( ), mExpressionTreeView->userExpressionLabels() };
  if ( dlg.exec() == QDialog::DialogCode::Accepted )
  {
    mExpressionTreeView->saveToUserExpressions( dlg.label().simplified(), dlg.expression(), dlg.helpText() );
  }
}

void QgsExpressionBuilderWidget::editSelectedUserExpression()
{
  // Get the item
  QgsExpressionItem *item = mExpressionTreeView->currentItem();
  if ( !item )
    return;

  // Don't handle remove if we are on a header node or the parent
  // is not the user group
  if ( item->getItemType() == QgsExpressionItem::Header ||
       ( item->parent() && item->parent()->text() != mUserExpressionsGroupName ) )
    return;

  QgsSettings settings;
  QString helpText = settings.value( QStringLiteral( "user/%1/helpText" ).arg( item->text() ), "", QgsSettings::Section::Expressions ).toString();
  QgsExpressionStoreDialog dlg { item->text(), item->getExpressionText(), helpText, mExpressionTreeView->userExpressionLabels() };

  if ( dlg.exec() == QDialog::DialogCode::Accepted )
  {
    // label has changed removed the old one before adding the new one
    if ( dlg.isLabelModified() )
    {
      mExpressionTreeView->removeFromUserExpressions( item->text() );
    }

    mExpressionTreeView->saveToUserExpressions( dlg.label().simplified(), dlg.expression(), dlg.helpText() );
  }
}

void QgsExpressionBuilderWidget::removeSelectedUserExpression()
{
  // Get the item
  QgsExpressionItem *item = mExpressionTreeView->currentItem();

  if ( !item )
    return;

  // Don't handle remove if we are on a header node or the parent
  // is not the user group
  if ( item->getItemType() == QgsExpressionItem::Header ||
       ( item->parent() && item->parent()->text() != mUserExpressionsGroupName ) )
    return;

  if ( QMessageBox::Yes == QMessageBox::question( this, tr( "Remove Stored Expression" ),
       tr( "Do you really want to remove stored expressions '%1'?" ).arg( item->text() ),
       QMessageBox::Yes | QMessageBox::No ) )
  {
    mExpressionTreeView->removeFromUserExpressions( item->text() );
  }

}

void QgsExpressionBuilderWidget::exportUserExpressions_pressed()
{
  QgsSettings settings;
  QString lastSaveDir = settings.value( QStringLiteral( "lastExportExpressionsDir" ), QDir::homePath(), QgsSettings::App ).toString();
  QString saveFileName = QFileDialog::getSaveFileName(
                           this,
                           tr( "Export User Expressions" ),
                           lastSaveDir,
                           tr( "User expressions" ) + " (*.json)" );

  if ( saveFileName.isEmpty() )
    return;

  QFileInfo saveFileInfo( saveFileName );

  if ( saveFileInfo.suffix().isEmpty() )
  {
    QString saveFileNameWithSuffix = saveFileName.append( ".json" );
    saveFileInfo = QFileInfo( saveFileNameWithSuffix );
  }

  settings.setValue( QStringLiteral( "lastExportExpressionsDir" ), saveFileInfo.absolutePath(), QgsSettings::App );

  QJsonDocument exportJson = mExpressionTreeView->exportUserExpressions();
  QFile jsonFile( saveFileName );

  if ( !jsonFile.open( QFile::WriteOnly | QIODevice::Truncate ) )
    QMessageBox::warning( this, tr( "Export user expressions" ), tr( "Error while creating the expressions file." ) );

  if ( ! jsonFile.write( exportJson.toJson() ) )
    QMessageBox::warning( this, tr( "Export user expressions" ), tr( "Error while creating the expressions file." ) );
  else
    jsonFile.close();
}

void QgsExpressionBuilderWidget::importUserExpressions_pressed()
{
  QgsSettings settings;
  QString lastImportDir = settings.value( QStringLiteral( "lastImportExpressionsDir" ), QDir::homePath(), QgsSettings::App ).toString();
  QString loadFileName = QFileDialog::getOpenFileName(
                           this,
                           tr( "Import User Expressions" ),
                           lastImportDir,
                           tr( "User expressions" ) + " (*.json)" );

  if ( loadFileName.isEmpty() )
    return;

  QFileInfo loadFileInfo( loadFileName );

  settings.setValue( QStringLiteral( "lastImportExpressionsDir" ), loadFileInfo.absolutePath(), QgsSettings::App );

  QFile jsonFile( loadFileName );

  if ( !jsonFile.open( QFile::ReadOnly ) )
    QMessageBox::warning( this, tr( "Import User Expressions" ), tr( "Error while reading the expressions file." ) );

  QTextStream jsonStream( &jsonFile );
  QString jsonString = jsonFile.readAll();
  jsonFile.close();

  QJsonDocument importJson = QJsonDocument::fromJson( jsonString.toUtf8() );

  if ( importJson.isNull() )
  {
    QMessageBox::warning( this, tr( "Import User Expressions" ), tr( "Error while reading the expressions file." ) );
    return;
  }

  mExpressionTreeView->loadExpressionsFromJson( importJson );
}


const QList<QgsExpressionItem *> QgsExpressionBuilderWidget::findExpressions( const QString &label )
{
  return mExpressionTreeView->findExpressions( label );
}

void QgsExpressionBuilderWidget::indicatorClicked( int line, int index, Qt::KeyboardModifiers state )
{
  if ( state & Qt::ControlModifier )
  {
    int position = txtExpressionString->positionFromLineIndex( line, index );
    long fncIndex = txtExpressionString->SendScintilla( QsciScintilla::SCI_INDICATORVALUEAT, FUNCTION_MARKER_ID, static_cast<long int>( position ) );
    QgsExpressionFunction *func = QgsExpression::Functions()[fncIndex];
    QString help = getFunctionHelp( func );
    txtHelpText->setText( help );
  }
}

void QgsExpressionBuilderWidget::onExpressionParsed( bool state )
{
  clearErrors();

  mExpressionValid = state;
  if ( state )
  {
    createMarkers( mExpressionPreviewWidget->rootNode() );
  }
  else
  {
    createErrorMarkers( mExpressionPreviewWidget->parserErrors() );
  }
}

QString QgsExpressionBuilderWidget::helpStylesheet() const
{
  //start with default QGIS report style
  QString style = QgsApplication::reportStyleSheet();

  //add some tweaks
  style += " .functionname {color: #0a6099; font-weight: bold;} "
           " .argument {font-family: monospace; color: #bf0c0c; font-style: italic; } "
           " td.argument { padding-right: 10px; }";

  return style;
}

QString QgsExpressionBuilderWidget::loadFunctionHelp( QgsExpressionItem *expressionItem )
{
  if ( !expressionItem )
    return QString();

  QString helpContents = expressionItem->getHelpText();

  // Return the function help that is set for the function if there is one.
  if ( helpContents.isEmpty() )
  {
    QString name = expressionItem->data( Qt::UserRole ).toString();

    if ( expressionItem->getItemType() == QgsExpressionItem::Field )
      helpContents = QgsExpression::helpText( QStringLiteral( "Field" ) );
    else
      helpContents = QgsExpression::helpText( name );
  }

  return "<head><style>" + helpStylesheet() + "</style></head><body>" + helpContents + "</body>";
}


// *************
// Menu provider

QMenu *QgsExpressionBuilderWidget::ExpressionTreeMenuProvider::createContextMenu( QgsExpressionItem *item )
{
  QMenu *menu = nullptr;
  QgsVectorLayer *layer = mExpressionBuilderWidget->layer();
  if ( item->getItemType() == QgsExpressionItem::Field && layer )
  {
    menu = new QMenu( mExpressionBuilderWidget );
    menu->addAction( tr( "Load First 10 Unique Values" ), mExpressionBuilderWidget, &QgsExpressionBuilderWidget::loadSampleValues );
    menu->addAction( tr( "Load All Unique Values" ), mExpressionBuilderWidget, &QgsExpressionBuilderWidget::loadAllValues );

    if ( formatterCanProvideAvailableValues( layer, item->data( QgsExpressionItem::ITEM_NAME_ROLE ).toString() ) )
    {
      menu->addAction( tr( "Load First 10 Unique Used Values" ), mExpressionBuilderWidget, &QgsExpressionBuilderWidget::loadSampleUsedValues );
      menu->addAction( tr( "Load All Unique Used Values" ), mExpressionBuilderWidget, &QgsExpressionBuilderWidget::loadAllUsedValues );
    }
  }
  return menu;
}
