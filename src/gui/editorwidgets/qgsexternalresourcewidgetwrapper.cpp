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

#include "qgsapplication.h"
#include "qgsexpressioncontextutils.h"
#include "qgsexternalresourcewidget.h"
#include "qgsexternalstoragefilewidget.h"
#include "qgsfilterlineedit.h"
#include "qgsproject.h"

#include <QLabel>
#include <QPushButton>
#include <QSettings>

#include "moc_qgsexternalresourcewidgetwrapper.cpp"

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
      return QgsVariantUtils::createNullVariant( field().type() );
    }
    else
    {
      return mLineEdit->text();
    }
  }

  return QgsVariantUtils::createNullVariant( field().type() );
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

    if ( mPropertyCollection.isActive( QgsEditorWidgetWrapper::Property::RootPath ) )
    {
      const QString path = mPropertyCollection.valueAsString( QgsEditorWidgetWrapper::Property::RootPath, expressionContext, QString(), &ok );
      if ( ok )
      {
        mQgsWidget->setDefaultRoot( path );
      }
    }
    if ( mPropertyCollection.isActive( QgsEditorWidgetWrapper::Property::DocumentViewerContent ) )
    {
      const QString dvcString = mPropertyCollection.valueAsString( QgsEditorWidgetWrapper::Property::DocumentViewerContent, expressionContext, u"NoContent"_s, &ok );
      if ( ok )
      {
        QgsExternalResourceWidget::DocumentViewerContent dvc;
        if ( dvcString.compare( "image"_L1, Qt::CaseInsensitive ) == 0 )
        {
          dvc = QgsExternalResourceWidget::Image;
        }
        else if ( dvcString.compare( "audio"_L1, Qt::CaseInsensitive ) == 0 )
        {
          dvc = QgsExternalResourceWidget::Audio;
        }
        else if ( dvcString.compare( "video"_L1, Qt::CaseInsensitive ) == 0 )
        {
          dvc = QgsExternalResourceWidget::Video;
        }
        else if ( dvcString.compare( "web"_L1, Qt::CaseInsensitive ) == 0 )
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
    mPropertyCollection.loadVariant( cfg.value( u"PropertyCollection"_s ), propertyDefinitions() );

    mQgsWidget->setStorageType( cfg.value( u"StorageType"_s ).toString() );
    mQgsWidget->setStorageAuthConfigId( cfg.value( u"StorageAuthConfigId"_s ).toString() );

    mQgsWidget->fileWidget()->setStorageUrlExpression( mPropertyCollection.isActive( QgsWidgetWrapper::Property::StorageUrl ) ? mPropertyCollection.property( QgsWidgetWrapper::Property::StorageUrl ).asExpression() : QgsExpression::quotedValue( cfg.value( u"StorageUrl"_s ).toString() ) );

    updateFileWidgetExpressionContext();

    if ( cfg.contains( u"UseLink"_s ) )
    {
      mQgsWidget->fileWidget()->setUseLink( cfg.value( u"UseLink"_s ).toBool() );
    }
    if ( cfg.contains( u"FullUrl"_s ) )
    {
      mQgsWidget->fileWidget()->setFullUrl( cfg.value( u"FullUrl"_s ).toBool() );
    }

    if ( !mPropertyCollection.isActive( QgsWidgetWrapper::Property::RootPath ) )
    {
      mQgsWidget->setDefaultRoot( cfg.value( u"DefaultRoot"_s ).toString() );
    }
    if ( cfg.contains( u"StorageMode"_s ) )
    {
      mQgsWidget->fileWidget()->setStorageMode( ( QgsFileWidget::StorageMode ) cfg.value( u"StorageMode"_s ).toInt() );
    }
    if ( cfg.contains( u"RelativeStorage"_s ) )
    {
      mQgsWidget->setRelativeStorage( ( QgsFileWidget::RelativeStorage ) cfg.value( u"RelativeStorage"_s ).toInt() );
    }
    if ( cfg.contains( u"FileWidget"_s ) )
    {
      mQgsWidget->setFileWidgetVisible( cfg.value( u"FileWidget"_s ).toBool() );
    }
    if ( cfg.contains( u"FileWidgetButton"_s ) )
    {
      mQgsWidget->fileWidget()->setFileWidgetButtonVisible( cfg.value( u"FileWidgetButton"_s ).toBool() );
    }
    if ( cfg.contains( u"DocumentViewer"_s ) )
    {
      mQgsWidget->setDocumentViewerContent( ( QgsExternalResourceWidget::DocumentViewerContent ) cfg.value( u"DocumentViewer"_s ).toInt() );
    }
    if ( cfg.contains( u"FileWidgetFilter"_s ) )
    {
      mQgsWidget->fileWidget()->setFilter( cfg.value( u"FileWidgetFilter"_s ).toString() );
    }
    if ( cfg.contains( u"DocumentViewerHeight"_s ) )
    {
      mQgsWidget->setDocumentViewerHeight( cfg.value( u"DocumentViewerHeight"_s ).toInt() );
    }
    if ( cfg.contains( u"DocumentViewerWidth"_s ) )
    {
      mQgsWidget->setDocumentViewerWidth( cfg.value( u"DocumentViewerWidth"_s ).toInt() );
    }
  }

  if ( mLineEdit )
    connect( mLineEdit, &QLineEdit::textChanged, this, [this]( const QString &value ) {
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
    const QgsExpression documentViewerContentExp = QgsExpression( mPropertyCollection.property( QgsEditorWidgetWrapper::Property::DocumentViewerContent ).expressionString() );
    const QgsExpression rootPathExp = QgsExpression( mPropertyCollection.property( QgsEditorWidgetWrapper::Property::RootPath ).expressionString() );

    if ( documentViewerContentExp.referencedColumns().contains( attribute ) || rootPathExp.referencedColumns().contains( attribute ) )
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
          mLineEdit->setStyleSheet( u"QgsFilterLineEdit { background-color: #dd7777; }"_s );
          break;

        case ConstraintResultFailSoft:
          mLineEdit->setStyleSheet( u"QgsFilterLineEdit { background-color: #ffd85d; }"_s );
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
