/***************************************************************************
    qgsrelationwidgetwrapper.cpp
     --------------------------------------
    Date                 : 14.5.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrelationwidgetwrapper.h"

#include "qgsrelationeditorwidget.h"
#include "qgsattributeeditorcontext.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include <QWidget>

QgsRelationWidgetWrapper::QgsRelationWidgetWrapper( QgsVectorLayer *vl, const QgsRelation &relation, QWidget *editor, QWidget *parent )
  : QgsWidgetWrapper( vl, editor, parent )
  , mRelation( relation )

{
}

QWidget *QgsRelationWidgetWrapper::createWidget( QWidget *parent )
{
  QgsAttributeForm *form = qobject_cast<QgsAttributeForm *>( parent );
  if ( form )
    connect( form, &QgsAttributeForm::widgetValueChanged, this, &QgsRelationWidgetWrapper::widgetValueChanged );

  return new QgsRelationEditorWidget( parent );
}

void QgsRelationWidgetWrapper::setFeature( const QgsFeature &feature )
{
  if ( mWidget && mRelation.isValid() )
    mWidget->setFeature( feature );
}

void QgsRelationWidgetWrapper::setVisible( bool visible )
{
  if ( mWidget )
    mWidget->setVisible( visible );
}

void QgsRelationWidgetWrapper::aboutToSave()
{
  if ( !mRelation.isValid() || !widget() || !widget()->isVisible() || mRelation.referencingLayer() ==  mRelation.referencedLayer() )
    return;

  // If the layer is already saved before, return
  const QgsAttributeEditorContext *ctx = &context();
  do
  {
    if ( ctx->relation().isValid() && ( ctx->relation().referencedLayer() == mRelation.referencingLayer()
                                        || ( mNmRelation.isValid() && ctx->relation().referencedLayer() == mNmRelation.referencedLayer() ) )
       )
    {
      return;
    }
    ctx = ctx->parentContext();
  }
  while ( ctx );

  // Calling isModified() will emit a beforeModifiedCheck()
  // signal that will make the embedded form to send any
  // outstanding widget changes to the edit buffer
  mRelation.referencingLayer()->isModified();

  if ( mNmRelation.isValid() )
    mNmRelation.referencedLayer()->isModified();
}

QgsRelation QgsRelationWidgetWrapper::relation() const
{
  return mRelation;
}

void QgsRelationWidgetWrapper::widgetValueChanged( const QString &attribute, const QVariant &newValue, bool attributeChanged )
{
  if ( mWidget && attributeChanged )
  {
    QgsFeature feature { mWidget->feature() };
    if ( feature.attribute( attribute ) != newValue )
    {
      feature.setAttribute( attribute, newValue );
      QgsAttributeEditorContext newContext { mWidget->editorContext() };
      newContext.setParentFormFeature( feature );
      mWidget->setEditorContext( newContext );
      mWidget->setFeature( feature, false );
      mWidget->parentFormValueChanged( attribute, newValue );
    }
  }
}

bool QgsRelationWidgetWrapper::showUnlinkButton() const
{
  Q_NOWARN_DEPRECATED_PUSH
  return mWidget->showUnlinkButton();
  Q_NOWARN_DEPRECATED_POP
}

void QgsRelationWidgetWrapper::setShowUnlinkButton( bool showUnlinkButton )
{
  Q_NOWARN_DEPRECATED_PUSH
  if ( mWidget )
    mWidget->setShowUnlinkButton( showUnlinkButton );
  Q_NOWARN_DEPRECATED_POP
}

void QgsRelationWidgetWrapper::setShowSaveChildEditsButton( bool showSaveChildEditsButton )
{
  Q_NOWARN_DEPRECATED_PUSH
  if ( mWidget )
    mWidget->setShowSaveChildEditsButton( showSaveChildEditsButton );
  Q_NOWARN_DEPRECATED_POP
}

bool QgsRelationWidgetWrapper::showLabel() const
{
  if ( mWidget )
    return mWidget->showLabel();
  else
    return false;
}

void QgsRelationWidgetWrapper::setShowLabel( bool showLabel )
{
  if ( mWidget )
    mWidget->setShowLabel( showLabel );
}

void QgsRelationWidgetWrapper::initWidget( QWidget *editor )
{
  QgsRelationEditorWidget *w = qobject_cast<QgsRelationEditorWidget *>( editor );

  // if the editor cannot be cast to relation editor, insert a new one
  if ( !w )
  {
    w = new QgsRelationEditorWidget( editor );
    w->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    if ( ! editor->layout() )
    {
      editor->setLayout( new QGridLayout() );
    }
    editor->layout()->addWidget( w );
  }

  QgsAttributeEditorContext myContext( QgsAttributeEditorContext( context(), mRelation, QgsAttributeEditorContext::Multiple, QgsAttributeEditorContext::Embed ) );

  if ( config( QStringLiteral( "force-suppress-popup" ), false ).toBool() )
  {
    const_cast<QgsVectorLayerTools *>( myContext.vectorLayerTools() )->setForceSuppressFormPopup( true );
  }

  /* TODO: this seems to have no effect
  if ( config( QStringLiteral( "hide-save-child-edits" ), false ).toBool() )
  {
    w->setShowSaveChildEditsButton( false );
  }
  */

  w->setEditorContext( myContext );

  mNmRelation = QgsProject::instance()->relationManager()->relation( config( QStringLiteral( "nm-rel" ) ).toString() );

  // If this widget is already embedded by the same relation, reduce functionality
  const QgsAttributeEditorContext *ctx = &context();
  do
  {
    if ( ( ctx->relation().name() == mRelation.name() && ctx->formMode() == QgsAttributeEditorContext::Embed )
         || ( mNmRelation.isValid() && ctx->relation().name() == mNmRelation.name() ) )
    {
      w->setVisible( false );
      break;
    }
    ctx = ctx->parentContext();
  }
  while ( ctx );

  w->setRelations( mRelation, mNmRelation );

  mWidget = w;
}

bool QgsRelationWidgetWrapper::valid() const
{
  return mWidget;
}

bool QgsRelationWidgetWrapper::showLinkButton() const
{
  return visibleButtons().testFlag( QgsAttributeEditorRelation::Button::Link );
}

void QgsRelationWidgetWrapper::setShowLinkButton( bool showLinkButton )
{
  Q_NOWARN_DEPRECATED_PUSH
  if ( mWidget )
    mWidget->setShowLinkButton( showLinkButton );
  Q_NOWARN_DEPRECATED_POP
}

bool QgsRelationWidgetWrapper::showSaveChildEditsButton() const
{
  return visibleButtons().testFlag( QgsAttributeEditorRelation::Button::SaveChildEdits );
}

void QgsRelationWidgetWrapper::setVisibleButtons( const QgsAttributeEditorRelation::Buttons &buttons )
{
  if ( mWidget )
    mWidget->setVisibleButtons( buttons );
}

QgsAttributeEditorRelation::Buttons QgsRelationWidgetWrapper::visibleButtons() const
{
  return mWidget->visibleButtons();
}
