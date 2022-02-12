/***************************************************************************
                qgssqlcomposerdialog.cpp
       Dialog to compose SQL queries

begin                : Apr 2016
copyright            : (C) 2016 Even Rouault
email                : even.rouault at spatialys.com

 Adapted/ported from DBManager dlg_query_builder
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssqlcomposerdialog.h"
#include "qgssqlstatement.h"
#include "qgshelp.h"
#include "qgsvectorlayer.h"

#include <QMessageBox>
#include <QKeyEvent>

#include <Qsci/qscilexer.h>

QgsSQLComposerDialog::QgsSQLComposerDialog( QWidget *parent, Qt::WindowFlags fl )
  : QgsSQLComposerDialog( nullptr, parent, fl )
{}

QgsSQLComposerDialog::QgsSQLComposerDialog( QgsVectorLayer *layer, QWidget *parent, Qt::WindowFlags fl )
  : QgsSubsetStringEditorInterface( parent, fl )
  , mLayer( layer )
{
  setupUi( this );
  connect( mTablesCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsSQLComposerDialog::mTablesCombo_currentIndexChanged );
  connect( mColumnsCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsSQLComposerDialog::mColumnsCombo_currentIndexChanged );
  connect( mSpatialPredicatesCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsSQLComposerDialog::mSpatialPredicatesCombo_currentIndexChanged );
  connect( mFunctionsCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsSQLComposerDialog::mFunctionsCombo_currentIndexChanged );
  connect( mOperatorsCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsSQLComposerDialog::mOperatorsCombo_currentIndexChanged );
  connect( mAddJoinButton, &QPushButton::clicked, this, &QgsSQLComposerDialog::mAddJoinButton_clicked );
  connect( mRemoveJoinButton, &QPushButton::clicked, this, &QgsSQLComposerDialog::mRemoveJoinButton_clicked );
  connect( mTableJoins, &QTableWidget::itemSelectionChanged, this, &QgsSQLComposerDialog::mTableJoins_itemSelectionChanged );

  mQueryEdit->setWrapMode( QsciScintilla::WrapWord );
  mQueryEdit->installEventFilter( this );
  mColumnsEditor->installEventFilter( this );
  mTablesEditor->installEventFilter( this );
  mTableJoins->installEventFilter( this );
  mWhereEditor->installEventFilter( this );
  mOrderEditor->installEventFilter( this );
  mTablesCombo->view()->installEventFilter( this );


  connect( mButtonBox->button( QDialogButtonBox::Reset ), &QAbstractButton::clicked,
           this, &QgsSQLComposerDialog::reset );

  connect( mQueryEdit, &QsciScintilla::textChanged,
           this, &QgsSQLComposerDialog::splitSQLIntoFields );
  connect( mColumnsEditor, &QTextEdit::textChanged,
           this, &QgsSQLComposerDialog::buildSQLFromFields );
  connect( mTablesEditor, &QLineEdit::textChanged,
           this, &QgsSQLComposerDialog::buildSQLFromFields );
  connect( mWhereEditor, &QTextEdit::textChanged,
           this, &QgsSQLComposerDialog::buildSQLFromFields );
  connect( mOrderEditor, &QTextEdit::textChanged,
           this, &QgsSQLComposerDialog::buildSQLFromFields );
  connect( mTableJoins, &QTableWidget::cellChanged,
           this, &QgsSQLComposerDialog::buildSQLFromFields );
  connect( mButtonBox, &QDialogButtonBox::helpRequested,
           this, &QgsSQLComposerDialog::showHelp );

  QStringList baseList;
  baseList << QStringLiteral( "SELECT" );
  baseList << QStringLiteral( "FROM" );
  baseList << QStringLiteral( "JOIN" );
  baseList << QStringLiteral( "ON" );
  baseList << QStringLiteral( "USING" );
  baseList << QStringLiteral( "WHERE" );
  baseList << QStringLiteral( "AND" );
  baseList << QStringLiteral( "OR" );
  baseList << QStringLiteral( "NOT" );
  baseList << QStringLiteral( "IS" );
  baseList << QStringLiteral( "NULL" );
  baseList << QStringLiteral( "LIKE" );
  baseList << QStringLiteral( "ORDER" );
  baseList << QStringLiteral( "BY" );
  addApis( baseList );

  QStringList operatorsList;
  operatorsList << QStringLiteral( "AND" );
  operatorsList << QStringLiteral( "OR" );
  operatorsList << QStringLiteral( "NOT" );
  operatorsList << QStringLiteral( "=" );
  operatorsList << QStringLiteral( "<" );
  operatorsList << QStringLiteral( "<=" );
  operatorsList << QStringLiteral( ">" );
  operatorsList << QStringLiteral( ">=" );
  operatorsList << QStringLiteral( "<>" );
  operatorsList << QStringLiteral( "IS" );
  operatorsList << QStringLiteral( "IS NOT" );
  operatorsList << QStringLiteral( "IN" );
  operatorsList << QStringLiteral( "LIKE" );
  operatorsList << QStringLiteral( "BETWEEN" );
  addOperators( operatorsList );

  mAggregatesCombo->hide();
  mFunctionsCombo->hide();
  mSpatialPredicatesCombo->hide();
  mStringFunctionsCombo->hide();

  delete mPageColumnsValues;
  mPageColumnsValues = nullptr;

  mRemoveJoinButton->setEnabled( false );

  mTableJoins->setRowCount( 0 );
  mTableJoins->setItem( 0, 0, new QTableWidgetItem( QString() ) );
  mTableJoins->setItem( 0, 1, new QTableWidgetItem( QString() ) );
}

QgsSQLComposerDialog::~QgsSQLComposerDialog()
{
  // Besides avoid memory leaks, this is useful since QSciAPIs::prepare()
  // starts a thread. If the dialog was killed before the thread had started,
  // he could run against a dead widget. This can happen in unit tests.
  delete mQueryEdit->lexer()->apis();
  mQueryEdit->lexer()->setAPIs( nullptr );
}

bool QgsSQLComposerDialog::eventFilter( QObject *obj, QEvent *event )
{
  if ( event->type() == QEvent::FocusIn )
  {
    if ( obj == mTablesCombo->view() )
      lastSearchedText.clear();
    else
      mFocusedObject = obj;
  }

  // Custom search in table combobox
  if ( event->type() == QEvent::KeyPress && obj == mTablesCombo->view() )
  {
    QString currentString = ( ( QKeyEvent * )event )->text();
    if ( !currentString.isEmpty() && ( ( currentString[0] >= 'a' && currentString[0] <= 'z' ) ||
                                       ( currentString[0] >= 'A' && currentString[0] <= 'Z' ) ||
                                       ( currentString[0] >= '0' && currentString[0] <= '9' ) ||
                                       currentString[0] == ':' || currentString[0] == '_' || currentString[0] == ' ' ||
                                       currentString[0] == '(' || currentString[0] == ')' ) )
    {
      // First attempt is concatenation of existing search text
      // Second attempt is just the new character
      const int attemptCount = ( lastSearchedText.isEmpty() ) ? 1 : 2;
      for ( int attempt = 0; attempt < attemptCount; attempt ++ )
      {
        if ( attempt == 0 )
          lastSearchedText += currentString;
        else
          lastSearchedText = currentString;

        // Find the string that contains the searched text, and in case
        // of several matches, pickup the one where the searched text is the
        // most at the beginning.
        int iBestCandidate = 0;
        int idxInTextOfBestCandidate = 1000;
        for ( int i = 1; i < mTablesCombo->count(); i++ )
        {
          const int idxInText = mTablesCombo->itemText( i ).indexOf( lastSearchedText, Qt::CaseInsensitive );
          if ( idxInText >= 0 && idxInText < idxInTextOfBestCandidate )
          {
            iBestCandidate = i;
            idxInTextOfBestCandidate = idxInText;
          }
        }
        if ( iBestCandidate > 0 )
        {
          mTablesCombo->view()->setCurrentIndex( mTablesCombo->model()->index( 0, 0 ).sibling( iBestCandidate, 0 ) );
          return true;
        }
      }
      lastSearchedText.clear();
    }
  }

  return QDialog::eventFilter( obj, event );
}

void QgsSQLComposerDialog::setTableSelectedCallback( TableSelectedCallback *tableSelectedCallback )
{
  mTableSelectedCallback = tableSelectedCallback;
}

void QgsSQLComposerDialog::setSQLValidatorCallback( SQLValidatorCallback *sqlValidatorCallback )
{
  mSQLValidatorCallback = sqlValidatorCallback;
}

void QgsSQLComposerDialog::setSql( const QString &sql )
{
  mResetSql = sql;
  mQueryEdit->setText( sql );
}

QString QgsSQLComposerDialog::sql() const
{
  return mQueryEdit->text();
}

void QgsSQLComposerDialog::accept()
{
  if ( mSQLValidatorCallback )
  {
    QString errorMsg, warningMsg;
    if ( !mSQLValidatorCallback->isValid( sql(), errorMsg, warningMsg ) )
    {
      if ( errorMsg.isEmpty() )
        errorMsg = tr( "An error occurred during evaluation of the SQL statement." );
      QMessageBox::critical( this, tr( "SQL Evaluation" ), errorMsg );
      return;
    }
    if ( !warningMsg.isEmpty() )
    {
      QMessageBox::warning( this, tr( "SQL Evaluation" ), warningMsg );
    }
  }
  if ( mLayer )
  {
    mLayer->setSubsetString( sql() );
  }
  QDialog::accept();
}

void QgsSQLComposerDialog::buildSQLFromFields()
{
  if ( mAlreadyModifyingFields )
    return;
  mAlreadyModifyingFields = true;
  QString sql( QStringLiteral( "SELECT " ) );
  if ( mDistinct )
    sql += QLatin1String( "DISTINCT " );
  sql += mColumnsEditor->toPlainText();
  sql += QLatin1String( " FROM " );
  sql += mTablesEditor->text();

  const int rows = mTableJoins->rowCount();
  for ( int i = 0; i < rows; i++ )
  {
    QTableWidgetItem *itemTable = mTableJoins->item( i, 0 );
    QTableWidgetItem *itemOn = mTableJoins->item( i, 1 );
    if ( itemTable && !itemTable->text().isEmpty() &&
         itemOn && !itemOn->text().isEmpty() )
    {
      sql += QLatin1String( " JOIN " );
      sql += itemTable->text();
      sql += QLatin1String( " ON " );
      sql += itemOn->text();
    }
  }

  if ( !mWhereEditor->toPlainText().isEmpty() )
  {
    sql += QLatin1String( " WHERE " );
    sql += mWhereEditor->toPlainText();
  }
  if ( !mOrderEditor->toPlainText().isEmpty() )
  {
    sql += QLatin1String( " ORDER BY " );
    sql += mOrderEditor->toPlainText();
  }
  mQueryEdit->setText( sql );

  mAlreadyModifyingFields = false;
}

void QgsSQLComposerDialog::splitSQLIntoFields()
{
  if ( mAlreadyModifyingFields )
    return;
  const QgsSQLStatement sql( mQueryEdit->text() );
  if ( sql.hasParserError() )
    return;
  const QgsSQLStatement::NodeSelect *nodeSelect = dynamic_cast<const QgsSQLStatement::NodeSelect *>( sql.rootNode() );
  if ( !nodeSelect )
    return;
  mDistinct = nodeSelect->distinct();
  const QList<QgsSQLStatement::NodeSelectedColumn *> columns = nodeSelect->columns();
  QString columnText;
  const auto constColumns = columns;
  for ( QgsSQLStatement::NodeSelectedColumn *column : constColumns )
  {
    if ( !columnText.isEmpty() )
      columnText += QLatin1String( ", " );
    columnText += column->dump();
  }

  const QList<QgsSQLStatement::NodeTableDef *> tables = nodeSelect->tables();
  QString tablesText;
  const auto constTables = tables;
  for ( QgsSQLStatement::NodeTableDef *table : constTables )
  {
    if ( !tablesText.isEmpty() )
      tablesText += QLatin1String( ", " );
    loadTableColumns( QgsSQLStatement::quotedIdentifierIfNeeded( table->name() ) );
    tablesText += table->dump();
  }

  QString whereText;
  QgsSQLStatement::Node *where = nodeSelect->where();
  if ( where )
    whereText = where->dump();

  QString orderText;
  const QList<QgsSQLStatement::NodeColumnSorted *> orderColumns = nodeSelect->orderBy();
  const auto constOrderColumns = orderColumns;
  for ( QgsSQLStatement::NodeColumnSorted *column : constOrderColumns )
  {
    if ( !orderText.isEmpty() )
      orderText += QLatin1String( ", " );
    orderText += column->dump();
  }

  const QList<QgsSQLStatement::NodeJoin *> joins = nodeSelect->joins();

  mAlreadyModifyingFields = true;
  mColumnsEditor->setPlainText( columnText );
  mTablesEditor->setText( tablesText );
  mWhereEditor->setPlainText( whereText );
  mOrderEditor->setPlainText( orderText );

  mTableJoins->setRowCount( joins.size() + 1 );
  int iRow = 0;
  const auto constJoins = joins;
  for ( QgsSQLStatement::NodeJoin *join : constJoins )
  {
    loadTableColumns( QgsSQLStatement::quotedIdentifierIfNeeded( join->tableDef()->name() ) );
    mTableJoins->setItem( iRow, 0, new QTableWidgetItem( join->tableDef()->dump() ) );
    if ( join->onExpr() )
      mTableJoins->setItem( iRow, 1, new QTableWidgetItem( join->onExpr()->dump() ) );
    else
      mTableJoins->setItem( iRow, 1, new QTableWidgetItem( QString() ) );
    iRow ++;
  }
  mTableJoins->setItem( iRow, 0, new QTableWidgetItem( QString() ) );
  mTableJoins->setItem( iRow, 1, new QTableWidgetItem( QString() ) );

  mAlreadyModifyingFields = false;
}

void QgsSQLComposerDialog::addTableNames( const QStringList &list )
{
  const auto constList = list;
  for ( const QString &name : constList )
    mapTableEntryTextToName[name] = name;
  mTablesCombo->addItems( list );
  addApis( list );
}

void QgsSQLComposerDialog::addTableNames( const QList<PairNameTitle> &listNameTitle )
{
  QStringList listCombo;
  QStringList listApi;
  const auto constListNameTitle = listNameTitle;
  for ( const PairNameTitle &pair : constListNameTitle )
  {
    listApi << pair.first;
    QString entryText( pair.first );
    if ( !pair.second.isEmpty() && pair.second != pair.first )
    {
      if ( pair.second.size() < 40 )
        entryText += " (" + pair.second + ")";
      else
        entryText += " (" + pair.second.mid( 0, 20 ) + QChar( 0x2026 ) + pair.second.mid( pair.second.size() - 20 ) + ")";
    }
    listCombo << entryText;
    mapTableEntryTextToName[entryText] = pair.first;
  }
  mTablesCombo->addItems( listCombo );
  addApis( listApi );
}

void QgsSQLComposerDialog::addColumnNames( const QStringList &list, const QString &tableName )
{
  QList<PairNameType> listPair;
  const auto constList = list;
  for ( const QString &name : constList )
    listPair << PairNameType( name, QString() );
  addColumnNames( listPair, tableName );
}

static QString sanitizeType( QString type )
{
  if ( type.startsWith( QLatin1String( "xs:" ) ) )
    return type.mid( 3 );
  if ( type.startsWith( QLatin1String( "xsd:" ) ) )
    return type.mid( 4 );
  if ( type == QLatin1String( "gml:AbstractGeometryType" ) )
    return QStringLiteral( "geometry" );
  return type;
}

void QgsSQLComposerDialog::addColumnNames( const QList<PairNameType> &list, const QString &tableName )
{
  mAlreadySelectedTables.insert( tableName );
  if ( mColumnsCombo->count() > 1 )
    mColumnsCombo->insertSeparator( mColumnsCombo->count() );

  QStringList listCombo;
  QStringList listApi;
  const auto constList = list;
  for ( const PairNameType &pair : constList )
  {
    listApi << pair.first;
    QString entryText( pair.first );
    if ( !pair.second.isEmpty() )
    {
      entryText += " (" + sanitizeType( pair.second ) + ")";
    }
    listCombo << entryText;
    mapColumnEntryTextToName[entryText] = pair.first;
  }
  mColumnsCombo->addItems( listCombo );

  addApis( listApi );
}

void QgsSQLComposerDialog::addOperators( const QStringList &list )
{
  mOperatorsCombo->addItems( list );
  addApis( list );
}

static QString getFunctionAbbridgedParameters( const QgsSQLComposerDialog::Function &f )
{
  if ( f.minArgs >= 0 && f.maxArgs > f.minArgs )
  {
    return QObject::tr( "%1 to %2 arguments" ).arg( f.minArgs ).arg( f.maxArgs );
  }
  else if ( f.minArgs == 0 && f.maxArgs == 0 )
  {
  }
  else if ( f.minArgs > 0 && f.maxArgs == f.minArgs )
  {
    return QObject::tr( "%n argument(s)", nullptr, f.minArgs );
  }
  else if ( f.minArgs >= 0 && f.maxArgs < 0 )
  {
    return QObject::tr( "%n argument(s) or more", nullptr, f.minArgs );
  }
  return QString();
}


void QgsSQLComposerDialog::getFunctionList( const QList<Function> &list,
    QStringList &listApi,
    QStringList &listCombo,
    QMap<QString, QString> &mapEntryTextToName )
{
  const auto constList = list;
  for ( const Function &f : constList )
  {
    listApi << f.name;
    QString entryText( f.name );
    entryText += QLatin1Char( '(' );
    if ( !f.argumentList.isEmpty() )
    {
      for ( int i = 0; i < f.argumentList.size(); i++ )
      {
        if ( f.minArgs >= 0 && i >= f.minArgs ) entryText += QLatin1Char( '[' );
        if ( i > 0 ) entryText += QLatin1String( ", " );
        if ( f.argumentList[i].name == QLatin1String( "number" ) && !f.argumentList[i].type.isEmpty() )
        {
          entryText += sanitizeType( f.argumentList[i].type );
        }
        else
        {
          entryText += f.argumentList[i].name;
          const QString sanitizedType( sanitizeType( f.argumentList[i].type ) );
          if ( !f.argumentList[i].type.isEmpty() &&
               f.argumentList[i].name != sanitizedType )
          {
            entryText += QLatin1String( ": " );
            entryText += sanitizedType;
          }
        }
        if ( f.minArgs >= 0 && i >= f.minArgs ) entryText += QLatin1Char( ']' );
      }
      if ( entryText.size() > 60 )
      {
        entryText = f.name;
        entryText += QLatin1Char( '(' );
        entryText += getFunctionAbbridgedParameters( f );
      }
    }
    else
    {
      entryText += getFunctionAbbridgedParameters( f );
    }
    entryText += QLatin1Char( ')' );
    if ( !f.returnType.isEmpty() )
      entryText += ": " + sanitizeType( f.returnType );
    listCombo << entryText;
    mapEntryTextToName[entryText] = f.name + "(";
  }
}

void QgsSQLComposerDialog::addSpatialPredicates( const QStringList &list )
{
  QList<Function> listFunction;
  const auto constList = list;
  for ( const QString &name : constList )
  {
    Function f;
    f.name = name;
    listFunction << f;
  }
  addSpatialPredicates( listFunction );
}

void QgsSQLComposerDialog::addSpatialPredicates( const QList<Function> &list )
{
  QStringList listApi;
  QStringList listCombo;
  getFunctionList( list, listApi, listCombo, mapSpatialPredicateEntryTextToName );
  mSpatialPredicatesCombo->addItems( listCombo );
  mSpatialPredicatesCombo->show();
  addApis( listApi );
}

void QgsSQLComposerDialog::addFunctions( const QStringList &list )
{
  QList<Function> listFunction;
  const auto constList = list;
  for ( const QString &name : constList )
  {
    Function f;
    f.name = name;
    listFunction << f;
  }
  addFunctions( listFunction );
}

void QgsSQLComposerDialog::addFunctions( const QList<Function> &list )
{
  QStringList listApi;
  QStringList listCombo;
  getFunctionList( list, listApi, listCombo, mapFunctionEntryTextToName );
  mFunctionsCombo->addItems( listCombo );
  mFunctionsCombo->show();
  addApis( listApi );
}

void QgsSQLComposerDialog::loadTableColumns( const QString &table )
{
  if ( mTableSelectedCallback )
  {
    if ( !mAlreadySelectedTables.contains( table ) )
    {
      mTableSelectedCallback->tableSelected( table );
      mAlreadySelectedTables.insert( table );
    }
  }
}

static void resetCombo( QComboBox *combo )
{
  // We do it in a deferred way, otherwise Valgrind complains when using QTest
  // since basically this call a recursive call to QComboBox::setCurrentIndex()
  // which cause internal QComboBox logic to operate on a destroyed object
  // However that isn't reproduce in live session. Anyway this hack is safe
  // in all modes.
  QMetaObject::invokeMethod( combo, "setCurrentIndex", Qt::QueuedConnection, Q_ARG( int, 0 ) );
}

void QgsSQLComposerDialog::mTablesCombo_currentIndexChanged( int )
{
  const int index = mTablesCombo->currentIndex();
  if ( index <= 0 )
    return;
  QObject *obj = mFocusedObject;
  const QString newText = mapTableEntryTextToName[mTablesCombo->currentText()];
  loadTableColumns( newText );
  if ( obj == mTablesEditor )
  {
    const QString currentText = mTablesEditor->text();
    if ( currentText.isEmpty() )
      mTablesEditor->setText( newText );
    else
      mTablesEditor->setText( currentText + ", " + newText );
  }
  else if ( obj == mTableJoins )
  {
    if ( mTableJoins->selectedItems().size() == 1 )
    {
      mTableJoins->selectedItems().at( 0 )->setText( newText );
    }
  }
  else if ( obj == mWhereEditor )
  {
    mWhereEditor->insertPlainText( newText );
  }
  else if ( obj == mOrderEditor )
  {
    mOrderEditor->insertPlainText( newText );
  }
  else if ( obj == mQueryEdit )
  {
    mQueryEdit->insertText( newText );
  }
  resetCombo( mTablesCombo );
}

void QgsSQLComposerDialog::mColumnsCombo_currentIndexChanged( int )
{
  const int index = mColumnsCombo->currentIndex();
  if ( index <= 0 )
    return;
  QObject *obj = mFocusedObject;
  const QString newText = mapColumnEntryTextToName[mColumnsCombo->currentText()];
  if ( obj == mColumnsEditor )
  {
    const QString currentText = mColumnsEditor->toPlainText();
    if ( currentText.isEmpty() )
      mColumnsEditor->insertPlainText( newText );
    else
      mColumnsEditor->insertPlainText( ",\n" + newText );
  }
  else if ( obj == mTableJoins )
  {
    if ( mTableJoins->selectedItems().size() == 1 &&
         mTableJoins->selectedItems().at( 0 )->column() == 1 )
    {
      const QString currentText( mTableJoins->selectedItems().at( 0 )->text() );
      if ( !currentText.isEmpty() && !currentText.contains( QLatin1String( "=" ) ) )
        mTableJoins->selectedItems().at( 0 )->setText( currentText + " = " + newText );
      else
        mTableJoins->selectedItems().at( 0 )->setText( currentText + newText );
    }
  }
  else if ( obj == mWhereEditor )
  {
    mWhereEditor->insertPlainText( newText );
  }
  else if ( obj == mOrderEditor )
  {
    mOrderEditor->insertPlainText( newText );
  }
  else if ( obj == mQueryEdit )
  {
    mQueryEdit->insertText( newText );
  }
  resetCombo( mColumnsCombo );
}

void QgsSQLComposerDialog::mFunctionsCombo_currentIndexChanged( int )
{
  functionCurrentIndexChanged( mFunctionsCombo, mapFunctionEntryTextToName );
}

void QgsSQLComposerDialog::mSpatialPredicatesCombo_currentIndexChanged( int )
{
  functionCurrentIndexChanged( mSpatialPredicatesCombo, mapSpatialPredicateEntryTextToName );
}

void QgsSQLComposerDialog::functionCurrentIndexChanged( QComboBox *combo,
    const QMap<QString, QString> &mapEntryTextToName )
{
  const int index = combo->currentIndex();
  if ( index <= 0 )
    return;
  QObject *obj = mFocusedObject;
  const QString newText = mapEntryTextToName[combo->currentText()];
  if ( obj == mColumnsEditor )
  {
    mColumnsEditor->insertPlainText( newText );
  }
  else if ( obj == mWhereEditor )
  {
    mWhereEditor->insertPlainText( newText );
  }
  else if ( obj == mQueryEdit )
  {
    mQueryEdit->insertText( newText );
  }
  resetCombo( combo );
}

void QgsSQLComposerDialog::mOperatorsCombo_currentIndexChanged( int )
{
  const int index = mOperatorsCombo->currentIndex();
  if ( index <= 0 )
    return;
  QObject *obj = mFocusedObject;
  const QString newText = mOperatorsCombo->currentText();
  if ( obj == mColumnsEditor )
  {
    mColumnsEditor->insertPlainText( newText );
  }
  else if ( obj == mWhereEditor )
  {
    mWhereEditor->insertPlainText( newText );
  }
  else if ( obj == mTableJoins )
  {
    if ( mTableJoins->selectedItems().size() == 1 &&
         mTableJoins->selectedItems().at( 0 )->column() == 1 )
    {
      const QString currentText( mTableJoins->selectedItems().at( 0 )->text() );
      mTableJoins->selectedItems().at( 0 )->setText( currentText + newText );
    }
  }
  else if ( obj == mQueryEdit )
  {
    mQueryEdit->insertText( newText );
  }
  resetCombo( mOperatorsCombo );
}

void QgsSQLComposerDialog::mAddJoinButton_clicked()
{
  int insertRow = mTableJoins->currentRow();
  const int rowCount = mTableJoins->rowCount();
  if ( insertRow < 0 )
    insertRow = rowCount;
  mTableJoins->setRowCount( rowCount + 1 );
  for ( int row = rowCount ; row > insertRow + 1; row -- )
  {
    mTableJoins->setItem( row, 0, mTableJoins->takeItem( row - 1, 0 ) );
    mTableJoins->setItem( row, 1, mTableJoins->takeItem( row - 1, 1 ) );
  }
  mTableJoins->setItem( ( insertRow == rowCount ) ? insertRow : insertRow + 1, 0, new QTableWidgetItem( QString() ) );
  mTableJoins->setItem( ( insertRow == rowCount ) ? insertRow : insertRow + 1, 1, new QTableWidgetItem( QString() ) );
}

void QgsSQLComposerDialog::mRemoveJoinButton_clicked()
{
  int row = mTableJoins->currentRow();
  if ( row < 0 )
    return;
  const int rowCount = mTableJoins->rowCount();
  for ( ; row < rowCount - 1; row ++ )
  {
    mTableJoins->setItem( row, 0, mTableJoins->takeItem( row + 1, 0 ) );
    mTableJoins->setItem( row, 1, mTableJoins->takeItem( row + 1, 1 ) );
  }
  mTableJoins->setRowCount( rowCount - 1 );

  buildSQLFromFields();
}

void QgsSQLComposerDialog::reset()
{
  mQueryEdit->setText( mResetSql );
}

void QgsSQLComposerDialog::mTableJoins_itemSelectionChanged()
{
  mRemoveJoinButton->setEnabled( mTableJoins->selectedItems().size() == 1 );
}

void QgsSQLComposerDialog::addApis( const QStringList &list )
{
  mApiList += list;

  delete mQueryEdit->lexer()->apis();
  QsciAPIs *apis = new QsciAPIs( mQueryEdit->lexer() );

  const auto constMApiList = mApiList;
  for ( const QString &str : constMApiList )
  {
    apis->add( str );
  }

  apis->prepare();
  mQueryEdit->lexer()->setAPIs( apis );
}

void QgsSQLComposerDialog::setSupportMultipleTables( bool on, const QString &mainTypename )
{
  mJoinsLabels->setVisible( on );
  mTableJoins->setVisible( on );
  mAddJoinButton->setVisible( on );
  mRemoveJoinButton->setVisible( on );
  mTablesCombo->setVisible( on );

  QString mainTypenameFormatted;
  if ( !mainTypename.isEmpty() )
    mainTypenameFormatted = "(" + mainTypename + ")";
  mQueryEdit->setToolTip( tr( "This is the SQL query editor. The SQL statement can select data from several tables, \n"
                              "but it must compulsory include the main typename %1 in the selected tables, \n"
                              "and only the geometry column of the main typename can be used as the geometry column of the resulting layer." ).arg( mainTypenameFormatted ) );
}

void QgsSQLComposerDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_ogc/ogc_client_support.html#ogc-wfs" ) );
}
