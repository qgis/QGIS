/***************************************************************************
  qgsactionwidgetwrapper.cpp - QgsActionWidgetWrapper

 ---------------------
 begin                : 14.8.2021
 copyright            : (C) 2021 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsactionwidgetwrapper.h"
#include "qgsactionmanager.h"
#include "qgsexpressioncontextutils.h"

#include <QLayout>

QgsActionWidgetWrapper::QgsActionWidgetWrapper( QgsVectorLayer *layer, QWidget *editor, QWidget *parent )
  : QgsWidgetWrapper( layer, editor, parent )
{
  connect( this, &QgsWidgetWrapper::contextChanged, [ = ]
  {
    const bool actionIsVisible {
      ( context().attributeFormMode() == QgsAttributeEditorContext::Mode::SingleEditMode ) ||
      ( context().attributeFormMode() == QgsAttributeEditorContext::Mode::AddFeatureMode ) };
    if ( mActionButton )
    {
      mActionButton->setVisible( actionIsVisible );
    }
  } );
}

void QgsActionWidgetWrapper::setAction( const QgsAction &action )
{
  mAction = action;
}

void QgsActionWidgetWrapper::setFeature( const QgsFeature &feature )
{
  mFeature = feature;
}

void QgsActionWidgetWrapper::setEnabled( bool enabled )
{
  if ( valid() && layer() )
  {
    mActionButton->setEnabled( !mAction.isEnabledOnlyWhenEditable() || enabled );
  }
}

bool QgsActionWidgetWrapper::valid() const
{
  return mAction.isValid() && mAction.runable();
}

QWidget *QgsActionWidgetWrapper::createWidget( QWidget *parent )
{
  return new QPushButton( parent );
}

void QgsActionWidgetWrapper::initWidget( QWidget *editor )
{

  mActionButton = qobject_cast<QPushButton *>( editor );

  if ( !mActionButton )
    return;

  if ( valid() && layer() )
  {
    const QString shortTitle { mAction.shortTitle() }; // might be empty
    const QString description { mAction.name() };  // mandatory
    const QIcon icon { mAction.icon() };  // might be invalid

    // Configure push button
    if ( ! icon.isNull() )
    {
      mActionButton->setIcon( icon );
      mActionButton->setToolTip( description );
    }
    else
    {
      mActionButton->setText( shortTitle.isEmpty() ? description : shortTitle );
      if ( ! shortTitle.isEmpty() )
      {
        mActionButton->setToolTip( description );
      }
    }

    if ( mAction.isEnabledOnlyWhenEditable() && !layer()->isEditable() )
    {
      mActionButton->setEnabled( false );
    }

    // Always connect
    connect( mActionButton, &QPushButton::clicked, this, [ & ]
    {
      const QgsAttributeEditorContext attributecontext = context();
      QgsExpressionContext expressionContext = layer()->createExpressionContext();
      expressionContext << QgsExpressionContextUtils::formScope( mFeature, attributecontext.attributeFormModeString() );
      expressionContext.setFeature( mFeature );
      mAction.run( layer(), mFeature, expressionContext );
    } );

  }

}

