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

#include "qgsexpressionbuilderwidget.h"
#include "qgslogger.h"
#include "qgsexpression.h"
#include "qgsexpressionfunction.h"
#include "qgsexpressionnodeimpl.h"
#include "qgsmessageviewer.h"
#include "qgsapplication.h"
#include "qgspythonrunner.h"
#include "qgsgeometry.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsvectorlayer.h"
#include "qgssettings.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgsrelation.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfieldformatterregistry.h"
#include "qgsfieldformatter.h"

#include <QMenu>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QInputDialog>
#include <QComboBox>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>


QgsExpressionBuilderWidget::QgsExpressionBuilderWidget( QWidget *parent )
  : QWidget( parent )
  , mProject( QgsProject::instance() )
{
  setupUi( this );

  connect( btnRun, &QToolButton::pressed, this, &QgsExpressionBuilderWidget::btnRun_pressed );
  connect( btnNewFile, &QToolButton::pressed, this, &QgsExpressionBuilderWidget::btnNewFile_pressed );
  connect( cmbFileNames, &QListWidget::currentItemChanged, this, &QgsExpressionBuilderWidget::cmbFileNames_currentItemChanged );
  connect( expressionTree, &QTreeView::doubleClicked, this, &QgsExpressionBuilderWidget::expressionTree_doubleClicked );
  connect( txtExpressionString, &QgsCodeEditorExpression::textChanged, this, &QgsExpressionBuilderWidget::txtExpressionString_textChanged );
  connect( txtPython, &QgsCodeEditorPython::textChanged, this, &QgsExpressionBuilderWidget::txtPython_textChanged );
  connect( txtSearchEditValues, &QgsFilterLineEdit::textChanged, this, &QgsExpressionBuilderWidget::txtSearchEditValues_textChanged );
  connect( txtSearchEdit, &QgsFilterLineEdit::textChanged, this, &QgsExpressionBuilderWidget::txtSearchEdit_textChanged );
  connect( lblPreview, &QLabel::linkActivated, this, &QgsExpressionBuilderWidget::lblPreview_linkActivated );
  connect( mValuesListView, &QListView::doubleClicked, this, &QgsExpressionBuilderWidget::mValuesListView_doubleClicked );

  txtHelpText->setOpenExternalLinks( true );

  mValueGroupBox->hide();
//  highlighter = new QgsExpressionHighlighter( txtExpressionString->document() );

  mModel = qgis::make_unique<QStandardItemModel>();
  mProxyModel = qgis::make_unique<QgsExpressionItemSearchProxy>();
  mProxyModel->setDynamicSortFilter( true );
  mProxyModel->setSourceModel( mModel.get() );
  expressionTree->setModel( mProxyModel.get() );
  expressionTree->setSortingEnabled( true );
  expressionTree->sortByColumn( 0, Qt::AscendingOrder );

  expressionTree->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( this, &QgsExpressionBuilderWidget::expressionParsed, this, &QgsExpressionBuilderWidget::setExpressionState );
  connect( expressionTree, &QWidget::customContextMenuRequested, this, &QgsExpressionBuilderWidget::showContextMenu );
  connect( expressionTree->selectionModel(), &QItemSelectionModel::currentChanged,
           this, &QgsExpressionBuilderWidget::currentChanged );

  connect( btnLoadAll, &QAbstractButton::pressed, this, &QgsExpressionBuilderWidget::loadAllValues );
  connect( btnLoadSample, &QAbstractButton::pressed, this, &QgsExpressionBuilderWidget::loadSampleValues );

  const auto pushButtons { mOperatorsGroupBox->findChildren<QPushButton *>() };
  for ( QPushButton *button : pushButtons )
  {
    connect( button, &QAbstractButton::pressed, this, &QgsExpressionBuilderWidget::operatorButtonClicked );
  }

  txtSearchEdit->setShowSearchIcon( true );
  txtSearchEdit->setPlaceholderText( tr( "Search…" ) );

  mValuesModel = qgis::make_unique<QStandardItemModel>();
  mProxyValues = qgis::make_unique<QSortFilterProxyModel>();
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

  txtExpressionString->setFoldingVisible( false );

  updateFunctionTree();

  if ( QgsPythonRunner::isValid() )
  {
    QgsPythonRunner::eval( QStringLiteral( "qgis.user.expressionspath" ), mFunctionsPath );
    updateFunctionFileList( mFunctionsPath );
  }
  else
  {
    tab_2->hide();
  }

  // select the first item in the function list
  // in order to avoid a blank help widget
  QModelIndex firstItem = mProxyModel->index( 0, 0, QModelIndex() );
  expressionTree->setCurrentIndex( firstItem );

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
  txtExpressionString->setAutoCompletionCaseSensitivity( true );
  txtExpressionString->setAutoCompletionSource( QsciScintilla::AcsAPIs );
  txtExpressionString->setCallTipsVisible( 0 );

  setExpectedOutputFormat( QString() );
  mFunctionBuilderHelp->setMarginVisible( false );
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
}

void QgsExpressionBuilderWidget::setLayer( QgsVectorLayer *layer )
{
  mLayer = layer;

  //TODO - remove existing layer scope from context

  if ( mLayer )
    mExpressionContext << QgsExpressionContextUtils::layerScope( mLayer );
}

void QgsExpressionBuilderWidget::currentChanged( const QModelIndex &index, const QModelIndex & )
{
  txtSearchEditValues->clear();

  // Get the item
  QModelIndex idx = mProxyModel->mapToSource( index );
  QgsExpressionItem *item = dynamic_cast<QgsExpressionItem *>( mModel->itemFromIndex( idx ) );
  if ( !item )
    return;

  bool isField = mLayer && item->getItemType() == QgsExpressionItem::Field;
  if ( isField )
  {
    loadFieldValues( mFieldValues.value( item->text() ) );
  }
  mValueGroupBox->setVisible( isField );
  mShowHelpButton->setText( isField ? tr( "Show Values" ) : tr( "Show Help" ) );

  // Show the help for the current item.
  QString help = loadFunctionHelp( item );
  txtHelpText->setText( help );
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
  updateFunctionTree();
  loadFieldNames();
  loadRecent( mRecentKey );
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
    myFileStream << txtPython->text() << endl;
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
    newFunctionFile( "default" );
    txtPython->setText( QString( "'''\n#Sample custom function file\n "
                                 "(uncomment to use and customize or Add button to create a new file) \n%1 \n '''" ).arg( txtPython->text() ) );
    saveFunctionFile( "default" );
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

void QgsExpressionBuilderWidget::expressionTree_doubleClicked( const QModelIndex &index )
{
  QModelIndex idx = mProxyModel->mapToSource( index );
  QgsExpressionItem *item = dynamic_cast<QgsExpressionItem *>( mModel->itemFromIndex( idx ) );
  if ( !item )
    return;

  // Don't handle the double-click if we are on a header node.
  if ( item->getItemType() == QgsExpressionItem::Header )
    return;

  // Insert the expression text or replace selected text
  txtExpressionString->insertText( item->getExpressionText() );
  txtExpressionString->setFocus();
}

void QgsExpressionBuilderWidget::loadFieldNames()
{
  // TODO We should really return a error the user of the widget that
  // the there is no layer set.
  if ( !mLayer )
    return;

  loadFieldNames( mLayer->fields() );
}


void QgsExpressionBuilderWidget::loadFieldNames( const QgsFields &fields )
{
  if ( fields.isEmpty() )
    return;

  txtExpressionString->setFields( fields );

  for ( int i = 0; i < fields.count(); ++i )
  {
    const QgsField field = fields.at( i );
    QIcon icon = fields.iconForField( i );
    registerItem( QStringLiteral( "Fields and Values" ), field.displayNameWithAlias(),
                  " \"" + field.name() + "\" ", QString(), QgsExpressionItem::Field, false, i, icon );
  }
}

void QgsExpressionBuilderWidget::loadFieldsAndValues( const QMap<QString, QStringList> &fieldValues )
{
  mFieldValues.clear();
  QgsFields fields;
  for ( auto it = fieldValues.constBegin(); it != fieldValues.constEnd(); ++it )
  {
    fields.append( QgsField( it.key() ) );
    const QStringList values = it.value();
    QVariantMap map;
    for ( const QString &value : values )
    {
      map.insert( value, value );
    }
    mFieldValues.insert( it.key(), map );
  }
  loadFieldNames( fields );
}

void QgsExpressionBuilderWidget::loadFieldsAndValues( const QMap<QString, QVariantMap> &fieldValues )
{
  QgsFields fields;
  for ( auto it = fieldValues.constBegin(); it != fieldValues.constEnd(); ++it )
  {
    fields.append( QgsField( it.key() ) );
  }
  loadFieldNames( fields );
  mFieldValues = fieldValues;
}

void QgsExpressionBuilderWidget::fillFieldValues( const QString &fieldName, int countLimit )
{
  // TODO We should really return a error the user of the widget that
  // the there is no layer set.
  if ( !mLayer )
    return;

  // TODO We should thread this so that we don't hold the user up if the layer is massive.

  const QgsFields fields = mLayer->fields();
  int fieldIndex = fields.lookupField( fieldName );

  if ( fieldIndex < 0 )
    return;

  const QgsEditorWidgetSetup setup = fields.at( fieldIndex ).editorWidgetSetup();
  const QgsFieldFormatter *formatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( setup.type() );

  QList<QVariant> values = mLayer->uniqueValues( fieldIndex, countLimit ).toList();
  std::sort( values.begin(), values.end() );

  for ( const QVariant &value : qgis::as_const( values ) )
  {
    QString strValue;
    if ( value.isNull() )
      strValue = QStringLiteral( "NULL" );
    else if ( value.type() == QVariant::Int || value.type() == QVariant::Double || value.type() == QVariant::LongLong )
      strValue = value.toString();
    else
      strValue = '\'' + value.toString().replace( '\'', QLatin1String( "''" ) ) + '\'';

    QString representedValue = formatter->representValue( mLayer, fieldIndex, setup.config(), QVariant(), value );
    if ( representedValue != value.toString() )
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

void QgsExpressionBuilderWidget::registerItem( const QString &group,
    const QString &label,
    const QString &expressionText,
    const QString &helpText,
    QgsExpressionItem::ItemType type, bool highlightedItem, int sortOrder, QIcon icon )
{
  QgsExpressionItem *item = new QgsExpressionItem( label, expressionText, helpText, type );
  item->setData( label, Qt::UserRole );
  item->setData( sortOrder, QgsExpressionItem::CUSTOM_SORT_ROLE );
  item->setIcon( icon );

  // Look up the group and insert the new function.
  if ( mExpressionGroups.contains( group ) )
  {
    QgsExpressionItem *groupNode = mExpressionGroups.value( group );
    groupNode->appendRow( item );
  }
  else
  {
    // If the group doesn't exist yet we make it first.
    QgsExpressionItem *newgroupNode = new QgsExpressionItem( QgsExpression::group( group ), QString(), QgsExpressionItem::Header );
    newgroupNode->setData( group, Qt::UserRole );
    //Recent group should always be last group
    newgroupNode->setData( group.startsWith( QLatin1String( "Recent (" ) ) ? 2 : 1, QgsExpressionItem::CUSTOM_SORT_ROLE );
    newgroupNode->appendRow( item );
    newgroupNode->setBackground( QBrush( QColor( 238, 238, 238 ) ) );
    mModel->appendRow( newgroupNode );
    mExpressionGroups.insert( group, newgroupNode );
  }

  if ( highlightedItem )
  {
    //insert a copy as a top level item
    QgsExpressionItem *topLevelItem = new QgsExpressionItem( label, expressionText, helpText, type );
    topLevelItem->setData( label, Qt::UserRole );
    item->setData( 0, QgsExpressionItem::CUSTOM_SORT_ROLE );
    QFont font = topLevelItem->font();
    font.setBold( true );
    topLevelItem->setFont( font );
    mModel->appendRow( topLevelItem );
  }

}

bool QgsExpressionBuilderWidget::isExpressionValid()
{
  return mExpressionValid;
}

void QgsExpressionBuilderWidget::saveToRecent( const QString &collection )
{
  QgsSettings settings;
  QString location = QStringLiteral( "/expressions/recent/%1" ).arg( collection );
  QStringList expressions = settings.value( location ).toStringList();
  expressions.removeAll( this->expressionText() );

  expressions.prepend( this->expressionText() );

  while ( expressions.count() > 20 )
  {
    expressions.pop_back();
  }

  settings.setValue( location, expressions );
  this->loadRecent( collection );
}

void QgsExpressionBuilderWidget::loadRecent( const QString &collection )
{
  mRecentKey = collection;
  QString name = tr( "Recent (%1)" ).arg( collection );
  if ( mExpressionGroups.contains( name ) )
  {
    QgsExpressionItem *node = mExpressionGroups.value( name );
    node->removeRows( 0, node->rowCount() );
  }

  QgsSettings settings;
  QString location = QStringLiteral( "/expressions/recent/%1" ).arg( collection );
  QStringList expressions = settings.value( location ).toStringList();
  int i = 0;
  const auto constExpressions = expressions;
  for ( const QString &expression : constExpressions )
  {
    this->registerItem( name, expression, expression, expression, QgsExpressionItem::ExpressionNode, false, i );
    i++;
  }
}

void QgsExpressionBuilderWidget::loadLayers()
{
  if ( !mProject )
    return;

  QMap<QString, QgsMapLayer *> layers = mProject->mapLayers();
  QMap<QString, QgsMapLayer *>::const_iterator layerIt = layers.constBegin();
  for ( ; layerIt != layers.constEnd(); ++layerIt )
  {
    registerItemForAllGroups( QStringList() << tr( "Map Layers" ), layerIt.value()->name(), QStringLiteral( "'%1'" ).arg( layerIt.key() ), formatLayerHelp( layerIt.value() ) );
  }
}

void QgsExpressionBuilderWidget::loadRelations()
{
  if ( !mProject )
    return;

  QMap<QString, QgsRelation> relations = mProject->relationManager()->relations();
  QMap<QString, QgsRelation>::const_iterator relIt = relations.constBegin();
  for ( ; relIt != relations.constEnd(); ++relIt )
  {
    registerItemForAllGroups( QStringList() << tr( "Relations" ), relIt->name(), QStringLiteral( "'%1'" ).arg( relIt->id() ), formatRelationHelp( relIt.value() ) );
  }
}

void QgsExpressionBuilderWidget::updateFunctionTree()
{
  mModel->clear();
  mExpressionGroups.clear();
  // TODO Can we move this stuff to QgsExpression, like the functions?
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "+" ), QStringLiteral( " + " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "-" ), QStringLiteral( " - " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "*" ), QStringLiteral( " * " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "/" ), QStringLiteral( " / " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "%" ), QStringLiteral( " % " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "^" ), QStringLiteral( " ^ " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "=" ), QStringLiteral( " = " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "~" ), QStringLiteral( " ~ " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( ">" ), QStringLiteral( " > " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "<" ), QStringLiteral( " < " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "<>" ), QStringLiteral( " <> " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "<=" ), QStringLiteral( " <= " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( ">=" ), QStringLiteral( " >= " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "[]" ), QStringLiteral( "[ ]" ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "||" ), QStringLiteral( " || " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "IN" ), QStringLiteral( " IN " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "LIKE" ), QStringLiteral( " LIKE " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "ILIKE" ), QStringLiteral( " ILIKE " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "IS" ), QStringLiteral( " IS " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "OR" ), QStringLiteral( " OR " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "AND" ), QStringLiteral( " AND " ) );
  registerItem( QStringLiteral( "Operators" ), QStringLiteral( "NOT" ), QStringLiteral( " NOT " ) );

  QString casestring = QStringLiteral( "CASE WHEN condition THEN result END" );
  registerItem( QStringLiteral( "Conditionals" ), QStringLiteral( "CASE" ), casestring );

  // use -1 as sort order here -- NULL should always show before the field list
  registerItem( QStringLiteral( "Fields and Values" ), QStringLiteral( "NULL" ), QStringLiteral( "NULL" ), QString(), QgsExpressionItem::ExpressionNode, false, -1 );

  // Load the functions from the QgsExpression class
  int count = QgsExpression::functionCount();
  for ( int i = 0; i < count; i++ )
  {
    QgsExpressionFunction *func = QgsExpression::Functions()[i];
    QString name = func->name();
    if ( name.startsWith( '_' ) ) // do not display private functions
      continue;
    if ( func->isDeprecated() ) // don't show deprecated functions
      continue;
    if ( func->isContextual() )
    {
      //don't show contextual functions by default - it's up the the QgsExpressionContext
      //object to provide them if supported
      continue;
    }
    if ( func->params() != 0 )
      name += '(';
    else if ( !name.startsWith( '$' ) )
      name += QLatin1String( "()" );
    registerItemForAllGroups( func->groups(), func->name(), ' ' + name + ' ', func->helpText(), QgsExpressionItem::ExpressionNode, mExpressionContext.isHighlightedFunction( func->name() ) );
  }

  // load relation names
  loadRelations();

  // load layer IDs
  loadLayers();

  loadExpressionContext();
}

void QgsExpressionBuilderWidget::setGeomCalculator( const QgsDistanceArea &da )
{
  mDa = da;
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
  updateFunctionTree();
  loadFieldNames();
  loadRecent( mRecentKey );
}

void QgsExpressionBuilderWidget::txtExpressionString_textChanged()
{
  QString text = expressionText();
  clearErrors();

  // If the string is empty the expression will still "fail" although
  // we don't show the user an error as it will be confusing.
  if ( text.isEmpty() )
  {
    lblPreview->clear();
    lblPreview->setStyleSheet( QString() );
    txtExpressionString->setToolTip( QString() );
    lblPreview->setToolTip( QString() );
    emit expressionParsed( false );
    setParserError( true );
    setEvalError( true );
    return;
  }


  QgsExpression exp( text );

  if ( mLayer )
  {
    // Only set calculator if we have layer, else use default.
    exp.setGeomCalculator( &mDa );

    if ( !mExpressionContext.feature().isValid() )
    {
      // no feature passed yet, try to get from layer
      QgsFeature f;
      mLayer->getFeatures( QgsFeatureRequest().setLimit( 1 ) ).nextFeature( f );
      mExpressionContext.setFeature( f );
    }
  }

  QVariant value = exp.evaluate( &mExpressionContext );
  if ( !exp.hasEvalError() )
  {
    lblPreview->setText( QgsExpression::formatPreviewString( value ) );
  }

  if ( exp.hasParserError() || exp.hasEvalError() )
  {
    QString errorString = exp.parserErrorString().replace( "\n", "<br>" );
    QString tooltip;
    if ( exp.hasParserError() )
      tooltip = QStringLiteral( "<b>%1:</b>"
                                "%2" ).arg( tr( "Parser Errors" ), errorString );
    // Only show the eval error if there is no parser error.
    if ( !exp.hasParserError() && exp.hasEvalError() )
      tooltip += QStringLiteral( "<b>%1:</b> %2" ).arg( tr( "Eval Error" ), exp.evalErrorString() );

    lblPreview->setText( tr( "Expression is invalid <a href=""more"">(more info)</a>" ) );
    lblPreview->setStyleSheet( QStringLiteral( "color: rgba(255, 6, 10,  255);" ) );
    txtExpressionString->setToolTip( tooltip );
    lblPreview->setToolTip( tooltip );
    emit expressionParsed( false );
    setParserError( exp.hasParserError() );
    setEvalError( exp.hasEvalError() );
    createErrorMarkers( exp.parserErrors() );
    return;
  }
  else
  {
    lblPreview->setStyleSheet( QString() );
    txtExpressionString->setToolTip( QString() );
    lblPreview->setToolTip( QString() );
    emit expressionParsed( true );
    setParserError( false );
    setEvalError( false );
    createMarkers( exp.rootNode() );
  }

}

void QgsExpressionBuilderWidget::loadExpressionContext()
{
  txtExpressionString->setExpressionContext( mExpressionContext );
  QStringList variableNames = mExpressionContext.filteredVariableNames();
  const auto constVariableNames = variableNames;
  for ( const QString &variable : constVariableNames )
  {
    registerItem( QStringLiteral( "Variables" ), variable, " @" + variable + ' ',
                  QgsExpression::formatVariableHelp( mExpressionContext.description( variable ), true, mExpressionContext.variable( variable ) ),
                  QgsExpressionItem::ExpressionNode,
                  mExpressionContext.isHighlightedVariable( variable ) );
  }

  // Load the functions from the expression context
  QStringList contextFunctions = mExpressionContext.functionNames();
  const auto constContextFunctions = contextFunctions;
  for ( const QString &functionName : constContextFunctions )
  {
    QgsExpressionFunction *func = mExpressionContext.function( functionName );
    QString name = func->name();
    if ( name.startsWith( '_' ) ) // do not display private functions
      continue;
    if ( func->params() != 0 )
      name += '(';
    registerItemForAllGroups( func->groups(), func->name(), ' ' + name + ' ', func->helpText(), QgsExpressionItem::ExpressionNode, mExpressionContext.isHighlightedFunction( func->name() ) );
  }
}

void QgsExpressionBuilderWidget::registerItemForAllGroups( const QStringList &groups, const QString &label, const QString &expressionText, const QString &helpText, QgsExpressionItem::ItemType type, bool highlightedItem, int sortOrder )
{
  const auto constGroups = groups;
  for ( const QString &group : constGroups )
  {
    registerItem( group, label, expressionText, helpText, type, highlightedItem, sortOrder );
  }
}

QString QgsExpressionBuilderWidget::formatRelationHelp( const QgsRelation &relation ) const
{
  QString text = QStringLiteral( "<p>%1</p>" ).arg( tr( "Inserts the relation ID for the relation named '%1'." ).arg( relation.name() ) );
  text.append( QStringLiteral( "<p>%1</p>" ).arg( tr( "Current value: '%1'" ).arg( relation.id() ) ) );
  return text;
}

QString QgsExpressionBuilderWidget::formatLayerHelp( const QgsMapLayer *layer ) const
{
  QString text = QStringLiteral( "<p>%1</p>" ).arg( tr( "Inserts the layer ID for the layer named '%1'." ).arg( layer->name() ) );
  text.append( QStringLiteral( "<p>%1</p>" ).arg( tr( "Current value: '%1'" ).arg( layer->id() ) ) );
  return text;
}

bool QgsExpressionBuilderWidget::parserError() const
{
  return mParserError;
}

void QgsExpressionBuilderWidget::setParserError( bool parserError )
{
  if ( parserError == mParserError )
    return;

  mParserError = parserError;
  emit parserErrorChanged();
}

void QgsExpressionBuilderWidget::loadFieldValues( const QVariantMap &values )
{
  mValuesModel->clear();
  for ( QVariantMap::ConstIterator it = values.constBegin(); it != values.constEnd(); ++ it )
  {
    QStandardItem *item = new QStandardItem( it.key() );
    item->setData( it.value() );
    mValuesModel->appendRow( item );
  }
}

bool QgsExpressionBuilderWidget::evalError() const
{
  return mEvalError;
}

void QgsExpressionBuilderWidget::setEvalError( bool evalError )
{
  if ( evalError == mEvalError )
    return;

  mEvalError = evalError;
  emit evalErrorChanged();
}

QStandardItemModel *QgsExpressionBuilderWidget::model()
{
  return mModel.get();
}

QgsProject *QgsExpressionBuilderWidget::project()
{
  return mProject;
}

void QgsExpressionBuilderWidget::setProject( QgsProject *project )
{
  mProject = project;
  updateFunctionTree();
}

void QgsExpressionBuilderWidget::showEvent( QShowEvent *e )
{
  QWidget::showEvent( e );
  txtExpressionString->setFocus();
}

void QgsExpressionBuilderWidget::createErrorMarkers( QList<QgsExpression::ParserError> errors )
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
      for ( QgsExpressionNodeCondition::WhenThen *cond : node->conditions() )
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

void QgsExpressionBuilderWidget::txtSearchEdit_textChanged()
{
  mProxyModel->setFilterWildcard( txtSearchEdit->text() );
  if ( txtSearchEdit->text().isEmpty() )
  {
    expressionTree->collapseAll();
  }
  else
  {
    expressionTree->expandAll();
    QModelIndex index = mProxyModel->index( 0, 0 );
    if ( mProxyModel->hasChildren( index ) )
    {
      QModelIndex child = mProxyModel->index( 0, 0, index );
      expressionTree->selectionModel()->setCurrentIndex( child, QItemSelectionModel::ClearAndSelect );
    }
  }
}

void QgsExpressionBuilderWidget::txtSearchEditValues_textChanged()
{
  mProxyValues->setFilterCaseSensitivity( Qt::CaseInsensitive );
  mProxyValues->setFilterWildcard( txtSearchEditValues->text() );
}

void QgsExpressionBuilderWidget::lblPreview_linkActivated( const QString &link )
{
  Q_UNUSED( link )
  QgsMessageViewer *mv = new QgsMessageViewer( this );
  mv->setWindowTitle( tr( "More Info on Expression Error" ) );
  mv->setMessageAsHtml( txtExpressionString->toolTip() );
  mv->exec();
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

void QgsExpressionBuilderWidget::showContextMenu( QPoint pt )
{
  QModelIndex idx = expressionTree->indexAt( pt );
  idx = mProxyModel->mapToSource( idx );
  QgsExpressionItem *item = dynamic_cast<QgsExpressionItem *>( mModel->itemFromIndex( idx ) );
  if ( !item )
    return;

  if ( item->getItemType() == QgsExpressionItem::Field && mLayer )
  {
    QMenu *menu = new QMenu( this );
    menu->addAction( tr( "Load First 10 Unique Values" ), this, SLOT( loadSampleValues() ) );
    menu->addAction( tr( "Load All Unique Values" ), this, SLOT( loadAllValues() ) );
    menu->popup( expressionTree->mapToGlobal( pt ) );
  }
}

void QgsExpressionBuilderWidget::loadSampleValues()
{
  QModelIndex idx = mProxyModel->mapToSource( expressionTree->currentIndex() );
  QgsExpressionItem *item = dynamic_cast<QgsExpressionItem *>( mModel->itemFromIndex( idx ) );
  // TODO We should really return a error the user of the widget that
  // the there is no layer set.
  if ( !mLayer || !item )
    return;

  mValueGroupBox->show();
  fillFieldValues( item->text(), 10 );
}

void QgsExpressionBuilderWidget::loadAllValues()
{
  QModelIndex idx = mProxyModel->mapToSource( expressionTree->currentIndex() );
  QgsExpressionItem *item = dynamic_cast<QgsExpressionItem *>( mModel->itemFromIndex( idx ) );
  // TODO We should really return a error the user of the widget that
  // the there is no layer set.
  if ( !mLayer || !item )
    return;

  mValueGroupBox->show();
  fillFieldValues( item->text(), -1 );
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

void QgsExpressionBuilderWidget::setExpressionState( bool state )
{
  mExpressionValid = state;
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





QgsExpressionItemSearchProxy::QgsExpressionItemSearchProxy()
{
  setFilterCaseSensitivity( Qt::CaseInsensitive );
}

bool QgsExpressionItemSearchProxy::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  QModelIndex index = sourceModel()->index( source_row, 0, source_parent );
  QgsExpressionItem::ItemType itemType = QgsExpressionItem::ItemType( sourceModel()->data( index, QgsExpressionItem::ITEM_TYPE_ROLE ).toInt() );

  int count = sourceModel()->rowCount( index );
  bool matchchild = false;
  for ( int i = 0; i < count; ++i )
  {
    if ( filterAcceptsRow( i, index ) )
    {
      matchchild = true;
      break;
    }
  }

  if ( itemType == QgsExpressionItem::Header && matchchild )
    return true;

  if ( itemType == QgsExpressionItem::Header )
    return false;

  return QSortFilterProxyModel::filterAcceptsRow( source_row, source_parent );
}

bool QgsExpressionItemSearchProxy::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  int leftSort = sourceModel()->data( left, QgsExpressionItem::CUSTOM_SORT_ROLE ).toInt();
  int rightSort = sourceModel()->data( right,  QgsExpressionItem::CUSTOM_SORT_ROLE ).toInt();
  if ( leftSort != rightSort )
    return leftSort < rightSort;

  QString leftString = sourceModel()->data( left, Qt::DisplayRole ).toString();
  QString rightString = sourceModel()->data( right, Qt::DisplayRole ).toString();

  //ignore $ prefixes when sorting
  if ( leftString.startsWith( '$' ) )
    leftString = leftString.mid( 1 );
  if ( rightString.startsWith( '$' ) )
    rightString = rightString.mid( 1 );

  return QString::localeAwareCompare( leftString, rightString ) < 0;
}
