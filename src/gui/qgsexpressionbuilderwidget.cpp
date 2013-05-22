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
  btnLoadAll->hide();
  btnLoadSample->hide();
  highlighter = new QgsExpressionHighlighter( txtExpressionString->document() );

  mModel = new QStandardItemModel( );
  mProxyModel = new QgsExpressionItemSearchProxy();
  mProxyModel->setSourceModel( mModel );
  expressionTree->setModel( mProxyModel );

  expressionTree->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( this, SIGNAL( expressionParsed( bool ) ), this, SLOT( setExpressionState( bool ) ) );
  connect( expressionTree, SIGNAL( customContextMenuRequested( const QPoint & ) ), this, SLOT( showContextMenu( const QPoint & ) ) );
  connect( expressionTree->selectionModel(), SIGNAL( currentChanged( const QModelIndex &, const QModelIndex & ) ),
           this, SLOT( currentChanged( const QModelIndex &, const QModelIndex & ) ) );

  connect( btnLoadAll, SIGNAL( pressed() ), this, SLOT( loadAllValues() ) );
  connect( btnLoadSample, SIGNAL( pressed() ), this, SLOT( loadSampleValues() ) );

  foreach ( QPushButton* button, mOperatorsGroupBox->findChildren<QPushButton *>() )
  {
    connect( button, SIGNAL( pressed() ), this, SLOT( operatorButtonClicked() ) );
  }

  // TODO Can we move this stuff to QgsExpression, like the functions?
  registerItem( "Operators", "+", " + ", tr( "Addition operator" ) );
  registerItem( "Operators", "-", " -" , tr( "Subtraction operator" ) );
  registerItem( "Operators", "*", " * ", tr( "Multiplication operator" ) );
  registerItem( "Operators", "/", " / ", tr( "Division operator" ) );
  registerItem( "Operators", "%", " % ", tr( "Modulo operator"  ) );
  registerItem( "Operators", "^", " ^ ", tr( "Power operator" ) );
  registerItem( "Operators", "=", " = ", tr( "Equal operator"  ) );
  registerItem( "Operators", ">", " > ", tr( "Greater as operator" ) );
  registerItem( "Operators", "<", " < ", tr( "Less than operator" ) );
  registerItem( "Operators", "<>", " <> ", tr( "Unequal operator" ) );
  registerItem( "Operators", "<=", " <= ", tr( "Less or equal operator" ) );
  registerItem( "Operators", ">=", " >= ", tr( "Greater or equal operator" ) );
  registerItem( "Operators", "||", " || ",
                QString( "<b>|| %1</b><br><i>%2</i><br><i>%3:</i>%4" )
                .arg( tr( "(String Concatenation)" ) )
                .arg( tr( "Joins two values together into a string" ) )
                .arg( tr( "Usage" ) )
                .arg( tr( "'Dia' || Diameter" ) ) );
  registerItem( "Operators", "LIKE", " LIKE " );
  registerItem( "Operators", "ILIKE", " ILIKE " );
  registerItem( "Operators", "IS", " IS " );
  registerItem( "Operators", "OR", " OR " );
  registerItem( "Operators", "AND", " AND " );
  registerItem( "Operators", "NOT", " NOT " );

  QString casestring = "CASE WHEN condition THEN result END";
  QString caseelsestring = "CASE WHEN condition THEN result ELSE result END";
  registerItem( "Conditionals", "CASE", casestring );
  registerItem( "Conditionals", "CASE ELSE", caseelsestring );

  // Load the functions from the QgsExpression class
  int count = QgsExpression::functionCount();
  for ( int i = 0; i < count; i++ )
  {
    QgsExpression::Function* func = QgsExpression::Functions()[i];
    QString name = func->name();
    if ( name.startsWith( "_" ) ) // do not display private functions
      continue;
    if ( func->params() >= 1 )
      name += "(";
    registerItem( func->group(), func->name(), " " + name + " ", func->helptext() );
  }

  QList<QgsExpression::Function*> specials = QgsExpression::specialColumns();
  for ( int i = 0; i < specials.size(); ++i )
  {
    QString name = specials[i]->name();
    registerItem( specials[i]->group(), name, " " + name + " " );
  }

#if QT_VERSION >= 0x040700
  txtSearchEdit->setPlaceholderText( tr( "Search" ) );
#endif
}


QgsExpressionBuilderWidget::~QgsExpressionBuilderWidget()
{

}

void QgsExpressionBuilderWidget::setLayer( QgsVectorLayer *layer )
{
  mLayer = layer;
}

void QgsExpressionBuilderWidget::currentChanged( const QModelIndex &index, const QModelIndex & )
{
  // Get the item
  QModelIndex idx = mProxyModel->mapToSource( index );
  QgsExpressionItem* item = dynamic_cast<QgsExpressionItem*>( mModel->itemFromIndex( idx ) );
  if ( item == 0 )
    return;

  if ( item->getItemType() != QgsExpressionItem::Field )
  {
    mValueListWidget->clear();
  }

  btnLoadAll->setVisible( item->getItemType() == QgsExpressionItem::Field );
  btnLoadSample->setVisible( item->getItemType() == QgsExpressionItem::Field );
  mValueGroupBox->setVisible( item->getItemType() == QgsExpressionItem::Field );

  // Show the help for the current item.
  QString help = loadFunctionHelp( item );
  txtHelpText->setText( help );
  txtHelpText->setToolTip( txtHelpText->toPlainText() );
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

  loadFieldNames( mLayer->pendingFields() );
}

void QgsExpressionBuilderWidget::loadFieldNames( const QgsFields& fields )
{
  if ( fields.isEmpty() )
    return;

  QStringList fieldNames;
  //foreach ( const QgsField& field, fields )
  for ( int i = 0; i < fields.count(); ++i )
  {
    QString fieldName = fields[i].name();
    fieldNames << fieldName;
    registerItem( "Fields and Values", fieldName, " \"" + fieldName + "\" ", "", QgsExpressionItem::Field );
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
  foreach ( QVariant value, values )
  {
    if ( value.isNull() )
      mValueListWidget->addItem( "NULL" );
    else if ( value.type() == QVariant::Int || value.type() == QVariant::Double || value.type() == QVariant::LongLong )
      mValueListWidget->addItem( value.toString() );
    else
      mValueListWidget->addItem( "'" + value.toString().replace( "'", "''" ) + "'" );
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
  item->setData( label, Qt::UserRole );
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
    newgroupNode->appendRow( item );
    mModel->appendRow( newgroupNode );
    mExpressionGroups.insert( group, newgroupNode );
  }
}

bool QgsExpressionBuilderWidget::isExpressionValid()
{
  return mExpressionValid;
}

void QgsExpressionBuilderWidget::setGeomCalculator( const QgsDistanceArea & da )
{
  mDa = da;
}

QString QgsExpressionBuilderWidget::expressionText()
{
  return txtExpressionString->toPlainText();
}

void QgsExpressionBuilderWidget::setExpressionText( const QString& expression )
{
  txtExpressionString->setPlainText( expression );
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
      mLayer->getFeatures( QgsFeatureRequest().setFlags(( mLayer->geometryType() != QGis::NoGeometry && exp.needsGeometry() ) ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) ).nextFeature( mFeature );
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
  else
  {
    // No layer defined
    QVariant value = exp.evaluate();
    if ( !exp.hasEvalError() )
    {
      lblPreview->setText( value.toString() );
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

void QgsExpressionBuilderWidget::setExpressionState( bool state )
{
  mExpressionValid = state;
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

  QString myStyle = QgsApplication::reportStyleSheet();
  return "<head><style>" + myStyle + "</style></head><body>" + helpContents + "</body>";
}
