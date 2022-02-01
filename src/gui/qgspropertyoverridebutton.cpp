/***************************************************************************
     qgspropertyoverridebutton.cpp
     -----------------------------
    Date                 : January 2017
    Copyright            : (C) 2017 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspropertyoverridebutton.h"

#include "qgsapplication.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsexpression.h"
#include "qgsmessageviewer.h"
#include "qgsvectorlayer.h"
#include "qgspanelwidget.h"
#include "qgspropertyassistantwidget.h"
#include "qgsauxiliarystorage.h"
#include "qgscolorschemeregistry.h"
#include "qgscolorbutton.h"
#include "qgsguiutils.h"

#include <QClipboard>
#include <QMenu>
#include <QMouseEvent>
#include <QPointer>
#include <QGroupBox>
#include <QRegularExpression>

QgsPropertyOverrideButton::QgsPropertyOverrideButton( QWidget *parent,
    const QgsVectorLayer *layer )
  : QToolButton( parent )
  , mVectorLayer( layer )

{
  setFocusPolicy( Qt::StrongFocus );

  int iconSize = QgsGuiUtils::scaleIconSize( 24 );

  // button width is 1.25 * icon size, height 1.1 * icon size. But we round to ensure even pixel sizes for equal margins
  setFixedSize( 2 * static_cast< int >( 1.25 * iconSize / 2.0 ), 2 * static_cast< int >( iconSize * 1.1 / 2.0 ) );

  setIconSize( QSize( iconSize, iconSize ) );
  setPopupMode( QToolButton::InstantPopup );

  connect( this, &QgsPropertyOverrideButton::activated, this, &QgsPropertyOverrideButton::updateSiblingWidgets );

  mDefineMenu = new QMenu( this );
  connect( mDefineMenu, &QMenu::aboutToShow, this, &QgsPropertyOverrideButton::aboutToShowMenu );
  connect( mDefineMenu, &QMenu::triggered, this, &QgsPropertyOverrideButton::menuActionTriggered );
  setMenu( mDefineMenu );

  mFieldsMenu = new QMenu( this );
  mActionDataTypes = new QAction( this );
  // list fields and types in submenu, since there may be many
  mActionDataTypes->setMenu( mFieldsMenu );

  mActionVariables = new QAction( tr( "Variable" ), this );
  mVariablesMenu = new QMenu( this );
  mActionVariables->setMenu( mVariablesMenu );

  mActionColors = new QAction( tr( "Color" ), this );
  mColorsMenu = new QMenu( this );
  mActionColors->setMenu( mColorsMenu );

  mActionActive = new QAction( this );
  QFont f = mActionActive->font();
  f.setBold( true );
  mActionActive->setFont( f );

  mActionDescription = new QAction( tr( "Description…" ), this );

  mActionCreateAuxiliaryField = new QAction( tr( "Store Data in the Project" ), this );
  mActionCreateAuxiliaryField->setCheckable( true );

  mActionExpDialog = new QAction( tr( "Edit…" ), this );
  mActionExpression = nullptr;
  mActionPasteExpr = new QAction( tr( "Paste" ), this );
  mActionCopyExpr = new QAction( tr( "Copy" ), this );
  mActionClearExpr = new QAction( tr( "Clear" ), this );
  mActionAssistant = new QAction( tr( "Assistant…" ), this );
  QFont assistantFont = mActionAssistant->font();
  assistantFont.setBold( true );
  mActionAssistant->setFont( assistantFont );
  mDefineMenu->addAction( mActionAssistant );
}


void QgsPropertyOverrideButton::init( int propertyKey, const QgsProperty &property, const QgsPropertiesDefinition &definitions, const QgsVectorLayer *layer, bool auxiliaryStorageEnabled )
{
  init( propertyKey, property, definitions.value( propertyKey ), layer, auxiliaryStorageEnabled );
}

void QgsPropertyOverrideButton::init( int propertyKey, const QgsProperty &property, const QgsPropertyDefinition &definition, const QgsVectorLayer *layer, bool auxiliaryStorageEnabled )
{
  mVectorLayer = layer;
  mAuxiliaryStorageEnabled = auxiliaryStorageEnabled;
  setToProperty( property );
  mPropertyKey = propertyKey;

  mDefinition = definition;
  mDataTypes = mDefinition.dataType();

  mInputDescription = mDefinition.helpText();
  mFullDescription.clear();
  mUsageInfo.clear();

  // set up data types string
  mDataTypesString.clear();

  QStringList ts;
  switch ( mDataTypes )
  {
    case QgsPropertyDefinition::DataTypeBoolean:
      ts << tr( "boolean" );
      FALLTHROUGH

    case QgsPropertyDefinition::DataTypeNumeric:
      ts << tr( "int" );
      ts << tr( "double" );
      FALLTHROUGH

    case QgsPropertyDefinition::DataTypeString:
      ts << tr( "string" );
      break;
  }

  if ( !ts.isEmpty() )
  {
    mDataTypesString = ts.join( QLatin1String( ", " ) );
    mActionDataTypes->setText( tr( "Field type: " ) + mDataTypesString );
  }

  updateFieldLists();
  updateGui();
  updateSiblingWidgets( isActive() );
}

void QgsPropertyOverrideButton::init( int propertyKey, const QgsAbstractPropertyCollection &collection, const QgsPropertiesDefinition &definitions, const QgsVectorLayer *layer, bool auxiliaryStorageEnabled )
{
  init( propertyKey, collection.property( propertyKey ), definitions, layer, auxiliaryStorageEnabled );
}


void QgsPropertyOverrideButton::updateFieldLists()
{
  mFieldNameList.clear();
  mFieldDisplayNameList.clear();
  mFieldIcons.clear();

  if ( mVectorLayer )
  {
    // store just a list of fields of unknown type or those that match the expected type
    const QgsFields fields = mVectorLayer->fields();
    int idx = 0;
    for ( const QgsField &f : fields )
    {
      bool fieldMatch = false;
      switch ( mDataTypes )
      {
        case QgsPropertyDefinition::DataTypeBoolean:
          fieldMatch = true;
          break;

        case QgsPropertyDefinition::DataTypeNumeric:
          fieldMatch = f.isNumeric() || f.type() == QVariant::String;
          break;

        case QgsPropertyDefinition::DataTypeString:
          fieldMatch = f.type() == QVariant::String;
          break;
      }

      if ( fieldMatch )
      {
        mFieldNameList << f.name();
        mFieldDisplayNameList << f.displayNameWithAlias();
        mFieldIcons << fields.iconForField( idx, true );
      }
      idx++;
    }
  }
}

QgsProperty QgsPropertyOverrideButton::toProperty() const
{
  return mProperty;
}

void QgsPropertyOverrideButton::setVectorLayer( const QgsVectorLayer *layer )
{
  mVectorLayer = layer;
}

void QgsPropertyOverrideButton::registerCheckedWidget( QWidget *widget, bool natural )
{
  const auto constMSiblingWidgets = mSiblingWidgets;
  for ( const SiblingWidget &sw : constMSiblingWidgets )
  {
    if ( widget == sw.mWidgetPointer.data() && sw.mSiblingType == SiblingCheckState )
      return;
  }
  mSiblingWidgets.append( SiblingWidget( QPointer<QWidget>( widget ), SiblingCheckState, natural ) );
  updateSiblingWidgets( isActive() );
}

void QgsPropertyOverrideButton::registerEnabledWidget( QWidget *widget, bool natural )
{
  const auto constMSiblingWidgets = mSiblingWidgets;
  for ( const SiblingWidget &sw : constMSiblingWidgets )
  {
    if ( widget == sw.mWidgetPointer.data() && sw.mSiblingType == SiblingEnableState )
      return;
  }
  mSiblingWidgets.append( SiblingWidget( QPointer<QWidget>( widget ), SiblingEnableState,  natural ) );
  updateSiblingWidgets( isActive() );
}

void QgsPropertyOverrideButton::registerVisibleWidget( QWidget *widget, bool natural )
{
  const auto constMSiblingWidgets = mSiblingWidgets;
  for ( const SiblingWidget &sw : constMSiblingWidgets )
  {
    if ( widget == sw.mWidgetPointer.data() && sw.mSiblingType == SiblingVisibility )
      return;
  }
  mSiblingWidgets.append( SiblingWidget( QPointer<QWidget>( widget ), SiblingVisibility, natural ) );
  updateSiblingWidgets( isActive() );
}

void QgsPropertyOverrideButton::registerExpressionWidget( QWidget *widget )
{
  const auto constMSiblingWidgets = mSiblingWidgets;
  for ( const SiblingWidget &sw : constMSiblingWidgets )
  {
    if ( widget == sw.mWidgetPointer.data() && sw.mSiblingType == SiblingExpressionText )
      return;
  }
  mSiblingWidgets.append( SiblingWidget( QPointer<QWidget>( widget ), SiblingExpressionText ) );
  updateSiblingWidgets( isActive() );
}


void QgsPropertyOverrideButton::mouseReleaseEvent( QMouseEvent *event )
{
  // Ctrl-click to toggle activated state
  if ( ( event->modifiers() & ( Qt::ControlModifier ) )
       || event->button() == Qt::RightButton )
  {
    setActivePrivate( !mProperty.isActive() );
    updateGui();
    emit changed();
    event->ignore();
    return;
  }

  // pass to default behavior
  QToolButton::mousePressEvent( event );
}

void QgsPropertyOverrideButton::setToProperty( const QgsProperty &property )
{
  if ( property )
  {
    switch ( property.propertyType() )
    {
      case QgsProperty::StaticProperty:
      case QgsProperty::InvalidProperty:
        break;
      case QgsProperty::FieldBasedProperty:
      {
        mFieldName = property.field();
        break;
      }
      case QgsProperty::ExpressionBasedProperty:
      {
        mExpressionString = property.expressionString();
        break;
      }
    }
  }
  else
  {
    mFieldName.clear();
    mExpressionString.clear();
  }
  mProperty = property;
  setActive( mProperty && mProperty.isActive() );
  updateSiblingWidgets( isActive() );
  updateGui();
}

///@cond PRIVATE
void QgsPropertyOverrideButton::aboutToShowMenu()
{
  mDefineMenu->clear();
  // update fields so that changes made to layer's fields are reflected
  updateFieldLists();

  bool hasExp = !mExpressionString.isEmpty();
  QString ddTitle = tr( "Data defined override" );

  QAction *ddTitleAct = mDefineMenu->addAction( ddTitle );
  QFont titlefont = ddTitleAct->font();
  titlefont.setItalic( true );
  ddTitleAct->setFont( titlefont );
  ddTitleAct->setEnabled( false );

  bool addActiveAction = false;
  if ( mProperty.propertyType() == QgsProperty::ExpressionBasedProperty && hasExp )
  {
    QgsExpression exp( mExpressionString );
    // whether expression is parse-able
    addActiveAction = !exp.hasParserError();
  }
  else if ( mProperty.propertyType() == QgsProperty::FieldBasedProperty )
  {
    // whether field exists
    addActiveAction = mFieldNameList.contains( mFieldName );
  }

  if ( addActiveAction )
  {
    ddTitleAct->setText( ddTitle + " (" + ( mProperty.propertyType() == QgsProperty::ExpressionBasedProperty ? tr( "expression" ) : tr( "field" ) ) + ')' );
    mDefineMenu->addAction( mActionActive );
    mActionActive->setText( mProperty.isActive() ? tr( "Deactivate" ) : tr( "Activate" ) );
    mActionActive->setData( QVariant( !mProperty.isActive() ) );
  }

  if ( !mFullDescription.isEmpty() )
  {
    mDefineMenu->addAction( mActionDescription );
  }

  mDefineMenu->addSeparator();

  // deactivate button if field already exists
  if ( mAuxiliaryStorageEnabled && mVectorLayer )
  {
    mDefineMenu->addAction( mActionCreateAuxiliaryField );

    const QgsAuxiliaryLayer *alayer = mVectorLayer->auxiliaryLayer();

    mActionCreateAuxiliaryField->setEnabled( true );
    mActionCreateAuxiliaryField->setChecked( false );

    int index = mVectorLayer->fields().indexFromName( mFieldName );
    int srcIndex;
    if ( index >= 0 && alayer && mVectorLayer->isAuxiliaryField( index, srcIndex ) )
    {
      mActionCreateAuxiliaryField->setEnabled( false );
      mActionCreateAuxiliaryField->setChecked( true );
    }
  }

  bool fieldActive = false;
  if ( !mDataTypesString.isEmpty() )
  {
    QAction *fieldTitleAct = mDefineMenu->addAction( tr( "Attribute Field" ) );
    fieldTitleAct->setFont( titlefont );
    fieldTitleAct->setEnabled( false );

    mDefineMenu->addAction( mActionDataTypes );

    mFieldsMenu->clear();

    if ( !mFieldNameList.isEmpty() )
    {

      for ( int j = 0; j < mFieldNameList.count(); ++j )
      {
        QString fldname = mFieldNameList.at( j );
        QAction *act = mFieldsMenu->addAction( mFieldDisplayNameList.at( j ) );
        act->setIcon( mFieldIcons.at( j ) );
        act->setData( QVariant( fldname ) );
        if ( mFieldName == fldname )
        {
          act->setCheckable( true );
          act->setChecked( mProperty.propertyType() == QgsProperty::FieldBasedProperty );
          fieldActive = mProperty.propertyType() == QgsProperty::FieldBasedProperty;
        }
      }
    }
    else
    {
      QAction *act = mFieldsMenu->addAction( tr( "No matching field types found" ) );
      act->setEnabled( false );
    }

    mDefineMenu->addSeparator();
  }

  mFieldsMenu->menuAction()->setCheckable( true );
  mFieldsMenu->menuAction()->setChecked( fieldActive && mProperty.propertyType() == QgsProperty::FieldBasedProperty && !mProperty.transformer() );

  bool colorActive = false;
  mColorsMenu->clear();
  if ( mDefinition.standardTemplate() == QgsPropertyDefinition::ColorWithAlpha
       || mDefinition.standardTemplate() == QgsPropertyDefinition::ColorNoAlpha )
  {
    // project colors menu
    QAction *colorTitleAct = mDefineMenu->addAction( tr( "Project Color" ) );
    colorTitleAct->setFont( titlefont );
    colorTitleAct->setEnabled( false );

    QList<QgsProjectColorScheme *> projectSchemes;
    QgsApplication::colorSchemeRegistry()->schemes( projectSchemes );
    if ( projectSchemes.length() > 0 )
    {
      QgsProjectColorScheme *scheme = projectSchemes.at( 0 );
      const QgsNamedColorList colors = scheme->fetchColors();
      for ( const auto &color : colors )
      {
        if ( color.second.isEmpty() )
          continue;

        QPixmap icon = QgsColorButton::createMenuIcon( color.first, mDefinition.standardTemplate() == QgsPropertyDefinition::ColorWithAlpha );
        QAction *act = mColorsMenu->addAction( color.second );
        act->setIcon( icon );
        if ( mProperty.propertyType() == QgsProperty::ExpressionBasedProperty && hasExp && mExpressionString == QStringLiteral( "project_color('%1')" ).arg( color.second ) )
        {
          act->setCheckable( true );
          act->setChecked( true );
          colorActive = true;
        }
      }
    }

    if ( mColorsMenu->actions().isEmpty() )
    {
      QAction *act = mColorsMenu->addAction( tr( "No colors set" ) );
      act->setEnabled( false );
    }

    mDefineMenu->addAction( mActionColors );
    mColorsMenu->menuAction()->setCheckable( true );
    mColorsMenu->menuAction()->setChecked( colorActive && !mProperty.transformer() );

    mDefineMenu->addSeparator();
  }

  QAction *exprTitleAct = mDefineMenu->addAction( tr( "Expression" ) );
  exprTitleAct->setFont( titlefont );
  exprTitleAct->setEnabled( false );

  mVariablesMenu->clear();
  bool variableActive = false;
  if ( mExpressionContextGenerator )
  {
    QgsExpressionContext context = mExpressionContextGenerator->createExpressionContext();
    QStringList variables = context.variableNames();
    const auto constVariables = variables;
    for ( const QString &variable : constVariables )
    {
      if ( context.isReadOnly( variable ) ) //only want to show user-set variables
        continue;
      if ( variable.startsWith( '_' ) ) //no hidden variables
        continue;

      QAction *act = mVariablesMenu->addAction( variable );
      act->setData( QVariant( variable ) );

      if ( mProperty.propertyType() == QgsProperty::ExpressionBasedProperty && hasExp && mExpressionString == '@' + variable )
      {
        act->setCheckable( true );
        act->setChecked( true );
        variableActive = true;
      }
    }
  }

  if ( mVariablesMenu->actions().isEmpty() )
  {
    QAction *act = mVariablesMenu->addAction( tr( "No variables set" ) );
    act->setEnabled( false );
  }

  mDefineMenu->addAction( mActionVariables );
  mVariablesMenu->menuAction()->setCheckable( true );
  mVariablesMenu->menuAction()->setChecked( variableActive && !mProperty.transformer() );

  if ( hasExp )
  {
    QString expString = mExpressionString;
    if ( expString.length() > 35 )
    {
      expString.truncate( 35 );
      expString.append( QChar( 0x2026 ) );
    }

    expString.prepend( tr( "Current: " ) );

    if ( !mActionExpression )
    {
      mActionExpression = new QAction( expString, this );
      mActionExpression->setCheckable( true );
    }
    else
    {
      mActionExpression->setText( expString );
    }
    mDefineMenu->addAction( mActionExpression );
    mActionExpression->setChecked( mProperty.propertyType() == QgsProperty::ExpressionBasedProperty && !variableActive && !colorActive && !mProperty.transformer() );

    mDefineMenu->addAction( mActionExpDialog );
    mDefineMenu->addAction( mActionCopyExpr );
    mDefineMenu->addAction( mActionPasteExpr );
  }
  else
  {
    mDefineMenu->addAction( mActionExpDialog );
    mDefineMenu->addAction( mActionPasteExpr );
  }

  if ( hasExp || !mFieldName.isEmpty() )
  {
    mDefineMenu->addSeparator();
    mDefineMenu->addAction( mActionClearExpr );
  }

  if ( !mDefinition.name().isEmpty() && mDefinition.supportsAssistant() )
  {
    mDefineMenu->addSeparator();
    mActionAssistant->setCheckable( mProperty.transformer() );
    mActionAssistant->setChecked( mProperty.transformer() );
    mDefineMenu->addAction( mActionAssistant );
  }
}

void QgsPropertyOverrideButton::menuActionTriggered( QAction *action )
{
  if ( action == mActionActive )
  {
    setActivePrivate( mActionActive->data().toBool() );
    updateGui();
    emit changed();
  }
  else if ( action == mActionDescription )
  {
    showDescriptionDialog();
  }
  else if ( action == mActionExpDialog )
  {
    showExpressionDialog();
  }
  else if ( action == mActionExpression )
  {
    mProperty.setExpressionString( mExpressionString );
    mProperty.setTransformer( nullptr );
    setActivePrivate( true );
    updateSiblingWidgets( isActive() );
    updateGui();
    emit changed();
  }
  else if ( action == mActionCopyExpr )
  {
    QApplication::clipboard()->setText( mExpressionString );
  }
  else if ( action == mActionPasteExpr )
  {
    QString exprString = QApplication::clipboard()->text();
    if ( !exprString.isEmpty() )
    {
      mExpressionString = exprString;
      mProperty.setExpressionString( mExpressionString );
      mProperty.setTransformer( nullptr );
      setActivePrivate( true );
      updateSiblingWidgets( isActive() );
      updateGui();
      emit changed();
    }
  }
  else if ( action == mActionClearExpr )
  {
    setActivePrivate( false );
    mProperty.setStaticValue( QVariant() );
    mProperty.setTransformer( nullptr );
    mExpressionString.clear();
    mFieldName.clear();
    updateSiblingWidgets( isActive() );
    updateGui();
    emit changed();
  }
  else if ( action == mActionAssistant )
  {
    showAssistant();
  }
  else if ( action == mActionCreateAuxiliaryField )
  {
    emit createAuxiliaryField();
  }
  else if ( mFieldsMenu->actions().contains( action ) )  // a field name clicked
  {
    if ( action->isEnabled() )
    {
      if ( mFieldName != action->text() )
      {
        mFieldName = action->data().toString();
      }
      mProperty.setField( mFieldName );
      mProperty.setTransformer( nullptr );
      setActivePrivate( true );
      updateSiblingWidgets( isActive() );
      updateGui();
      emit changed();
    }
  }
  else if ( mVariablesMenu->actions().contains( action ) )  // a variable name clicked
  {
    if ( mExpressionString != action->text().prepend( "@" ) )
    {
      mExpressionString = action->data().toString().prepend( "@" );
    }
    mProperty.setExpressionString( mExpressionString );
    mProperty.setTransformer( nullptr );
    setActivePrivate( true );
    updateSiblingWidgets( isActive() );
    updateGui();
    emit changed();
  }
  else if ( mColorsMenu->actions().contains( action ) )  // a color name clicked
  {
    if ( mExpressionString != QStringLiteral( "project_color('%1')" ).arg( action->text() ) )
    {
      mExpressionString = QStringLiteral( "project_color('%1')" ).arg( action->text() );
    }
    mProperty.setExpressionString( mExpressionString );
    mProperty.setTransformer( nullptr );
    setActivePrivate( true );
    updateSiblingWidgets( isActive() );
    updateGui();
    emit changed();
  }
}
///@endcond

void QgsPropertyOverrideButton::showDescriptionDialog()
{
  QgsMessageViewer *mv = new QgsMessageViewer( this );
  mv->setWindowTitle( tr( "Data Definition Description" ) );
  mv->setMessageAsHtml( mFullDescription );
  mv->exec();
}


void QgsPropertyOverrideButton::showExpressionDialog()
{
  QgsExpressionContext context = mExpressionContextGenerator ? mExpressionContextGenerator->createExpressionContext() : QgsExpressionContext();

  // build sensible initial expression text - see https://github.com/qgis/QGIS/issues/26526
  QString currentExpression = ( mProperty.propertyType() == QgsProperty::StaticProperty && !mProperty.staticValue().isValid() ) ? QString()
                              : mProperty.asExpression();

  QgsExpressionBuilderDialog d( const_cast<QgsVectorLayer *>( mVectorLayer ), currentExpression, this, QStringLiteral( "generic" ), context );
  d.setExpectedOutputFormat( mInputDescription );
  if ( d.exec() == QDialog::Accepted )
  {
    mExpressionString = d.expressionText().trimmed();
    bool active = mProperty.isActive();
    mProperty.setExpressionString( mExpressionString );
    mProperty.setTransformer( nullptr );
    mProperty.setActive( !mExpressionString.isEmpty() );
    if ( mProperty.isActive() != active )
      emit activated( mProperty.isActive() );
    updateSiblingWidgets( isActive() );
    updateGui();
    emit changed();
  }
  activateWindow(); // reset focus to parent window
}

void QgsPropertyOverrideButton::showAssistant()
{
  //first step - try to convert any existing expression to a transformer if one doesn't
  //already exist
  if ( !mProperty.transformer() )
  {
    ( void )mProperty.convertToTransformer();
  }

  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  QgsPropertyAssistantWidget *widget = new QgsPropertyAssistantWidget( panel, mDefinition, mProperty, mVectorLayer );
  widget->registerExpressionContextGenerator( mExpressionContextGenerator );
  widget->setSymbol( mSymbol ); // we only show legend preview in dialog version

  if ( panel && panel->dockMode() )
  {
    connect( widget, &QgsPropertyAssistantWidget::widgetChanged, this, [this, widget]
    {
      widget->updateProperty( this->mProperty );
      mExpressionString = this->mProperty.asExpression();
      mFieldName = this->mProperty.field();
      updateSiblingWidgets( isActive() );
      this->emit changed();
    } );

    // if the source layer is removed, we need to dismiss the assistant immediately
    if ( mVectorLayer )
      connect( mVectorLayer, &QObject::destroyed, widget, &QgsPanelWidget::acceptPanel );

    connect( widget, &QgsPropertyAssistantWidget::panelAccepted, this, [ = ] { updateGui(); } );

    panel->openPanel( widget );
    return;
  }
  else
  {
    // Show the dialog version if not in a panel
    QDialog *dlg = new QDialog( this );
    QString key = QStringLiteral( "/UI/paneldialog/%1" ).arg( widget->panelTitle() );
    QgsSettings settings;
    dlg->restoreGeometry( settings.value( key ).toByteArray() );
    dlg->setWindowTitle( widget->panelTitle() );
    dlg->setLayout( new QVBoxLayout() );
    dlg->layout()->addWidget( widget );
    QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Help | QDialogButtonBox::Ok );
    connect( buttonBox, &QDialogButtonBox::accepted, dlg, &QDialog::accept );
    connect( buttonBox, &QDialogButtonBox::rejected, dlg, &QDialog::reject );
    connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsPropertyOverrideButton::showHelp );
    dlg->layout()->addWidget( buttonBox );

    if ( dlg->exec() == QDialog::Accepted )
    {
      widget->updateProperty( mProperty );
      mExpressionString = mProperty.asExpression();
      mFieldName = mProperty.field();
      widget->acceptPanel();
      updateSiblingWidgets( isActive() );
      updateGui();

      emit changed();
    }
    settings.setValue( key, dlg->saveGeometry() );
  }
}

void QgsPropertyOverrideButton::updateGui()
{
  bool hasExp = !mExpressionString.isEmpty();
  bool hasField = !mFieldName.isEmpty();

  QIcon icon = QgsApplication::getThemeIcon( QStringLiteral( "/mIconDataDefine.svg" ) );
  QString deftip = tr( "undefined" );
  QString deftype;
  if ( mProperty.propertyType() == QgsProperty::ExpressionBasedProperty && hasExp )
  {
    icon = mProperty.isActive() ? QgsApplication::getThemeIcon( QStringLiteral( "/mIconDataDefineExpressionOn.svg" ) ) : QgsApplication::getThemeIcon( QStringLiteral( "/mIconDataDefineExpression.svg" ) );

    QRegularExpression rx( QStringLiteral( "^project_color\\('(.*)'\\)$" ) );
    QRegularExpressionMatch match = rx.match( mExpressionString );
    if ( match.hasMatch() )
    {
      icon = mProperty.isActive() ? QgsApplication::getThemeIcon( QStringLiteral( "/mIconDataDefineColorOn.svg" ) ) : QgsApplication::getThemeIcon( QStringLiteral( "/mIconDataDefineColor.svg" ) );
      deftip = match.captured( 1 );
      deftype = tr( "project color" );
    }
    else
    {
      QgsExpression exp( mExpressionString );
      if ( exp.hasParserError() )
      {
        icon = QgsApplication::getThemeIcon( QStringLiteral( "/mIconDataDefineExpressionError.svg" ) );
        deftip = tr( "Parse error: %1" ).arg( exp.parserErrorString() );
      }
      else
      {
        deftip = mExpressionString;
      }
    }
  }
  else if ( mProperty.propertyType() != QgsProperty::ExpressionBasedProperty && hasField )
  {
    icon = mProperty.isActive() ? QgsApplication::getThemeIcon( QStringLiteral( "/mIconDataDefineOn.svg" ) ) : QgsApplication::getThemeIcon( QStringLiteral( "/mIconDataDefine.svg" ) );

    if ( !mFieldNameList.contains( mFieldName ) && !mProperty.transformer() )
    {
      icon = QgsApplication::getThemeIcon( QStringLiteral( "/mIconDataDefineError.svg" ) );
      deftip = tr( "'%1' field missing" ).arg( mFieldName );
    }
    else
    {
      deftip = mFieldName;
    }
  }

  setIcon( icon );

  // build full description for tool tip and popup dialog
  mFullDescription = tr( "<b><u>Data defined override</u></b><br>" );

  mFullDescription += tr( "<b>Active: </b>%1&nbsp;&nbsp;&nbsp;<i>(ctrl|right-click toggles)</i><br>" ).arg( mProperty.isActive() ? tr( "yes" ) : tr( "no" ) );

  if ( !mUsageInfo.isEmpty() )
  {
    mFullDescription += tr( "<b>Usage:</b><br>%1<br>" ).arg( mUsageInfo );
  }

  if ( !mInputDescription.isEmpty() )
  {
    mFullDescription += tr( "<b>Expected input:</b><br>%1<br>" ).arg( mInputDescription );
  }

  if ( !mDataTypesString.isEmpty() )
  {
    mFullDescription += tr( "<b>Valid input types:</b><br>%1<br>" ).arg( mDataTypesString );
  }

  if ( deftype.isEmpty() && deftip != tr( "undefined" ) )
  {
    deftype = mProperty.propertyType() == QgsProperty::ExpressionBasedProperty ? tr( "expression" ) : tr( "field" );
  }

  // truncate long expressions, or tool tip may be too wide for screen
  if ( deftip.length() > 75 )
  {
    deftip.truncate( 75 );
    deftip.append( QChar( 0x2026 ) );
  }

  mFullDescription += tr( "<b>Current definition (%1):</b><br>%2" ).arg( deftype, deftip );

  setToolTip( mFullDescription );

}

void QgsPropertyOverrideButton::setActivePrivate( bool active )
{
  if ( mProperty.isActive() != active )
  {
    mProperty.setActive( active );
    emit activated( mProperty.isActive() );
  }
}

void QgsPropertyOverrideButton::updateSiblingWidgets( bool state )
{
  const auto constMSiblingWidgets = mSiblingWidgets;
  for ( const SiblingWidget &sw : constMSiblingWidgets )
  {
    switch ( sw.mSiblingType )
    {

      case SiblingCheckState:
      {
        // don't uncheck, only set to checked
        if ( state )
        {
          QAbstractButton *btn = qobject_cast< QAbstractButton * >( sw.mWidgetPointer.data() );
          if ( btn && btn->isCheckable() )
          {
            btn->setChecked( sw.mNatural ? state : !state );
          }
          else
          {
            QGroupBox *grpbx = qobject_cast< QGroupBox * >( sw.mWidgetPointer.data() );
            if ( grpbx && grpbx->isCheckable() )
            {
              grpbx->setChecked( sw.mNatural ? state : !state );
            }
          }
        }
        break;
      }

      case SiblingEnableState:
      {
        QLineEdit *le = qobject_cast< QLineEdit * >( sw.mWidgetPointer.data() );
        if ( le )
          le->setReadOnly( sw.mNatural ? !state : state );
        else
          sw.mWidgetPointer.data()->setEnabled( sw.mNatural ? state : !state );
        break;
      }

      case SiblingVisibility:
      {
        sw.mWidgetPointer.data()->setVisible( sw.mNatural ? state : !state );
        break;
      }

      case SiblingExpressionText:
      {
        QLineEdit *le = qobject_cast<QLineEdit *>( sw.mWidgetPointer.data() );
        if ( le )
        {
          le->setText( mProperty.asExpression() );
        }
        else
        {
          QTextEdit *te = qobject_cast<QTextEdit *>( sw.mWidgetPointer.data() );
          if ( te )
          {
            te->setText( mProperty.asExpression() );
          }
        }
        break;
      }

      case SiblingLinkedWidget:
      {
        if ( QgsColorButton *cb = qobject_cast< QgsColorButton * >( sw.mWidgetPointer.data() ) )
        {
          if ( state && mProperty.isProjectColor() )
          {
            QRegularExpression rx( QStringLiteral( "^project_color\\('(.*)'\\)$" ) );
            QRegularExpressionMatch match = rx.match( mExpressionString );
            if ( match.hasMatch() )
            {
              cb->linkToProjectColor( match.captured( 1 ) );
            }
          }
          else
          {
            cb->linkToProjectColor( QString() );
          }
        }
        break;
      }
    }
  }
}



void QgsPropertyOverrideButton::setActive( bool active )
{
  if ( mProperty.isActive() != active )
  {
    mProperty.setActive( active );
    updateGui();
    emit changed();
    emit activated( mProperty.isActive() );
  }
}

void QgsPropertyOverrideButton::registerExpressionContextGenerator( QgsExpressionContextGenerator *generator )
{
  mExpressionContextGenerator = generator;
}

void QgsPropertyOverrideButton::registerLinkedWidget( QWidget *widget )
{
  for ( const SiblingWidget &sw : std::as_const( mSiblingWidgets ) )
  {
    if ( widget == sw.mWidgetPointer.data() && sw.mSiblingType == SiblingLinkedWidget )
      return;
  }
  mSiblingWidgets.append( SiblingWidget( QPointer<QWidget>( widget ), SiblingLinkedWidget ) );

  if ( QgsColorButton *cb = qobject_cast< QgsColorButton * >( widget ) )
  {
    connect( cb, &QgsColorButton::unlinked, this, [ = ]
    {
      setActive( false );
      updateGui();
    } );
  }

  updateSiblingWidgets( isActive() );
}

void QgsPropertyOverrideButton::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#data-defined" ) );
}
