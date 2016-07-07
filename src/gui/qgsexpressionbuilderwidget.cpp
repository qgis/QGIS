/***************************************************************************
    qgisexpressionbuilderwidget.cpp - A genric expression string builder widget.
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
#include "qgsmessageviewer.h"
#include "qgsapplication.h"
#include "qgspythonrunner.h"
#include "qgsgeometry.h"
#include "qgsfeature.h"

#include <QSettings>
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
    , mAutoSave( true )
    , mLayer( nullptr )
    , highlighter( nullptr )
    , mExpressionValid( false )
{
  setupUi( this );

  mValueGroupBox->hide();
  mLoadGroupBox->hide();
//  highlighter = new QgsExpressionHighlighter( txtExpressionString->document() );

  mModel = new QStandardItemModel();
  mProxyModel = new QgsExpressionItemSearchProxy();
  mProxyModel->setDynamicSortFilter( true );
  mProxyModel->setSourceModel( mModel );
  expressionTree->setModel( mProxyModel );
  expressionTree->setSortingEnabled( true );
  expressionTree->sortByColumn( 0, Qt::AscendingOrder );

  expressionTree->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( this, SIGNAL( expressionParsed( bool ) ), this, SLOT( setExpressionState( bool ) ) );
  connect( expressionTree, SIGNAL( customContextMenuRequested( const QPoint & ) ), this, SLOT( showContextMenu( const QPoint & ) ) );
  connect( expressionTree->selectionModel(), SIGNAL( currentChanged( const QModelIndex &, const QModelIndex & ) ),
           this, SLOT( currentChanged( const QModelIndex &, const QModelIndex & ) ) );

  connect( btnLoadAll, SIGNAL( pressed() ), this, SLOT( loadAllValues() ) );
  connect( btnLoadSample, SIGNAL( pressed() ), this, SLOT( loadSampleValues() ) );

  Q_FOREACH ( QPushButton* button, mOperatorsGroupBox->findChildren<QPushButton *>() )
  {
    connect( button, SIGNAL( pressed() ), this, SLOT( operatorButtonClicked() ) );
  }

  txtSearchEdit->setPlaceholderText( tr( "Search" ) );

  mValuesModel = new QStringListModel();
  mProxyValues = new QSortFilterProxyModel();
  mProxyValues->setSourceModel( mValuesModel );
  mValuesListView->setModel( mProxyValues );
  txtSearchEditValues->setPlaceholderText( tr( "Search" ) );

  QSettings settings;
  splitter->restoreState( settings.value( "/windows/QgsExpressionBuilderWidget/splitter" ).toByteArray() );
  editorSplit->restoreState( settings.value( "/windows/QgsExpressionBuilderWidget/editorsplitter" ).toByteArray() );
  functionsplit->restoreState( settings.value( "/windows/QgsExpressionBuilderWidget/functionsplitter" ).toByteArray() );

  txtExpressionString->setFoldingVisible( false );

  updateFunctionTree();

  if ( QgsPythonRunner::isValid() )
  {
    QgsPythonRunner::eval( "qgis.user.expressionspath", mFunctionsPath );
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

  lblAutoSave->setText( "" );
}


QgsExpressionBuilderWidget::~QgsExpressionBuilderWidget()
{
  QSettings settings;
  settings.setValue( "/windows/QgsExpressionBuilderWidget/splitter", splitter->saveState() );
  settings.setValue( "/windows/QgsExpressionBuilderWidget/editorsplitter", editorSplit->saveState() );
  settings.setValue( "/windows/QgsExpressionBuilderWidget/functionsplitter", functionsplit->saveState() );

  delete mModel;
  delete mProxyModel;
  delete mValuesModel;
  delete mProxyValues;
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
  txtSearchEditValues->setText( QString( "" ) );

  // Get the item
  QModelIndex idx = mProxyModel->mapToSource( index );
  QgsExpressionItem* item = dynamic_cast<QgsExpressionItem*>( mModel->itemFromIndex( idx ) );
  if ( !item )
    return;

  if ( item->getItemType() == QgsExpressionItem::Field && mFieldValues.contains( item->text() ) )
  {
    const QStringList& values = mFieldValues[item->text()];
    mValuesModel->setStringList( values );
  }

  mLoadGroupBox->setVisible( item->getItemType() == QgsExpressionItem::Field && mLayer );
  mValueGroupBox->setVisible(( item->getItemType() == QgsExpressionItem::Field && mLayer ) || mValuesListView->model()->rowCount() > 0 );

  // Show the help for the current item.
  QString help = loadFunctionHelp( item );
  txtHelpText->setText( help );
}

void QgsExpressionBuilderWidget::on_btnRun_pressed()
{
  if ( !cmbFileNames->currentItem() )
    return;

  QString file = cmbFileNames->currentItem()->text();
  saveFunctionFile( file );
  runPythonCode( txtPython->text() );
}

void QgsExpressionBuilderWidget::runPythonCode( const QString& code )
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

  if ( !fileName.endsWith( ".py" ) )
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

void QgsExpressionBuilderWidget::updateFunctionFileList( const QString& path )
{
  mFunctionsPath = path;
  QDir dir( path );
  dir.setNameFilters( QStringList() << "*.py" );
  QStringList files = dir.entryList( QDir::Files );
  cmbFileNames->clear();
  Q_FOREACH ( const QString& name, files )
  {
    QFileInfo info( mFunctionsPath + QDir::separator() + name );
    if ( info.baseName() == "__init__" ) continue;
    QListWidgetItem* item = new QListWidgetItem( QgsApplication::getThemeIcon( "console/iconTabEditorConsole.png" ), info.baseName() );
    cmbFileNames->addItem( item );
  }
  if ( !cmbFileNames->currentItem() )
    cmbFileNames->setCurrentRow( 0 );
}

void QgsExpressionBuilderWidget::newFunctionFile( const QString& fileName )
{
  QList<QListWidgetItem*> items = cmbFileNames->findItems( fileName, Qt::MatchExactly );
  if ( !items.isEmpty() )
    return;

  QString templatetxt;
  QgsPythonRunner::eval( "qgis.user.expressions.template", templatetxt );
  txtPython->setText( templatetxt );
  cmbFileNames->insertItem( 0, fileName );
  cmbFileNames->setCurrentRow( 0 );
  saveFunctionFile( fileName );
}

void QgsExpressionBuilderWidget::on_btnNewFile_pressed()
{
  bool ok;
  QString text = QInputDialog::getText( this, tr( "Enter new file name" ),
                                        tr( "File name:" ), QLineEdit::Normal,
                                        "", &ok );
  if ( ok && !text.isEmpty() )
  {
    newFunctionFile( text );
  }
}

void QgsExpressionBuilderWidget::on_cmbFileNames_currentItemChanged( QListWidgetItem* item, QListWidgetItem* lastitem )
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
  if ( !path.endsWith( ".py" ) )
    path.append( ".py" );

  txtPython->loadScript( path );
}

void QgsExpressionBuilderWidget::loadFunctionCode( const QString& code )
{
  txtPython->setText( code );
}

void QgsExpressionBuilderWidget::on_expressionTree_doubleClicked( const QModelIndex &index )
{
  QModelIndex idx = mProxyModel->mapToSource( index );
  QgsExpressionItem* item = dynamic_cast<QgsExpressionItem*>( mModel->itemFromIndex( idx ) );
  if ( !item )
    return;

  // Don't handle the double click it we are on a header node.
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

void QgsExpressionBuilderWidget::loadFieldNames( const QgsFields& fields )
{
  if ( fields.isEmpty() )
    return;

  QStringList fieldNames;
  //Q_FOREACH ( const QgsField& field, fields )
  fieldNames.reserve( fields.count() );
  for ( int i = 0; i < fields.count(); ++i )
  {
    QString fieldName = fields.at( i ).name();
    fieldNames << fieldName;
    registerItem( "Fields and Values", fieldName, " \"" + fieldName + "\" ", "", QgsExpressionItem::Field, false, i );
  }
//  highlighter->addFields( fieldNames );
}

void QgsExpressionBuilderWidget::loadFieldsAndValues( const QMap<QString, QStringList> &fieldValues )
{
  QgsFields fields;
  Q_FOREACH ( const QString& fieldName, fieldValues.keys() )
  {
    fields.append( QgsField( fieldName ) );
  }
  loadFieldNames( fields );
  mFieldValues = fieldValues;
}

void QgsExpressionBuilderWidget::fillFieldValues( const QString& fieldName, int countLimit )
{
  // TODO We should really return a error the user of the widget that
  // the there is no layer set.
  if ( !mLayer )
    return;

  // TODO We should thread this so that we don't hold the user up if the layer is massive.

  int fieldIndex = mLayer->fieldNameIndex( fieldName );

  if ( fieldIndex < 0 )
    return;

  QList<QVariant> values;
  QStringList strValues;
  mLayer->uniqueValues( fieldIndex, values, countLimit );
  Q_FOREACH ( const QVariant& value, values )
  {
    QString strValue;
    if ( value.isNull() )
      strValue = "NULL";
    else if ( value.type() == QVariant::Int || value.type() == QVariant::Double || value.type() == QVariant::LongLong )
      strValue = value.toString();
    else
      strValue = '\'' + value.toString().replace( '\'', "''" ) + '\'';
    strValues.append( strValue );
  }
  mValuesModel->setStringList( strValues );
  mFieldValues[fieldName] = strValues;
}

void QgsExpressionBuilderWidget::registerItem( const QString& group,
    const QString& label,
    const QString& expressionText,
    const QString& helpText,
    QgsExpressionItem::ItemType type, bool highlightedItem, int sortOrder )
{
  QgsExpressionItem* item = new QgsExpressionItem( label, expressionText, helpText, type );
  item->setData( label, Qt::UserRole );
  item->setData( sortOrder, QgsExpressionItem::CustomSortRole );

  // Look up the group and insert the new function.
  if ( mExpressionGroups.contains( group ) )
  {
    QgsExpressionItem *groupNode = mExpressionGroups.value( group );
    groupNode->appendRow( item );
  }
  else
  {
    // If the group doesn't exist yet we make it first.
    QgsExpressionItem *newgroupNode = new QgsExpressionItem( QgsExpression::group( group ), "", QgsExpressionItem::Header );
    newgroupNode->setData( group, Qt::UserRole );
    //Recent group should always be last group
    newgroupNode->setData( group.startsWith( "Recent (" ) ? 2 : 1, QgsExpressionItem::CustomSortRole );
    newgroupNode->appendRow( item );
    newgroupNode->setBackground( QBrush( QColor( "#eee" ) ) );
    mModel->appendRow( newgroupNode );
    mExpressionGroups.insert( group, newgroupNode );
  }

  if ( highlightedItem )
  {
    //insert a copy as a top level item
    QgsExpressionItem* topLevelItem = new QgsExpressionItem( label, expressionText, helpText, type );
    topLevelItem->setData( label, Qt::UserRole );
    item->setData( 0, QgsExpressionItem::CustomSortRole );
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

void QgsExpressionBuilderWidget::saveToRecent( const QString& collection )
{
  QSettings settings;
  QString location = QString( "/expressions/recent/%1" ).arg( collection );
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

void QgsExpressionBuilderWidget::loadRecent( const QString& collection )
{
  mRecentKey = collection;
  QString name = tr( "Recent (%1)" ).arg( collection );
  if ( mExpressionGroups.contains( name ) )
  {
    QgsExpressionItem* node = mExpressionGroups.value( name );
    node->removeRows( 0, node->rowCount() );
  }

  QSettings settings;
  QString location = QString( "/expressions/recent/%1" ).arg( collection );
  QStringList expressions = settings.value( location ).toStringList();
  int i = 0;
  Q_FOREACH ( const QString& expression, expressions )
  {
    this->registerItem( name, expression, expression, expression, QgsExpressionItem::ExpressionNode, false, i );
    i++;
  }
}

void QgsExpressionBuilderWidget::updateFunctionTree()
{
  mModel->clear();
  mExpressionGroups.clear();
  // TODO Can we move this stuff to QgsExpression, like the functions?
  registerItem( "Operators", "+", " + " );
  registerItem( "Operators", "-", " - " );
  registerItem( "Operators", "*", " * " );
  registerItem( "Operators", "/", " / " );
  registerItem( "Operators", "%", " % " );
  registerItem( "Operators", "^", " ^ " );
  registerItem( "Operators", "=", " = " );
  registerItem( "Operators", ">", " > " );
  registerItem( "Operators", "<", " < " );
  registerItem( "Operators", "<>", " <> " );
  registerItem( "Operators", "<=", " <= " );
  registerItem( "Operators", ">=", " >= " );
  registerItem( "Operators", "||", " || " );
  registerItem( "Operators", "IN", " IN " );
  registerItem( "Operators", "LIKE", " LIKE " );
  registerItem( "Operators", "ILIKE", " ILIKE " );
  registerItem( "Operators", "IS", " IS " );
  registerItem( "Operators", "OR", " OR " );
  registerItem( "Operators", "AND", " AND " );
  registerItem( "Operators", "NOT", " NOT " );

  QString casestring = "CASE WHEN condition THEN result END";
  registerItem( "Conditionals", "CASE", casestring );

  registerItem( "Fields and Values", "NULL", "NULL" );

  // Load the functions from the QgsExpression class
  int count = QgsExpression::functionCount();
  for ( int i = 0; i < count; i++ )
  {
    QgsExpression::Function* func = QgsExpression::Functions()[i];
    QString name = func->name();
    if ( name.startsWith( '_' ) ) // do not display private functions
      continue;
    if ( func->group() == "deprecated" ) // don't show deprecated functions
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
      name += "()";
    registerItem( func->group(), func->name(), ' ' + name + ' ', func->helptext() );
  }

  QList<QgsExpression::Function*> specials = QgsExpression::specialColumns();
  for ( int i = 0; i < specials.size(); ++i )
  {
    QString name = specials[i]->name();
    registerItem( specials[i]->group(), name, ' ' + name + ' ' );
  }

  loadExpressionContext();
}

void QgsExpressionBuilderWidget::setGeomCalculator( const QgsDistanceArea & da )
{
  mDa = da;
}

QString QgsExpressionBuilderWidget::expressionText()
{
  return txtExpressionString->text();
}

void QgsExpressionBuilderWidget::setExpressionText( const QString& expression )
{
  txtExpressionString->setText( expression );
}

void QgsExpressionBuilderWidget::setExpressionContext( const QgsExpressionContext &context )
{
  mExpressionContext = context;

  loadExpressionContext();
}

void QgsExpressionBuilderWidget::on_txtExpressionString_textChanged()
{
  QString text = expressionText();

  // If the string is empty the expression will still "fail" although
  // we don't show the user an error as it will be confusing.
  if ( text.isEmpty() )
  {
    lblPreview->setText( "" );
    lblPreview->setStyleSheet( "" );
    txtExpressionString->setToolTip( "" );
    lblPreview->setToolTip( "" );
    emit expressionParsed( false );
    return;
  }

  QgsExpression exp( text );

  if ( mLayer )
  {
    // Only set calculator if we have layer, else use default.
    exp.setGeomCalculator( mDa );

    if ( !mFeature.isValid() )
    {
      mLayer->getFeatures().nextFeature( mFeature );
    }

    if ( mFeature.isValid() )
    {
      mExpressionContext.setFeature( mFeature );
      QVariant value = exp.evaluate( &mExpressionContext );
      if ( !exp.hasEvalError() )
        lblPreview->setText( QgsExpression::formatPreviewString( value ) );
    }
    else
    {
      // The feature is invalid because we don't have one but that doesn't mean user can't
      // build a expression string.  They just get no preview.
      lblPreview->setText( "" );
    }
  }
  else
  {
    // No layer defined
    QVariant value = exp.evaluate( &mExpressionContext );
    if ( !exp.hasEvalError() )
    {
      lblPreview->setText( QgsExpression::formatPreviewString( value ) );
    }
  }

  if ( exp.hasParserError() || exp.hasEvalError() )
  {
    QString tooltip = QString( "<b>%1:</b><br>%2" ).arg( tr( "Parser Error" ), exp.parserErrorString() );
    if ( exp.hasEvalError() )
      tooltip += QString( "<br><br><b>%1:</b><br>%2" ).arg( tr( "Eval Error" ), exp.evalErrorString() );

    lblPreview->setText( tr( "Expression is invalid <a href=""more"">(more info)</a>" ) );
    lblPreview->setStyleSheet( "color: rgba(255, 6, 10,  255);" );
    txtExpressionString->setToolTip( tooltip );
    lblPreview->setToolTip( tooltip );
    emit expressionParsed( false );
    return;
  }
  else
  {
    lblPreview->setStyleSheet( "" );
    txtExpressionString->setToolTip( "" );
    lblPreview->setToolTip( "" );
    emit expressionParsed( true );
  }
}

void QgsExpressionBuilderWidget::loadExpressionContext()
{
  QStringList variableNames = mExpressionContext.filteredVariableNames();
  Q_FOREACH ( const QString& variable, variableNames )
  {
    registerItem( "Variables", variable, " @" + variable + ' ',
                  QgsExpression::variableHelpText( variable, true, mExpressionContext.variable( variable ) ),
                  QgsExpressionItem::ExpressionNode,
                  mExpressionContext.isHighlightedVariable( variable ) );
  }

  // Load the functions from the expression context
  QStringList contextFunctions = mExpressionContext.functionNames();
  Q_FOREACH ( const QString& functionName, contextFunctions )
  {
    QgsExpression::Function* func = mExpressionContext.function( functionName );
    QString name = func->name();
    if ( name.startsWith( '_' ) ) // do not display private functions
      continue;
    if ( func->params() != 0 )
      name += '(';
    registerItem( func->group(), func->name(), ' ' + name + ' ', func->helptext() );
  }
}

void QgsExpressionBuilderWidget::showEvent( QShowEvent *e )
{
  QWidget::showEvent( e );
  txtExpressionString->setFocus();
}

void QgsExpressionBuilderWidget::on_txtSearchEdit_textChanged()
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

void QgsExpressionBuilderWidget::on_txtSearchEditValues_textChanged()
{
  mProxyValues->setFilterCaseSensitivity( Qt::CaseInsensitive );
  mProxyValues->setFilterWildcard( txtSearchEditValues->text() );
}

void QgsExpressionBuilderWidget::on_lblPreview_linkActivated( const QString& link )
{
  Q_UNUSED( link );
  QgsMessageViewer * mv = new QgsMessageViewer( this );
  mv->setWindowTitle( tr( "More info on expression error" ) );
  mv->setMessageAsHtml( txtExpressionString->toolTip() );
  mv->exec();
}

void QgsExpressionBuilderWidget::on_mValuesListView_doubleClicked( const QModelIndex &index )
{
  // Insert the item text or replace selected text
  txtExpressionString->insertText( ' ' + index.data( Qt::DisplayRole ).toString() + ' ' );
  txtExpressionString->setFocus();
}

void QgsExpressionBuilderWidget::operatorButtonClicked()
{
  QPushButton* button = dynamic_cast<QPushButton*>( sender() );

  // Insert the button text or replace selected text
  txtExpressionString->insertText( ' ' + button->text() + ' ' );
  txtExpressionString->setFocus();
}

void QgsExpressionBuilderWidget::showContextMenu( QPoint pt )
{
  QModelIndex idx = expressionTree->indexAt( pt );
  idx = mProxyModel->mapToSource( idx );
  QgsExpressionItem* item = dynamic_cast<QgsExpressionItem*>( mModel->itemFromIndex( idx ) );
  if ( !item )
    return;

  if ( item->getItemType() == QgsExpressionItem::Field && mLayer )
  {
    QMenu* menu = new QMenu( this );
    menu->addAction( tr( "Load top 10 unique values" ), this, SLOT( loadSampleValues() ) );
    menu->addAction( tr( "Load all unique values" ), this, SLOT( loadAllValues() ) );
    menu->popup( expressionTree->mapToGlobal( pt ) );
  }
}

void QgsExpressionBuilderWidget::loadSampleValues()
{
  QModelIndex idx = mProxyModel->mapToSource( expressionTree->currentIndex() );
  QgsExpressionItem* item = dynamic_cast<QgsExpressionItem*>( mModel->itemFromIndex( idx ) );
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
  QgsExpressionItem* item = dynamic_cast<QgsExpressionItem*>( mModel->itemFromIndex( idx ) );
  // TODO We should really return a error the user of the widget that
  // the there is no layer set.
  if ( !mLayer || !item )
    return;

  mValueGroupBox->show();
  fillFieldValues( item->text(), -1 );
}

void QgsExpressionBuilderWidget::on_txtPython_textChanged()
{
  lblAutoSave->setText( "Saving..." );
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

  QListWidgetItem* item = cmbFileNames->currentItem();
  if ( !item )
    return;

  QString file = item->text();
  saveFunctionFile( file );
  lblAutoSave->setText( "Saved" );
  QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect();
  lblAutoSave->setGraphicsEffect( effect );
  QPropertyAnimation *anim = new QPropertyAnimation( effect, "opacity" );
  anim->setDuration( 2000 );
  anim->setStartValue( 1.0 );
  anim->setEndValue( 0.0 );
  anim->setEasingCurve( QEasingCurve::OutQuad );
  anim->start( QAbstractAnimation::DeleteWhenStopped );
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

QString QgsExpressionBuilderWidget::loadFunctionHelp( QgsExpressionItem* expressionItem )
{
  if ( !expressionItem )
    return "";

  QString helpContents = expressionItem->getHelpText();

  // Return the function help that is set for the function if there is one.
  if ( helpContents.isEmpty() )
  {
    QString name = expressionItem->data( Qt::UserRole ).toString();

    if ( expressionItem->getItemType() == QgsExpressionItem::Field )
      helpContents = QgsExpression::helptext( "Field" );
    else
      helpContents = QgsExpression::helptext( name );
  }

  return "<head><style>" + helpStylesheet() + "</style></head><body>" + helpContents + "</body>";
}





QgsExpressionItemSearchProxy::QgsExpressionItemSearchProxy()
{
  setFilterCaseSensitivity( Qt::CaseInsensitive );
}

bool QgsExpressionItemSearchProxy::filterAcceptsRow( int source_row, const QModelIndex& source_parent ) const
{
  QModelIndex index = sourceModel()->index( source_row, 0, source_parent );
  QgsExpressionItem::ItemType itemType = QgsExpressionItem::ItemType( sourceModel()->data( index, QgsExpressionItem::ItemTypeRole ).toInt() );

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

bool QgsExpressionItemSearchProxy::lessThan( const QModelIndex& left, const QModelIndex& right ) const
{
  int leftSort = sourceModel()->data( left, QgsExpressionItem::CustomSortRole ).toInt();
  int rightSort = sourceModel()->data( right,  QgsExpressionItem::CustomSortRole ).toInt();
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
