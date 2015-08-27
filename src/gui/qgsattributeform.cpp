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
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QTabWidget>
#include <QUiLoader>

int QgsAttributeForm::sFormCounter = 0;

QgsAttributeForm::QgsAttributeForm( QgsVectorLayer* vl, const QgsFeature &feature, const QgsAttributeEditorContext &context, QWidget* parent )
    : QWidget( parent )
    , mLayer( vl )
    , mContext( context )
    , mButtonBox( 0 )
    , mFormNr( sFormCounter++ )
    , mIsSaving( false )
    , mIsAddDialog( false )
    , mEditCommandMessage( tr( "Attributes changed" ) )
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

  // Make sure that changes are taken into account if somebody tries to figure out if there have been some
  if ( !mIsAddDialog )
    connect( mLayer, SIGNAL( beforeModifiedCheck() ), this, SLOT( save() ) );
}

void QgsAttributeForm::showButtonBox()
{
  mButtonBox->show();

  disconnect( mLayer, SIGNAL( beforeModifiedCheck() ), this, SLOT( save() ) );
}

void QgsAttributeForm::disconnectButtonBox()
{
  disconnect( mButtonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  disconnect( mButtonBox, SIGNAL( rejected() ), this, SLOT( resetValues() ) );
}

void QgsAttributeForm::addInterface( QgsAttributeFormInterface* iface )
{
  mInterfaces.append( iface );
}

bool QgsAttributeForm::editable()
{
  return mFeature.isValid() && mLayer->isEditable();
}

void QgsAttributeForm::setIsAddDialog( bool isAddDialog )
{
  mIsAddDialog = isAddDialog;

  synchronizeEnabledState();
}

void QgsAttributeForm::changeAttribute( const QString& field, const QVariant& value )
{
  Q_FOREACH ( QgsWidgetWrapper* ww, mWidgets )
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

  Q_FOREACH ( QgsAttributeFormInterface* iface, mInterfaces )
  {
    iface->featureChanged();
  }
}

bool QgsAttributeForm::save()
{
  if ( mIsSaving )
    return true;

  mIsSaving = true;

  bool changedLayer = false;

  bool success = true;

  emit beforeSave( success );

  // Somebody wants to prevent this form from saving
  if ( !success )
    return false;

  QgsFeature updatedFeature = QgsFeature( mFeature );

  if ( mFeature.isValid() || mIsAddDialog )
  {
    bool doUpdate = false;

    // An add dialog should perform an action by default
    // and not only if attributes have "changed"
    if ( mIsAddDialog )
      doUpdate = true;

    QgsAttributes src = mFeature.attributes();
    QgsAttributes dst = mFeature.attributes();

    Q_FOREACH ( QgsWidgetWrapper* ww, mWidgets )
    {
      QgsEditorWidgetWrapper* eww = qobject_cast<QgsEditorWidgetWrapper*>( ww );
      if ( eww )
      {
        QVariant dstVar = dst[eww->fieldIdx()];
        QVariant srcVar = eww->value();
        // need to check dstVar.isNull() != srcVar.isNull()
        // otherwise if dstVar=NULL and scrVar=0, then dstVar = srcVar
        if (( dstVar != srcVar || dstVar.isNull() != srcVar.isNull() ) && srcVar.isValid() && mLayer->fieldEditable( eww->fieldIdx() ) )
        {
          dst[eww->fieldIdx()] = srcVar;

          doUpdate = true;
        }
      }
    }

    updatedFeature.setAttributes( dst );

    Q_FOREACH ( QgsAttributeFormInterface* iface, mInterfaces )
    {
      if ( !iface->acceptChanges( updatedFeature ) )
      {
        doUpdate = false;
      }
    }

    if ( doUpdate )
    {
      if ( mIsAddDialog )
      {
        mFeature.setValid( true );
        mLayer->beginEditCommand( mEditCommandMessage );
        bool res = mLayer->addFeature( updatedFeature );
        if ( res )
        {
          mFeature.setAttributes( updatedFeature.attributes() );
          mLayer->endEditCommand();
          mIsAddDialog = false;
          changedLayer = true;
        }
        else
          mLayer->destroyEditCommand();
      }
      else
      {
        mLayer->beginEditCommand( mEditCommandMessage );

        int n = 0;
        for ( int i = 0; i < dst.count(); ++i )
        {
          if (( dst[i] == src[i] && dst[i].isNull() == src[i].isNull() )  // If field is not changed...
              || !dst[i].isValid()                                       // or the widget returns invalid (== do not change)
              || !mLayer->fieldEditable( i ) )                           // or the field cannot be edited ...
          {
            continue;
          }

          QgsDebugMsg( QString( "Updating field %1" ).arg( i ) );
          QgsDebugMsg( QString( "dst:'%1' (type:%2, isNull:%3, isValid:%4)" )
                       .arg( dst[i].toString() ).arg( dst[i].typeName() ).arg( dst[i].isNull() ).arg( dst[i].isValid() ) );
          QgsDebugMsg( QString( "src:'%1' (type:%2, isNull:%3, isValid:%4)" )
                       .arg( src[i].toString() ).arg( src[i].typeName() ).arg( src[i].isNull() ).arg( src[i].isValid() ) );

          success &= mLayer->changeAttributeValue( mFeature.id(), i, dst[i], src[i] );
          n++;
        }

        if ( success && n > 0 )
        {
          mLayer->endEditCommand();
          mFeature.setAttributes( dst );
          changedLayer = true;
        }
        else
        {
          mLayer->destroyEditCommand();
        }
      }
    }
  }

  emit featureSaved( updatedFeature );

  // [MD] Refresh canvas only when absolutely necessary - it interferes with other stuff (#11361).
  // This code should be revisited - and the signals should be fired (+ layer repainted)
  // only when actually doing any changes. I am unsure if it is actually a good idea
  // to call save() whenever some code asks for vector layer's modified status
  // (which is the case when attribute table is open)
  if ( changedLayer )
    mLayer->triggerRepaint();

  mIsSaving = false;

  return success;
}

void QgsAttributeForm::resetValues()
{
  Q_FOREACH ( QgsWidgetWrapper* ww, mWidgets )
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
  Q_UNUSED( idx ) // only used for Q_ASSERT
  if ( mFeature.isValid() )
  {
    QgsAttributes attrs = mFeature.attributes();
    attrs.insert( idx, QVariant( layer()->fields()[idx].type() ) );
    mFeature.setFields( layer()->fields() );
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
    mFeature.setFields( layer()->fields() );
    mFeature.setAttributes( attrs );
  }
  init();
  setFeature( mFeature );
}

void QgsAttributeForm::refreshFeature()
{
  if ( mLayer->isEditable() || !mFeature.isValid() )
    return;

  // reload feature if layer changed although not editable
  // (datasource probably changed bypassing QgsVectorLayer)
  if ( !mLayer->getFeatures( QgsFeatureRequest().setFilterFid( mFeature.id() ) ).nextFeature( mFeature ) )
    return;

  init();
  setFeature( mFeature );
}

void QgsAttributeForm::synchronizeEnabledState()
{
  bool isEditable = ( mFeature.isValid() || mIsAddDialog ) && mLayer->isEditable();

  Q_FOREACH ( QgsWidgetWrapper* ww, mWidgets )
  {
    bool fieldEditable = true;
    QgsEditorWidgetWrapper* eww = qobject_cast<QgsEditorWidgetWrapper*>( ww );
    if ( eww )
    {
      fieldEditable = mLayer->fieldEditable( eww->fieldIdx() );
    }
    ww->setEnabled( isEditable && fieldEditable );
  }

  QPushButton* okButton = mButtonBox->button( QDialogButtonBox::Ok );
  if ( okButton )
    okButton->setEnabled( isEditable );
}

void QgsAttributeForm::init()
{
  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

  // Cleanup of any previously shown widget, we start from scratch
  QWidget* formWidget = 0;

  bool buttonBoxVisible = true;
  // Cleanup button box but preserve visibility
  if ( mButtonBox )
  {
    buttonBoxVisible = mButtonBox->isVisible();
    delete mButtonBox;
    mButtonBox = 0;
  }

  qDeleteAll( mWidgets );
  mWidgets.clear();

  while ( QWidget* w = this->findChild<QWidget*>() )
  {
    delete w;
  }
  delete layout();

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
      formWidget->setWindowFlags( Qt::Widget );
      layout()->addWidget( formWidget );
      formWidget->show();
      file.close();
      mButtonBox = findChild<QDialogButtonBox*>();
      createWrappers();

      formWidget->installEventFilter( this );
    }
  }

  // Tab layout
  if ( !formWidget && mLayer->editorLayout() == QgsVectorLayer::TabLayout )
  {
    QTabWidget* tabWidget = new QTabWidget();
    layout()->addWidget( tabWidget );

    Q_FOREACH ( QgsAttributeEditorElement *widgDef, mLayer->attributeEditorElements() )
    {
      QWidget* tabPage = new QWidget( tabWidget );

      tabWidget->addTab( tabPage, widgDef->name() );
      QGridLayout* tabPageLayout = new QGridLayout();
      tabPage->setLayout( tabPageLayout );

      if ( widgDef->type() == QgsAttributeEditorElement::AeTypeContainer )
      {
        QgsAttributeEditorContainer* containerDef = dynamic_cast<QgsAttributeEditorContainer*>( widgDef );
        if ( !containerDef )
          continue;

        containerDef->setIsGroupBox( false ); // Toplevel widgets are tabs not groupboxes
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
    QGridLayout* gridLayout = new QGridLayout( formWidget );
    formWidget->setLayout( gridLayout );

    // put the form into a scroll area to nicely handle cases with lots of attributes

    QScrollArea* scrollArea = new QScrollArea( this );
    scrollArea->setWidget( formWidget );
    scrollArea->setWidgetResizable( true );
    scrollArea->setFrameShape( QFrame::NoFrame );
    scrollArea->setFrameShadow( QFrame::Plain );
    scrollArea->setFocusProxy( this );
    layout()->addWidget( scrollArea );

    int row = 0;
    Q_FOREACH ( const QgsField& field, mLayer->fields().toList() )
    {
      int idx = mLayer->fieldNameIndex( field.name() );
      if ( idx < 0 )
        continue;

      //show attribute alias if available
      QString fieldName = mLayer->attributeDisplayName( idx );

      const QString widgetType = mLayer->editorWidgetV2( idx );

      if ( widgetType == "Hidden" )
        continue;

      const QgsEditorWidgetConfig widgetConfig = mLayer->editorWidgetV2Config( idx );
      bool labelOnTop = mLayer->labelOnTop( idx );

      // This will also create the widget
      QWidget *l = new QLabel( fieldName );
      QgsEditorWidgetWrapper* eww = QgsEditorWidgetRegistry::instance()->create( widgetType, mLayer, idx, widgetConfig, 0, this, mContext );
      QWidget *w = eww ? eww->widget() : new QLabel( QString( "<p style=\"color: red; font-style: italic;\">Failed to create widget with type '%1'</p>" ).arg( widgetType ) );

      if ( w )
        w->setObjectName( field.name() );

      if ( eww )
        addWidgetWrapper( eww );

      if ( labelOnTop )
      {
        gridLayout->addWidget( l, row++, 0, 1, 2 );
        gridLayout->addWidget( w, row++, 0, 1, 2 );
      }
      else
      {
        gridLayout->addWidget( l, row, 0 );
        gridLayout->addWidget( w, row++, 1 );
      }
    }

    Q_FOREACH ( const QgsRelation& rel, QgsProject::instance()->relationManager()->referencedRelations( mLayer ) )
    {
      QgsRelationWidgetWrapper* rww = new QgsRelationWidgetWrapper( mLayer, rel, 0, this );
      rww->setContext( mContext );
      gridLayout->addWidget( rww->widget(), row++, 0, 1, 2 );
      mWidgets.append( rww );
    }
  }

  if ( !mButtonBox )
  {
    mButtonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
    mButtonBox->setObjectName( "buttonBox" );
    layout()->addWidget( mButtonBox );
  }

  mButtonBox->setVisible( buttonBoxVisible );

  connectWrappers();

  connect( mButtonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( mButtonBox, SIGNAL( rejected() ), this, SLOT( resetValues() ) );

  connect( mLayer, SIGNAL( editingStarted() ), this, SLOT( synchronizeEnabledState() ) );
  connect( mLayer, SIGNAL( editingStopped() ), this, SLOT( synchronizeEnabledState() ) );

  Q_FOREACH ( QgsAttributeFormInterface* iface, mInterfaces )
  {
    iface->initForm();
  }
  QApplication::restoreOverrideCursor();
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

    static int sFormId = 0;
    mPyFormVarName = QString( "_qgis_featureform_%1_%2" ).arg( mFormNr ).arg( sFormId++ );

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

QWidget* QgsAttributeForm::createWidgetFromDef( const QgsAttributeEditorElement *widgetDef, QWidget *parent, QgsVectorLayer *vl, QgsAttributeEditorContext &context, QString &labelText, bool &labelOnTop )
{
  QWidget *newWidget = 0;

  switch ( widgetDef->type() )
  {
    case QgsAttributeEditorElement::AeTypeField:
    {
      const QgsAttributeEditorField* fieldDef = dynamic_cast<const QgsAttributeEditorField*>( widgetDef );
      if ( !fieldDef )
        break;

      int fldIdx = vl->fieldNameIndex( fieldDef->name() );
      if ( fldIdx < vl->fields().count() && fldIdx >= 0 )
      {
        const QString widgetType = mLayer->editorWidgetV2( fldIdx );
        const QgsEditorWidgetConfig widgetConfig = mLayer->editorWidgetV2Config( fldIdx );

        QgsEditorWidgetWrapper* eww = QgsEditorWidgetRegistry::instance()->create( widgetType, mLayer, fldIdx, widgetConfig, 0, this, mContext );
        newWidget = eww->widget();
        addWidgetWrapper( eww );

        newWidget->setObjectName( mLayer->fields()[ fldIdx ].name() );
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
      if ( !container )
        break;

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

      QGridLayout* gbLayout = new QGridLayout();
      myContainer->setLayout( gbLayout );

      int index = 0;

      QList<QgsAttributeEditorElement*> children = container->children();

      Q_FOREACH ( QgsAttributeEditorElement* childDef, children )
      {
        QString labelText;
        bool labelOnTop;
        QWidget* editor = createWidgetFromDef( childDef, myContainer, vl, context, labelText, labelOnTop );

        if ( labelText.isNull() )
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
            gbLayout->addWidget( editor, index, 0, 1, 2 );
          }
          else
          {
            gbLayout->addWidget( mypLabel, index, 0 );
            gbLayout->addWidget( editor, index, 1 );
          }
        }

        ++index;
      }
      QWidget* spacer = new QWidget();
      spacer->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );
      gbLayout->addWidget( spacer, index, 0 );

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

void QgsAttributeForm::addWidgetWrapper( QgsEditorWidgetWrapper* eww )
{
  Q_FOREACH ( QgsWidgetWrapper* ww, mWidgets )
  {
    QgsEditorWidgetWrapper* meww = qobject_cast<QgsEditorWidgetWrapper*>( ww );
    if ( meww )
    {
      if ( meww->field() == eww->field() )
      {
        connect( meww, SIGNAL( valueChanged( QVariant ) ), eww, SLOT( setValue( QVariant ) ) );
        connect( eww, SIGNAL( valueChanged( QVariant ) ), meww, SLOT( setValue( QVariant ) ) );
        break;
      }
    }
  }

  mWidgets.append( eww );
}

void QgsAttributeForm::createWrappers()
{
  QList<QWidget*> myWidgets = findChildren<QWidget*>();
  const QList<QgsField> fields = mLayer->fields().toList();

  Q_FOREACH ( QWidget* myWidget, myWidgets )
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
        rww->widget(); // Will initialize the widget
        mWidgets.append( rww );
      }
    }
    else
    {
      Q_FOREACH ( const QgsField& field, fields )
      {
        if ( field.name() == myWidget->objectName() )
        {
          const QString widgetType = mLayer->editorWidgetV2( field.name() );
          const QgsEditorWidgetConfig widgetConfig = mLayer->editorWidgetV2Config( field.name() );
          int idx = mLayer->fieldNameIndex( field.name() );

          QgsEditorWidgetWrapper* eww = QgsEditorWidgetRegistry::instance()->create( widgetType, mLayer, idx, widgetConfig, myWidget, this, mContext );
          addWidgetWrapper( eww );
        }
      }
    }
  }
}

void QgsAttributeForm::connectWrappers()
{
  bool isFirstEww = true;

  Q_FOREACH ( QgsWidgetWrapper* ww, mWidgets )
  {
    QgsEditorWidgetWrapper* eww = qobject_cast<QgsEditorWidgetWrapper*>( ww );

    if ( eww )
    {
      if ( isFirstEww )
      {
        setFocusProxy( eww->widget() );
        isFirstEww = false;
      }

      connect( eww, SIGNAL( valueChanged( const QVariant& ) ), this, SLOT( onAttributeChanged( const QVariant& ) ) );
    }
  }
}


bool QgsAttributeForm::eventFilter( QObject* object, QEvent* e )
{
  Q_UNUSED( object )

  if ( e->type() == QEvent::KeyPress )
  {
    QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>( e );
    if ( keyEvent && keyEvent->key() == Qt::Key_Escape )
    {
      // Re-emit to this form so it will be forwarded to parent
      event( e );
      return true;
    }
  }

  return false;
}
