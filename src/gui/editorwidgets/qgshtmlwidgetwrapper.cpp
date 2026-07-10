/***************************************************************************
  qgshtmlwidgetwrapper.h

 ---------------------
 begin                : 23.03.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgshtmlwidgetwrapper.h"

#include "qgsattributeform.h"
#include "qgsexpressioncontextutils.h"
#include "qgsvaluerelationfieldformatter.h"
#include "qgswebframe.h"

#include <QScreen>
#include <QString>

#include "moc_qgshtmlwidgetwrapper.cpp"

using namespace Qt::StringLiterals;

QgsHtmlWidgetWrapper::QgsHtmlWidgetWrapper( QgsVectorLayer *layer, QWidget *editor, QWidget *parent )
  : QgsWidgetWrapper( layer, editor, parent )
{
  connect( this, &QgsWidgetWrapper::contextChanged, this, &QgsHtmlWidgetWrapper::updateHtmlCode );
}

bool QgsHtmlWidgetWrapper::valid() const
{
  return true;
}

QWidget *QgsHtmlWidgetWrapper::createWidget( QWidget *parent )
{
  QgsAttributeForm *form = qobject_cast<QgsAttributeForm *>( parent );

  if ( form )
  {
    mFormFeature = form->feature();
    connect( form, &QgsAttributeForm::widgetValueChanged, this, [this]( const QString &attribute, const QVariant &newValue, bool attributeChanged ) {
      if ( attributeChanged )
      {
        mFeature.setAttribute( attribute, newValue );
        if ( mRequiresFormScope )
        {
          mFormFeature.setAttribute( attribute, newValue );
        }
        updateHtmlCode();
      }
    } );
  }

  return new QgsWebView( parent );
}

void QgsHtmlWidgetWrapper::initWidget( QWidget *editor )
{
  mWidget = qobject_cast<QgsWebView *>( editor );

  if ( !mWidget )
    return;

  updateHtmlCode();
}

void QgsHtmlWidgetWrapper::reinitWidget()
{
  if ( !mWidget )
    return;

  initWidget( mWidget );
}

void QgsHtmlWidgetWrapper::setHtmlCode( const QString &htmlCode )
{
  mHtmlCode = htmlCode;

  mRequiresFormScope = false;
  mNeedsGeometry = false;

  QgsExpressionContext expressionContext = layer() ? layer()->createExpressionContext() : QgsExpressionContext();
  const thread_local QRegularExpression
    expRe( QStringLiteral( R"re(expression.evaluate\s*\(\s*"(.*)"\))re" ), QRegularExpression::PatternOption::MultilineOption | QRegularExpression::PatternOption::DotMatchesEverythingOption );
  QRegularExpressionMatchIterator matchIt = expRe.globalMatch( mHtmlCode );
  while ( matchIt.hasNext() && ( !mRequiresFormScope || !mNeedsGeometry ) )
  {
    const QRegularExpressionMatch match = matchIt.next();
    QString expression = match.captured( 1 );
    expression = expression.replace( "\\\""_L1, "\""_L1 );

    QgsExpression exp = QgsExpression( expression );
    mRequiresFormScope |= QgsValueRelationFieldFormatter::expressionRequiresFormScope( exp );
    exp.prepare( &expressionContext );
    mNeedsGeometry |= exp.needsGeometry();
  }

  updateHtmlCode();
}

void QgsHtmlWidgetWrapper::updateHtmlCode()
{
  if ( !mWidget )
    return;

  QString htmlCode = mHtmlCode;

  const QgsAttributeEditorContext attributecontext = context();
  QgsExpressionContext expressionContext = layer()->createExpressionContext();
  expressionContext << QgsExpressionContextUtils::formScope( mFormFeature, attributecontext.attributeFormModeString() );
  if ( attributecontext.parentFormFeature().isValid() )
  {
    expressionContext << QgsExpressionContextUtils::parentFormScope( attributecontext.parentFormFeature() );
  }
  expressionContext.setFeature( mFeature );

  const thread_local QRegularExpression
    expRe( QStringLiteral( R"re(<script>\s*document\.write\(\s*expression.evaluate\s*\(\s*"(.*)"\s*\)\s*\)\s*;\s*<\/script>)re" ), QRegularExpression::PatternOption::MultilineOption | QRegularExpression::PatternOption::DotMatchesEverythingOption );
  QRegularExpressionMatch match = expRe.match( htmlCode );
  while ( match.hasMatch() )
  {
    QString expression = match.captured( 1 );
    expression = expression.replace( "\\\""_L1, "\""_L1 );

    QgsExpression exp = QgsExpression( expression );
    exp.prepare( &expressionContext );
    QVariant result = exp.evaluate( &expressionContext );

    QString resultString;
    switch ( static_cast<QMetaType::Type>( result.typeId() ) )
    {
      case QMetaType::Bool:
        resultString = result.toBool() ? u"true"_s : u"false"_s;
        break;
      case QMetaType::Int:
      case QMetaType::UInt:
      case QMetaType::Double:
      case QMetaType::LongLong:
      case QMetaType::ULongLong:
      case QMetaType::QString:
      default:
        resultString = result.toString();
        break;
    }
    htmlCode = htmlCode.mid( 0, match.capturedStart( 0 ) ) + resultString + htmlCode.mid( match.capturedEnd( 0 ) );
    match = expRe.match( htmlCode );
  }

  mWidget->setHtml( htmlCode );
}

void QgsHtmlWidgetWrapper::setFeature( const QgsFeature &feature )
{
  if ( !mWidget )
    return;

  mFeature = feature;
  mFormFeature = feature;
  updateHtmlCode();
}

bool QgsHtmlWidgetWrapper::needsGeometry() const
{
  return mNeedsGeometry;
}
