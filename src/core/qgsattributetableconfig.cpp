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
#include "qgsfields.h"
#include <QStringList>

QVector<QgsAttributeTableConfig::ColumnConfig> QgsAttributeTableConfig::columns() const
{
  return mColumns;
}

bool QgsAttributeTableConfig::isEmpty() const
{
  return mColumns.isEmpty();
}

int QgsAttributeTableConfig::size() const
{
  return mColumns.size();
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

void QgsAttributeTableConfig::setColumns( const QVector<ColumnConfig> &columns )
{
  mColumns = columns;
}

void QgsAttributeTableConfig::update( const QgsFields &fields )
{
  QStringList columns;

  bool containsActionColumn = false;

  for ( int i = mColumns.count() - 1; i >= 0; --i )
  {
    const ColumnConfig &column = mColumns.at( i );
    if ( column.type == Field )
    {
      if ( fields.indexOf( column.name ) == -1 )
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

  for ( const auto &field : fields )
  {
    if ( !columns.contains( field.name() ) )
    {
      ColumnConfig newColumn;
      newColumn.hidden = false;
      newColumn.type = Field;
      newColumn.name = field.name();
      if ( containsActionColumn )
      {
        mColumns.insert( mColumns.size() - 1, newColumn );
      }
      else
      {
        mColumns.append( newColumn );
      }
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
  const auto constMColumns = mColumns;
  for ( const ColumnConfig &columnConfig : constMColumns )
  {
    if ( columnConfig.type == Action && !columnConfig.hidden )
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

void QgsAttributeTableConfig::setActionWidgetStyle( ActionWidgetStyle actionWidgetStyle )
{
  mActionWidgetStyle = actionWidgetStyle;
}


void QgsAttributeTableConfig::readXml( const QDomNode &node )
{
  mColumns.clear();

  const QDomNode configNode = node.namedItem( QStringLiteral( "attributetableconfig" ) );
  if ( !configNode.isNull() )
  {
    const QDomNode columnsNode = configNode.toElement().namedItem( QStringLiteral( "columns" ) );

    const QDomNodeList columns = columnsNode.childNodes();

    for ( int i = 0; i < columns.size(); ++i )
    {
      const QDomElement columnElement = columns.at( i ).toElement();

      ColumnConfig column;

      if ( columnElement.attribute( QStringLiteral( "type" ) ) == QLatin1String( "actions" ) )
      {
        column.type = Action;
      }
      else
      {
        column.type = Field;
        column.name = columnElement.attribute( QStringLiteral( "name" ) );
      }

      column.hidden = columnElement.attribute( QStringLiteral( "hidden" ) ) == QLatin1String( "1" );
      column.width = columnElement.attribute( QStringLiteral( "width" ), QStringLiteral( "-1" ) ).toDouble();

      mColumns.append( column );
    }

    if ( configNode.toElement().attribute( QStringLiteral( "actionWidgetStyle" ) ) == QLatin1String( "buttonList" ) )
      mActionWidgetStyle = ButtonList;
    else
      mActionWidgetStyle = DropDown;
  }
  else
  {
    // Before QGIS 2.16 the attribute table would hide "Hidden" widgets.
    // They are migrated to hidden columns here.
    const QDomNodeList editTypeNodes = node.namedItem( QStringLiteral( "edittypes" ) ).childNodes();

    for ( int i = 0; i < editTypeNodes.size(); i++ )
    {
      const QDomElement editTypeElement = editTypeNodes.at( i ).toElement();

      if ( editTypeElement.attribute( QStringLiteral( "widgetv2type" ) ) == QLatin1String( "Hidden" ) )
      {
        ColumnConfig column;

        column.name = editTypeElement.attribute( QStringLiteral( "name" ) );
        column.hidden = true;
        column.type = Field;
        mColumns.append( column );
      }
    }
  }

  mSortExpression = configNode.toElement().attribute( QStringLiteral( "sortExpression" ) );
  const Qt::SortOrder sortOrder = static_cast<Qt::SortOrder>( configNode.toElement().attribute( QStringLiteral( "sortOrder" ) ).toInt() );
  setSortOrder( sortOrder );
}

QString QgsAttributeTableConfig::sortExpression() const
{
  return mSortExpression;
}

void QgsAttributeTableConfig::setSortExpression( const QString &sortExpression )
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

bool QgsAttributeTableConfig::operator!=( const QgsAttributeTableConfig &other ) const
{
  return mSortExpression != other.mSortExpression || mColumns != other.mColumns || mActionWidgetStyle != other.mActionWidgetStyle || mSortOrder != other.mSortOrder;
}

Qt::SortOrder QgsAttributeTableConfig::sortOrder() const
{
  return mSortOrder;
}

void QgsAttributeTableConfig::setSortOrder( Qt::SortOrder sortOrder )
{
  // fix https://hub.qgis.org/issues/15803
  if ( sortOrder != Qt::AscendingOrder && sortOrder != Qt::DescendingOrder )
  {
    sortOrder = Qt::AscendingOrder;
  }

  mSortOrder = sortOrder;
}

void QgsAttributeTableConfig::writeXml( QDomNode &node ) const
{
  QDomDocument doc( node.ownerDocument() );

  QDomElement configElement  = doc.createElement( QStringLiteral( "attributetableconfig" ) );
  configElement.setAttribute( QStringLiteral( "actionWidgetStyle" ), mActionWidgetStyle == ButtonList ? "buttonList" : "dropDown" );

  configElement.setAttribute( QStringLiteral( "sortExpression" ), mSortExpression );

  configElement.setAttribute( QStringLiteral( "sortOrder" ), mSortOrder );

  QDomElement columnsElement  = doc.createElement( QStringLiteral( "columns" ) );

  const auto constMColumns = mColumns;
  for ( const ColumnConfig &column : constMColumns )
  {
    QDomElement columnElement = doc.createElement( QStringLiteral( "column" ) );

    if ( column.type == Action )
    {
      columnElement.setAttribute( QStringLiteral( "type" ), QStringLiteral( "actions" ) );
    }
    else
    {
      columnElement.setAttribute( QStringLiteral( "type" ), QStringLiteral( "field" ) );
      columnElement.setAttribute( QStringLiteral( "name" ), column.name );
    }

    columnElement.setAttribute( QStringLiteral( "hidden" ), column.hidden );
    columnElement.setAttribute( QStringLiteral( "width" ), QString::number( column.width ) );

    columnsElement.appendChild( columnElement );
  }

  configElement.appendChild( columnsElement );

  node.appendChild( configElement );
}

bool QgsAttributeTableConfig::hasSameColumns( const QgsAttributeTableConfig &other ) const
{
  if ( columns().size() == other.columns().size() )
  {
    for ( int i = 0; i < columns().size(); i++ )
    {
      if ( columns().at( i ).name != other.columns().at( i ).name ||
           columns().at( i ).type != other.columns().at( i ).type ||
           columns().at( i ).hidden != other.columns().at( i ).hidden )
      {
        return false;
      }
    }
    return true;
  }

  return false;
}

bool QgsAttributeTableConfig::ColumnConfig::operator== ( const ColumnConfig &other ) const
{
  return type == other.type && name == other.name && hidden == other.hidden && width == other.width;
}
