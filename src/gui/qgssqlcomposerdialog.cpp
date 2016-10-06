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

#include <QMessageBox>
#include <QKeyEvent>

#include <Qsci/qscilexer.h>

QgsSQLComposerDialog::TableSelectedCallback::~TableSelectedCallback()
{
}

QgsSQLComposerDialog::SQLValidatorCallback::~SQLValidatorCallback()
{
}

QgsSQLComposerDialog::QgsSQLComposerDialog( QWidget * parent, Qt::WindowFlags fl )
    : QDialog( parent, fl )
    , mTableSelectedCallback( nullptr )
    , mSQLValidatorCallback( nullptr )
    , mFocusedObject( nullptr )
    , mAlreadyModifyingFields( false )
    , mDistinct( false )
{
  setupUi( this );

  mQueryEdit->setWrapMode( QsciScintilla::WrapWord );
  mQueryEdit->installEventFilter( this );
  mColumnsEditor->installEventFilter( this );
  mTablesEditor->installEventFilter( this );
  mTableJoins->installEventFilter( this );
  mWhereEditor->installEventFilter( this );
  mOrderEditor->installEventFilter( this );
  mTablesCombo->view()->installEventFilter( this );


  connect( mButtonBox->button( QDialogButtonBox::Reset ), SIGNAL( clicked() ),
           this, SLOT( reset() ) );

  connect( mQueryEdit, SIGNAL( textChanged() ),
           this, SLOT( splitSQLIntoFields() ) );
  connect( mColumnsEditor, SIGNAL( textChanged() ),
           this, SLOT( buildSQLFromFields() ) );
  connect( mTablesEditor, SIGNAL( textChanged( const QString & ) ),
           this, SLOT( buildSQLFromFields() ) );
  connect( mWhereEditor, SIGNAL( textChanged() ),
           this, SLOT( buildSQLFromFields() ) );
  connect( mOrderEditor, SIGNAL( textChanged() ),
           this, SLOT( buildSQLFromFields() ) );
  connect( mTableJoins, SIGNAL( cellChanged( int, int ) ),
           this, SLOT( buildSQLFromFields() ) );

  QStringList baseList;
  baseList << "SELECT";
  baseList << "FROM";
  baseList << "JOIN";
  baseList << "ON";
  baseList << "USING";
  baseList << "WHERE";
  baseList << "AND";
  baseList << "OR";
  baseList << "NOT";
  baseList << "IS";
  baseList << "NULL";
  baseList << "LIKE";
  baseList << "ORDER";
  baseList << "BY";
  addApis( baseList );

  QStringList operatorsList;
  operatorsList << "AND";
  operatorsList << "OR";
  operatorsList << "NOT";
  operatorsList << "=";
  operatorsList << "<";
  operatorsList << "<=";
  operatorsList << ">";
  operatorsList << ">=";
  operatorsList << "<>";
  operatorsList << "IS";
  operatorsList << "IS NOT";
  operatorsList << "IN";
  operatorsList << "LIKE";
  operatorsList << "BETWEEN";
  addOperators( operatorsList );

  mAggregatesCombo->hide();
  mFunctionsCombo->hide();
  mSpatialPredicatesCombo->hide();
  mStringFunctionsCombo->hide();

  delete mPageColumnsValues;
  mPageColumnsValues = nullptr;

  mRemoveJoinButton->setEnabled( false );

  mTableJoins->setRowCount( 0 );
  mTableJoins->setItem( 0, 0, new QTableWidgetItem( "" ) );
  mTableJoins->setItem( 0, 1, new QTableWidgetItem( "" ) );
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
    QString currentString = (( QKeyEvent* )event )->text();
    if ( !currentString.isEmpty() && (( currentString[0] >= 'a' && currentString[0] <= 'z' ) ||
                                      ( currentString[0] >= 'A' && currentString[0] <= 'Z' ) ||
                                      ( currentString[0] >= '0' && currentString[0] <= '9' ) ||
                                      currentString[0] == ':' || currentString[0] == '_' || currentString[0] == ' ' ||
                                      currentString[0] == '(' || currentString[0] == ')' ) )
    {
      // First attempt is concatenation of existing search text
      // Second attempt is just the new character
      int attemptCount = ( lastSearchedText.isEmpty() ) ? 1 : 2;
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
          int idxInText = mTablesCombo->itemText( i ).indexOf( lastSearchedText, Qt::CaseInsensitive );
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

  return QObject::eventFilter( obj, event );
}

void QgsSQLComposerDialog::setTableSelectedCallback( TableSelectedCallback* tableSelectedCallback )
{
  mTableSelectedCallback = tableSelectedCallback;
}

void QgsSQLComposerDialog::setSQLValidatorCallback( SQLValidatorCallback* sqlValidatorCallback )
{
  mSQLValidatorCallback = sqlValidatorCallback;
}

void QgsSQLComposerDialog::setSql( const QString& sql )
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
        errorMsg = tr( "An error occurred during evaluation of the SQL statement" );
      QMessageBox::critical( this, tr( "SQL error" ), errorMsg );
      return;
    }
    if ( !warningMsg.isEmpty() )
    {
      QMessageBox::warning( this, tr( "SQL warning" ), warningMsg );
    }
  }
  QDialog::accept();
}

void QgsSQLComposerDialog::buildSQLFromFields()
{
  if ( mAlreadyModifyingFields )
    return;
  mAlreadyModifyingFields = true;
  QString sql( "SELECT " );
  if ( mDistinct )
    sql += "DISTINCT ";
  sql += mColumnsEditor->toPlainText();
  sql += " FROM ";
  sql += mTablesEditor->text();

  int rows = mTableJoins->rowCount();
  for ( int i = 0;i < rows;i++ )
  {
    QTableWidgetItem * itemTable = mTableJoins->item( i, 0 );
    QTableWidgetItem * itemOn = mTableJoins->item( i, 1 );
    if ( itemTable && !itemTable->text().isEmpty() &&
         itemOn && !itemOn->text().isEmpty() )
    {
      sql += " JOIN ";
      sql += itemTable->text();
      sql += " ON ";
      sql += itemOn->text();
    }
  }

  if ( !mWhereEditor->toPlainText().isEmpty() )
  {
    sql += " WHERE ";
    sql += mWhereEditor->toPlainText();
  }
  if ( !mOrderEditor->toPlainText().isEmpty() )
  {
    sql += " ORDER BY ";
    sql += mOrderEditor->toPlainText();
  }
  mQueryEdit->setText( sql );

  mAlreadyModifyingFields = false;
}

void QgsSQLComposerDialog::splitSQLIntoFields()
{
  if ( mAlreadyModifyingFields )
    return;
  QgsSQLStatement sql( mQueryEdit->text() );
  if ( sql.hasParserError() )
    return;
  const QgsSQLStatement::NodeSelect* nodeSelect = dynamic_cast<const QgsSQLStatement::NodeSelect*>( sql.rootNode() );
  if ( nodeSelect == nullptr )
    return;
  mDistinct = nodeSelect->distinct();
  QList<QgsSQLStatement::NodeSelectedColumn*> columns = nodeSelect->columns();
  QString columnText;
  Q_FOREACH ( QgsSQLStatement::NodeSelectedColumn* column, columns )
  {
    if ( !columnText.isEmpty() )
      columnText += ", ";
    columnText += column->dump();
  }

  QList<QgsSQLStatement::NodeTableDef*> tables = nodeSelect->tables();
  QString tablesText;
  Q_FOREACH ( QgsSQLStatement::NodeTableDef* table, tables )
  {
    if ( !tablesText.isEmpty() )
      tablesText += ", ";
    loadTableColumns( QgsSQLStatement::quotedIdentifierIfNeeded( table->name() ) );
    tablesText += table->dump();
  }

  QString whereText;
  QgsSQLStatement::Node* where = nodeSelect->where();
  if ( where != nullptr )
    whereText = where->dump();

  QString orderText;
  QList<QgsSQLStatement::NodeColumnSorted*> orderColumns = nodeSelect->orderBy();
  Q_FOREACH ( QgsSQLStatement::NodeColumnSorted* column, orderColumns )
  {
    if ( !orderText.isEmpty() )
      orderText += ", ";
    orderText += column->dump();
  }

  QList<QgsSQLStatement::NodeJoin*> joins = nodeSelect->joins();

  mAlreadyModifyingFields = true;
  mColumnsEditor->setPlainText( columnText );
  mTablesEditor->setText( tablesText );
  mWhereEditor->setPlainText( whereText );
  mOrderEditor->setPlainText( orderText );

  mTableJoins->setRowCount( joins.size() + 1 );
  int iRow = 0;
  Q_FOREACH ( QgsSQLStatement::NodeJoin* join, joins )
  {
    loadTableColumns( QgsSQLStatement::quotedIdentifierIfNeeded( join->tableDef()->name() ) );
    mTableJoins->setItem( iRow, 0 , new QTableWidgetItem( join->tableDef()->dump() ) );
    if ( join->onExpr() )
      mTableJoins->setItem( iRow, 1 , new QTableWidgetItem( join->onExpr()->dump() ) );
    else
      mTableJoins->setItem( iRow, 1 , new QTableWidgetItem( "" ) );
    iRow ++;
  }
  mTableJoins->setItem( iRow, 0, new QTableWidgetItem( "" ) );
  mTableJoins->setItem( iRow, 1, new QTableWidgetItem( "" ) );

  mAlreadyModifyingFields = false;
}

void QgsSQLComposerDialog::addTableNames( const QStringList& list )
{
  Q_FOREACH ( const QString& name, list )
    mapTableEntryTextToName[name] = name;
  mTablesCombo->addItems( list );
  addApis( list );
}

void QgsSQLComposerDialog::addTableNames( const QList<PairNameTitle> & listNameTitle )
{
  QStringList listCombo;
  QStringList listApi;
  Q_FOREACH ( const PairNameTitle& pair, listNameTitle )
  {
    listApi << pair.first;
    QString entryText( pair.first );
    if ( !pair.second.isEmpty() && pair.second != pair.first )
    {
      if ( pair.second.size() < 40 )
        entryText += " (" + pair.second + ")";
      else
        entryText += " (" + pair.second.mid( 0, 20 ) + "..." + pair.second.mid( pair.second.size() - 20 ) + ")";
    }
    listCombo << entryText;
    mapTableEntryTextToName[entryText] = pair.first;
  }
  mTablesCombo->addItems( listCombo );
  addApis( listApi );
}

void QgsSQLComposerDialog::addColumnNames( const QStringList& list, const QString& tableName )
{
  QList<PairNameType> listPair;
  Q_FOREACH ( const QString& name, list )
    listPair << PairNameType( name, QString() );
  addColumnNames( listPair, tableName );
}

static QString sanitizeType( QString type )
{
  if ( type.startsWith( "xs:" ) )
    return type.mid( 3 );
  if ( type.startsWith( "xsd:" ) )
    return type.mid( 4 );
  if ( type == "gml:AbstractGeometryType" )
    return "geometry";
  return type;
}

void QgsSQLComposerDialog::addColumnNames( const QList<PairNameType>& list, const QString& tableName )
{
  mAlreadySelectedTables.insert( tableName );
  if ( mColumnsCombo->count() > 1 )
    mColumnsCombo->insertSeparator( mColumnsCombo->count() );

  QStringList listCombo;
  QStringList listApi;
  Q_FOREACH ( const PairNameType& pair, list )
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

void QgsSQLComposerDialog::addOperators( const QStringList& list )
{
  mOperatorsCombo->addItems( list );
  addApis( list );
}

static QString getFunctionAbbridgedParameters( const QgsSQLComposerDialog::Function& f )
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
    if ( f.minArgs == 1 )
      return QObject::tr( "1 argument" );
    else
      return QObject::tr( "%1 arguments" ).arg( f.minArgs );
  }
  else if ( f.minArgs >= 0 && f.maxArgs < 0 )
  {
    if ( f.minArgs > 1 )
      return QObject::tr( "%1 arguments or more" ).arg( f.minArgs );
    else if ( f.minArgs == 1 )
      return QObject::tr( "1 argument or more" );
    else
      return QObject::tr( "0 argument or more" );
  }
  return QString();
}


void QgsSQLComposerDialog::getFunctionList( const QList<Function>& list,
    QStringList& listApi,
    QStringList& listCombo,
    QMap<QString, QString>& mapEntryTextToName )
{
  Q_FOREACH ( const Function& f, list )
  {
    listApi << f.name;
    QString entryText( f.name );
    entryText += "(";
    if ( f.argumentList.size() )
    {
      for ( int i = 0;i < f.argumentList.size();i++ )
      {
        if ( f.minArgs >= 0 && i >= f.minArgs ) entryText += "[";
        if ( i > 0 ) entryText += ", ";
        if ( f.argumentList[i].name == "number" && !f.argumentList[i].type.isEmpty() )
        {
          entryText += sanitizeType( f.argumentList[i].type );
        }
        else
        {
          entryText += f.argumentList[i].name;
          QString sanitizedType( sanitizeType( f.argumentList[i].type ) );
          if ( !f.argumentList[i].type.isEmpty() &&
               f.argumentList[i].name != sanitizedType )
          {
            entryText += ": ";
            entryText += sanitizedType;
          }
        }
        if ( f.minArgs >= 0 && i >= f.minArgs ) entryText += "]";
      }
      if ( entryText.size() > 60 )
      {
        entryText = f.name ;
        entryText += "(";
        entryText += getFunctionAbbridgedParameters( f );
      }
    }
    else
    {
      entryText += getFunctionAbbridgedParameters( f );
    }
    entryText += ")";
    if ( !f.returnType.isEmpty() )
      entryText += ": " + sanitizeType( f.returnType );
    listCombo << entryText;
    mapEntryTextToName[entryText] = f.name + "(";
  }
}

void QgsSQLComposerDialog::addSpatialPredicates( const QStringList& list )
{
  QList<Function> listFunction;
  Q_FOREACH ( const QString& name, list )
  {
    Function f;
    f.name = name;
    listFunction << f;
  }
  addSpatialPredicates( listFunction );
}

void QgsSQLComposerDialog::addSpatialPredicates( const QList<Function>& list )
{
  QStringList listApi;
  QStringList listCombo;
  getFunctionList( list, listApi, listCombo, mapSpatialPredicateEntryTextToName );
  mSpatialPredicatesCombo->addItems( listCombo );
  mSpatialPredicatesCombo->show();
  addApis( listApi );
}

void QgsSQLComposerDialog::addFunctions( const QStringList& list )
{
  QList<Function> listFunction;
  Q_FOREACH ( const QString& name, list )
  {
    Function f;
    f.name = name;
    listFunction << f;
  }
  addFunctions( listFunction );
}

void QgsSQLComposerDialog::addFunctions( const QList<Function>& list )
{
  QStringList listApi;
  QStringList listCombo;
  getFunctionList( list, listApi, listCombo, mapFunctionEntryTextToName );
  mFunctionsCombo->addItems( listCombo );
  mFunctionsCombo->show();
  addApis( listApi );
}

void QgsSQLComposerDialog::loadTableColumns( const QString& table )
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

static void resetCombo( QComboBox* combo )
{
  // We do it in a defered way, otherwise Valgrind complains when using QTest
  // since basically this call a recursive call to QComboBox::setCurrentIndex()
  // which cause internal QComboBox logic to operate on a destroyed object
  // However that isn't reproduce in live session. Anyway this hack is safe
  // in all modes.
  QMetaObject::invokeMethod( combo, "setCurrentIndex", Qt::QueuedConnection, Q_ARG( int, 0 ) );
}

void QgsSQLComposerDialog::on_mTablesCombo_currentIndexChanged( int )
{
  int index = mTablesCombo->currentIndex();
  if ( index <= 0 )
    return;
  QObject* obj = mFocusedObject;
  QString newText = mapTableEntryTextToName[mTablesCombo->currentText()];
  loadTableColumns( newText );
  if ( obj == mTablesEditor )
  {
    QString currentText = mTablesEditor->text();
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

void QgsSQLComposerDialog::on_mColumnsCombo_currentIndexChanged( int )
{
  int index = mColumnsCombo->currentIndex();
  if ( index <= 0 )
    return;
  QObject* obj = mFocusedObject;
  QString newText = mapColumnEntryTextToName[mColumnsCombo->currentText()];
  if ( obj == mColumnsEditor )
  {
    QString currentText = mColumnsEditor->toPlainText();
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
      QString currentText( mTableJoins->selectedItems().at( 0 )->text() );
      if ( !currentText.isEmpty() && !currentText.contains( "=" ) )
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

void QgsSQLComposerDialog::on_mFunctionsCombo_currentIndexChanged( int )
{
  functionCurrentIndexChanged( mFunctionsCombo, mapFunctionEntryTextToName );
}

void QgsSQLComposerDialog::on_mSpatialPredicatesCombo_currentIndexChanged( int )
{
  functionCurrentIndexChanged( mSpatialPredicatesCombo, mapSpatialPredicateEntryTextToName );
}

void QgsSQLComposerDialog::functionCurrentIndexChanged( QComboBox* combo,
    const QMap<QString, QString>& mapEntryTextToName )
{
  int index = combo->currentIndex();
  if ( index <= 0 )
    return;
  QObject* obj = mFocusedObject;
  QString newText = mapEntryTextToName[combo->currentText()];
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

void QgsSQLComposerDialog::on_mOperatorsCombo_currentIndexChanged( int )
{
  int index = mOperatorsCombo->currentIndex();
  if ( index <= 0 )
    return;
  QObject* obj = mFocusedObject;
  QString newText = mOperatorsCombo->currentText();
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
      QString currentText( mTableJoins->selectedItems().at( 0 )->text() );
      mTableJoins->selectedItems().at( 0 )->setText( currentText + newText );
    }
  }
  else if ( obj == mQueryEdit )
  {
    mQueryEdit->insertText( newText );
  }
  resetCombo( mOperatorsCombo );
}

void QgsSQLComposerDialog::on_mAddJoinButton_clicked()
{
  int insertRow = mTableJoins->currentRow();
  int rowCount = mTableJoins->rowCount();
  if ( insertRow < 0 )
    insertRow = rowCount;
  mTableJoins->setRowCount( rowCount + 1 );
  for ( int row = rowCount ; row > insertRow + 1; row -- )
  {
    mTableJoins->setItem( row, 0, mTableJoins->takeItem( row - 1, 0 ) );
    mTableJoins->setItem( row, 1, mTableJoins->takeItem( row - 1, 1 ) );
  }
  mTableJoins->setItem(( insertRow == rowCount ) ? insertRow : insertRow + 1, 0, new QTableWidgetItem( "" ) );
  mTableJoins->setItem(( insertRow == rowCount ) ? insertRow : insertRow + 1, 1, new QTableWidgetItem( "" ) );
}

void QgsSQLComposerDialog::on_mRemoveJoinButton_clicked()
{
  int row = mTableJoins->currentRow();
  if ( row < 0 )
    return;
  int rowCount = mTableJoins->rowCount();
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

void QgsSQLComposerDialog::on_mTableJoins_itemSelectionChanged()
{
  mRemoveJoinButton->setEnabled( mTableJoins->selectedItems().size() == 1 );
}

void QgsSQLComposerDialog::addApis( const QStringList& list )
{
  mApiList += list;

  delete mQueryEdit->lexer()->apis();
  QsciAPIs* apis = new QsciAPIs( mQueryEdit->lexer() );

  Q_FOREACH ( const QString& str, mApiList )
  {
    apis->add( str );
  }

  apis->prepare();
  mQueryEdit->lexer()->setAPIs( apis );
}

void QgsSQLComposerDialog::setSupportMultipleTables( bool on, QString mainTypename )
{
  mJoinsLabels->setVisible( on );
  mTableJoins->setVisible( on );
  mAddJoinButton->setVisible( on );
  mRemoveJoinButton->setVisible( on );
  mTablesCombo->setVisible( on );

  QString mainTypenameFormatted;
  if ( !mainTypename.isEmpty() )
    mainTypenameFormatted = " (" + mainTypename + ")";
  mQueryEdit->setToolTip( tr( "This is the SQL query editor. The SQL statement can select data from several tables, \n"
                              "but it must compulsory include the main typename%1 in the selected tables, \n"
                              "and only the geometry column of the main typename can be used as the geometry column of the resulting layer." ).arg( mainTypenameFormatted ) );
}
