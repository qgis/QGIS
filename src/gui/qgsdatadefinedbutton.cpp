/***************************************************************************
    qgsdatadefinedbutton.cpp - Data defined selector button
     --------------------------------------
    Date                 : 27-April-2013
    Copyright            : (C) 2013 by Larry Shaffer
    Email                : larrys at dakcarto dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatadefinedbutton.h"

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

QgsDataDefinedButton::QgsDataDefinedButton( QWidget* parent,
    const QgsVectorLayer* vl,
    const QgsDataDefined* datadefined,
    const DataTypes& datatypes,
    const QString& description )
    : QToolButton( parent )
    , mExpressionContextCallback( nullptr )
    , mExpressionContextCallbackContext( nullptr )
{
  // set up static icons
  if ( mIconDataDefine.isNull() )
  {
    mIconDataDefine = QgsApplication::getThemeIcon( "/mIconDataDefine.svg" );
    mIconDataDefineOn = QgsApplication::getThemeIcon( "/mIconDataDefineOn.svg" );
    mIconDataDefineError = QgsApplication::getThemeIcon( "/mIconDataDefineError.svg" );
    mIconDataDefineExpression = QgsApplication::getThemeIcon( "/mIconDataDefineExpression.svg" );
    mIconDataDefineExpressionOn = QgsApplication::getThemeIcon( "/mIconDataDefineExpressionOn.svg" );
    mIconDataDefineExpressionError = QgsApplication::getThemeIcon( "/mIconDataDefineExpressionError.svg" );
  }

  setFocusPolicy( Qt::StrongFocus );

  // set default tool button icon properties
  setFixedSize( 30, 26 );
  setStyleSheet( QString( "QToolButton{ background: none; border: 1px solid rgba(0, 0, 0, 0%);} QToolButton:focus { border: 1px solid palette(highlight); }" ) );
  setIconSize( QSize( 24, 24 ) );
  setPopupMode( QToolButton::InstantPopup );

  mDefineMenu = new QMenu( this );
  connect( mDefineMenu, SIGNAL( aboutToShow() ), this, SLOT( aboutToShowMenu() ) );
  connect( mDefineMenu, SIGNAL( triggered( QAction* ) ), this, SLOT( menuActionTriggered( QAction* ) ) );
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

  // set up sibling widget connections
  connect( this, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( disableEnabledWidgets( bool ) ) );
  connect( this, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( checkCheckedWidgets( bool ) ) );

  init( vl, datadefined, datatypes, description );
}

QgsDataDefinedButton::~QgsDataDefinedButton()
{
  mEnabledWidgets.clear();
  mCheckedWidgets.clear();
}

void QgsDataDefinedButton::updateFieldLists()
{
  mFieldNameList.clear();
  mFieldTypeList.clear();

  if ( mVectorLayer )
  {
    // store just a list of fields of unknown type or those that match the expected type
    Q_FOREACH ( const QgsField& f, mVectorLayer->fields() )
    {
      bool fieldMatch = false;
      // NOTE: these are the only QVariant enums supported at this time (see QgsField)
      QString fieldType;
      switch ( f.type() )
      {
        case QVariant::String:
          fieldMatch = mDataTypes.testFlag( String );
          fieldType = tr( "string" );
          break;
        case QVariant::Int:
          fieldMatch = mDataTypes.testFlag( Int ) || mDataTypes.testFlag( Double );
          fieldType = tr( "integer" );
          break;
        case QVariant::Double:
          fieldMatch = mDataTypes.testFlag( Double );
          fieldType = tr( "double" );
          break;
        case QVariant::Invalid:
        default:
          fieldMatch = true; // field type is unknown
          fieldType = tr( "unknown type" );
      }
      if ( fieldMatch || mDataTypes.testFlag( AnyType ) )
      {
        mFieldNameList << f.name();
        mFieldTypeList << fieldType;
      }
    }
  }
}

void QgsDataDefinedButton::init( const QgsVectorLayer* vl,
                                 const QgsDataDefined* datadefined,
                                 const DataTypes& datatypes,
                                 const QString& description )
{
  mVectorLayer = vl;
  // construct default property if none or incorrect passed in
  if ( !datadefined )
  {
    mProperty.insert( "active", "0" );
    mProperty.insert( "useexpr", "0" );
    mProperty.insert( "expression", QString() );
    mProperty.insert( "field", QString() );
  }
  else
  {
    mProperty.insert( "active", datadefined->isActive() ? "1" : "0" );
    mProperty.insert( "useexpr", datadefined->useExpression() ? "1" : "0" );
    mProperty.insert( "expression", datadefined->expressionString() );
    mProperty.insert( "field", datadefined->field() );
  }

  mDataTypes = datatypes;
  mFieldNameList.clear();
  mFieldTypeList.clear();

  mInputDescription = description;
  mFullDescription.clear();
  mUsageInfo.clear();
  mCurrentDefinition.clear();

  // set up data types string
  mDataTypesString.clear();

  QStringList ts;
  if ( mDataTypes.testFlag( String ) )
  {
    ts << tr( "string" );
  }
  if ( mDataTypes.testFlag( Int ) )
  {
    ts << tr( "int" );
  }
  if ( mDataTypes.testFlag( Double ) )
  {
    ts << tr( "double" );
  }

  if ( !ts.isEmpty() )
  {
    mDataTypesString = ts.join( ", " );
    mActionDataTypes->setText( tr( "Field type: " ) + mDataTypesString );
  }

  updateFieldLists();
  updateGui();
}


void QgsDataDefinedButton::updateDataDefined( QgsDataDefined *dd ) const
{
  if ( !dd )
    return;

  dd->setActive( isActive() );
  dd->setExpressionString( getExpression() );
  dd->setField( getField() );
  dd->setUseExpression( useExpression() );
}

QgsDataDefined QgsDataDefinedButton::currentDataDefined() const
{
  QgsDataDefined dd;
  updateDataDefined( &dd );
  return dd;
}

void QgsDataDefinedButton::mouseReleaseEvent( QMouseEvent *event )
{
  // Ctrl-click to toggle activated state
  if (( event->modifiers() & ( Qt::ControlModifier ) )
      || event->button() == Qt::RightButton )
  {
    setActive( !isActive() );
    updateGui();
    event->ignore();
    return;
  }

  // pass to default behaviour
  QToolButton::mousePressEvent( event );
}

void QgsDataDefinedButton::aboutToShowMenu()
{
  mDefineMenu->clear();
  // update fields so that changes made to layer's fields are reflected
  updateFieldLists();

  bool hasExp = !getExpression().isEmpty();
  bool hasField = !getField().isEmpty();
  QString ddTitle = tr( "Data defined override" );

  QAction* ddTitleAct = mDefineMenu->addAction( ddTitle );
  QFont titlefont = ddTitleAct->font();
  titlefont.setItalic( true );
  ddTitleAct->setFont( titlefont );
  ddTitleAct->setEnabled( false );

  bool addActiveAction = false;
  if ( useExpression() && hasExp )
  {
    QgsExpression exp( getExpression() );
    // whether expression is parse-able
    addActiveAction = !exp.hasParserError();
  }
  else if ( !useExpression() && hasField )
  {
    // whether field exists
    addActiveAction = mFieldNameList.contains( getField() );
  }

  if ( addActiveAction )
  {
    ddTitleAct->setText( ddTitle + " (" + ( useExpression() ? tr( "expression" ) : tr( "field" ) ) + ')' );
    mDefineMenu->addAction( mActionActive );
    mActionActive->setText( isActive() ? tr( "Deactivate" ) : tr( "Activate" ) );
    mActionActive->setData( QVariant( isActive() ? false : true ) );
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
        if ( getField() == fldname )
        {
          act->setCheckable( true );
          act->setChecked( !useExpression() );
          fieldActive = !useExpression();
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
  if ( mExpressionContextCallback )
  {
    QgsExpressionContext context = mExpressionContextCallback( mExpressionContextCallbackContext );
    QStringList variables = context.variableNames();
    Q_FOREACH ( const QString& variable, variables )
    {
      if ( context.isReadOnly( variable ) ) //only want to show user-set variables
        continue;
      if ( variable.startsWith( '_' ) ) //no hidden variables
        continue;

      QAction* act = mVariablesMenu->addAction( variable );
      act->setData( QVariant( variable ) );

      if ( useExpression() && hasExp && getExpression() == '@' + variable )
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
    QString expString = getExpression();
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
    mActionExpression->setChecked( useExpression() && !variableActive );

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

  if ( mAssistant.data() )
  {
    mDefineMenu->addSeparator();
    mDefineMenu->addAction( mActionAssistant );
  }
}

void QgsDataDefinedButton::menuActionTriggered( QAction* action )
{
  if ( action == mActionActive )
  {
    setActive( mActionActive->data().toBool() );
    updateGui();
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
    setUseExpression( true );
    setActive( true );
    updateGui();
  }
  else if ( action == mActionCopyExpr )
  {
    QApplication::clipboard()->setText( getExpression() );
  }
  else if ( action == mActionPasteExpr )
  {
    QString exprString = QApplication::clipboard()->text();
    if ( !exprString.isEmpty() )
    {
      setExpression( exprString );
      setUseExpression( true );
      setActive( true );
      updateGui();
    }
  }
  else if ( action == mActionClearExpr )
  {
    // only deactivate if defined expression is being used
    if ( isActive() && useExpression() )
    {
      setUseExpression( false );
      setActive( false );
    }
    setExpression( QString() );
    updateGui();
  }
  else if ( action == mActionAssistant )
  {
    showAssistant();
  }
  else if ( mFieldsMenu->actions().contains( action ) )  // a field name clicked
  {
    if ( action->isEnabled() )
    {
      if ( getField() != action->text() )
      {
        setField( action->data().toString() );
      }
      setUseExpression( false );
      setActive( true );
      updateGui();
    }
  }
  else if ( mVariablesMenu->actions().contains( action ) )  // a variable name clicked
  {
    if ( getExpression() != action->text().prepend( "@" ) )
    {
      setExpression( action->data().toString().prepend( "@" ) );
    }
    setUseExpression( true );
    setActive( true );
    updateGui();
  }
}

void QgsDataDefinedButton::showDescriptionDialog()
{
  QgsMessageViewer* mv = new QgsMessageViewer( this );
  mv->setWindowTitle( tr( "Data definition description" ) );
  mv->setMessageAsHtml( mFullDescription );
  mv->exec();
}

void QgsDataDefinedButton::showAssistant()
{
  if ( !mAssistant.data() )
    return;

  if ( mAssistant->exec() == QDialog::Accepted )
  {
    QgsDataDefined dd = mAssistant->dataDefined();
    setUseExpression( dd.useExpression() );
    setActive( dd.isActive() );
    if ( dd.isActive() && dd.useExpression() )
      setExpression( dd.expressionString() );
    else if ( dd.isActive() )
      setField( dd.field() );
    updateGui();
  }
  activateWindow(); // reset focus to parent window
}

void QgsDataDefinedButton::showExpressionDialog()
{
  QgsExpressionContext context = mExpressionContextCallback ? mExpressionContextCallback( mExpressionContextCallbackContext ) : QgsExpressionContext();

  QgsExpressionBuilderDialog d( const_cast<QgsVectorLayer*>( mVectorLayer ), getExpression(), this, "generic", context );
  if ( d.exec() == QDialog::Accepted )
  {
    QString newExp = d.expressionText();
    setExpression( d.expressionText().trimmed() );
    bool hasExp = !newExp.isEmpty();

    setUseExpression( hasExp );
    setActive( hasExp );
    updateGui();
  }
  activateWindow(); // reset focus to parent window
}

void QgsDataDefinedButton::updateGui()
{
  QString oldDef = mCurrentDefinition;
  QString newDef( "" );
  bool hasExp = !getExpression().isEmpty();
  bool hasField = !getField().isEmpty();

  if ( useExpression() && !hasExp )
  {
    setActive( false );
    setUseExpression( false );
  }
  else if ( !useExpression() && !hasField )
  {
    setActive( false );
  }

  QIcon icon = mIconDataDefine;
  QString deftip = tr( "undefined" );
  if ( useExpression() && hasExp )
  {
    icon = isActive() ? mIconDataDefineExpressionOn : mIconDataDefineExpression;
    newDef = deftip = getExpression();

    QgsExpression exp( getExpression() );
    if ( exp.hasParserError() )
    {
      setActive( false );
      icon = mIconDataDefineExpressionError;
      deftip = tr( "Parse error: %1" ).arg( exp.parserErrorString() );
      newDef = "";
    }
  }
  else if ( !useExpression() && hasField )
  {
    icon = isActive() ? mIconDataDefineOn : mIconDataDefine;
    newDef = deftip = getField();

    if ( !mFieldNameList.contains( getField() ) )
    {
      setActive( false );
      icon = mIconDataDefineError;
      deftip = tr( "'%1' field missing" ).arg( getField() );
      newDef = "";
    }
  }

  setIcon( icon );

  // update and emit current definition
  if ( newDef != oldDef )
  {
    mCurrentDefinition = newDef;
    emit dataDefinedChanged( mCurrentDefinition );
  }

  // build full description for tool tip and popup dialog
  mFullDescription = tr( "<b><u>Data defined override</u></b><br>" );

  mFullDescription += tr( "<b>Active: </b>%1&nbsp;&nbsp;&nbsp;<i>(ctrl|right-click toggles)</i><br>" ).arg( isActive() ? tr( "yes" ) : tr( "no" ) );

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
    deftype = QString( " (%1)" ).arg( useExpression() ? tr( "expression" ) : tr( "field" ) );
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

void QgsDataDefinedButton::setActive( bool active )
{
  if ( isActive() != active )
  {
    mProperty.insert( "active", active ? "1" : "0" );
    emit dataDefinedActivated( active );
  }
}

void QgsDataDefinedButton::registerEnabledWidgets( const QList<QWidget*>& wdgts )
{
  for ( int i = 0; i < wdgts.size(); ++i )
  {
    registerEnabledWidget( wdgts.at( i ) );
  }
}

void QgsDataDefinedButton::registerEnabledWidget( QWidget* wdgt )
{
  QPointer<QWidget> wdgtP( wdgt );
  if ( !mEnabledWidgets.contains( wdgtP ) )
  {
    mEnabledWidgets.append( wdgtP );
  }
}

QList<QWidget*> QgsDataDefinedButton::registeredEnabledWidgets()
{
  QList<QWidget*> wdgtList;
  wdgtList.reserve( mEnabledWidgets.size() );
  for ( int i = 0; i < mEnabledWidgets.size(); ++i )
  {
    wdgtList << mEnabledWidgets.at( i );
  }
  return wdgtList;
}

void QgsDataDefinedButton::disableEnabledWidgets( bool disable )
{
  for ( int i = 0; i < mEnabledWidgets.size(); ++i )
  {
    mEnabledWidgets.at( i )->setDisabled( disable );
  }
}

void QgsDataDefinedButton::registerCheckedWidgets( const QList<QWidget*>& wdgts )
{
  for ( int i = 0; i < wdgts.size(); ++i )
  {
    registerCheckedWidget( wdgts.at( i ) );
  }
}

void QgsDataDefinedButton::registerCheckedWidget( QWidget* wdgt )
{
  QPointer<QWidget> wdgtP( wdgt );
  if ( !mCheckedWidgets.contains( wdgtP ) )
  {
    mCheckedWidgets.append( wdgtP );
  }
}

QList<QWidget*> QgsDataDefinedButton::registeredCheckedWidgets()
{
  QList<QWidget*> wdgtList;
  wdgtList.reserve( mCheckedWidgets.size() );
  for ( int i = 0; i < mCheckedWidgets.size(); ++i )
  {
    wdgtList << mCheckedWidgets.at( i );
  }
  return wdgtList;
}

void QgsDataDefinedButton::registerGetExpressionContextCallback( QgsDataDefinedButton::ExpressionContextCallback fnGetExpressionContext, const void *context )
{
  mExpressionContextCallback = fnGetExpressionContext;
  mExpressionContextCallbackContext = context;
}

void QgsDataDefinedButton::setAssistant( const QString& title, QgsDataDefinedAssistant *assistant )
{
  mActionAssistant->setText( title.isEmpty() ? tr( "Assistant..." ) : title );
  mAssistant.reset( assistant );
  mAssistant.data()->setParent( this, Qt::Dialog );
}

QgsDataDefinedAssistant *QgsDataDefinedButton::assistant()
{
  return mAssistant.data();
}

void QgsDataDefinedButton::checkCheckedWidgets( bool check )
{
  // don't uncheck, only set to checked
  if ( !check )
  {
    return;
  }
  for ( int i = 0; i < mCheckedWidgets.size(); ++i )
  {
    QAbstractButton *btn = qobject_cast< QAbstractButton * >( mCheckedWidgets.at( i ) );
    if ( btn && btn->isCheckable() )
    {
      btn->setChecked( true );
      continue;
    }
    QGroupBox *grpbx = qobject_cast< QGroupBox * >( mCheckedWidgets.at( i ) );
    if ( grpbx && grpbx->isCheckable() )
    {
      grpbx->setChecked( true );
    }
  }
}

QString QgsDataDefinedButton::trString()
{
  // just something to reduce translation redundancy
  return tr( "string " );
}

QString QgsDataDefinedButton::charDesc()
{
  return tr( "single character" );
}

QString QgsDataDefinedButton::boolDesc()
{
  return tr( "bool [<b>1</b>=True|<b>0</b>=False]" );
}

QString QgsDataDefinedButton::anyStringDesc()
{
  return tr( "string of variable length" );
}

QString QgsDataDefinedButton::intDesc()
{
  return tr( "int [&lt;= 0 =&gt;]" );
}

QString QgsDataDefinedButton::intPosDesc()
{
  return tr( "int [&gt;= 0]" );
}

QString QgsDataDefinedButton::intPosOneDesc()
{
  return tr( "int [&gt;= 1]" );
}

QString QgsDataDefinedButton::doubleDesc()
{
  return tr( "double [&lt;= 0.0 =&gt;]" );
}

QString QgsDataDefinedButton::doublePosDesc()
{
  return tr( "double [&gt;= 0.0]" );
}

QString QgsDataDefinedButton::double0to1Desc()
{
  return tr( "double [0.0-1.0]" );
}

QString QgsDataDefinedButton::doubleXYDesc()
{
  return tr( "double coord [<b>X,Y</b>] as &lt;= 0.0 =&gt;" );
}

QString QgsDataDefinedButton::double180RotDesc()
{
  return tr( "double [-180.0 - 180.0]" );
}

QString QgsDataDefinedButton::intTranspDesc()
{
  return tr( "int [0-100]" );
}

QString QgsDataDefinedButton::unitsMmMuDesc()
{
  return trString() + "[<b>MM</b>|<b>MapUnit</b>]";
}

QString QgsDataDefinedButton::unitsMmMuPercentDesc()
{
  return trString() + "[<b>MM</b>|<b>MapUnit</b>|<b>Percent</b>]";
}

QString QgsDataDefinedButton::colorNoAlphaDesc()
{
  return tr( "string [<b>r,g,b</b>] as int 0-255" );
}

QString QgsDataDefinedButton::colorAlphaDesc()
{
  return tr( "string [<b>r,g,b,a</b>] as int 0-255" );
}

QString QgsDataDefinedButton::textHorzAlignDesc()
{
  return trString() + "[<b>Left</b>|<b>Center</b>|<b>Right</b>]";
}

QString QgsDataDefinedButton::textVertAlignDesc()
{
  return trString() + "[<b>Bottom</b>|<b>Middle</b>|<b>Top</b>]";
}

QString QgsDataDefinedButton::penJoinStyleDesc()
{
  return trString() + "[<b>bevel</b>|<b>miter</b>|<b>round</b>]";
}

QString QgsDataDefinedButton::blendModesDesc()
{
  return trString() + QLatin1String( "[<b>Normal</b>|<b>Lighten</b>|<b>Screen</b>|<b>Dodge</b>|<br>"
                                     "<b>Addition</b>|<b>Darken</b>|<b>Multiply</b>|<b>Burn</b>|<b>Overlay</b>|<br>"
                                     "<b>SoftLight</b>|<b>HardLight</b>|<b>Difference</b>|<b>Subtract</b>]" );
}

QString QgsDataDefinedButton::svgPathDesc()
{
  return trString() + QLatin1String( "[<b>filepath</b>] as<br>"
                                     "<b>''</b>=empty|absolute|search-paths-relative|<br>"
                                     "project-relative|URL" );
}

QString QgsDataDefinedButton::filePathDesc()
{
  return tr( "string [<b>filepath</b>]" );
}

QString QgsDataDefinedButton::paperSizeDesc()
{
  return trString() + QLatin1String( "[<b>A5</b>|<b>A4</b>|<b>A3</b>|<b>A2</b>|<b>A1</b>|<b>A0</b>"
                                     "<b>B5</b>|<b>B4</b>|<b>B3</b>|<b>B2</b>|<b>B1</b>|<b>B0</b>"
                                     "<b>Legal</b>|<b>Ansi A</b>|<b>Ansi B</b>|<b>Ansi C</b>|<b>Ansi D</b>|<b>Ansi E</b>"
                                     "<b>Arch A</b>|<b>Arch B</b>|<b>Arch C</b>|<b>Arch D</b>|<b>Arch E</b>|<b>Arch E1</b>]"
                                   );
}

QString QgsDataDefinedButton::paperOrientationDesc()
{
  return trString() + QLatin1String( "[<b>portrait</b>|<b>landscape</b>]" );
}

QString QgsDataDefinedButton::horizontalAnchorDesc()
{
  return trString() + QLatin1String( "[<b>left</b>|<b>center</b>|<b>right</b>]" );
}

QString QgsDataDefinedButton::verticalAnchorDesc()
{
  return trString() + QLatin1String( "[<b>top</b>|<b>center</b>|<b>bottom</b>]" );
}

QString QgsDataDefinedButton::gradientTypeDesc()
{
  return trString() + QLatin1String( "[<b>linear</b>|<b>radial</b>|<b>conical</b>]" );
}

QString QgsDataDefinedButton::gradientCoordModeDesc()
{
  return trString() + QLatin1String( "[<b>feature</b>|<b>viewport</b>]" );
}

QString QgsDataDefinedButton::gradientSpreadDesc()
{
  return trString() + QLatin1String( "[<b>pad</b>|<b>repeat</b>|<b>reflect</b>]" );
}

QString QgsDataDefinedButton::lineStyleDesc()
{
  return trString() + QLatin1String( "[<b>no</b>|<b>solid</b>|<b>dash</b>|<b>dot</b>|<b>dash dot</b>|<b>dash dot dot</b>]" );
}

QString QgsDataDefinedButton::capStyleDesc()
{
  return trString() + QLatin1String( "[<b>square</b>|<b>flat</b>|<b>round</b>]" );
}

QString QgsDataDefinedButton::fillStyleDesc()
{
  return trString() + QLatin1String( "[<b>solid</b>|<b>horizontal</b>|<b>vertical</b>|<b>cross</b>|<b>b_diagonal</b>|<b>f_diagonal"
                                     "</b>|<b>diagonal_x</b>|<b>dense1</b>|<b>dense2</b>|<b>dense3</b>|<b>dense4</b>|<b>dense5"
                                     "</b>|<b>dense6</b>|<b>dense7</b>|<b>no]" );
}

QString QgsDataDefinedButton::markerStyleDesc()
{
  return trString() + QLatin1String( "[<b>circle</b>|<b>rectangle</b>|<b>diamond</b>|<b>cross</b>|<b>triangle"
                                     "</b>|<b>right_half_triangle</b>|<b>left_half_triangle</b>|<b>semi_circle</b>]" );
}

QString QgsDataDefinedButton::customDashDesc()
{
  return tr( "[<b><dash>;<space></b>] e.g. '8;2;1;2'" );
}
