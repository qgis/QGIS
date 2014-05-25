/***************************************************************************
    qgsattributeform.cpp
     --------------------------------------
    Date                 : 3.5.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributeform.h"

#include "qgsattributeeditor.h"
#include "qgsattributeforminterface.h"
#include "qgsattributeformlegacyinterface.h"
#include "qgseditorwidgetregistry.h"
#include "qgsproject.h"
#include "qgspythonrunner.h"
#include "qgsrelationwidgetwrapper.h"

#include <QDir>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QTabWidget>
#include <QUiLoader>

int QgsAttributeForm::sFormCounter = 0;

QgsAttributeForm::QgsAttributeForm( QgsVectorLayer* vl, const QgsFeature feature, QgsAttributeEditorContext context, QWidget* parent )
    : QWidget( parent )
    , mLayer( vl )
    , mContext( context )
    , mFormNr( sFormCounter++ )
    , mIsSaving( false )
{
  init();
  initPython();
  setFeature( feature );

  connect( vl, SIGNAL( attributeAdded( int ) ), this, SLOT( onAttributeAdded( int ) ) );
  connect( vl, SIGNAL( attributeDeleted( int ) ), this, SLOT( onAttributeDeleted( int ) ) );
}

QgsAttributeForm::~QgsAttributeForm()
{
  cleanPython();
  qDeleteAll( mInterfaces );
}

void QgsAttributeForm::hideButtonBox()
{
  mButtonBox->hide();
}

void QgsAttributeForm::showButtonBox()
{
  mButtonBox->show();
}

void QgsAttributeForm::addInterface( QgsAttributeFormInterface* iface )
{
  mInterfaces.append( iface );
}

bool QgsAttributeForm::editable()
{
  return mFeature.isValid() && mLayer->isEditable() ;
}

void QgsAttributeForm::changeAttribute( const QString& field, const QVariant& value )
{
  Q_FOREACH( QgsWidgetWrapper* ww, mWidgets )
  {
    QgsEditorWidgetWrapper* eww = qobject_cast<QgsEditorWidgetWrapper*>( ww );
    if ( eww && eww->field().name() == field )
    {
      eww->setValue( value );
    }
  }
}

void QgsAttributeForm::setFeature( const QgsFeature& feature )
{
  mFeature = feature;

  resetValues();

  synchronizeEnabledState();

  Q_FOREACH( QgsAttributeFormInterface* iface, mInterfaces )
  {
    iface->featureChanged();
  }
}

bool QgsAttributeForm::save()
{
  if ( mIsSaving )
    return true;

  mIsSaving = true;

  bool success = true;

  emit beforeSave( success );

  // Somebody wants to prevent this form from saving
  if ( !success )
    return false;

  if ( mFeature.isValid() )
  {
    bool doUpdate = false;

    QgsAttributes src = mFeature.attributes();
    QgsAttributes dst = mFeature.attributes();

    Q_FOREACH( QgsWidgetWrapper* ww, mWidgets )
    {
      QgsEditorWidgetWrapper* eww = qobject_cast<QgsEditorWidgetWrapper*>( ww );
      if ( eww )
      {
        QVariant dstVar = dst[eww->fieldIdx()];
        QVariant srcVar = eww->value();
        if ( dstVar != srcVar && srcVar.isValid() )
        {
          dst[eww->fieldIdx()] = eww->value();

          doUpdate = true;
        }
      }
    }

    QgsFeature updatedFeature = QgsFeature( mFeature );
    updatedFeature.setAttributes( dst );

    Q_FOREACH( QgsAttributeFormInterface* iface, mInterfaces )
    {
      if ( !iface->acceptChanges( updatedFeature ) )
      {
        doUpdate = false;
      }
    }

    if ( doUpdate )
    {
      mLayer->beginEditCommand( tr( "Attributes changed" ) );

      for ( int i = 0; i < dst.count(); ++i )
      {
        if ( dst[i] == src[i] || !src[i].isValid() )
          continue;

        success &= mLayer->changeAttributeValue( mFeature.id(), i, dst[i], src[i] );
      }

      if ( success )
      {
        mLayer->endEditCommand();
        mFeature.setAttributes( dst );
      }
      else
      {
        mLayer->destroyEditCommand();
      }
    }
  }

  mIsSaving = false;

  return success;
}

void QgsAttributeForm::resetValues()
{
  Q_FOREACH( QgsWidgetWrapper* ww, mWidgets )
  {
    ww->setFeature( mFeature );
  }
}

void QgsAttributeForm::onAttributeChanged( const QVariant& value )
{
  QgsEditorWidgetWrapper* eww = qobject_cast<QgsEditorWidgetWrapper*>( sender() );

  Q_ASSERT( eww );

  emit attributeChanged( eww->field().name(), value );
}

void QgsAttributeForm::onAttributeAdded( int idx )
{
  if ( mFeature.isValid() )
  {
    QgsAttributes attrs = mFeature.attributes();
    Q_ASSERT( attrs.size() == idx );
    attrs.append( QVariant() );
    mFeature.setFields( &layer()->pendingFields() );
    mFeature.setAttributes( attrs );
  }
  init();
  setFeature( mFeature );
}

void QgsAttributeForm::onAttributeDeleted( int idx )
{
  if ( mFeature.isValid() )
  {
    QgsAttributes attrs = mFeature.attributes();
    attrs.remove( idx );
    mFeature.setFields( &layer()->pendingFields() );
    mFeature.setAttributes( attrs );
  }
  init();
  setFeature( mFeature );
}

void QgsAttributeForm::synchronizeEnabledState()
{
  Q_FOREACH( QgsWidgetWrapper* ww, mWidgets )
  {
    if ( mFeature.isValid() && mLayer->isEditable() )
    {
      ww->setEnabled( true );
    }
    else
    {
      ww->setEnabled( false );
    }
  }

  QPushButton* okButton = mButtonBox->button( QDialogButtonBox::Ok );
  if ( okButton )
    okButton->setEnabled( mFeature.isValid() && mLayer->isEditable() );
}

void QgsAttributeForm::init()
{
  QWidget* formWidget = 0;

  qDeleteAll( mWidgets );
  mWidgets.clear();

  while ( QWidget* w = this->findChild<QWidget*>() )
  {
    delete w;
  }
  delete this->layout();

  // Get a layout
  setLayout( new QGridLayout( this ) );

  // Try to load Ui-File for layout
  if ( mLayer->editorLayout() == QgsVectorLayer::UiFileLayout && !mLayer->editForm().isEmpty() )
  {
    QFile file( mLayer->editForm() );

    if ( file.open( QFile::ReadOnly ) )
    {
      QUiLoader loader;

      QFileInfo fi( mLayer->editForm() );
      loader.setWorkingDirectory( fi.dir() );
      formWidget = loader.load( &file, this );
      layout()->addWidget( formWidget );
      formWidget->show();
      file.close();
      createWrappers();
    }
  }

  // Tab layout
  if ( !formWidget && mLayer->editorLayout() == QgsVectorLayer::TabLayout )
  {
    QTabWidget* tabWidget = new QTabWidget( this );
    layout()->addWidget( tabWidget );

    Q_FOREACH( const QgsAttributeEditorElement *widgDef, mLayer->attributeEditorElements() )
    {
      QWidget* tabPage = new QWidget( tabWidget );

      tabWidget->addTab( tabPage, widgDef->name() );
      QGridLayout *tabPageLayout = new QGridLayout( tabPage );

      if ( widgDef->type() == QgsAttributeEditorElement::AeTypeContainer )
      {
        QString dummy1;
        bool dummy2;
        tabPageLayout->addWidget( createWidgetFromDef( widgDef, tabPage, mLayer, mContext, dummy1, dummy2 ) );
      }
      else
      {
        QgsDebugMsg( "No support for fields in attribute editor on top level" );
      }
    }
    formWidget = tabWidget;
  }

  // Autogenerate Layout
  // If there is still no layout loaded (defined as autogenerate or other methods failed)
  if ( !formWidget )
  {
    formWidget = new QWidget( this );
    QFormLayout* formLayout = new QFormLayout( formWidget );
    formWidget->setLayout( formLayout );
    layout()->addWidget( formWidget );

    Q_FOREACH( const QgsField& field, mLayer->pendingFields().toList() )
    {
      int idx = mLayer->fieldNameIndex( field.name() );
      //show attribute alias if available
      QString fieldName = mLayer->attributeDisplayName( idx );

      const QString widgetType = mLayer->editorWidgetV2( idx );
      const QgsEditorWidgetConfig widgetConfig = mLayer->editorWidgetV2Config( idx );

      // This will also create the widget
      QgsEditorWidgetWrapper* eww = QgsEditorWidgetRegistry::instance()->create( widgetType, mLayer, idx, widgetConfig, 0, this, mContext );
      if ( eww )
      {
        mWidgets.append( eww );
        formLayout->addRow( new QLabel( fieldName ), eww->widget() );
      }
      else
      {
        formLayout->addRow( new QLabel( fieldName ), new QLabel( QString( "<p style=\"color: red; font-style: italic;\">Failed to create widget with type '%1'</p>" ).arg( widgetType ) ) );
      }
    }

    Q_FOREACH( const QgsRelation& rel, QgsProject::instance()->relationManager()->referencedRelations( mLayer ) )
    {
      QgsRelationWidgetWrapper* rww = new QgsRelationWidgetWrapper( mLayer, rel, 0, this );
      rww->setContext( mContext );
      formLayout->addRow( rww->widget() );
      mWidgets.append( rww );
    }
  }

  mButtonBox = findChild<QDialogButtonBox*>();

  if ( !mButtonBox )
  {
    mButtonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
    mButtonBox->setObjectName( "buttonBox" );
    layout()->addWidget( mButtonBox );
  }

  connectWrappers();

  connect( mButtonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( mButtonBox, SIGNAL( rejected() ), this, SLOT( resetValues() ) );

  connect( mLayer, SIGNAL( beforeModifiedCheck() ), this, SLOT( save() ) );
  connect( mLayer, SIGNAL( editingStarted() ), this, SLOT( synchronizeEnabledState() ) );
  connect( mLayer, SIGNAL( editingStopped() ), this, SLOT( synchronizeEnabledState() ) );

  Q_FOREACH( QgsAttributeFormInterface* iface, mInterfaces )
  {
    iface->initForm();
  }
}

void QgsAttributeForm::cleanPython()
{
  if ( !mPyFormVarName.isNull() )
  {
    QString expr = QString( "if locals().has_key('%1'): del %1\n" ).arg( mPyFormVarName );
    QgsPythonRunner::run( expr );
  }
}

void QgsAttributeForm::initPython()
{
  cleanPython();

  // Init Python
  if ( !mLayer->editFormInit().isEmpty() )
  {
    QString module = mLayer->editFormInit();

    int pos = module.lastIndexOf( "." );
    if ( pos >= 0 )
    {
      QgsPythonRunner::run( QString( "import %1" ).arg( module.left( pos ) ) );
    }

    /* Reload the module if the DEBUGMODE switch has been set in the module.
    If set to False you have to reload QGIS to reset it to True due to Python
    module caching */
    QString reload = QString( "if hasattr(%1,'DEBUGMODE') and %1.DEBUGMODE:"
                              " reload(%1)" ).arg( module.left( pos ) );

    QgsPythonRunner::run( reload );

    QgsPythonRunner::run( "import inspect" );
    QString numArgs;
    QgsPythonRunner::eval( QString( "len(inspect.getargspec(%1)[0])" ).arg( module ), numArgs );

    mPyFormVarName = QString( "_qgis_featureform_%1" ).arg( mFormNr );

    QString form = QString( "%1 = sip.wrapinstance( %2, qgis.gui.QgsAttributeForm )" )
                   .arg( mPyFormVarName )
                   .arg(( unsigned long ) this );

    QgsPythonRunner::run( form );

    QgsDebugMsg( QString( "running featureForm init: %1" ).arg( mPyFormVarName ) );

    // Legacy
    if ( numArgs == "3" )
    {
      addInterface( new QgsAttributeFormLegacyInterface( module, mPyFormVarName, this ) );
    }
    else
    {
#if 0
      QString expr = QString( "%1(%2)" )
                     .arg( mLayer->editFormInit() )
                     .arg( mPyFormVarName );
      QgsAttributeFormInterface* iface = QgsPythonRunner::evalToSipObject<QgsAttributeFormInterface*>( expr, "QgsAttributeFormInterface" );
      if ( iface )
        addInterface( iface );
#endif
    }
  }
}

QWidget* QgsAttributeForm::createWidgetFromDef( const QgsAttributeEditorElement* widgetDef, QWidget* parent, QgsVectorLayer* vl, QgsAttributeEditorContext& context, QString& labelText, bool& labelOnTop )
{
  QWidget *newWidget = 0;

  switch ( widgetDef->type() )
  {
    case QgsAttributeEditorElement::AeTypeField:
    {
      const QgsAttributeEditorField* fieldDef = dynamic_cast<const QgsAttributeEditorField*>( widgetDef );
      int fldIdx = fieldDef->idx();
      if ( fldIdx < vl->pendingFields().count() && fldIdx >= 0 )
      {
        const QString widgetType = mLayer->editorWidgetV2( fldIdx );
        const QgsEditorWidgetConfig widgetConfig = mLayer->editorWidgetV2Config( fldIdx );

        QgsEditorWidgetWrapper* eww = QgsEditorWidgetRegistry::instance()->create( widgetType, mLayer, fldIdx, widgetConfig, 0, this, mContext );
        newWidget = eww->widget();
        mWidgets.append( eww );
      }

      labelOnTop = mLayer->labelOnTop( fieldDef->idx() );
      labelText = mLayer->attributeDisplayName( fieldDef->idx() );
      break;
    }

    case QgsAttributeEditorElement::AeTypeRelation:
    {
      const QgsAttributeEditorRelation* relDef = dynamic_cast<const QgsAttributeEditorRelation*>( widgetDef );

      QgsRelationWidgetWrapper* rww = new QgsRelationWidgetWrapper( mLayer, relDef->relation(), 0, this );
      rww->setContext( context );
      newWidget = rww->widget();
      mWidgets.append( rww );
      labelText = QString::null;
      labelOnTop = true;
      break;
    }

    case QgsAttributeEditorElement::AeTypeContainer:
    {
      const QgsAttributeEditorContainer* container = dynamic_cast<const QgsAttributeEditorContainer*>( widgetDef );
      QWidget* myContainer;

      if ( container->isGroupBox() )
      {
        QGroupBox* groupBox = new QGroupBox( parent );
        groupBox->setTitle( container->name() );
        myContainer = groupBox;
        newWidget = myContainer;
      }
      else
      {
        QScrollArea *scrollArea = new QScrollArea( parent );

        myContainer = new QWidget( scrollArea );

        scrollArea->setWidget( myContainer );
        scrollArea->setWidgetResizable( true );
        scrollArea->setFrameShape( QFrame::NoFrame );

        newWidget = scrollArea;
      }

      QGridLayout* gbLayout = new QGridLayout( myContainer );
      myContainer->setLayout( gbLayout );

      int index = 0;

      QList<QgsAttributeEditorElement*> children = container->children();

      Q_FOREACH( QgsAttributeEditorElement* childDef, children )
      {
        QString labelText;
        bool labelOnTop;
        QWidget* editor = createWidgetFromDef( childDef, myContainer, vl, context, labelText, labelOnTop );

        if ( labelText == QString::null )
        {
          gbLayout->addWidget( editor, index, 0, 1, 2 );
        }
        else
        {
          QLabel* mypLabel = new QLabel( labelText );
          if ( labelOnTop )
          {
            gbLayout->addWidget( mypLabel, index, 0, 1, 2 );
            ++index;
            gbLayout->addWidget( editor, index, 0, 1 , 2 );
          }
          else
          {
            gbLayout->addWidget( mypLabel, index, 0 );
            gbLayout->addWidget( editor, index, 1 );
          }
        }

        ++index;
      }
      gbLayout->addItem( new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding ), index , 0 );

      labelText = QString::null;
      labelOnTop = true;
      break;
    }

    default:
      QgsDebugMsg( "Unknown attribute editor widget type encountered..." );
      break;
  }

  return newWidget;
}

void QgsAttributeForm::createWrappers()
{
  QList<QWidget*> myWidgets = findChildren<QWidget*>();
  const QList<QgsField> fields = mLayer->pendingFields().toList();

  Q_FOREACH( QWidget* myWidget, myWidgets )
  {
    // Check the widget's properties for a relation definition
    QVariant vRel = myWidget->property( "qgisRelation" );
    if ( vRel.isValid() )
    {
      QgsRelationManager* relMgr = QgsProject::instance()->relationManager();
      QgsRelation relation = relMgr->relation( vRel.toString() );
      if ( relation.isValid() )
      {
        QgsRelationWidgetWrapper* rww = new QgsRelationWidgetWrapper( mLayer, relation, myWidget, this );
        rww->setConfig( QgsEditorWidgetConfig() );
        rww->setContext( mContext );
        mWidgets.append( rww );
      }
    }
    else
    {
      Q_FOREACH( const QgsField& field, fields )
      {
        if ( field.name() == myWidget->objectName() )
        {
          const QString widgetType = mLayer->editorWidgetV2( field.name() );
          const QgsEditorWidgetConfig widgetConfig = mLayer->editorWidgetV2Config( field.name() );
          int idx = mLayer->fieldNameIndex( field.name() );

          QgsEditorWidgetWrapper* eww = QgsEditorWidgetRegistry::instance()->create( widgetType, mLayer, idx, widgetConfig, myWidget, this, mContext );
          mWidgets.append( eww );
        }
      }
    }
  }
}

void QgsAttributeForm::connectWrappers()
{
  Q_FOREACH( QgsWidgetWrapper* ww, mWidgets )
  {
    QgsEditorWidgetWrapper* eww = qobject_cast<QgsEditorWidgetWrapper*>( ww );

    if ( eww )
      connect( eww, SIGNAL( valueChanged( const QVariant& ) ), this, SLOT( onAttributeChanged( const QVariant& ) ) );
  }
}
