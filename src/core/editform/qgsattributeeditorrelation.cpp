/***************************************************************************
  qgsattributeeditorrelation.cpp - QgsAttributeEditorRelation

 ---------------------
 begin                : 12.01.2021
 copyright            : (C) 2021 by Denis Rouzaud
 email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsattributeeditorrelation.h"
#include "qgsrelationmanager.h"
#include "qgsxmlutils.h"

bool QgsAttributeEditorRelation::init( QgsRelationManager *relationManager )
{
  mRelation = relationManager->relation( mRelationId );
  return mRelation.isValid();
}

QgsAttributeEditorElement *QgsAttributeEditorRelation::clone( QgsAttributeEditorElement *parent ) const
{
  QgsAttributeEditorRelation *element = new QgsAttributeEditorRelation( mRelationId, parent );
  element->mRelation = mRelation;
  element->mButtons = mButtons;
  element->mForceSuppressFormPopup = mForceSuppressFormPopup;
  element->mNmRelationId = mNmRelationId;
  element->mLabel = mLabel;
  element->mRelationEditorConfig = mRelationEditorConfig;
  element->mRelationWidgetTypeId = mRelationWidgetTypeId;

  return element;
}

void QgsAttributeEditorRelation::saveConfiguration( QDomElement &elem, QDomDocument &doc ) const
{
  elem.setAttribute( QStringLiteral( "relation" ), mRelation.id() );
  elem.setAttribute( QStringLiteral( "forceSuppressFormPopup" ), mForceSuppressFormPopup );
  elem.setAttribute( QStringLiteral( "nmRelationId" ), mNmRelationId.toString() );
  elem.setAttribute( QStringLiteral( "label" ), mLabel );
  elem.setAttribute( QStringLiteral( "relationWidgetTypeId" ), mRelationWidgetTypeId );

  QDomElement elemConfig = QgsXmlUtils::writeVariant( mRelationEditorConfig, doc );
  elemConfig.setTagName( QStringLiteral( "editor_configuration" ) );
  elem.appendChild( elemConfig );
}

void QgsAttributeEditorRelation::loadConfiguration( const QDomElement &element, const QString &layerId, const QgsReadWriteContext &context, const QgsFields &fields )
{
  Q_UNUSED( layerId )
  Q_UNUSED( context )
  Q_UNUSED( fields )

  QVariantMap config = QgsXmlUtils::readVariant( element.firstChildElement( "editor_configuration" ) ).toMap();

  // load defaults
  if ( config.isEmpty() )
    config = relationEditorConfiguration();

  // pre QGIS 3.18 compatibility
  if ( ! config.contains( QStringLiteral( "buttons" ) ) )
  {
    if ( element.hasAttribute( "buttons" ) )
    {
      Q_NOWARN_DEPRECATED_PUSH
      // QgsAttributeEditorRelation::Button has been deprecated in favor of QgsRelationEditorWidget::Button
      // we cannot use it here since the new flags are in gui, while the current code is in core
      // TODO: remove this compatibility code in QGIS 4
      //       or make the enum private if we really want to keep the backward compatibility (but not worth it!)
      const QString buttonString = element.attribute( QStringLiteral( "buttons" ), qgsFlagValueToKeys( QgsAttributeEditorRelation::Button::AllButtons ) );
      config.insert( "buttons", qgsFlagValueToKeys( qgsFlagKeysToValue( buttonString, QgsAttributeEditorRelation::Button::AllButtons ) ) );
      Q_NOWARN_DEPRECATED_POP
    }
    else
    {
      // pre QGIS 3.16 compatibility
      Q_NOWARN_DEPRECATED_PUSH
      QgsAttributeEditorRelation::Buttons buttons = QgsAttributeEditorRelation::Button::AllButtons;
      buttons.setFlag( QgsAttributeEditorRelation::Button::Link, element.attribute( QStringLiteral( "showLinkButton" ), QStringLiteral( "1" ) ).toInt() );
      buttons.setFlag( QgsAttributeEditorRelation::Button::Unlink, element.attribute( QStringLiteral( "showUnlinkButton" ), QStringLiteral( "1" ) ).toInt() );
      buttons.setFlag( QgsAttributeEditorRelation::Button::SaveChildEdits, element.attribute( QStringLiteral( "showSaveChildEditsButton" ), QStringLiteral( "1" ) ).toInt() );
      Q_NOWARN_DEPRECATED_POP
      config.insert( "buttons", qgsFlagValueToKeys( buttons ) );
    }
  }

  setRelationEditorConfiguration( config );

  setForceSuppressFormPopup( element.attribute( QStringLiteral( "forceSuppressFormPopup" ), 0 ).toInt() );

  if ( element.hasAttribute( QStringLiteral( "nmRelationId" ) ) )
  {
    setNmRelationId( element.attribute( QStringLiteral( "nmRelationId" ) ) );
  }

  if ( element.hasAttribute( "label" ) )
  {
    const QString label = element.attribute( QStringLiteral( "label" ) );
    setLabel( label );
  }
  if ( element.hasAttribute( "relationWidgetTypeId" ) )
  {
    const QString relationWidgetTypeId = element.attribute( QStringLiteral( "relationWidgetTypeId" ) );
    setRelationWidgetTypeId( relationWidgetTypeId );
  }
}

QString QgsAttributeEditorRelation::typeIdentifier() const
{
  return QStringLiteral( "attributeEditorRelation" );
}

void QgsAttributeEditorRelation::setForceSuppressFormPopup( bool forceSuppressFormPopup )
{
  mForceSuppressFormPopup = forceSuppressFormPopup;
}

bool QgsAttributeEditorRelation::forceSuppressFormPopup() const
{
  return mForceSuppressFormPopup;
}

void QgsAttributeEditorRelation::setNmRelationId( const QVariant &nmRelationId )
{
  mNmRelationId = nmRelationId;
}

QVariant QgsAttributeEditorRelation::nmRelationId() const
{
  return mNmRelationId;
}

void QgsAttributeEditorRelation::setLabel( const QString &label )
{
  mLabel = label;
}

QString QgsAttributeEditorRelation::label() const
{
  return mLabel;
}

QString QgsAttributeEditorRelation::relationWidgetTypeId() const
{
  return mRelationWidgetTypeId;
}

void QgsAttributeEditorRelation::setRelationWidgetTypeId( const QString &relationWidgetTypeId )
{
  mRelationWidgetTypeId = relationWidgetTypeId;
}

QVariantMap QgsAttributeEditorRelation::relationEditorConfiguration() const
{
  return mRelationEditorConfig;
}

void QgsAttributeEditorRelation::setRelationEditorConfiguration( const QVariantMap &config )
{
  mRelationEditorConfig = config;
}
