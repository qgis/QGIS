/***************************************************************************
  qgsattributetableconfig.cpp - QgsAttributeTableConfig

 ---------------------
 begin                : 27.4.2016
 copyright            : (C) 2016 by mku
 email                : [your-email-here]
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsattributetableconfig.h"

#include <QStringList>

QgsAttributeTableConfig::QgsAttributeTableConfig()
    : mActionWidgetStyle( DropDown )
{

}

QVector<QgsAttributeTableConfig::ColumnConfig> QgsAttributeTableConfig::columns() const
{
  return mColumns;
}

void QgsAttributeTableConfig::setColumns( const QVector<ColumnConfig>& columns )
{
  mColumns = columns;
}

void QgsAttributeTableConfig::update( const QgsFields& fields )
{
  QStringList columns;

  bool containsActionColumn = false;

  for ( int i = mColumns.count() - 1; i >= 0; --i )
  {
    const ColumnConfig& column = mColumns.at( i );
    if ( column.mType == Field )
    {
      if ( fields.fieldNameIndex( column.mName ) == -1 )
      {
        mColumns.remove( i );
      }
      else
      {
        columns.append( column.mName );
      }
    }
    else if ( column.mType == Action )
    {
      containsActionColumn = true;
    }
  }

  Q_FOREACH ( const QgsField& field, fields )
  {
    if ( !columns.contains( field.name() ) )
    {
      ColumnConfig newColumn;
      newColumn.mHidden = false;
      newColumn.mType = Field;
      newColumn.mName = field.name();

      mColumns.append( newColumn );
    }
  }

  if ( !containsActionColumn )
  {
    ColumnConfig actionConfig;

    actionConfig.mType = Action;
    actionConfig.mHidden = true;

    mColumns.append( actionConfig );
  }
}

bool QgsAttributeTableConfig::actionWidgetVisible() const
{
  Q_FOREACH ( const ColumnConfig& columnConfig, mColumns )
  {
    if ( columnConfig.mType == Action && columnConfig.mHidden == false )
      return true;
  }
  return false;
}

void QgsAttributeTableConfig::setActionWidgetVisible( bool visible )
{
  for ( int i = 0; i < mColumns.size(); ++i )
  {
    if ( mColumns.at( i ).mType == Action )
    {
      mColumns[i].mHidden = !visible;
    }
  }
}

QgsAttributeTableConfig::ActionWidgetStyle QgsAttributeTableConfig::actionWidgetStyle() const
{
  return mActionWidgetStyle;
}

void QgsAttributeTableConfig::setActionWidgetStyle( const ActionWidgetStyle& actionWidgetStyle )
{
  mActionWidgetStyle = actionWidgetStyle;
}


void QgsAttributeTableConfig::readXml( const QDomNode& node )
{
  mColumns.clear();

  QDomNode configNode = node.namedItem( "attributetableconfig" );
  if ( !configNode.isNull() )
  {
    QDomNode columnsNode = configNode.toElement().namedItem( "columns" );

    QDomNodeList columns = columnsNode.childNodes();

    for ( int i = 0; i < columns.size(); ++i )
    {
      QDomElement columnElement = columns.at( i ).toElement();

      ColumnConfig column;

      if ( columnElement.attribute( "type" ) == "actions" )
      {
        column.mType = Action;
      }
      else
      {
        column.mType = Field;
        column.mName = columnElement.attribute( "name" );
      }

      column.mHidden = columnElement.attribute( "hidden" ) == "1";

      mColumns.append( column );
    }

    if ( configNode.toElement().attribute( "actionWidgetStyle" ) == "buttonList" )
      mActionWidgetStyle = ButtonList;
    else
      mActionWidgetStyle = DropDown;
  }
  else
  {
    // Before QGIS 2.16 the attribute table would hide "Hidden" widgets.
    // They are migrated to hidden columns here.
    QDomNodeList editTypeNodes = node.namedItem( "edittypes" ).childNodes();

    for ( int i = 0; i < editTypeNodes.size(); i++ )
    {
      QDomElement editTypeElement = editTypeNodes.at( i ).toElement();

      if ( editTypeElement.attribute( "widgetv2type" ) == "Hidden" )
      {
        ColumnConfig column;

        column.mName = editTypeElement.attribute( "name" );
        column.mHidden = true;
        column.mType = Field;
        mColumns.append( column );
      }
    }
  }
}

void QgsAttributeTableConfig::writeXml( QDomNode& node ) const
{
  QDomDocument doc( node.ownerDocument() );

  QDomElement configElement  = doc.createElement( "attributetableconfig" );
  configElement.setAttribute( "actionWidgetStyle", mActionWidgetStyle == ButtonList ? "buttonList" : "dropDown" );

  QDomElement columnsElement  = doc.createElement( "columns" );

  Q_FOREACH ( const ColumnConfig& column, mColumns )
  {
    QDomElement columnElement = doc.createElement( "column" );

    if ( column.mType == Action )
    {
      columnElement.setAttribute( "type", "actions" );
    }
    else
    {
      columnElement.setAttribute( "type", "field" );
      columnElement.setAttribute( "name", column.mName );
    }

    columnElement.setAttribute( "hidden", column.mHidden );

    columnsElement.appendChild( columnElement );
  }

  configElement.appendChild( columnsElement );

  node.appendChild( configElement );
}
