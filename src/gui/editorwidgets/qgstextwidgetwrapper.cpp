/***************************************************************************
  qgstextwidgetwrapper.h

 ---------------------
 begin                : 28.12.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstextwidgetwrapper.h"
#include "moc_qgstextwidgetwrapper.cpp"
#include "qgsexpressioncontextutils.h"
#include "qgsattributeform.h"
#include "qgsvaluerelationfieldformatter.h"
#include <QScreen>

QgsTextWidgetWrapper::QgsTextWidgetWrapper( QgsVectorLayer *layer, QWidget *editor, QWidget *parent )
  : QgsWidgetWrapper( layer, editor, parent )
{
  connect( this, &QgsWidgetWrapper::contextChanged, this, &QgsTextWidgetWrapper::updateTextContext );
}

bool QgsTextWidgetWrapper::valid() const
{
  return true;
}

QWidget *QgsTextWidgetWrapper::createWidget( QWidget *parent )
{
  QgsAttributeForm *form = qobject_cast<QgsAttributeForm *>( parent );

  if ( form )
  {
    mFormFeature = form->feature();
    connect( form, &QgsAttributeForm::widgetValueChanged, this, [=]( const QString &attribute, const QVariant &newValue, bool attributeChanged ) {
      if ( attributeChanged )
      {
        if ( mRequiresFormScope )
        {
          mFormFeature.setAttribute( attribute, newValue );
          updateTextContext();
        }
      }
    } );
  }
  return new QLabel( parent );
}

void QgsTextWidgetWrapper::initWidget( QWidget *editor )
{
  mWidget = qobject_cast<QLabel *>( editor );

  if ( !mWidget )
    return;

  mWidget->setText( QgsExpression::replaceExpressionText( mText, &mTextContext ) );
  mWidget->setOpenExternalLinks( true );

  const thread_local QRegularExpression sRegEx { QStringLiteral( "\\[%(.*?)%\\]" ), QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption };

  mNeedsGeometry = false;
  QRegularExpressionMatchIterator matchIt { sRegEx.globalMatch( mText ) };
  while ( !mNeedsGeometry && matchIt.hasNext() )
  {
    const QRegularExpressionMatch match { matchIt.next() };
    const QgsExpression exp { match.captured( 1 ) };
    mNeedsGeometry = exp.needsGeometry();
  }
}

void QgsTextWidgetWrapper::reinitWidget()
{
  if ( !mWidget )
    return;

  initWidget( mWidget );
}


void QgsTextWidgetWrapper::setText( const QString &text )
{
  mText = text;

  bool ok = false;
  const thread_local QRegularExpression sRegEx( QStringLiteral( "\\[%(.*?)%\\]" ), QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption );
  QRegularExpressionMatchIterator matchIt = sRegEx.globalMatch( mText );
  while ( !ok && matchIt.hasNext() )
  {
    const QRegularExpressionMatch match = matchIt.next();
    const QgsExpression exp = match.captured( 1 );
    ok = QgsValueRelationFieldFormatter::expressionRequiresFormScope( exp );
  }
  mRequiresFormScope = ok;

  reinitWidget();
}

bool QgsTextWidgetWrapper::needsGeometry() const
{
  return mNeedsGeometry;
}

void QgsTextWidgetWrapper::updateTextContext()
{
  if ( !mWidget )
    return;

  const QgsAttributeEditorContext attributeContext = context();
  mTextContext = layer()->createExpressionContext();
  mTextContext << QgsExpressionContextUtils::formScope( mFormFeature, attributeContext.attributeFormModeString() );
  if ( attributeContext.parentFormFeature().isValid() )
  {
    mTextContext << QgsExpressionContextUtils::parentFormScope( attributeContext.parentFormFeature() );
  }
  mTextContext.setFeature( mFeature );
  mWidget->setText( QgsExpression::replaceExpressionText( mText, &mTextContext ) );
}

void QgsTextWidgetWrapper::setFeature( const QgsFeature &feature )
{
  if ( !mWidget )
    return;

  mFeature = feature;
  mFormFeature = feature;

  updateTextContext();
}
