/***************************************************************************
                              qgsserverfeatureid.cpp
                              -----------------------
  begin                : May 17, 2019
  copyright            : (C) 2019 by René-Luc DHONT
  email                : rldhont at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsserverfeatureid.h"
#include "qgsfeature.h"
#include "qgsfeaturerequest.h"
#include "qgsvectordataprovider.h"
#include "qgsexpression.h"

QString QgsServerFeatureId::getServerFid( const QgsFeature &feature, const QgsAttributeList &pkAttributes )
{
  if ( pkAttributes.isEmpty() )
  {
    return QString::number( feature.id() );
  }

  QStringList pkValues;
  QgsAttributeList::const_iterator it = pkAttributes.constBegin();
  if ( it != pkAttributes.constEnd() )
  {
    pkValues.append( feature.attribute( *it ).toString() );
  }
  return pkValues.join( pkSeparator() );
}

QgsFeatureRequest QgsServerFeatureId::updateFeatureRequestFromServerFids( QgsFeatureRequest &featureRequest, const QStringList &serverFids, const QgsVectorDataProvider *provider )
{
  const QgsAttributeList &pkAttributes = provider->pkAttributeIndexes();

  if ( pkAttributes.isEmpty() )
  {
    QgsFeatureIds fids;
    for ( const QString &serverFid : serverFids )
    {
      fids.insert( serverFid.toLongLong() );
    }
    featureRequest.setFilterFids( fids );
    return featureRequest;
  }

  QStringList expList;
  for ( const QString &serverFid : serverFids )
  {
    expList.append( QgsServerFeatureId::getExpressionFromServerFid( serverFid, provider ) );
  }

  if ( expList.count() == 1 )
  {
    featureRequest.setFilterExpression( expList.at( 0 ) );
  }
  else
  {
    QString fullExpression;
    for ( const QString &exp : qgis::as_const( expList ) )
    {
      if ( !fullExpression.isEmpty() )
      {
        fullExpression.append( QStringLiteral( " OR " ) );
      }
      fullExpression.append( QStringLiteral( "( " ) );
      fullExpression.append( exp );
      fullExpression.append( QStringLiteral( " )" ) );
    }
    featureRequest.setFilterExpression( fullExpression );
  }

  return featureRequest;
}

QString QgsServerFeatureId::getExpressionFromServerFid( const QString &serverFid, const QgsVectorDataProvider *provider )
{
  const QgsAttributeList &pkAttributes = provider->pkAttributeIndexes();

  if ( pkAttributes.isEmpty() )
  {
    return QString();
  }

  const QgsFields &fields = provider->fields();

  QString expressionString;
  QStringList pkValues = serverFid.split( pkSeparator() );
  int pkExprSize = std::min( pkAttributes.size(), pkValues.size() );
  for ( int i = 0; i < pkExprSize; ++i )
  {
    if ( i > 0 )
    {
      expressionString.append( QStringLiteral( " AND " ) );
    }

    QString fieldName = fields[ pkAttributes.at( i ) ].name();
    expressionString.append( QgsExpression::createFieldEqualityExpression( fieldName, QVariant( pkValues.at( i ) ) ) );
  }

  return expressionString;
}

QString QgsServerFeatureId::pkSeparator()
{
  return QStringLiteral( "@@" );
}
