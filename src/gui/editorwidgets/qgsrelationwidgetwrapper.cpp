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
#include "qgsabstractrelationeditorwidget.h"
#include "qgsrelationwidgetregistry.h"
#include "qgsgui.h"
#include <QWidget>

QgsRelationWidgetWrapper::QgsRelationWidgetWrapper( QgsVectorLayer *vl, const QgsRelation &relation, QWidget *editor, QWidget *parent )
  : QgsRelationWidgetWrapper( QStringLiteral( "relation_editor" ), vl, relation, editor, parent )
{
}

QgsRelationWidgetWrapper::QgsRelationWidgetWrapper( const QString &relationEditorName, QgsVectorLayer *vl, const QgsRelation &relation, QWidget *editor, QWidget *parent )
  : QgsWidgetWrapper( vl, editor, parent )
  , mRelation( relation )
  , mRelationEditorId( relationEditorName )
{
}

QWidget *QgsRelationWidgetWrapper::createWidget( QWidget *parent )
{
  QgsAttributeForm *form = qobject_cast<QgsAttributeForm *>( parent );
  if ( form )
    connect( form, &QgsAttributeForm::widgetValueChanged, this, &QgsRelationWidgetWrapper::widgetValueChanged );

  QgsAbstractRelationEditorWidget *relationEditorWidget = QgsGui::relationWidgetRegistry()->create( mRelationEditorId, widgetConfig(), parent );

  if ( !relationEditorWidget )
  {
    QgsLogger::warning( QStringLiteral( "Failed to create relation widget \"%1\", fallback to \"basic\" relation widget" ).arg( mRelationEditorId ) );
    relationEditorWidget = QgsGui::relationWidgetRegistry()->create( QStringLiteral( "relation_editor" ), widgetConfig(), parent );
  }

  connect( relationEditorWidget, &QgsAbstractRelationEditorWidget::relatedFeaturesChanged, this, &QgsRelationWidgetWrapper::relatedFeaturesChanged );

  return relationEditorWidget;
}

void QgsRelationWidgetWrapper::setFeature( const QgsFeature &feature )
{
  if ( mWidget && mRelation.isValid() )
    mWidget->setFeature( feature );
}

void QgsRelationWidgetWrapper::setMultiEditFeatureIds( const QgsFeatureIds &fids )
{
  if ( mWidget && mRelation.isValid() )
    mWidget->setMultiEditFeatureIds( fids );
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
  return visibleButtons().testFlag( QgsAttributeEditorRelation::Button::Unlink );
  Q_NOWARN_DEPRECATED_POP
}

void QgsRelationWidgetWrapper::setShowUnlinkButton( bool showUnlinkButton )
{
  Q_NOWARN_DEPRECATED_PUSH
  setVisibleButtons( visibleButtons().setFlag( QgsAttributeEditorRelation::Unlink, showUnlinkButton ) );
  Q_NOWARN_DEPRECATED_POP
}

void QgsRelationWidgetWrapper::setShowSaveChildEditsButton( bool showSaveChildEditsButton )
{
  Q_NOWARN_DEPRECATED_PUSH
  setVisibleButtons( visibleButtons().setFlag( QgsAttributeEditorRelation::SaveChildEdits, showSaveChildEditsButton ) );
  Q_NOWARN_DEPRECATED_POP
}

bool QgsRelationWidgetWrapper::showLabel() const
{
  return false;
}

void QgsRelationWidgetWrapper::setShowLabel( bool showLabel )
{
  Q_UNUSED( showLabel )
}

void QgsRelationWidgetWrapper::initWidget( QWidget *editor )
{
  QgsAbstractRelationEditorWidget *w = qobject_cast<QgsAbstractRelationEditorWidget *>( editor );

  // if the editor cannot be cast to relation editor, insert a new one
  if ( !w )
  {
    w = QgsGui::relationWidgetRegistry()->create( mRelationEditorId, widgetConfig(), editor );
    editor->layout()->addWidget( w );
  }

  const QgsAttributeEditorContext myContext( QgsAttributeEditorContext( context(), mRelation, QgsAttributeEditorContext::Multiple, QgsAttributeEditorContext::Embed ) );

  // read the legacy config of force-suppress-popup to support settings made on autoconfigurated forms
  // it will be overwritten on specific widget configuration
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

  // read the legacy config of nm-rel to support settings made on autoconfigurated forms
  // it will be overwritten on specific widget configuration
  mNmRelation = QgsProject::instance()->relationManager()->relation( config( QStringLiteral( "nm-rel" ) ).toString() );

  // If this widget is already embedded by the same relation, reduce functionality
  const QgsAttributeEditorContext *ctx = &context();
  do
  {
    if ( ( ctx->relation().id() == mRelation.id() && ctx->formMode() == QgsAttributeEditorContext::Embed )
         || ( mNmRelation.isValid() && ctx->relation().id() == mNmRelation.id() ) )
    {
      w->setVisible( false );
      break;
    }
    ctx = ctx->parentContext();
  }
  while ( ctx );

  w->setEditorContext( myContext );
  w->setRelations( mRelation, mNmRelation );

  mWidget = w;
}

bool QgsRelationWidgetWrapper::valid() const
{
  return mWidget;
}

bool QgsRelationWidgetWrapper::showLinkButton() const
{
  Q_NOWARN_DEPRECATED_PUSH
  return visibleButtons().testFlag( QgsAttributeEditorRelation::Button::Link );
  Q_NOWARN_DEPRECATED_POP
}

void QgsRelationWidgetWrapper::setShowLinkButton( bool showLinkButton )
{
  Q_NOWARN_DEPRECATED_PUSH
  setVisibleButtons( visibleButtons().setFlag( QgsAttributeEditorRelation::Link, showLinkButton ) );
  Q_NOWARN_DEPRECATED_POP
}

bool QgsRelationWidgetWrapper::showSaveChildEditsButton() const
{
  Q_NOWARN_DEPRECATED_PUSH
  return visibleButtons().testFlag( QgsAttributeEditorRelation::Button::SaveChildEdits );
  Q_NOWARN_DEPRECATED_POP
}

void QgsRelationWidgetWrapper::setVisibleButtons( const QgsAttributeEditorRelation::Buttons &buttons )
{
  if ( ! mWidget )
    return;
  QVariantMap config = mWidget->config();
  config.insert( "buttons", qgsFlagValueToKeys( buttons ) );

  mWidget->setConfig( config );
}

QgsAttributeEditorRelation::Buttons QgsRelationWidgetWrapper::visibleButtons() const
{
  return qgsFlagKeysToValue( mWidget->config().value( QStringLiteral( "buttons" ) ).toString(), QgsAttributeEditorRelation::AllButtons );
}

void QgsRelationWidgetWrapper::setForceSuppressFormPopup( bool forceSuppressFormPopup )
{
  if ( mWidget )
  {
    mWidget->setForceSuppressFormPopup( forceSuppressFormPopup );
    //it's set to true if one widget is configured like this but the setting is done generally (influencing all widgets).
    const_cast<QgsVectorLayerTools *>( mWidget->editorContext().vectorLayerTools() )->setForceSuppressFormPopup( forceSuppressFormPopup );
  }
}

bool QgsRelationWidgetWrapper::forceSuppressFormPopup() const
{
  if ( mWidget )
    return mWidget->forceSuppressFormPopup();

  return false;
}

void QgsRelationWidgetWrapper::setNmRelationId( const QVariant &nmRelationId )
{
  if ( mWidget )
  {
    mNmRelation = QgsProject::instance()->relationManager()->relation( nmRelationId.toString() );

    // If this widget is already embedded by the same relation, reduce functionality
    const QgsAttributeEditorContext *ctx = &context();
    while ( ctx && ctx->relation().isValid() )
    {
      if ( ( ctx->relation().id() == mRelation.id() && ctx->formMode() == QgsAttributeEditorContext::Embed )
           || ( mNmRelation.isValid() && ctx->relation().id() == mNmRelation.id() ) )
      {
        mWidget->setVisible( false );
        break;
      }
      ctx = ctx->parentContext();
    }

    mWidget->setRelations( mRelation, mNmRelation );
  }
}

QVariant QgsRelationWidgetWrapper::nmRelationId() const
{
  if ( mWidget )
    return mWidget->nmRelationId();
  return QVariant();
}


void QgsRelationWidgetWrapper::setLabel( const QString &label )
{
  Q_UNUSED( label )
}

QString QgsRelationWidgetWrapper::label() const
{
  return QString();
}

void QgsRelationWidgetWrapper::setWidgetConfig( const QVariantMap &config )
{
  if ( mWidget )
    mWidget->setConfig( config );
}

QVariantMap QgsRelationWidgetWrapper::widgetConfig() const
{
  return mWidget ? mWidget->config() : QVariantMap();
}
