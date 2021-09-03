/***************************************************************************
    qgsfieldvalueslineedit.cpp
     -------------------------
    Date                 : 20-08-2016
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfieldvalueslineedit.h"
#include "qgsvectorlayer.h"
#include "qgsfloatingwidget.h"

#include <QCompleter>
#include <QStringListModel>
#include <QTimer>
#include <QHBoxLayout>

QgsFieldValuesLineEdit::QgsFieldValuesLineEdit( QWidget *parent )
  : QgsFilterLineEdit( parent )
{
  QCompleter *c = new QCompleter( this );
  c->setCaseSensitivity( Qt::CaseInsensitive );
  c->setFilterMode( Qt::MatchContains );
  c->setCompletionMode( QCompleter::UnfilteredPopupCompletion );
  setCompleter( c );
  connect( this, &QgsFieldValuesLineEdit::textEdited, this, &QgsFieldValuesLineEdit::requestCompleterUpdate );
  mShowPopupTimer.setSingleShot( true );
  mShowPopupTimer.setInterval( 100 );
  connect( &mShowPopupTimer, &QTimer::timeout, this, &QgsFieldValuesLineEdit::triggerCompleterUpdate );
}

QgsFieldValuesLineEdit::~QgsFieldValuesLineEdit()
{
  if ( mGatherer )
  {
    mGatherer->stop();
    mGatherer->wait(); // mGatherer is deleted when wait completes
  }
}

void QgsFieldValuesLineEdit::setLayer( QgsVectorLayer *layer )
{
  if ( mLayer == layer )
    return;

  mLayer = layer;
  emit layerChanged( layer );
}

void QgsFieldValuesLineEdit::setAttributeIndex( int index )
{
  if ( mAttributeIndex == index )
    return;

  mAttributeIndex = index;
  emit attributeIndexChanged( index );
}

void QgsFieldValuesLineEdit::requestCompleterUpdate()
{
  mUpdateRequested = true;
  mShowPopupTimer.start();
}

void QgsFieldValuesLineEdit::triggerCompleterUpdate()
{
  mShowPopupTimer.stop();
  const QString currentText = text();

  if ( currentText.isEmpty() )
  {
    if ( mGatherer )
      mGatherer->stop();
    return;
  }

  updateCompletionList( currentText );
}

void QgsFieldValuesLineEdit::updateCompletionList( const QString &text )
{
  if ( text.isEmpty() )
  {
    if ( mGatherer )
      mGatherer->stop();
    return;
  }

  mUpdateRequested = true;
  if ( mGatherer )
  {
    mRequestedCompletionText = text;
    mGatherer->stop();
    return;
  }

  mGatherer = new QgsFieldValuesLineEditValuesGatherer( mLayer, mAttributeIndex );
  mGatherer->setSubstring( text );

  connect( mGatherer, &QgsFieldValuesLineEditValuesGatherer::collectedValues, this, &QgsFieldValuesLineEdit::updateCompleter );
  connect( mGatherer, &QgsFieldValuesLineEditValuesGatherer::finished, this, &QgsFieldValuesLineEdit::gathererThreadFinished );

  mGatherer->start();
}

void QgsFieldValuesLineEdit::gathererThreadFinished()
{
  const bool wasCanceled = mGatherer->wasCanceled();

  delete mGatherer;
  mGatherer = nullptr;

  if ( wasCanceled )
  {
    const QString text = mRequestedCompletionText;
    mRequestedCompletionText.clear();
    updateCompletionList( text );
    return;
  }
}

void QgsFieldValuesLineEdit::updateCompleter( const QStringList &values )
{
  mUpdateRequested = false;
  completer()->setModel( new QStringListModel( values ) );
  completer()->complete();
}


// just internal guff - definitely not for exposing to public API!
///@cond PRIVATE

void QgsFieldValuesLineEditValuesGatherer::run()
{
  mWasCanceled = false;
  if ( mSubstring.isEmpty() )
  {
    emit collectedValues( QStringList() );
    return;
  }

  // allow responsive cancellation
  mFeedback = new QgsFeedback();
  // just get 100 values... maybe less/more would be useful?
  mValues = mLayer->uniqueStringsMatching( mAttributeIndex, mSubstring, 100, mFeedback );

  // be overly cautious - it's *possible* stop() might be called between deleting mFeedback and nulling it
  mFeedbackMutex.lock();
  delete mFeedback;
  mFeedback = nullptr;
  mFeedbackMutex.unlock();

  emit collectedValues( mValues );
}

void QgsFieldValuesLineEditValuesGatherer::stop()
{
  // be cautious, in case gatherer stops naturally just as we are canceling it and mFeedback gets deleted
  mFeedbackMutex.lock();
  if ( mFeedback )
    mFeedback->cancel();
  mFeedbackMutex.unlock();

  mWasCanceled = true;
}

///@endcond
