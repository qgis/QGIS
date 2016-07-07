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

bool QgsAttributeTableConfig::isEmpty() const
{
  return mColumns.isEmpty();
}

int QgsAttributeTableConfig::mapVisibleColumnToIndex( int visibleColumn ) const
{
  for ( int i = 0; i < mColumns.size(); ++i )
  {
    if ( mColumns.at( i ).hidden )
    {
      visibleColumn++;
      continue;
    }
    if ( visibleColumn == i )
      return i;
  }
  return -1;
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
    if ( column.type == Field )
    {
      if ( fields.fieldNameIndex( column.name ) == -1 )
      {
        mColumns.remove( i );
      }
      else
      {
        columns.append( column.name );
      }
    }
    else if ( column.type == Action )
    {
      containsActionColumn = true;
    }
  }

  Q_FOREACH ( const QgsField& field, fields )
  {
    if ( !columns.contains( field.name() ) )
    {
      ColumnConfig newColumn;
      newColumn.hidden = false;
      newColumn.type = Field;
      newColumn.name = field.name();

      mColumns.append( newColumn );
    }
  }

  if ( !containsActionColumn )
  {
    ColumnConfig actionConfig;

    actionConfig.type = Action;
    actionConfig.hidden = true;

    mColumns.append( actionConfig );
  }
}

bool QgsAttributeTableConfig::actionWidgetVisible() const
{
  Q_FOREACH ( const ColumnConfig& columnConfig, mColumns )
  {
    if ( columnConfig.type == Action && columnConfig.hidden == false )
      return true;
  }
  return false;
}

void QgsAttributeTableConfig::setActionWidgetVisible( bool visible )
{
  for ( int i = 0; i < mColumns.size(); ++i )
  {
    if ( mColumns.at( i ).type == Action )
    {
      mColumns[i].hidden = !visible;
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
        column.type = Action;
      }
      else
      {
        column.type = Field;
        column.name = columnElement.attribute( "name" );
      }

      column.hidden = columnElement.attribute( "hidden" ) == "1";
      column.width = columnElement.attribute( "width", "-1" ).toDouble();

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

        column.name = editTypeElement.attribute( "name" );
        column.hidden = true;
        column.type = Field;
        mColumns.append( column );
      }
    }
  }

  mSortExpression = configNode.toElement().attribute( "sortExpression" );
  mSortOrder = static_cast<Qt::SortOrder>( configNode.toElement().attribute( "sortOrder" ).toInt() );
}

QString QgsAttributeTableConfig::sortExpression() const
{
  return mSortExpression;
}

void QgsAttributeTableConfig::setSortExpression( const QString& sortExpression )
{
  mSortExpression = sortExpression;
}

int QgsAttributeTableConfig::columnWidth( int column ) const
{
  return mColumns.at( column ).width;
}

void QgsAttributeTableConfig::setColumnWidth( int column, int width )
{
  mColumns[ column ].width = width;
}

bool QgsAttributeTableConfig::columnHidden( int column ) const
{
  return mColumns.at( column ).hidden;
}

void QgsAttributeTableConfig::setColumnHidden( int column, bool hidden )
{
  mColumns[ column ].hidden = hidden;
}

bool QgsAttributeTableConfig::operator!=( const QgsAttributeTableConfig& other ) const
{
  return mSortExpression != other.mSortExpression || mColumns != other.mColumns || mActionWidgetStyle != other.mActionWidgetStyle || mSortOrder != other.mSortOrder;
}

Qt::SortOrder QgsAttributeTableConfig::sortOrder() const
{
  return mSortOrder;
}

void QgsAttributeTableConfig::setSortOrder( const Qt::SortOrder& sortOrder )
{
  mSortOrder = sortOrder;
}

void QgsAttributeTableConfig::writeXml( QDomNode& node ) const
{
  QDomDocument doc( node.ownerDocument() );

  QDomElement configElement  = doc.createElement( "attributetableconfig" );
  configElement.setAttribute( "actionWidgetStyle", mActionWidgetStyle == ButtonList ? "buttonList" : "dropDown" );

  configElement.setAttribute( "sortExpression", mSortExpression );

  configElement.setAttribute( "sortOrder", mSortOrder );

  QDomElement columnsElement  = doc.createElement( "columns" );

  Q_FOREACH ( const ColumnConfig& column, mColumns )
  {
    QDomElement columnElement = doc.createElement( "column" );

    if ( column.type == Action )
    {
      columnElement.setAttribute( "type", "actions" );
    }
    else
    {
      columnElement.setAttribute( "type", "field" );
      columnElement.setAttribute( "name", column.name );
    }

    columnElement.setAttribute( "hidden", column.hidden );
    columnElement.setAttribute( "width", QString::number( column.width ) );

    columnsElement.appendChild( columnElement );
  }

  configElement.appendChild( columnsElement );

  node.appendChild( configElement );
}

bool QgsAttributeTableConfig::ColumnConfig::operator== ( const ColumnConfig& other ) const
{
  return type == other.type && name == other.name && hidden == other.hidden && width == other.width;
}
