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

#include <QAction>
#include <QClipboard>
#include <QDesktopServices>
#include <QJsonArray>
#include <QLabel>
#include <QPushButton>
#include <QToolTip>
#include <QUrl>

QgsJsonEditWidget::QgsJsonEditWidget( QWidget *parent )
  : QWidget( parent )
  , mCopyValueAction( new QAction( tr( "Copy Value" ), this ) )
  , mCopyKeyAction( new QAction( tr( "Copy Key" ), this ) )
{
  setupUi( this );

  setView( View::Text );

  mCodeEditorJson->setReadOnly( true );
  mCodeEditorJson->setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );
  mCodeEditorJson->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
  mCodeEditorJson->indicatorDefine( QsciScintilla::PlainIndicator, SCINTILLA_UNDERLINE_INDICATOR_INDEX );
  mCodeEditorJson->SendScintilla( QsciScintillaBase::SCI_SETINDICATORCURRENT, SCINTILLA_UNDERLINE_INDICATOR_INDEX );
  mCodeEditorJson->SendScintilla( QsciScintillaBase::SCI_SETMOUSEDWELLTIME, 400 );

  mTreeWidget->setContextMenuPolicy( Qt::ActionsContextMenu );
  mTreeWidget->addAction( mCopyValueAction );
  mTreeWidget->addAction( mCopyKeyAction );

  connect( mTextToolButton, &QToolButton::clicked, this, &QgsJsonEditWidget::textToolButtonClicked );
  connect( mTreeToolButton, &QToolButton::clicked, this, &QgsJsonEditWidget::treeToolButtonClicked );

  connect( mCopyValueAction, &QAction::triggered, this, &QgsJsonEditWidget::copyValueActionTriggered );
  connect( mCopyKeyAction, &QAction::triggered, this, &QgsJsonEditWidget::copyKeyActionTriggered );

  connect( mCodeEditorJson, &QgsCodeEditorJson::textChanged, this, &QgsJsonEditWidget::codeEditorJsonTextChanged );
  // Signal indicatorClicked is used because indicatorReleased has a bug in Scintilla and the keyboard modifier state
  // parameter is not correct. Merge request was submittet to fix it: https://sourceforge.net/p/scintilla/code/merge-requests/26/
  connect( mCodeEditorJson, &QsciScintilla::indicatorClicked, this, &QgsJsonEditWidget::codeEditorJsonIndicatorClicked );
  connect( mCodeEditorJson, &QsciScintillaBase::SCN_DWELLSTART, this, &QgsJsonEditWidget::codeEditorJsonDwellStart );
  connect( mCodeEditorJson, &QsciScintillaBase::SCN_DWELLEND, this, &QgsJsonEditWidget::codeEditorJsonDwellEnd );
}

QgsCodeEditorJson *QgsJsonEditWidget::jsonEditor()
{
  return mCodeEditorJson;
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

void QgsJsonEditWidget::copyValueActionTriggered()
{
  if ( !mTreeWidget->currentItem() )
    return;

  const QJsonValue jsonValue = QJsonValue::fromVariant( mTreeWidget->currentItem()->data( static_cast<int>( TreeWidgetColumn::Value ), Qt::UserRole ) );

  switch ( jsonValue.type() )
  {
    case QJsonValue::Null:
    case QJsonValue::Bool:
    case QJsonValue::Double:
    case QJsonValue::Undefined:
      QApplication::clipboard()->setText( mTreeWidget->currentItem()->text( static_cast<int>( TreeWidgetColumn::Value ) ) );
      break;
    case QJsonValue::String:
      QApplication::clipboard()->setText( jsonValue.toString() );
      break;
    case QJsonValue::Array:
    {
      const QJsonDocument jsonDocument( jsonValue.toArray() );
      QApplication::clipboard()->setText( jsonDocument.toJson() );
    }
    break;
    case QJsonValue::Object:
    {
      const QJsonDocument jsonDocument( jsonValue.toObject() );
      QApplication::clipboard()->setText( jsonDocument.toJson() );
    }
    break;
  }
}

void QgsJsonEditWidget::copyKeyActionTriggered()
{
  if ( !mTreeWidget->currentItem() )
    return;

  QApplication::clipboard()->setText( mTreeWidget->currentItem()->text( static_cast<int>( TreeWidgetColumn::Key ) ) );
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

  const int position = mCodeEditorJson->positionFromLineIndex( line, index );
  const int clickableLinkListIndex = mCodeEditorJson->SendScintilla( QsciScintillaBase::SCI_INDICATORVALUEAT,
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

  const int clickableLinkListIndex = mCodeEditorJson->SendScintilla( QsciScintillaBase::SCI_INDICATORVALUEAT,
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
      treeWidgetItem->setFont( 0, monospaceFont() );
      refreshTreeViewItem( treeWidgetItem, jsonValue );
      mTreeWidget->addTopLevelItem( treeWidgetItem );
      mTreeWidget->expandItem( treeWidgetItem );
    }
  }
  else if ( jsonDocument.isArray() )
  {
    const QJsonArray array = jsonDocument.array();
    const auto arraySize = array.size();
    // Limit the number of rows we display, otherwise for pathological cases
    // like https://github.com/qgis/QGIS/pull/55847#issuecomment-1902077683
    // a unbounded number of elements will just stall the GUI forever.
    constexpr decltype( arraySize ) MAX_ELTS = 200;
    // If there are too many elements, disable URL highighting as it
    // performs very poorly.
    if ( arraySize > MAX_ELTS )
      mEnableUrlHighlighting = false;
    for ( auto index = decltype( arraySize ) {0}; index < arraySize; index++ )
    {
      QTreeWidgetItem *treeWidgetItem = new QTreeWidgetItem( mTreeWidget, QStringList() << QString::number( index ) );
      treeWidgetItem->setFont( 0, monospaceFont() );
      if ( arraySize <= MAX_ELTS || ( index < MAX_ELTS / 2 || index + MAX_ELTS / 2 > arraySize ) )
      {
        refreshTreeViewItem( treeWidgetItem, array.at( index ) );
        mTreeWidget->addTopLevelItem( treeWidgetItem );
        mTreeWidget->expandItem( treeWidgetItem );
      }
      else if ( index == MAX_ELTS / 2 )
      {
        index = arraySize - MAX_ELTS / 2;
        refreshTreeViewItem( treeWidgetItem, tr( "... truncated ..." ) );
        mTreeWidget->addTopLevelItem( treeWidgetItem );
        mTreeWidget->expandItem( treeWidgetItem );
      }
    }
  }

  mTreeWidget->resizeColumnToContents( static_cast<int>( TreeWidgetColumn::Key ) );
}

void QgsJsonEditWidget::refreshTreeViewItem( QTreeWidgetItem *treeWidgetItem, const QJsonValue &jsonValue )
{
  treeWidgetItem->setData( static_cast<int>( TreeWidgetColumn::Value ), Qt::UserRole, jsonValue.toVariant() );

  switch ( jsonValue.type() )
  {
    case QJsonValue::Null:
      refreshTreeViewItemValue( treeWidgetItem,
                                QStringLiteral( "null" ),
                                QgsCodeEditor::color( QgsCodeEditorColorScheme::ColorRole::Keyword ) );
      break;
    case QJsonValue::Bool:
      refreshTreeViewItemValue( treeWidgetItem,
                                jsonValue.toBool() ? QStringLiteral( "true" ) : QStringLiteral( "false" ),
                                QgsCodeEditor::color( QgsCodeEditorColorScheme::ColorRole::Keyword ) );
      break;
    case QJsonValue::Double:
      refreshTreeViewItemValue( treeWidgetItem,
                                QString::number( jsonValue.toDouble() ),
                                QgsCodeEditor::color( QgsCodeEditorColorScheme::ColorRole::Number ) );
      break;
    case QJsonValue::String:
    {
      const QString jsonValueString = jsonValue.toString();
      if ( !mEnableUrlHighlighting || QUrl( jsonValueString ).scheme().isEmpty() )
      {
        refreshTreeViewItemValue( treeWidgetItem,
                                  jsonValueString,
                                  QgsCodeEditor::color( QgsCodeEditorColorScheme::ColorRole::DoubleQuote ) );
      }
      else
      {
        QLabel *label = new QLabel( QString( "<a href='%1'>%1</a>" ).arg( jsonValueString ) );
        label->setOpenExternalLinks( true );
        label->setFont( monospaceFont() );
        mTreeWidget->setItemWidget( treeWidgetItem, static_cast<int>( TreeWidgetColumn::Value ), label );

        mClickableLinkList.append( jsonValueString );
        mCodeEditorJson->SendScintilla( QsciScintillaBase::SCI_SETINDICATORVALUE, static_cast< int >( mClickableLinkList.size() ) );
        mCodeEditorJson->SendScintilla( QsciScintillaBase::SCI_INDICATORFILLRANGE,
                                        mCodeEditorJson->text().indexOf( jsonValueString ),
                                        jsonValueString.size() );
      }
    }
    break;
    case QJsonValue::Array:
    {
      const QJsonArray jsonArray = jsonValue.toArray();
      const auto arraySize = jsonArray.size();
      // Limit the number of rows we display, otherwise for pathological cases
      // like https://github.com/qgis/QGIS/pull/55847#issuecomment-1902077683
      // a unbounded number of elements will just stall the GUI forever.
      constexpr decltype( arraySize ) MAX_ELTS = 200;
      // If there are too many elements, disable URL highighting as it
      // performs very poorly.
      if ( arraySize > MAX_ELTS )
        mEnableUrlHighlighting = false;
      for ( auto index = decltype( arraySize ) {0}; index < arraySize; index++ )
      {
        QTreeWidgetItem *treeWidgetItemChild = new QTreeWidgetItem( treeWidgetItem, QStringList() << QString::number( index ) );
        treeWidgetItemChild->setFont( 0, monospaceFont() );
        if ( arraySize <= MAX_ELTS || ( index < MAX_ELTS / 2 || index + MAX_ELTS / 2 > arraySize ) )
        {
          refreshTreeViewItem( treeWidgetItemChild, jsonArray.at( index ) );
          treeWidgetItem->addChild( treeWidgetItemChild );
          treeWidgetItem->setExpanded( true );
        }
        else if ( index == MAX_ELTS / 2 )
        {
          index = arraySize - MAX_ELTS / 2;
          refreshTreeViewItem( treeWidgetItemChild, tr( "... truncated ..." ) );
          treeWidgetItem->addChild( treeWidgetItemChild );
          treeWidgetItem->setExpanded( true );
        }
      }
    }
    break;
    case QJsonValue::Object:
    {
      const QJsonObject jsonObject = jsonValue.toObject();
      const QStringList keys = jsonObject.keys();
      for ( const QString &key : keys )
      {
        QTreeWidgetItem *treeWidgetItemChild = new QTreeWidgetItem( treeWidgetItem, QStringList() << key );
        treeWidgetItemChild->setFont( 0, monospaceFont() );
        refreshTreeViewItem( treeWidgetItemChild, jsonObject.value( key ) );
        treeWidgetItem->addChild( treeWidgetItemChild );
        treeWidgetItem->setExpanded( true );
      }
    }
    break;
    case QJsonValue::Undefined:
      refreshTreeViewItemValue( treeWidgetItem,
                                QStringLiteral( "Undefined value" ),
                                QgsCodeEditor::color( QgsCodeEditorColorScheme::ColorRole::DoubleQuote ) );
      break;
  }
}

void QgsJsonEditWidget::refreshTreeViewItemValue( QTreeWidgetItem *treeWidgetItem, const QString &jsonValueString, const QColor &textColor )
{
  QLabel *label = new QLabel( jsonValueString );
  label->setFont( monospaceFont() );

  if ( textColor.isValid() )
    label->setStyleSheet( QStringLiteral( "color: %1;" ).arg( textColor.name() ) );
  mTreeWidget->setItemWidget( treeWidgetItem, static_cast<int>( TreeWidgetColumn::Value ), label );
}

QFont QgsJsonEditWidget::monospaceFont() const
{
  QFont f = QgsCodeEditor::getMonospaceFont();
  // use standard widget font size, not code editor font size
  f.setPointSize( font().pointSize() );
  return f;
}
