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

#include <QSettings>
#include <QMenu>
#include <QFile>
#include <QTextStream>

QgsExpressionBuilderWidget::QgsExpressionBuilderWidget( QWidget *parent )
    : QWidget( parent )
{
  setupUi( this );

  mValueGroupBox->hide();
  // The open and save button are for future.
  btnOpen->hide();
  btnSave->hide();
  highlighter = new QgsExpressionHighlighter( txtExpressionString->document() );

  mModel = new QStandardItemModel( );
  mProxyModel = new QgsExpressionItemSearchProxy();
  mProxyModel->setSourceModel( mModel );
  expressionTree->setModel( mProxyModel );

  expressionTree->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( expressionTree, SIGNAL( customContextMenuRequested( const QPoint & ) ), this, SLOT( showContextMenu( const QPoint & ) ) );

  foreach( QPushButton* button, this->mOperatorsGroupBox->findChildren<QPushButton *>() )
  {
    connect( button, SIGNAL( pressed() ), this, SLOT( operatorButtonClicked() ) );
  }

  // TODO Can we move this stuff to QgsExpression, like the functions?
  registerItem( tr( "Operators" ), "+", " + " );
  registerItem( tr( "Operators" ), "-", " -" );
  registerItem( tr( "Operators" ), "*", " * " );
  registerItem( tr( "Operators" ), "/", " / " );
  registerItem( tr( "Operators" ), "%", " % " );
  registerItem( tr( "Operators" ), "^", " ^ " );
  registerItem( tr( "Operators" ), "=", " = " );
  registerItem( tr( "Operators" ), ">", " > " );
  registerItem( tr( "Operators" ), "<", " < " );
  registerItem( tr( "Operators" ), "<>", " <> " );
  registerItem( tr( "Operators" ), "<=", " <= " );
  registerItem( tr( "Operators" ), ">=", " >= " );
  registerItem( tr( "Operators" ), "||", " || ",
                QString( "<b>|| %1</b><br><i>%2</i><br><i>%3:</i>%4" )
                .arg( tr( "(String Concatenation)" ) )
                .arg( tr( "Joins two values together into a string" ) )
                .arg( tr( "Usage" ) )
                .arg( tr( "'Dia' || Diameter" ) ) );
  registerItem( tr( "Operators" ), "LIKE", " LIKE " );
  registerItem( tr( "Operators" ), "ILIKE", " ILIKE " );
  registerItem( tr( "Operators" ), "IS", " IS NOT " );
  registerItem( tr( "Operators" ), "OR", " OR " );
  registerItem( tr( "Operators" ), "AND", " AND " );
  registerItem( tr( "Operators" ), "NOT", " NOT " );


  // Load the functions from the QgsExpression class
  int count = QgsExpression::functionCount();
  for ( int i = 0; i < count; i++ )
  {
    QgsExpression::FunctionDef func = QgsExpression::BuiltinFunctions()[i];
    QString name = func.mName;
    if ( func.mParams >= 1 )
      name += "(";
    registerItem( func.mGroup, func.mName, " " + name + " " );
  };
}


QgsExpressionBuilderWidget::~QgsExpressionBuilderWidget()
{

}

void QgsExpressionBuilderWidget::setLayer( QgsVectorLayer *layer )
{
  mLayer = layer;
}

void QgsExpressionBuilderWidget::on_expressionTree_clicked( const QModelIndex &index )
{
  // Get the item
  QModelIndex idx = mProxyModel->mapToSource( index );
  QgsExpressionItem* item = dynamic_cast<QgsExpressionItem*>( mModel->itemFromIndex( idx ) );
  if ( item == 0 )
    return;

  // Loading field values are handled with a
  // right click so we just show the help.
  if ( item->getItemType() == QgsExpressionItem::Field )
  {
    txtHelpText->setHtml( tr( "Double click to add field name to expression string. <br> "
                              "Or right click to select loading value options then "
                              "double click an item in the value list to add it to the expression string." ) );
    txtHelpText->setToolTip( txtHelpText->toPlainText() );
  }
  else
  {
    // Show the help for the current item.
    mValueGroupBox->hide();
    mValueListWidget->clear();
    QString help = loadFunctionHelp( item );
    txtHelpText->setText( help );
    txtHelpText->setToolTip( txtHelpText->toPlainText() );
  }
}

void QgsExpressionBuilderWidget::on_expressionTree_doubleClicked( const QModelIndex &index )
{
  QModelIndex idx = mProxyModel->mapToSource( index );
  QgsExpressionItem* item = dynamic_cast<QgsExpressionItem*>( mModel->itemFromIndex( idx ) );
  if ( item == 0 )
    return;

  // Don't handle the double click it we are on a header node.
  if ( item->getItemType() == QgsExpressionItem::Header )
    return;

  // Insert the expression text.
  txtExpressionString->insertPlainText( item->getExpressionText() );
}

void QgsExpressionBuilderWidget::loadFieldNames()
{
  // TODO We should really return a error the user of the widget that
  // the there is no layer set.
  if ( !mLayer )
    return;

  const QgsFieldMap fieldMap = mLayer->pendingFields();
  loadFieldNames( fieldMap );
}

void QgsExpressionBuilderWidget::loadFieldNames( QgsFieldMap fields )
{
  if ( fields.isEmpty() )
    return;

  QStringList fieldNames;
  foreach( QgsField field, fields )
  {
    QString fieldName = field.name();
    fieldNames << fieldName;
    registerItem( tr( "Fields" ), fieldName, " \"" + fieldName + "\" ", "", QgsExpressionItem::Field );
  }
  highlighter->addFields( fieldNames );
}

void QgsExpressionBuilderWidget::fillFieldValues( int fieldIndex, int countLimit )
{
  // TODO We should really return a error the user of the widget that
  // the there is no layer set.
  if ( !mLayer )
    return;

  // TODO We should thread this so that we don't hold the user up if the layer is massive.
  mValueListWidget->clear();
  mValueListWidget->setUpdatesEnabled( false );
  mValueListWidget->blockSignals( true );

  QList<QVariant> values;
  mLayer->uniqueValues( fieldIndex, values, countLimit );
  foreach( QVariant value, values )
  {
    mValueListWidget->addItem( value.toString() );
  }

  mValueListWidget->setUpdatesEnabled( true );
  mValueListWidget->blockSignals( false );
}

void QgsExpressionBuilderWidget::registerItem( QString group,
    QString label,
    QString expressionText,
    QString helpText,
    QgsExpressionItem::ItemType type )
{
  QgsExpressionItem* item = new QgsExpressionItem( label, expressionText, helpText, type );
  // Look up the group and insert the new function.
  if ( mExpressionGroups.contains( group ) )
  {
    QgsExpressionItem* groupNode = mExpressionGroups.value( group );
    groupNode->appendRow( item );
  }
  else
  {
    // If the group doesn't exsit yet we make it first.
    QgsExpressionItem* newgroupNode = new QgsExpressionItem( group, "", QgsExpressionItem::Header );
    newgroupNode->appendRow( item );
    mModel->appendRow( newgroupNode );
    mExpressionGroups.insert( group , newgroupNode );
  }
}

QString QgsExpressionBuilderWidget::getExpressionString()
{
  return txtExpressionString->toPlainText();
}

void QgsExpressionBuilderWidget::setExpressionString( const QString expressionString )
{
  txtExpressionString->setPlainText( expressionString );
}

void QgsExpressionBuilderWidget::on_txtExpressionString_textChanged()
{
  QString text = txtExpressionString->toPlainText();

  // If the string is empty the expression will still "fail" although
  // we don't show the user an error as it will be confusing.
  if ( text.isEmpty() )
  {
    lblPreview->setText( "" );
    lblPreview->setStyleSheet( "" );
    txtExpressionString->setToolTip( "" );
    lblPreview->setToolTip( "" );
    // Return false for isValid because a null expression is still invalid.
    emit expressionParsed( false );
    return;
  }

  QgsExpression exp( text );

  // TODO We could do this without a layer.
  // Maybe just calling exp.evaluate()?
  if ( mLayer )
  {
    if ( !mFeature.isValid() )
    {
      mLayer->select( mLayer->pendingAllAttributesList() );
      mLayer->nextFeature( mFeature );
    }

    if ( mFeature.isValid() )
    {
      QVariant value = exp.evaluate( &mFeature, mLayer->pendingFields() );
      if ( !exp.hasEvalError() )
        lblPreview->setText( value.toString() );
    }
    else
    {
      // The feature is invalid because we don't have one but that doesn't mean user can't
      // build a expression string.  They just get no preview.
      lblPreview->setText( "" );
    }
  }

  if ( exp.hasParserError() || exp.hasEvalError() )
  {
    QString tooltip = QString( "<b>%1:</b><br>%2" ).arg( tr( "Parser Error" ) ).arg( exp.parserErrorString() );
    if ( exp.hasEvalError() )
      tooltip += QString( "<br><br><b>%1:</b><br>%2" ).arg( tr( "Eval Error" ) ).arg( exp.evalErrorString() );

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

void QgsExpressionBuilderWidget::on_txtSearchEdit_textChanged()
{
  mProxyModel->setFilterWildcard( txtSearchEdit->text() );
  if ( txtSearchEdit->text().isEmpty() )
    expressionTree->collapseAll();
  else
    expressionTree->expandAll();
}

void QgsExpressionBuilderWidget::on_lblPreview_linkActivated( QString link )
{
  Q_UNUSED( link );
  QgsMessageViewer * mv = new QgsMessageViewer( this );
  mv->setWindowTitle( tr( "More info on expression error" ) );
  mv->setMessageAsHtml( txtExpressionString->toolTip() );
  mv->exec();
}

void QgsExpressionBuilderWidget::on_mValueListWidget_itemDoubleClicked( QListWidgetItem *item )
{
  txtExpressionString->insertPlainText( " " + item->text() + " " );
}

void QgsExpressionBuilderWidget::operatorButtonClicked()
{
  QPushButton* button = dynamic_cast<QPushButton*>( sender() );
  txtExpressionString->insertPlainText( " " + button->text() + " " );
}

void QgsExpressionBuilderWidget::showContextMenu( const QPoint & pt )
{
  QModelIndex idx = expressionTree->indexAt( pt );
  idx = mProxyModel->mapToSource( idx );
  QgsExpressionItem* item = dynamic_cast<QgsExpressionItem*>( mModel->itemFromIndex( idx ) );
  if ( !item )
    return;

  if ( item->getItemType() == QgsExpressionItem::Field )
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
  if ( !mLayer )
    return;

  mValueGroupBox->show();
  int fieldIndex = mLayer->fieldNameIndex( item->text() );
  fillFieldValues( fieldIndex, 10 );
}

void QgsExpressionBuilderWidget::loadAllValues()
{
  QModelIndex idx = mProxyModel->mapToSource( expressionTree->currentIndex() );
  QgsExpressionItem* item = dynamic_cast<QgsExpressionItem*>( mModel->itemFromIndex( idx ) );
  // TODO We should really return a error the user of the widget that
  // the there is no layer set.
  if ( !mLayer )
    return;

  mValueGroupBox->show();
  int fieldIndex = mLayer->fieldNameIndex( item->text() );
  fillFieldValues( fieldIndex, -1 );
}

QString QgsExpressionBuilderWidget::loadFunctionHelp( QgsExpressionItem* functionName )
{
  if ( functionName == NULL )
    return "";

  // set up the path to the help file
  QString helpFilesPath = QgsApplication::pkgDataPath() + "/resources/function_help/";
  /*
   * determine the locale and create the file name from
   * the context id
   */
  QString lang = QLocale::system().name();

  QSettings settings;
  if ( settings.value( "locale/overrideFlag", false ).toBool() )
  {
    QLocale l( settings.value( "locale/userLocale", "en_US" ).toString() );
    lang = l.name();
  }
  /*
   * If the language isn't set on the system, assume en_US,
   * otherwise we get the banner at the top of the help file
   * saying it isn't available in "your" language. Some systems
   * may be installed without the LANG environment being set.
   */
  if ( lang.length() == 0 || lang == "C" )
  {
    lang = "en_US";
  }
  QString fullHelpPath = helpFilesPath + functionName->text() + "-" + lang;
  // get the help content and title from the localized file
  QString helpContents;
  QFile file( fullHelpPath );
  // check to see if the localized version exists
  if ( !file.exists() )
  {
    // change the file name to the en_US version (default)
    fullHelpPath = helpFilesPath + functionName->text() + "-en_US";
    file.setFileName( fullHelpPath );

    // Check for some sort of english locale and if not found, include
    // translate this for us message
    if ( !lang.contains( "en_" ) )
    {
      helpContents = "<i>" + tr( "This help file is not available in your language %1. If you would like to translate it, please contact the QGIS development team." ).arg( lang ) + "</i><hr />";
    }

  }
  if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    helpContents = tr( "This help file does not exist for your language:<p><b>%1</b><p>If you would like to create it, contact the QGIS development team" )
                   .arg( fullHelpPath );
  }
  else
  {
    QTextStream in( &file );
    in.setCodec( "UTF-8" ); // Help files must be in Utf-8
    while ( !in.atEnd() )
    {
      QString line = in.readLine();
      helpContents += line;
    }
  }
  file.close();

  // Set the browser text to the help contents
  QString myStyle = QgsApplication::reportStyleSheet();
  helpContents = "<head><style>" + myStyle + "</style></head><body>" + helpContents + "</body>";
  return helpContents;
}
