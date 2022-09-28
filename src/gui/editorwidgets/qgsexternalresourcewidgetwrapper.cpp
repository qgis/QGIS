/***************************************************************************
    qgsexternalresourcewidgetwrapper.cpp
     --------------------------------------
 begin                : 16.12.2015
 copyright            : (C) 2015 by Denis Rouzaud
 email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexternalresourcewidgetwrapper.h"

#include <QPushButton>
#include <QSettings>
#include <QLabel>

#include "qgsproject.h"
#include "qgsexternalresourcewidget.h"
#include "qgsfilterlineedit.h"
#include "qgsapplication.h"
#include "qgsexpressioncontextutils.h"
#include "qgsexternalstoragefilewidget.h"


QgsExternalResourceWidgetWrapper::QgsExternalResourceWidgetWrapper( QgsVectorLayer *layer, int fieldIdx, QWidget *editor, QgsMessageBar *messageBar, QWidget *parent )
  : QgsEditorWidgetWrapper( layer, fieldIdx, editor, parent )
  , mMessageBar( messageBar )
{
}

QVariant QgsExternalResourceWidgetWrapper::value() const
{
  if ( mQgsWidget )
  {
    return mQgsWidget->documentPath( field().type() );
  }

  if ( mLineEdit )
  {
    if ( mLineEdit->text().isEmpty() || mLineEdit->text() == QgsApplication::nullRepresentation() )
    {
      return QVariant( field().type() );
    }
    else
    {
      return mLineEdit->text();
    }
  }

  return QVariant( field().type() );
}

void QgsExternalResourceWidgetWrapper::showIndeterminateState()
{
  if ( mLineEdit )
  {
    whileBlocking( mLineEdit )->clear();
  }

  if ( mLabel )
  {
    mLabel->clear();
  }

  if ( mQgsWidget )
  {
    whileBlocking( mQgsWidget )->setDocumentPath( QString() );
  }
}

bool QgsExternalResourceWidgetWrapper::valid() const
{
  return mLineEdit || mLabel || mQgsWidget;
}

void QgsExternalResourceWidgetWrapper::updateProperties( const QgsFeature &feature )
{
  if ( mQgsWidget && mPropertyCollection.hasActiveProperties() )
  {
    QgsExpressionContext expressionContext( QgsExpressionContextUtils::globalProjectLayerScopes( layer() ) );
    expressionContext.setFeature( feature );
    bool ok = false;

    if ( mPropertyCollection.isActive( QgsEditorWidgetWrapper::RootPath ) )
    {
      const QString path = mPropertyCollection.valueAsString( QgsEditorWidgetWrapper::RootPath, expressionContext, QString(), &ok );
      if ( ok )
      {
        mQgsWidget->setDefaultRoot( path );
      }
    }
    if ( mPropertyCollection.isActive( QgsEditorWidgetWrapper::DocumentViewerContent ) )
    {
      const QString dvcString = mPropertyCollection.valueAsString( QgsEditorWidgetWrapper::DocumentViewerContent, expressionContext, QStringLiteral( "NoContent" ), &ok );
      if ( ok )
      {
        QgsExternalResourceWidget::DocumentViewerContent dvc;
        if ( dvcString.compare( QLatin1String( "image" ), Qt::CaseInsensitive ) == 0 )
        {
          dvc = QgsExternalResourceWidget::Image;
        }
        else if ( dvcString.compare( QLatin1String( "web" ), Qt::CaseInsensitive ) == 0 )
        {
          dvc = QgsExternalResourceWidget::Web;
        }
        else
        {
          dvc = QgsExternalResourceWidget::NoContent;
        }
        mQgsWidget->setDocumentViewerContent( dvc );
      }
    }
  }
}

void QgsExternalResourceWidgetWrapper::setFeature( const QgsFeature &feature )
{
  updateProperties( feature );
  QgsEditorWidgetWrapper::setFeature( feature );

  if ( mQgsWidget )
  {
    updateFileWidgetExpressionContext();
  }
}

QWidget *QgsExternalResourceWidgetWrapper::createWidget( QWidget *parent )
{
  mForm = qobject_cast<QgsAttributeForm *>( parent );

  if ( mForm )
    connect( mForm, &QgsAttributeForm::widgetValueChanged, this, &QgsExternalResourceWidgetWrapper::widgetValueChanged );

  return new QgsExternalResourceWidget( parent );
}

void QgsExternalResourceWidgetWrapper::initWidget( QWidget *editor )
{
  mLineEdit = qobject_cast<QLineEdit *>( editor );
  mLabel = qobject_cast<QLabel *>( editor );
  mQgsWidget = qobject_cast<QgsExternalResourceWidget *>( editor );

  if ( mLineEdit )
  {
    QgsFilterLineEdit *fle = qobject_cast<QgsFilterLineEdit *>( editor );
    if ( fle )
    {
      fle->setNullValue( QgsApplication::nullRepresentation() );
    }
  }
  else
    mLineEdit = editor->findChild<QLineEdit *>();

  if ( mQgsWidget )
  {
    mQgsWidget->setMessageBar( mMessageBar );

    mQgsWidget->fileWidget()->setStorageMode( QgsFileWidget::GetFile );

    const QVariantMap cfg = config();
    mPropertyCollection.loadVariant( cfg.value( QStringLiteral( "PropertyCollection" ) ), propertyDefinitions() );

    mQgsWidget->setStorageType( cfg.value( QStringLiteral( "StorageType" ) ).toString() );
    mQgsWidget->setStorageAuthConfigId( cfg.value( QStringLiteral( "StorageAuthConfigId" ) ).toString() );

    mQgsWidget->fileWidget()->setStorageUrlExpression( mPropertyCollection.isActive( QgsWidgetWrapper::StorageUrl ) ?
        mPropertyCollection.property( QgsWidgetWrapper::StorageUrl ).asExpression() :
        QgsExpression::quotedValue( cfg.value( QStringLiteral( "StorageUrl" ) ).toString() ) );

    updateFileWidgetExpressionContext();

    if ( cfg.contains( QStringLiteral( "UseLink" ) ) )
    {
      mQgsWidget->fileWidget()->setUseLink( cfg.value( QStringLiteral( "UseLink" ) ).toBool() );
    }
    if ( cfg.contains( QStringLiteral( "FullUrl" ) ) )
    {
      mQgsWidget->fileWidget()->setFullUrl( cfg.value( QStringLiteral( "FullUrl" ) ).toBool() );
    }

    if ( !mPropertyCollection.isActive( QgsWidgetWrapper::RootPath ) )
    {
      mQgsWidget->setDefaultRoot( cfg.value( QStringLiteral( "DefaultRoot" ) ).toString() );
    }
    if ( cfg.contains( QStringLiteral( "StorageMode" ) ) )
    {
      mQgsWidget->fileWidget()->setStorageMode( ( QgsFileWidget::StorageMode )cfg.value( QStringLiteral( "StorageMode" ) ).toInt() );
    }
    if ( cfg.contains( QStringLiteral( "RelativeStorage" ) ) )
    {
      mQgsWidget->setRelativeStorage( ( QgsFileWidget::RelativeStorage )cfg.value( QStringLiteral( "RelativeStorage" ) ).toInt() );
    }
    if ( cfg.contains( QStringLiteral( "FileWidget" ) ) )
    {
      mQgsWidget->setFileWidgetVisible( cfg.value( QStringLiteral( "FileWidget" ) ).toBool() );
    }
    if ( cfg.contains( QStringLiteral( "FileWidgetButton" ) ) )
    {
      mQgsWidget->fileWidget()->setFileWidgetButtonVisible( cfg.value( QStringLiteral( "FileWidgetButton" ) ).toBool() );
    }
    if ( cfg.contains( QStringLiteral( "DocumentViewer" ) ) )
    {
      mQgsWidget->setDocumentViewerContent( ( QgsExternalResourceWidget::DocumentViewerContent )cfg.value( QStringLiteral( "DocumentViewer" ) ).toInt() );
    }
    if ( cfg.contains( QStringLiteral( "FileWidgetFilter" ) ) )
    {
      mQgsWidget->fileWidget()->setFilter( cfg.value( QStringLiteral( "FileWidgetFilter" ) ).toString() );
    }
    if ( cfg.contains( QStringLiteral( "DocumentViewerHeight" ) ) )
    {
      mQgsWidget->setDocumentViewerHeight( cfg.value( QStringLiteral( "DocumentViewerHeight" ) ).toInt( ) );
    }
    if ( cfg.contains( QStringLiteral( "DocumentViewerWidth" ) ) )
    {
      mQgsWidget->setDocumentViewerWidth( cfg.value( QStringLiteral( "DocumentViewerWidth" ) ).toInt( ) );
    }
  }

  if ( mLineEdit )
    connect( mLineEdit, &QLineEdit::textChanged, this, [ = ]( const QString & value )
  {
    Q_NOWARN_DEPRECATED_PUSH
    emit valueChanged( value );
    Q_NOWARN_DEPRECATED_POP
    emit valuesChanged( value );
  } );

}

void QgsExternalResourceWidgetWrapper::updateValues( const QVariant &value, const QVariantList & )
{
  if ( mLineEdit )
  {
    if ( QgsVariantUtils::isNull( value ) )
    {
      mLineEdit->setText( QgsApplication::nullRepresentation() );
    }
    else
    {
      mLineEdit->setText( value.toString() );
    }
  }

  if ( mLabel )
  {
    mLabel->setText( value.toString() );
    Q_NOWARN_DEPRECATED_PUSH
    emit valueChanged( value.toString() );
    Q_NOWARN_DEPRECATED_POP
    emit valuesChanged( value.toString() ); // emit signal that value has changed, do not do it for other widgets
  }

  if ( mQgsWidget )
  {
    if ( QgsVariantUtils::isNull( value ) )
    {
      mQgsWidget->setDocumentPath( QgsApplication::nullRepresentation() );
    }
    else
    {
      mQgsWidget->setDocumentPath( value.toString() );
    }
  }
}

void QgsExternalResourceWidgetWrapper::setEnabled( bool enabled )
{
  if ( mLineEdit )
    mLineEdit->setReadOnly( !enabled );

  if ( mQgsWidget )
    mQgsWidget->setReadOnly( !enabled );
}

void QgsExternalResourceWidgetWrapper::widgetValueChanged( const QString &attribute, const QVariant &newValue, bool attributeChanged )
{
  Q_UNUSED( newValue );
  if ( attributeChanged )
  {
    const QgsExpression documentViewerContentExp = QgsExpression( mPropertyCollection.property( QgsEditorWidgetWrapper::DocumentViewerContent ).expressionString() );
    const QgsExpression rootPathExp = QgsExpression( mPropertyCollection.property( QgsEditorWidgetWrapper::RootPath ).expressionString() );

    if ( documentViewerContentExp.referencedColumns().contains( attribute ) ||
         rootPathExp.referencedColumns().contains( attribute ) )
    {
      const QgsFeature feature = mForm->currentFormFeature();
      updateProperties( feature );
    }
  }
}

void QgsExternalResourceWidgetWrapper::updateConstraintWidgetStatus()
{
  if ( mLineEdit )
  {
    if ( !constraintResultVisible() )
    {
      widget()->setStyleSheet( QString() );
    }
    else
    {
      switch ( constraintResult() )
      {
        case ConstraintResultPass:
          mLineEdit->setStyleSheet( QString() );
          break;

        case ConstraintResultFailHard:
          mLineEdit->setStyleSheet( QStringLiteral( "QgsFilterLineEdit { background-color: #dd7777; }" ) );
          break;

        case ConstraintResultFailSoft:
          mLineEdit->setStyleSheet( QStringLiteral( "QgsFilterLineEdit { background-color: #ffd85d; }" ) );
          break;
      }
    }
  }
}

void QgsExternalResourceWidgetWrapper::updateFileWidgetExpressionContext()
{
  if ( !mQgsWidget || !layer() )
    return;

  QgsExpressionContext expressionContext( layer()->createExpressionContext() );
  expressionContext.setFeature( formFeature() );
  expressionContext.appendScope( QgsExpressionContextUtils::formScope( formFeature() ) );
  if ( context().parentFormFeature().isValid() )
  {
    expressionContext.appendScope( QgsExpressionContextUtils::parentFormScope( context().parentFormFeature() ) );
  }

  mQgsWidget->fileWidget()->setExpressionContext( expressionContext );
}
