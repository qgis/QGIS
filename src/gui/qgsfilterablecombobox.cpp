/***************************************************************************
                             qgsfilterablecombobox.cpp
                             ------------------------
    Date                 : November 2026
    Copyright            : (C) 2026 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfilterablecombobox.h"

#include "qgsfilterlineedit.h"

#include <QAbstractItemView>
#include <QCompleter>
#include <QEvent>
#include <QFocusEvent>
#include <QTimer>

#include "moc_qgsfilterablecombobox.cpp"

QgsFilterableComboBox::QgsFilterableComboBox( QWidget *parent )
  : QComboBox( parent )
{
  setSizeAdjustPolicy( QComboBox::SizeAdjustPolicy::AdjustToMinimumContentsLengthWithIcon );
  setEditable( true );
  setInsertPolicy( QComboBox::InsertPolicy::NoInsert );
  setMaxVisibleItems( 30 );
  mLineEdit = new QgsFilterLineEdit( this );
  setLineEdit( mLineEdit );
  mLineEdit->setShowSearchIcon( false );
  mLineEdit->setShowClearButton( false );
  mLineEdit->setSelectOnFocus( true );
  mLineEdit->installEventFilter( this );

  mCompleter = new QCompleter( model(), this );
  mCompleter->setCompletionMode( QCompleter::CompletionMode::PopupCompletion );
  mCompleter->setFilterMode( Qt::MatchFlag::MatchContains );
  mCompleter->setCaseSensitivity( Qt::CaseSensitivity::CaseInsensitive );
  mCompleter->setMaxVisibleItems( 30 );
  setCompleter( mCompleter );

  mCompleter->popup()->installEventFilter( this );
  view()->installEventFilter( this );

  // we only want the search icon to show WHILE a filter is being actively entered, not in the combobox default state
  // => immediately remove it ater the user picks an entry from the completer or activates the standard combobox dropdown
  connect( mCompleter, qOverload< const QString &>( &QCompleter::activated ), this, [this]( const QString & ) { mLineEdit->setShowSearchIcon( false ); } );
  connect( this, &QComboBox::activated, this, [this] { mLineEdit->setShowSearchIcon( false ); } );

  // show search icon when the filter text is being entered
  connect( mLineEdit, &QLineEdit::textEdited, this, [this] {
    if ( mLineEdit->hasFocus() )
    {
      mLineEdit->setShowSearchIcon( true );
    }
  } );

  // record valid selections, so we can restore these if the user enters a partial filter
  // that doesn't match one of our valid items. We DO NOT allow ad-hoc entry in this widget.
  connect( this, &QComboBox::currentIndexChanged, this, [this]( int index ) {
    if ( index >= 0 )
    {
      mLastValidIndex = model()->index( index, modelColumn(), rootModelIndex() );
    }
  } );

  connect( lineEdit(), &QLineEdit::editingFinished, this, [this] {
    const QString text = currentText();
    // MatchFixedString = case insensitive
    const int index = findText( text, Qt::MatchFlag::MatchFixedString );
    if ( index >= 0 )
    {
      setCurrentIndex( index );
      setCurrentText( itemText( index ) );
    }
    else if ( mLastValidIndex.isValid() )
    {
      // user defocused combobox while entering a filter string -- we don't allow
      // free-from strings, so reset to last selection
      setCurrentIndex( mLastValidIndex.row() );
      setCurrentText( itemText( mLastValidIndex.row() ) );
    }
    mLineEdit->setShowSearchIcon( false );
  } );

  // have to listen to both the completer and the combobox signals to get pre-selected/highlight items
  connect( mCompleter, qOverload< const QModelIndex &>( &QCompleter::highlighted ), this, &QgsFilterableComboBox::onItemHighlightedByCompleter );
  connect( this, &QComboBox::highlighted, this, &QgsFilterableComboBox::onItemHighlighted );
}

bool QgsFilterableComboBox::eventFilter( QObject *obj, QEvent *event )
{
  if ( obj == mLineEdit )
  {
    if ( event->type() == QEvent::Type::FocusIn )
    {
      const Qt::FocusReason reason = static_cast<QFocusEvent *>( event )->reason();
      // immediately show the popup list and select all the current text when the widget is first focused
      if ( reason != Qt::FocusReason::PopupFocusReason && reason != Qt::FocusReason::MouseFocusReason )
      {
        QTimer::singleShot( 0, this, &QgsFilterableComboBox::openCompleterPopup );
      }
    }
    else if ( event->type() == QEvent::Type::MouseButtonRelease )
    {
      // when the line edit part of the widget is FIRST clicked select all and show the popup immediately.
      // (The extra logic here prevents us doing that again when the user makes mouse clicks while already entering text,
      // eg selecting characters to delete/replace)
      if ( static_cast<QMouseEvent *>( event )->button() == Qt::LeftButton && mLineEdit->hasFocus() && !mCompleter->popup()->isVisible() )
      {
        QTimer::singleShot( 0, this, &QgsFilterableComboBox::openCompleterPopup );
      }
    }
    else if ( event->type() == QEvent::Type::FocusOut )
    {
      mLineEdit->setShowSearchIcon( false );
    }
  }
  else if ( ( obj == mCompleter->popup() || obj == view() ) && event->type() == QEvent::Type::Hide )
  {
    clearPreselection();
  }

  return QComboBox::eventFilter( obj, event );
}

void QgsFilterableComboBox::openCompleterPopup()
{
  if ( !mLineEdit->hasFocus() || view()->isVisible() )
    return;

  mCompleter->setCompletionPrefix( QString() );
  mCompleter->complete();
}

void QgsFilterableComboBox::onItemHighlightedByCompleter( const QModelIndex &index )
{
  if ( index.isValid() )
  {
    mHasPreselectedItem = true;
    const QVariant userData = index.data( Qt::UserRole );
    emit itemDataPreselected( userData );
  }
  else
  {
    clearPreselection();
  }
}

void QgsFilterableComboBox::onItemHighlighted( int index )
{
  if ( index >= 0 )
  {
    mHasPreselectedItem = true;
    emit itemDataPreselected( itemData( index ) );
  }
  else
  {
    clearPreselection();
  }
}

void QgsFilterableComboBox::clearPreselection()
{
  if ( mHasPreselectedItem )
  {
    mHasPreselectedItem = false;
    emit itemDataPreselectionCleared();
  }
}
