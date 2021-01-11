/***************************************************************************
  qgsattributeeditorelement.cpp - QgsAttributeEditorElement

 ---------------------
 begin                : 18.8.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsattributeeditorelement.h"
#include "qgsrelationmanager.h"
#include "qgsxmlutils.h"


void QgsAttributeEditorContainer::addChildElement( QgsAttributeEditorElement *widget )
{
  mChildren.append( widget );
}

void QgsAttributeEditorContainer::setName( const QString &name )
{
  mName = name;
}

QgsOptionalExpression QgsAttributeEditorContainer::visibilityExpression() const
{
  return mVisibilityExpression;
}

void QgsAttributeEditorContainer::setVisibilityExpression( const QgsOptionalExpression &visibilityExpression )
{
  if ( visibilityExpression == mVisibilityExpression )
    return;

  mVisibilityExpression = visibilityExpression;
}

QColor QgsAttributeEditorContainer::backgroundColor() const
{
  return mBackgroundColor;
}

void QgsAttributeEditorContainer::setBackgroundColor( const QColor &backgroundColor )
{
  mBackgroundColor = backgroundColor;
}

QList<QgsAttributeEditorElement *> QgsAttributeEditorContainer::findElements( QgsAttributeEditorElement::AttributeEditorType type ) const
{
  QList<QgsAttributeEditorElement *> results;

  const auto constMChildren = mChildren;
  for ( QgsAttributeEditorElement *elem : constMChildren )
  {
    if ( elem->type() == type )
    {
      results.append( elem );
    }

    if ( elem->type() == AeTypeContainer )
    {
      QgsAttributeEditorContainer *cont = dynamic_cast<QgsAttributeEditorContainer *>( elem );
      if ( cont )
        results += cont->findElements( type );
    }
  }

  return results;
}

void QgsAttributeEditorContainer::clear()
{
  qDeleteAll( mChildren );
  mChildren.clear();
}

int QgsAttributeEditorContainer::columnCount() const
{
  return mColumnCount;
}

void QgsAttributeEditorContainer::setColumnCount( int columnCount )
{
  mColumnCount = columnCount;
}

QgsAttributeEditorElement *QgsAttributeEditorContainer::clone( QgsAttributeEditorElement *parent ) const
{
  QgsAttributeEditorContainer *element = new QgsAttributeEditorContainer( name(), parent );

  const auto childElements = children();

  for ( QgsAttributeEditorElement *child : childElements )
  {
    element->addChildElement( child->clone( element ) );
  }
  element->mIsGroupBox = mIsGroupBox;
  element->mColumnCount = mColumnCount;
  element->mVisibilityExpression = mVisibilityExpression;

  return element;
}

void QgsAttributeEditorContainer::saveConfiguration( QDomElement &elem, QDomDocument &doc ) const
{
  Q_UNUSED( doc )
  elem.setAttribute( QStringLiteral( "columnCount" ), mColumnCount );
  elem.setAttribute( QStringLiteral( "groupBox" ), mIsGroupBox ? 1 : 0 );
  elem.setAttribute( QStringLiteral( "visibilityExpressionEnabled" ), mVisibilityExpression.enabled() ? 1 : 0 );
  elem.setAttribute( QStringLiteral( "visibilityExpression" ), mVisibilityExpression->expression() );
  if ( mBackgroundColor.isValid() )
    elem.setAttribute( QStringLiteral( "backgroundColor" ), mBackgroundColor.name( ) );
  const auto constMChildren = mChildren;
  for ( QgsAttributeEditorElement *child : constMChildren )
  {
    QDomDocument doc = elem.ownerDocument();
    elem.appendChild( child->toDomElement( doc ) );
  }
}

QString QgsAttributeEditorContainer::typeIdentifier() const
{
  return QStringLiteral( "attributeEditorContainer" );
}


QgsAttributeEditorElement *QgsAttributeEditorField::clone( QgsAttributeEditorElement *parent ) const
{
  QgsAttributeEditorField *element = new QgsAttributeEditorField( name(), mIdx, parent );

  return element;
}

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

  return element;
}
void QgsAttributeEditorField::saveConfiguration( QDomElement &elem, QDomDocument &doc ) const
{
  Q_UNUSED( doc )
  elem.setAttribute( QStringLiteral( "index" ), mIdx );
}

QString QgsAttributeEditorField::typeIdentifier() const
{
  return QStringLiteral( "attributeEditorField" );
}

QDomElement QgsAttributeEditorElement::toDomElement( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( typeIdentifier() );
  elem.setAttribute( QStringLiteral( "name" ), mName );
  elem.setAttribute( QStringLiteral( "showLabel" ), mShowLabel );
  saveConfiguration( elem, doc );
  return elem;
}

bool QgsAttributeEditorElement::showLabel() const
{
  return mShowLabel;
}

void QgsAttributeEditorElement::setShowLabel( bool showLabel )
{
  mShowLabel = showLabel;
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

QgsAttributeEditorElement *QgsAttributeEditorQmlElement::clone( QgsAttributeEditorElement *parent ) const
{
  QgsAttributeEditorQmlElement *element = new QgsAttributeEditorQmlElement( name(), parent );
  element->setQmlCode( mQmlCode );

  return element;
}

QString QgsAttributeEditorQmlElement::qmlCode() const
{
  return mQmlCode;
}

void QgsAttributeEditorQmlElement::setQmlCode( const QString &qmlCode )
{
  mQmlCode = qmlCode;
}

void QgsAttributeEditorQmlElement::saveConfiguration( QDomElement &elem, QDomDocument &doc ) const
{
  QDomText codeElem = doc.createTextNode( mQmlCode );
  elem.appendChild( codeElem );
}

QString QgsAttributeEditorQmlElement::typeIdentifier() const
{
  return QStringLiteral( "attributeEditorQmlElement" );
}

QgsAttributeEditorElement *QgsAttributeEditorHtmlElement::clone( QgsAttributeEditorElement *parent ) const
{
  QgsAttributeEditorHtmlElement *element = new QgsAttributeEditorHtmlElement( name(), parent );
  element->setHtmlCode( mHtmlCode );

  return element;
}

QString QgsAttributeEditorHtmlElement::htmlCode() const
{
  return mHtmlCode;
}

void QgsAttributeEditorHtmlElement::setHtmlCode( const QString &htmlCode )
{
  mHtmlCode = htmlCode;
}

void QgsAttributeEditorHtmlElement::saveConfiguration( QDomElement &elem, QDomDocument &doc ) const
{
  QDomText codeElem = doc.createTextNode( mHtmlCode );
  elem.appendChild( codeElem );
}

QString QgsAttributeEditorHtmlElement::typeIdentifier() const
{
  return QStringLiteral( "attributeEditorHtmlElement" );
}

