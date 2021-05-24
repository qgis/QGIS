/***************************************************************************
    qgsjsoneditwidget.cpp
     --------------------------------------
    Date                 : 3.5.2021
    Copyright            : (C) 2021 Damiano Lombardi
    Email                : damiano at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsjsoneditwidget.h"

#include <QDesktopServices>
#include <QJsonArray>
#include <QLabel>
#include <QPushButton>
#include <QToolTip>
#include <QUrl>

QgsJsonEditWidget::QgsJsonEditWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  setView( View::Text );

  mCodeEditorJson->setReadOnly( true );
  mCodeEditorJson->setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );
  mCodeEditorJson->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
  mCodeEditorJson->indicatorDefine( QsciScintilla::PlainIndicator, SCINTILLA_UNDERLINE_INDICATOR_INDEX );
  mCodeEditorJson->SendScintilla( QsciScintillaBase::SCI_SETINDICATORCURRENT, SCINTILLA_UNDERLINE_INDICATOR_INDEX );
  mCodeEditorJson->SendScintilla( QsciScintillaBase::SCI_SETMOUSEDWELLTIME, 400 );

  mTreeWidget->setStyleSheet( QStringLiteral( "font-family: %1;" ).arg( QgsCodeEditor::getMonospaceFont().family() ) );

  connect( mTextToolButton, &QToolButton::clicked, this, &QgsJsonEditWidget::textToolButtonClicked );
  connect( mTreeToolButton, &QToolButton::clicked, this, &QgsJsonEditWidget::treeToolButtonClicked );

  connect( mCodeEditorJson, &QgsCodeEditorJson::textChanged, this, &QgsJsonEditWidget::codeEditorJsonTextChanged );
  // Signal indicatorClicked is used because indicatorReleased has a bug in Scintilla and the keyboard modifier state
  // parameter is not correct. Merge request was submittet to fix it: https://sourceforge.net/p/scintilla/code/merge-requests/26/
  connect( mCodeEditorJson, &QsciScintilla::indicatorClicked, this, &QgsJsonEditWidget::codeEditorJsonIndicatorClicked );
  connect( mCodeEditorJson, &QsciScintillaBase::SCN_DWELLSTART, this, &QgsJsonEditWidget::codeEditorJsonDwellStart );
  connect( mCodeEditorJson, &QsciScintillaBase::SCN_DWELLEND, this, &QgsJsonEditWidget::codeEditorJsonDwellEnd );
}

void QgsJsonEditWidget::setJsonText( const QString &jsonText )
{
  mJsonText = jsonText;
  mClickableLinkList.clear();

  const QJsonDocument jsonDocument = QJsonDocument::fromJson( mJsonText.toUtf8() );

  mCodeEditorJson->blockSignals( true );
  if ( jsonDocument.isNull() )
  {
    mCodeEditorJson->setText( mJsonText );
  }
  else
  {
    switch ( mFormatJsonMode )
    {
      case FormatJson::Indented:
        mCodeEditorJson->setText( jsonDocument.toJson( QJsonDocument::Indented ) );
        break;
      case FormatJson::Compact:
        mCodeEditorJson->setText( jsonDocument.toJson( QJsonDocument::Compact ) );
        break;
      case FormatJson::Disabled:
        mCodeEditorJson->setText( mJsonText );
        break;
    }
  }
  mCodeEditorJson->blockSignals( false );

  refreshTreeView( jsonDocument );
}

QString QgsJsonEditWidget::jsonText() const
{
  return mJsonText;
}

void QgsJsonEditWidget::setView( QgsJsonEditWidget::View view ) const
{
  switch ( view )
  {
    case View::Text:
    {
      mStackedWidget->setCurrentWidget( mStackedWidgetPageText );
      mTextToolButton->setChecked( true );
      mTreeToolButton->setChecked( false );
    }
    break;
    case View::Tree:
    {
      mStackedWidget->setCurrentWidget( mStackedWidgetPageTree );
      mTreeToolButton->setChecked( true );
      mTextToolButton->setChecked( false );
    }
    break;
  }
}

void QgsJsonEditWidget::setFormatJsonMode( QgsJsonEditWidget::FormatJson formatJson )
{
  mFormatJsonMode = formatJson;
}

void QgsJsonEditWidget::setControlsVisible( bool visible )
{
  mControlsWidget->setVisible( visible );
}

void QgsJsonEditWidget::textToolButtonClicked( bool checked )
{
  if ( checked )
    setView( View::Text );
  else
    setView( View::Tree );
}

void QgsJsonEditWidget::treeToolButtonClicked( bool checked )
{
  if ( checked )
    setView( View::Tree );
  else
    setView( View::Text );
}

void QgsJsonEditWidget::codeEditorJsonTextChanged()
{
  mJsonText = mCodeEditorJson->text();
  const QJsonDocument jsonDocument = QJsonDocument::fromJson( mJsonText.toUtf8() );
  refreshTreeView( jsonDocument );
}

void QgsJsonEditWidget::codeEditorJsonIndicatorClicked( int line, int index, Qt::KeyboardModifiers state )
{
  if ( !state.testFlag( Qt::ControlModifier ) )
    return;

  int position = mCodeEditorJson->positionFromLineIndex( line, index );
  int clickableLinkListIndex = mCodeEditorJson->SendScintilla( QsciScintillaBase::SCI_INDICATORVALUEAT,
                               SCINTILLA_UNDERLINE_INDICATOR_INDEX,
                               position );
  if ( clickableLinkListIndex <= 0 )
    return;

  QDesktopServices::openUrl( mClickableLinkList.at( clickableLinkListIndex - 1 ) );
}

void QgsJsonEditWidget::codeEditorJsonDwellStart( int position, int x, int y )
{
  Q_UNUSED( x )
  Q_UNUSED( y )

  int clickableLinkListIndex = mCodeEditorJson->SendScintilla( QsciScintillaBase::SCI_INDICATORVALUEAT,
                               SCINTILLA_UNDERLINE_INDICATOR_INDEX,
                               position );
  if ( clickableLinkListIndex <= 0 )
    return;

  QToolTip::showText( QCursor::pos(),
                      tr( "%1\nCTRL + click to follow link" ).arg( mClickableLinkList.at( clickableLinkListIndex - 1 ) ) );
}

void QgsJsonEditWidget::codeEditorJsonDwellEnd( int position, int x, int y )
{
  Q_UNUSED( position )
  Q_UNUSED( x )
  Q_UNUSED( y )
  QToolTip::hideText();
}

void QgsJsonEditWidget::refreshTreeView( const QJsonDocument &jsonDocument )
{
  mTreeWidget->clear();

  if ( jsonDocument.isNull() )
  {
    setView( View::Text );
    mTextToolButton->setDisabled( true );
    mTreeToolButton->setDisabled( true );
    mTreeToolButton->setToolTip( tr( "Invalid JSON, tree view not available" ) );
    return;
  }
  else
  {
    mTextToolButton->setEnabled( true );
    mTreeToolButton->setEnabled( true );
    mTreeToolButton->setToolTip( tr( "Tree view" ) );
  }

  if ( jsonDocument.isObject() )
  {
    const QStringList keys = jsonDocument.object().keys();
    for ( const QString &key : keys )
    {
      const QJsonValue jsonValue = jsonDocument.object().value( key );
      QTreeWidgetItem *treeWidgetItem = new QTreeWidgetItem( mTreeWidget, QStringList() << key );
      refreshTreeViewItemValue( jsonValue, treeWidgetItem );
      mTreeWidget->addTopLevelItem( treeWidgetItem );
      mTreeWidget->expandItem( treeWidgetItem );
    }
  }
  else if ( jsonDocument.isArray() )
  {
    for ( int index = 0; index < jsonDocument.array().size(); index++ )
    {
      QTreeWidgetItem *treeWidgetItem = new QTreeWidgetItem( mTreeWidget, QStringList() << QString::number( index ) );
      refreshTreeViewItemValue( jsonDocument.array().at( index ), treeWidgetItem );
      mTreeWidget->addTopLevelItem( treeWidgetItem );
      mTreeWidget->expandItem( treeWidgetItem );
    }
  }

  mTreeWidget->resizeColumnToContents( static_cast<int>( TreeWidgetColumn::Key ) );
}

void QgsJsonEditWidget::refreshTreeViewItemValue( const QJsonValue &jsonValue, QTreeWidgetItem *treeWidgetItemParent )
{
  switch ( jsonValue.type() )
  {
    case QJsonValue::Null:
      treeWidgetItemParent->setText( static_cast<int>( TreeWidgetColumn::Value ), QStringLiteral( "null" ) );
      break;
    case QJsonValue::Bool:
      treeWidgetItemParent->setText( static_cast<int>( TreeWidgetColumn::Value ), jsonValue.toBool() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
      break;
    case QJsonValue::Double:
      treeWidgetItemParent->setText( static_cast<int>( TreeWidgetColumn::Value ), QString::number( jsonValue.toDouble() ) );
      break;
    case QJsonValue::String:
    {
      const QString jsonValueString = jsonValue.toString();
      if ( QUrl( jsonValueString ).scheme().isEmpty() )
      {
        treeWidgetItemParent->setText( static_cast<int>( TreeWidgetColumn::Value ), jsonValueString );
      }
      else
      {
        QLabel *label = new QLabel( QString( "<a href='%1'>%1</a>" ).arg( jsonValueString ) );
        mTreeWidget->setItemWidget( treeWidgetItemParent, static_cast<int>( TreeWidgetColumn::Value ), label );

        connect( label, &QLabel::linkActivated, this, []( const QString & link )
        {
          QDesktopServices::openUrl( link );
        } );

        mClickableLinkList.append( jsonValueString );
        mCodeEditorJson->SendScintilla( QsciScintillaBase::SCI_SETINDICATORVALUE, mClickableLinkList.size() );
        mCodeEditorJson->SendScintilla( QsciScintillaBase::SCI_INDICATORFILLRANGE,
                                        mCodeEditorJson->text().indexOf( jsonValueString ),
                                        jsonValueString.size() );
      }
    }
    break;
    case QJsonValue::Array:
    {
      const QJsonArray jsonArray = jsonValue.toArray();
      for ( int index = 0; index < jsonArray.size(); index++ )
      {
        QTreeWidgetItem *treeWidgetItem = new QTreeWidgetItem( treeWidgetItemParent, QStringList() << QString::number( index ) );
        refreshTreeViewItemValue( jsonArray.at( index ), treeWidgetItem );
        treeWidgetItemParent->addChild( treeWidgetItem );
        treeWidgetItemParent->setExpanded( true );
      }
    }
    break;
    case QJsonValue::Object:
    {
      const QJsonObject jsonObject = jsonValue.toObject();
      const QStringList keys = jsonObject.keys();
      for ( const QString &key : keys )
      {
        QTreeWidgetItem *treeWidgetItem = new QTreeWidgetItem( treeWidgetItemParent, QStringList() << key );
        refreshTreeViewItemValue( jsonObject.value( key ), treeWidgetItem );
        treeWidgetItemParent->addChild( treeWidgetItem );
        treeWidgetItemParent->setExpanded( true );
      }
    }
    break;
    case QJsonValue::Undefined:
      treeWidgetItemParent->setText( static_cast<int>( TreeWidgetColumn::Value ), QStringLiteral( "Undefined value" ) );
      break;
  }
}
