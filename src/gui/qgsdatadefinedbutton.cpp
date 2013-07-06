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


QIcon QgsDataDefinedButton::mIconDataDefine;
QIcon QgsDataDefinedButton::mIconDataDefineOn;
QIcon QgsDataDefinedButton::mIconDataDefineError;
QIcon QgsDataDefinedButton::mIconDataDefineExpression;
QIcon QgsDataDefinedButton::mIconDataDefineExpressionOn;
QIcon QgsDataDefinedButton::mIconDataDefineExpressionError;

QgsDataDefinedButton::QgsDataDefinedButton( QWidget* parent,
    const QgsVectorLayer* vl,
    const QgsDataDefined* datadefined,
    DataTypes datatypes,
    QString description )
    : QToolButton( parent )
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

  // set default tool button icon properties
  setFixedSize( 28, 24 );
  setStyleSheet( QString( "QToolButton{ background: none; border: none;}" ) );
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

  mActionActive = new QAction( this );
  QFont f = mActionActive->font();
  f.setBold( true );
  mActionActive->setFont( f );

  mActionDescription = new QAction( tr( "Description..." ), this );

  mActionExpDialog = new QAction( tr( "Edit..." ), this );
  mActionExpression = 0;
  mActionPasteExpr = new QAction( tr( "Paste" ), this );
  mActionCopyExpr = new QAction( tr( "Copy" ), this );
  mActionClearExpr = new QAction( tr( "Clear" ), this );

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

void QgsDataDefinedButton::init( const QgsVectorLayer* vl,
                                 const QgsDataDefined* datadefined,
                                 DataTypes datatypes,
                                 QString description )
{
  mVectorLayer = vl;
  // construct default property if none or incorrect passed in
  if ( !datadefined )
  {
    mProperty.insert( "active", "0" );
    mProperty.insert( "useexpr", "0" );
    mProperty.insert( "expression", "" );
    mProperty.insert( "field", "" );
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
  mFullDescription = QString( "" );
  mUsageInfo = QString( "" );
  mCurrentDefinition = QString( "" );

  // set up data types string
  mDataTypesString = QString( "" );

  QStringList ts;
  if ( mDataTypes.testFlag( AnyType ) || mDataTypes.testFlag( String ) )
  {
    ts << tr( "string" );
  }
  if ( mDataTypes.testFlag( AnyType ) || mDataTypes.testFlag( Int ) )
  {
    ts << tr( "int" );
  }
  if ( mDataTypes.testFlag( AnyType ) || mDataTypes.testFlag( Double ) )
  {
    ts << tr( "double" );
  }

  if ( !ts.isEmpty() )
  {
    mDataTypesString = ts.join( ", " );
    mActionDataTypes->setText( tr( "Field type: " ) + mDataTypesString );
  }

  if ( mVectorLayer )
  {
    // store just a list of fields of unknown type or those that match the expected type
    const QgsFields& fields = mVectorLayer->pendingFields();
    for ( int i = 0; i < fields.count(); ++i )
    {
      const QgsField& f = fields.at( i );
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
          fieldMatch = mDataTypes.testFlag( Int );
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

  updateGui();
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
    ddTitleAct->setText( ddTitle + " (" + ( useExpression() ? tr( "expression" ) : tr( "field" ) ) + ")" );
    mDefineMenu->addAction( mActionActive );
    mActionActive->setText( isActive() ? tr( "Deactivate" ) : tr( "Activate" ) );
    mActionActive->setData( QVariant( isActive() ? false : true ) );
  }

  if ( !mFullDescription.isEmpty() )
  {
    mDefineMenu->addAction( mActionDescription );
  }

  mDefineMenu->addSeparator();

  if ( !mDataTypesString.isEmpty() )
  {
    QAction* fieldTitleAct = mDefineMenu->addAction( tr( "Attribute field" ) );
    fieldTitleAct->setFont( titlefont );
    fieldTitleAct->setEnabled( false );

    mDefineMenu->addAction( mActionDataTypes );

    mFieldsMenu->clear();

    if ( mFieldNameList.size() > 0 )
    {

      for ( int j = 0; j < mFieldNameList.count(); ++j )
      {
        QString fldname = mFieldNameList.at( j );
        QAction* act = mFieldsMenu->addAction( fldname + "    (" + mFieldTypeList.at( j ) + ")" );
        act->setData( QVariant( fldname ) );
        if ( getField() == fldname )
        {
          act->setCheckable( true );
          act->setChecked( !useExpression() );
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

  QAction* exprTitleAct = mDefineMenu->addAction( tr( "Expression" ) );
  exprTitleAct->setFont( titlefont );
  exprTitleAct->setEnabled( false );

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
    mActionExpression->setChecked( useExpression() );

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
    setExpression( QString( "" ) );
    updateGui();
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
}

void QgsDataDefinedButton::showDescriptionDialog()
{
  QgsMessageViewer* mv = new QgsMessageViewer( this );
  mv->setWindowTitle( tr( "Data definition description" ) );
  mv->setMessageAsHtml( mFullDescription );
  mv->exec();
}

void QgsDataDefinedButton::showExpressionDialog()
{
  QgsExpressionBuilderDialog d( const_cast<QgsVectorLayer*>( mVectorLayer ), getExpression() );
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

  mFullDescription += tr( "<b>Current definition%1:</b><br>%2" ).arg( deftype ).arg( deftip );

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

void QgsDataDefinedButton::registerEnabledWidgets( QList<QWidget*> wdgts )
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

void QgsDataDefinedButton::registerCheckedWidgets( QList<QWidget*> wdgts )
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
  for ( int i = 0; i < mCheckedWidgets.size(); ++i )
  {
    wdgtList << mCheckedWidgets.at( i );
  }
  return wdgtList;
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
  return trString() + "[<b>Bevel</b>|<b>Miter</b>|<b>Round</b>]";
}

QString QgsDataDefinedButton::blendModesDesc()
{
  return trString() + QString( "[<b>Normal</b>|<b>Lighten</b>|<b>Screen</b>|<b>Dodge</b>|<br>"
                               "<b>Addition</b>|<b>Darken</b>|<b>Multiply</b>|<b>Burn</b>|<b>Overlay</b>|<br>"
                               "<b>SoftLight</b>|<b>HardLight</b>|<b>Difference</b>|<b>Subtract</b>" );
}

QString QgsDataDefinedButton::svgPathDesc()
{
  return trString() + QString( "[<b>filepath</b>] as<br>"
                               "<b>''</b>=empty|absolute|search-paths-relative|<br>"
                               "project-relative|URL" );
}
