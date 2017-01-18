/***************************************************************************
     qgsdatadefinedbuttonv2.cpp
     --------------------------
    Date                 : March 2016
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatadefinedbuttonv2.h"

#include <qgsapplication.h>
#include <qgsdatadefined.h>
#include <qgsexpressionbuilderdialog.h>
#include <qgsexpression.h>
#include <qgsmessageviewer.h>
#include <qgsvectorlayer.h>

#include <QClipboard>
#include <QMenu>
#include <QMouseEvent>
#include <QPointer>
#include <QGroupBox>

QgsDataDefinedButtonV2::QgsDataDefinedButtonV2( QWidget* parent,
    const QgsVectorLayer* layer )
    : QToolButton( parent )
    , mVectorLayer( layer )
    , mActive( false )
    , mUseExpression( false )
    , mExpressionContextGenerator( nullptr )
{
  setFocusPolicy( Qt::StrongFocus );

  // set default tool button icon properties
  setFixedSize( 30, 26 );
  setStyleSheet( QString( "QToolButton{ background: none; border: 1px solid rgba(0, 0, 0, 0%);} QToolButton:focus { border: 1px solid palette(highlight); }" ) );
  setIconSize( QSize( 24, 24 ) );
  setPopupMode( QToolButton::InstantPopup );

  mDefineMenu = new QMenu( this );
  connect( mDefineMenu, &QMenu::aboutToShow, this, &QgsDataDefinedButtonV2::aboutToShowMenu );
  connect( mDefineMenu, &QMenu::triggered, this, &QgsDataDefinedButtonV2::menuActionTriggered );
  setMenu( mDefineMenu );

  mFieldsMenu = new QMenu( this );
  mActionDataTypes = new QAction( this );
  // list fields and types in submenu, since there may be many
  mActionDataTypes->setMenu( mFieldsMenu );

  mActionVariables = new QAction( tr( "Variable" ), this );
  mVariablesMenu = new QMenu( this );
  mActionVariables->setMenu( mVariablesMenu );

  mActionActive = new QAction( this );
  QFont f = mActionActive->font();
  f.setBold( true );
  mActionActive->setFont( f );

  mActionDescription = new QAction( tr( "Description..." ), this );

  mActionExpDialog = new QAction( tr( "Edit..." ), this );
  mActionExpression = nullptr;
  mActionPasteExpr = new QAction( tr( "Paste" ), this );
  mActionCopyExpr = new QAction( tr( "Copy" ), this );
  mActionClearExpr = new QAction( tr( "Clear" ), this );
  mActionAssistant = new QAction( tr( "Assistant..." ), this );
  QFont assistantFont = mActionAssistant->font();
  assistantFont.setBold( true );
  mActionAssistant->setFont( assistantFont );
  mDefineMenu->addAction( mActionAssistant );
}

void QgsDataDefinedButtonV2::init( int propertyKey, const QgsAbstractProperty* property, const QgsPropertiesDefinition& definitions, const QgsVectorLayer* layer )
{
  mVectorLayer = layer;
  setToProperty( property );
  mPropertyKey = propertyKey;

  const QgsPropertyDefinition& def = definitions.value( propertyKey );
  mDataTypes = def.dataType();

  mInputDescription = def.helpText();
  mFullDescription.clear();
  mUsageInfo.clear();

  // set up data types string
  mDataTypesString.clear();

  QStringList ts;
  switch ( mDataTypes )
  {
    case QgsPropertyDefinition::DataTypeBoolean:
      ts << tr( "boolean" );
      FALLTHROUGH;

    case QgsPropertyDefinition::DataTypeNumeric:
      ts << tr( "int" );
      ts << tr( "double" );
      FALLTHROUGH;

    case QgsPropertyDefinition::DataTypeString:
      ts << tr( "string" );
      break;
  }

  if ( !ts.isEmpty() )
  {
    mDataTypesString = ts.join( ", " );
    mActionDataTypes->setText( tr( "Field type: " ) + mDataTypesString );
  }

  updateFieldLists();
  updateGui();
}

void QgsDataDefinedButtonV2::init( int propertyKey, const QgsAbstractPropertyCollection& collection, const QgsPropertiesDefinition& definitions , const QgsVectorLayer* layer )
{
  init( propertyKey, collection.property( propertyKey ), definitions, layer );
}


void QgsDataDefinedButtonV2::updateFieldLists()
{
  mFieldNameList.clear();
  mFieldTypeList.clear();

  if ( mVectorLayer )
  {
    // store just a list of fields of unknown type or those that match the expected type
    Q_FOREACH ( const QgsField& f, mVectorLayer->fields() )
    {
      bool fieldMatch = false;
      QString fieldType;

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

      switch ( f.type() )
      {
        case QVariant::String:
          fieldType = tr( "string" );
          break;
        case QVariant::Int:
          fieldType = tr( "integer" );
          break;
        case QVariant::Double:
          fieldType = tr( "double" );
          break;
        case QVariant::Bool:
          fieldType = tr( "boolean" );
          break;
        default:
          fieldType = tr( "unknown type" );
      }
      if ( fieldMatch )
      {
        mFieldNameList << f.name();
        mFieldTypeList << fieldType;
      }
    }
  }
}

QgsAbstractProperty* QgsDataDefinedButtonV2::toProperty()
{
  QgsAbstractProperty* p = nullptr;
  if ( mUseExpression )
  {
    p = new QgsExpressionBasedProperty( mExpressionString, mActive );
  }
  else if ( !mFieldName.isEmpty() )
  {
    p = new QgsFieldBasedProperty( mFieldName, mActive );
  }
  return p;
}

void QgsDataDefinedButtonV2::setVectorLayer( const QgsVectorLayer* layer )
{
  mVectorLayer = layer;
}

void QgsDataDefinedButtonV2::registerCheckedWidget( QWidget* widget )
{
  //TODO
}

void QgsDataDefinedButtonV2::setAssistant( const QString& title, QgsDataDefinedAssistant* assistant ) {}

QgsDataDefinedAssistant*QgsDataDefinedButtonV2::assistant() { return nullptr; }

void QgsDataDefinedButtonV2::mouseReleaseEvent( QMouseEvent *event )
{
  // Ctrl-click to toggle activated state
  if (( event->modifiers() & ( Qt::ControlModifier ) )
      || event->button() == Qt::RightButton )
  {
    setActivePrivate( !mActive );
    updateGui();
    emit changed();
    event->ignore();
    return;
  }

  // pass to default behavior
  QToolButton::mousePressEvent( event );
}

void QgsDataDefinedButtonV2::setToProperty( const QgsAbstractProperty *property )
{
  if ( property )
  {
    switch ( property->propertyType() )
    {
      case QgsAbstractProperty::StaticProperty:
        break;
      case QgsAbstractProperty::FieldBasedProperty:
      {
        const QgsFieldBasedProperty* p = static_cast< const QgsFieldBasedProperty* >( property );
        mFieldName = p->field();
        mUseExpression = false;
        break;
      }
      case QgsAbstractProperty::ExpressionBasedProperty:
      {
        const QgsExpressionBasedProperty* p = static_cast< const QgsExpressionBasedProperty* >( property );
        mExpressionString = p->expressionString();
        mUseExpression = true;
        break;
      }
    }
  }
  else
  {
    mFieldName.clear();
    mUseExpression = false;
    mExpressionString.clear();
  }
  setActive( property && property->isActive() );
  updateGui();
}

void QgsDataDefinedButtonV2::aboutToShowMenu()
{
  mDefineMenu->clear();
  // update fields so that changes made to layer's fields are reflected
  updateFieldLists();

  bool hasExp = !mExpressionString.isEmpty();
  bool hasField = !mFieldName.isEmpty();
  QString ddTitle = tr( "Data defined override" );

  QAction* ddTitleAct = mDefineMenu->addAction( ddTitle );
  QFont titlefont = ddTitleAct->font();
  titlefont.setItalic( true );
  ddTitleAct->setFont( titlefont );
  ddTitleAct->setEnabled( false );

  bool addActiveAction = false;
  if ( mUseExpression && hasExp )
  {
    QgsExpression exp( mExpressionString );
    // whether expression is parse-able
    addActiveAction = !exp.hasParserError();
  }
  else if ( !mUseExpression && hasField )
  {
    // whether field exists
    addActiveAction = mFieldNameList.contains( mFieldName );
  }

  if ( addActiveAction )
  {
    ddTitleAct->setText( ddTitle + " (" + ( mUseExpression ? tr( "expression" ) : tr( "field" ) ) + ')' );
    mDefineMenu->addAction( mActionActive );
    mActionActive->setText( mActive ? tr( "Deactivate" ) : tr( "Activate" ) );
    mActionActive->setData( QVariant( mActive ? false : true ) );
  }

  if ( !mFullDescription.isEmpty() )
  {
    mDefineMenu->addAction( mActionDescription );
  }

  mDefineMenu->addSeparator();

  bool fieldActive = false;
  if ( !mDataTypesString.isEmpty() )
  {
    QAction* fieldTitleAct = mDefineMenu->addAction( tr( "Attribute field" ) );
    fieldTitleAct->setFont( titlefont );
    fieldTitleAct->setEnabled( false );

    mDefineMenu->addAction( mActionDataTypes );

    mFieldsMenu->clear();

    if ( !mFieldNameList.isEmpty() )
    {

      for ( int j = 0; j < mFieldNameList.count(); ++j )
      {
        QString fldname = mFieldNameList.at( j );
        QAction* act = mFieldsMenu->addAction( fldname + "    (" + mFieldTypeList.at( j ) + ')' );
        act->setData( QVariant( fldname ) );
        if ( mFieldName == fldname )
        {
          act->setCheckable( true );
          act->setChecked( !mUseExpression );
          fieldActive = !mUseExpression;
        }
      }
    }
    else
    {
      QAction* act = mFieldsMenu->addAction( tr( "No matching field types found" ) );
      act->setEnabled( false );
    }

    mDefineMenu->addSeparator();
  }

  mFieldsMenu->menuAction()->setCheckable( true );
  mFieldsMenu->menuAction()->setChecked( fieldActive );

  QAction* exprTitleAct = mDefineMenu->addAction( tr( "Expression" ) );
  exprTitleAct->setFont( titlefont );
  exprTitleAct->setEnabled( false );

  mVariablesMenu->clear();
  bool variableActive = false;
  if ( mExpressionContextGenerator )
  {
    QgsExpressionContext context = mExpressionContextGenerator->createExpressionContext();
    QStringList variables = context.variableNames();
    Q_FOREACH ( const QString& variable, variables )
    {
      if ( context.isReadOnly( variable ) ) //only want to show user-set variables
        continue;
      if ( variable.startsWith( '_' ) ) //no hidden variables
        continue;

      QAction* act = mVariablesMenu->addAction( variable );
      act->setData( QVariant( variable ) );

      if ( mUseExpression && hasExp && mExpressionString == '@' + variable )
      {
        act->setCheckable( true );
        act->setChecked( true );
        variableActive = true;
      }
    }
  }

  if ( mVariablesMenu->actions().isEmpty() )
  {
    QAction* act = mVariablesMenu->addAction( tr( "No variables set" ) );
    act->setEnabled( false );
  }

  mDefineMenu->addAction( mActionVariables );
  mVariablesMenu->menuAction()->setCheckable( true );
  mVariablesMenu->menuAction()->setChecked( variableActive );

  if ( hasExp )
  {
    QString expString = mExpressionString;
    if ( expString.length() > 35 )
    {
      expString.truncate( 35 );
      expString.append( "..." );
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
    mActionExpression->setChecked( mUseExpression && !variableActive );

    mDefineMenu->addAction( mActionExpDialog );
    mDefineMenu->addAction( mActionCopyExpr );
    mDefineMenu->addAction( mActionPasteExpr );
    mDefineMenu->addAction( mActionClearExpr );
  }
  else
  {
    mDefineMenu->addAction( mActionExpDialog );
    mDefineMenu->addAction( mActionPasteExpr );
  }
}

void QgsDataDefinedButtonV2::menuActionTriggered( QAction* action )
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
    mUseExpression = true;
    setActivePrivate( true );
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
      mUseExpression = true;
      setActivePrivate( mActive );
      updateGui();
      emit changed();
    }
  }
  else if ( action == mActionClearExpr )
  {
    // only deactivate if defined expression is being used
    if ( mActive && mUseExpression )
    {
      mUseExpression = false;
      setActivePrivate( false );
    }
    mExpressionString.clear();
    updateGui();
    emit changed();
  }
  else if ( action == mActionAssistant )
  {
    //showAssistant();
  }
  else if ( mFieldsMenu->actions().contains( action ) )  // a field name clicked
  {
    if ( action->isEnabled() )
    {
      if ( mFieldName != action->text() )
      {
        mFieldName = action->data().toString();
      }
      mUseExpression = false;
      setActivePrivate( true );
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
    mUseExpression = true;
    setActivePrivate( true );
    updateGui();
    emit changed();
  }
}

void QgsDataDefinedButtonV2::showDescriptionDialog()
{
  QgsMessageViewer* mv = new QgsMessageViewer( this );
  mv->setWindowTitle( tr( "Data definition description" ) );
  mv->setMessageAsHtml( mFullDescription );
  mv->exec();
}


void QgsDataDefinedButtonV2::showExpressionDialog()
{
  QgsExpressionContext context = mExpressionContextGenerator ? mExpressionContextGenerator->createExpressionContext() : QgsExpressionContext();

  QgsExpressionBuilderDialog d( const_cast<QgsVectorLayer*>( mVectorLayer ), mExpressionString, this, QStringLiteral( "generic" ), context );
  if ( d.exec() == QDialog::Accepted )
  {
    QString newExp = d.expressionText();
    mExpressionString = d.expressionText().trimmed();
    bool hasExp = !newExp.isEmpty();

    mUseExpression = hasExp;
    setActivePrivate( hasExp );
    updateGui();
    emit changed();
  }
  activateWindow(); // reset focus to parent window
}

void QgsDataDefinedButtonV2::updateGui()
{
  bool hasExp = !mExpressionString.isEmpty();
  bool hasField = !mFieldName.isEmpty();

  if ( mUseExpression && !hasExp )
  {
    setActive( false );
    mUseExpression = false;
  }
  else if ( !mUseExpression && !hasField )
  {
    setActive( false );
  }

  QIcon icon = QgsApplication::getThemeIcon( QStringLiteral( "/mIconDataDefine.svg" ) );
  QString deftip = tr( "undefined" );
  if ( mUseExpression && hasExp )
  {
    icon = mActive ? QgsApplication::getThemeIcon( QStringLiteral( "/mIconDataDefineExpressionOn.svg" ) ) : QgsApplication::getThemeIcon( QStringLiteral( "/mIconDataDefineExpression.svg" ) );

    QgsExpression exp( mExpressionString );
    if ( exp.hasParserError() )
    {
      setActive( false );
      icon = QgsApplication::getThemeIcon( QStringLiteral( "/mIconDataDefineExpressionError.svg" ) );
      deftip = tr( "Parse error: %1" ).arg( exp.parserErrorString() );
    }
  }
  else if ( !mUseExpression && hasField )
  {
    icon = mActive ? QgsApplication::getThemeIcon( QStringLiteral( "/mIconDataDefineOn.svg" ) ) : QgsApplication::getThemeIcon( QStringLiteral( "/mIconDataDefine.svg" ) );

    if ( !mFieldNameList.contains( mFieldName ) )
    {
      setActive( false );
      icon = QgsApplication::getThemeIcon( QStringLiteral( "/mIconDataDefineError.svg" ) );
      deftip = tr( "'%1' field missing" ).arg( mFieldName );
    }
  }

  setIcon( icon );

  // build full description for tool tip and popup dialog
  mFullDescription = tr( "<b><u>Data defined override</u></b><br>" );

  mFullDescription += tr( "<b>Active: </b>%1&nbsp;&nbsp;&nbsp;<i>(ctrl|right-click toggles)</i><br>" ).arg( mActive ? tr( "yes" ) : tr( "no" ) );

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

  QString deftype( "" );
  if ( deftip != tr( "undefined" ) )
  {
    deftype = QString( " (%1)" ).arg( mUseExpression ? tr( "expression" ) : tr( "field" ) );
  }

  // truncate long expressions, or tool tip may be too wide for screen
  if ( deftip.length() > 75 )
  {
    deftip.truncate( 75 );
    deftip.append( "..." );
  }

  mFullDescription += tr( "<b>Current definition %1:</b><br>%2" ).arg( deftype, deftip );

  setToolTip( mFullDescription );

}

void QgsDataDefinedButtonV2::setActivePrivate( bool active )
{
  if ( mActive != active )
  {
    mActive = active;
    emit activated( mActive );
  }
}

void QgsDataDefinedButtonV2::setActive( bool active )
{
  if ( mActive != active )
  {
    mActive = active;
    emit changed();
    emit activated( mActive );
  }
}

void QgsDataDefinedButtonV2::registerExpressionContextGenerator( QgsExpressionContextGenerator* generator )
{
  mExpressionContextGenerator = generator;
}
