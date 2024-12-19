/***************************************************************************
                         qgsprocessingpointcloudexpressionlineedit.cpp
                         ---------------------
    begin                : April 2023
    copyright            : (C) 2023 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingpointcloudexpressionlineedit.h"
#include "moc_qgsprocessingpointcloudexpressionlineedit.cpp"
#include "qgsgui.h"
#include "qgsapplication.h"
#include "qgsfilterlineedit.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudexpression.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QListView>
#include <QPushButton>
#include <QMessageBox>

/// @cond PRIVATE

QgsProcessingPointCloudExpressionLineEdit::QgsProcessingPointCloudExpressionLineEdit( QWidget *parent )
  : QWidget( parent )
{
  mLineEdit = new QgsFilterLineEdit();
  mLineEdit->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );

  mButton = new QToolButton();
  mButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  mButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconExpression.svg" ) ) );
  connect( mButton, &QAbstractButton::clicked, this, &QgsProcessingPointCloudExpressionLineEdit::editExpression );

  QHBoxLayout *layout = new QHBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( mLineEdit );
  layout->addWidget( mButton );
  setLayout( layout );

  setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
  setFocusProxy( mLineEdit );
  connect( mLineEdit, &QLineEdit::textChanged, this, static_cast<void ( QgsProcessingPointCloudExpressionLineEdit::* )( const QString & )>( &QgsProcessingPointCloudExpressionLineEdit::expressionEdited ) );

  setExpression( expression() );
}

QgsProcessingPointCloudExpressionLineEdit::~QgsProcessingPointCloudExpressionLineEdit() = default;

void QgsProcessingPointCloudExpressionLineEdit::setLayer( QgsPointCloudLayer *layer )
{
  mLayer = layer;
}

QgsPointCloudLayer *QgsProcessingPointCloudExpressionLineEdit::layer() const
{
  return mLayer;
}

QString QgsProcessingPointCloudExpressionLineEdit::expression() const
{
  if ( mLineEdit )
    return mLineEdit->text();

  return QString();
}

void QgsProcessingPointCloudExpressionLineEdit::setExpression( const QString &newExpression )
{
  if ( mLineEdit )
    mLineEdit->setText( newExpression );
}

void QgsProcessingPointCloudExpressionLineEdit::editExpression()
{
  const QString currentExpression = expression();
  QgsProcessingPointCloudExpressionDialog dlg( mLayer );
  dlg.setExpression( currentExpression );

  if ( dlg.exec() )
  {
    const QString newExpression = dlg.expression();
    setExpression( newExpression );
  }
}

void QgsProcessingPointCloudExpressionLineEdit::expressionEdited()
{
  emit expressionChanged( expression() );
}

void QgsProcessingPointCloudExpressionLineEdit::expressionEdited( const QString &expression )
{
  emit expressionChanged( expression );
}


QgsProcessingPointCloudExpressionDialog::QgsProcessingPointCloudExpressionDialog( QgsPointCloudLayer *layer, const QString &startExpression, QWidget *parent )
  : QDialog( parent )
  , mLayer( layer )
  , mInitialText( startExpression )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  mModelAttributes = new QStandardItemModel();
  mModelValues = new QStandardItemModel();
  lstAttributes->setModel( mModelAttributes );
  lstValues->setModel( mModelValues );

  populateAttributes();

  connect( lstAttributes->selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsProcessingPointCloudExpressionDialog::lstAttributes_currentChanged );
  connect( lstAttributes, &QListView::doubleClicked, this, &QgsProcessingPointCloudExpressionDialog::lstAttributes_doubleClicked );
  connect( lstValues, &QListView::doubleClicked, this, &QgsProcessingPointCloudExpressionDialog::lstValues_doubleClicked );
  connect( btnEqual, &QPushButton::clicked, this, &QgsProcessingPointCloudExpressionDialog::btnEqual_clicked );
  connect( btnLessThan, &QPushButton::clicked, this, &QgsProcessingPointCloudExpressionDialog::btnLessThan_clicked );
  connect( btnGreaterThan, &QPushButton::clicked, this, &QgsProcessingPointCloudExpressionDialog::btnGreaterThan_clicked );
  connect( btnIn, &QPushButton::clicked, this, &QgsProcessingPointCloudExpressionDialog::btnIn_clicked );
  connect( btnNotIn, &QPushButton::clicked, this, &QgsProcessingPointCloudExpressionDialog::btnNotIn_clicked );
  connect( btnLessEqual, &QPushButton::clicked, this, &QgsProcessingPointCloudExpressionDialog::btnLessEqual_clicked );
  connect( btnGreaterEqual, &QPushButton::clicked, this, &QgsProcessingPointCloudExpressionDialog::btnGreaterEqual_clicked );
  connect( btnNotEqual, &QPushButton::clicked, this, &QgsProcessingPointCloudExpressionDialog::btnNotEqual_clicked );
  connect( btnAnd, &QPushButton::clicked, this, &QgsProcessingPointCloudExpressionDialog::btnAnd_clicked );
  connect( btnOr, &QPushButton::clicked, this, &QgsProcessingPointCloudExpressionDialog::btnOr_clicked );

  QPushButton *pbn = new QPushButton( tr( "&Test" ) );
  buttonBox->addButton( pbn, QDialogButtonBox::ActionRole );
  connect( pbn, &QAbstractButton::clicked, this, &QgsProcessingPointCloudExpressionDialog::test );

  pbn = new QPushButton( tr( "&Clear" ) );
  buttonBox->addButton( pbn, QDialogButtonBox::ActionRole );
  connect( pbn, &QAbstractButton::clicked, this, &QgsProcessingPointCloudExpressionDialog::clear );

  mTxtSql->setText( mInitialText );
}

void QgsProcessingPointCloudExpressionDialog::setExpression( const QString &text )
{
  mTxtSql->setText( text );
}

QString QgsProcessingPointCloudExpressionDialog::expression()
{
  return mTxtSql->text();
}

void QgsProcessingPointCloudExpressionDialog::populateAttributes()
{
  if ( !mLayer )
  {
    return;
  }

  const QgsFields &fields = mLayer->dataProvider()->attributes().toFields();
  mTxtSql->setFields( fields );
  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    QStandardItem *myItem = new QStandardItem( fields.at( idx ).displayNameWithAlias() );
    mModelAttributes->insertRow( mModelAttributes->rowCount(), myItem );
  }
}

void QgsProcessingPointCloudExpressionDialog::lstAttributes_currentChanged( const QModelIndex &current, const QModelIndex &previous )
{
  Q_UNUSED( previous )

  mModelValues->clear();
  const QString attribute = current.data().toString();
  if ( attribute.compare( QLatin1String( "Classification" ), Qt::CaseInsensitive ) == 0 )
  {
    const QMap<int, QString> codes = QgsPointCloudDataProvider::translatedLasClassificationCodes();
    for ( int i = 0; i <= 18; ++i )
    {
      QStandardItem *item = new QStandardItem( QString( "%1: %2" ).arg( i ).arg( codes.value( i ) ) );
      item->setData( i, Qt::UserRole );
      mModelValues->insertRow( mModelValues->rowCount(), item );
    }
  }
  else
  {
    const QgsPointCloudStatistics stats = mLayer->statistics();
    double value = stats.minimum( attribute );
    QString valueString = std::isnan( value ) ? tr( "n/a" ) : QString::number( value );
    QStandardItem *item = new QStandardItem( tr( "Minimum: %1" ).arg( valueString ) );
    item->setData( value, Qt::UserRole );
    mModelValues->insertRow( mModelValues->rowCount(), item );

    value = stats.maximum( attribute );
    valueString = std::isnan( value ) ? tr( "n/a" ) : QString::number( value );
    item = new QStandardItem( tr( "Maximum: %1" ).arg( valueString ) );
    item->setData( value, Qt::UserRole );
    mModelValues->insertRow( mModelValues->rowCount(), item );

    value = stats.mean( attribute );
    valueString = std::isnan( value ) ? tr( "n/a" ) : QString::number( value );
    item = new QStandardItem( tr( "Mean: %1" ).arg( valueString ) );
    item->setData( value, Qt::UserRole );
    mModelValues->insertRow( mModelValues->rowCount(), item );

    value = stats.stDev( attribute );
    valueString = std::isnan( value ) ? tr( "n/a" ) : QString::number( value );
    item = new QStandardItem( tr( "StdDev: %1" ).arg( valueString ) );
    item->setData( value, Qt::UserRole );
    mModelValues->insertRow( mModelValues->rowCount(), item );
  }
}

void QgsProcessingPointCloudExpressionDialog::lstAttributes_doubleClicked( const QModelIndex &index )
{
  mTxtSql->insertText( QStringLiteral( "%1 " ).arg( mModelAttributes->data( index ).toString() ) );
  mTxtSql->setFocus();
}

void QgsProcessingPointCloudExpressionDialog::lstValues_doubleClicked( const QModelIndex &index )
{
  mTxtSql->insertText( QStringLiteral( "%1 " ).arg( mModelValues->data( index, Qt::UserRole ).toString() ) );
  mTxtSql->setFocus();
}

void QgsProcessingPointCloudExpressionDialog::btnEqual_clicked()
{
  mTxtSql->insertText( QStringLiteral( "= " ) );
  mTxtSql->setFocus();
}

void QgsProcessingPointCloudExpressionDialog::btnLessThan_clicked()
{
  mTxtSql->insertText( QStringLiteral( "< " ) );
  mTxtSql->setFocus();
}

void QgsProcessingPointCloudExpressionDialog::btnGreaterThan_clicked()
{
  mTxtSql->insertText( QStringLiteral( "> " ) );
  mTxtSql->setFocus();
}

void QgsProcessingPointCloudExpressionDialog::btnIn_clicked()
{
  mTxtSql->insertText( QStringLiteral( "IN () " ) );
  int i, j;
  mTxtSql->getCursorPosition( &i, &j );
  mTxtSql->setCursorPosition( i, j - 2 );
  mTxtSql->setFocus();
}

void QgsProcessingPointCloudExpressionDialog::btnNotIn_clicked()
{
  mTxtSql->insertText( QStringLiteral( "NOT IN () " ) );
  int i, j;
  mTxtSql->getCursorPosition( &i, &j );
  mTxtSql->setCursorPosition( i, j - 2 );
  mTxtSql->setFocus();
}

void QgsProcessingPointCloudExpressionDialog::btnLessEqual_clicked()
{
  mTxtSql->insertText( QStringLiteral( "<= " ) );
  mTxtSql->setFocus();
}

void QgsProcessingPointCloudExpressionDialog::btnGreaterEqual_clicked()
{
  mTxtSql->insertText( QStringLiteral( ">= " ) );
  mTxtSql->setFocus();
}

void QgsProcessingPointCloudExpressionDialog::btnNotEqual_clicked()
{
  mTxtSql->insertText( QStringLiteral( "!= " ) );
  mTxtSql->setFocus();
}

void QgsProcessingPointCloudExpressionDialog::btnAnd_clicked()
{
  mTxtSql->insertText( QStringLiteral( "AND " ) );
  mTxtSql->setFocus();
}

void QgsProcessingPointCloudExpressionDialog::btnOr_clicked()
{
  mTxtSql->insertText( QStringLiteral( "OR " ) );
  mTxtSql->setFocus();
}

void QgsProcessingPointCloudExpressionDialog::test()
{
  QgsPointCloudExpression expression( mTxtSql->text() );

  if ( !expression.isValid() && !mTxtSql->text().isEmpty() )
  {
    QMessageBox::warning( this, tr( "Query Result" ), tr( "An error occurred while parsing the expression:\n%1" ).arg( expression.parserErrorString() ) );
  }
  else
  {
    const QSet<QString> attributes = expression.referencedAttributes();
    int offset;
    for ( const auto &attribute : attributes )
    {
      if ( mLayer && mLayer->dataProvider() && !mLayer->dataProvider()->attributes().find( attribute, offset ) )
      {
        QMessageBox::warning( this, tr( "Query Result" ), tr( "\"%1\" not recognized as an available attribute." ).arg( attribute ) );
        return;
      }
    }
    QMessageBox::information( this, tr( "Query Result" ), tr( "The expression was successfully parsed." ) );
  }
}

void QgsProcessingPointCloudExpressionDialog::clear()
{
  mTxtSql->clear();
}

///@endcond
