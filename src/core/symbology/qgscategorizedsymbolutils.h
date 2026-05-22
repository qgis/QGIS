/***************************************************************************
    qgscategorizedsymbolutils.h
    ---------------------
    begin                : May 2026
    copyright            : (C) 2026 by Jean Felder
    email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexpression.h"
#include "qgsfields.h"
#include "qgsvariantutils.h"

#include <QString>

#define SIP_NO_FILE

using namespace Qt::StringLiterals;


/**
 * \ingroup core
 * \brief Helper class to manipulate categories
 * \since QGIS 4.2
 */
template<typename RendererType> class QgsCategorizedSymbolUtils
{
  public:
    using Categories = RendererType::Categories;

    /**
     * Builds a filter expression from categories.
     *
     * Generates an expression based on active categories for a field or expression.
     *
     * \param attributeName Name of the field or expression.
     * \param fields list of fields
     * \param categories renderer categories
     *
     * \return a filter expression or an empty string if everything is active
     */
    static QString buildCategorizedFilter( const QString &attributeName, const QgsFields &fields, const Categories &categories )
    {
      const int attributeNumber = fields.lookupField( attributeName );
      const bool isExpression = ( attributeNumber == -1 );

      bool hasDefault = false;
      bool defaultActive = false;
      bool allActive = true;
      bool noneActive = true;

      // we need to build lists of both inactive and active values, as either list may be required
      // depending on whether the default category is active or not
      QString activeValues;
      QString inactiveValues;

      for ( const typename RendererType::Category &category : categories )
      {
        const QVariant variantValue = category.value();
        if ( variantValue == "" || QgsVariantUtils::isNull( variantValue ) )
        {
          hasDefault = true;
          defaultActive = category.renderState();
        }
        noneActive = noneActive && !category.renderState();
        allActive = allActive && category.renderState();

        const bool isList = variantValue.userType() == QMetaType::Type::QVariantList;
        const QString quotedValue = QgsExpression::quotedValue( variantValue, static_cast<QMetaType::Type>( variantValue.userType() ) );

        if ( !category.renderState() )
        {
          if ( quotedValue != "" )
          {
            if ( isList )
            {
              const QVariantList list = variantValue.toList();
              for ( const QVariant &v : list )
              {
                if ( !inactiveValues.isEmpty() )
                  inactiveValues.append( ',' );

                inactiveValues.append( QgsExpression::quotedValue( v, isExpression ? static_cast<QMetaType::Type>( v.userType() ) : fields.at( attributeNumber ).type() ) );
              }
            }
            else
            {
              if ( !inactiveValues.isEmpty() )
                inactiveValues.append( ',' );

              inactiveValues.append( quotedValue );
            }
          }
        }
        else
        {
          if ( quotedValue != "" )
          {
            if ( isList )
            {
              const QVariantList list = variantValue.toList();
              for ( const QVariant &v : list )
              {
                if ( !activeValues.isEmpty() )
                  activeValues.append( ',' );

                activeValues.append( QgsExpression::quotedValue( v, isExpression ? static_cast<QMetaType::Type>( v.userType() ) : fields.at( attributeNumber ).type() ) );
              }
            }
            else
            {
              if ( !activeValues.isEmpty() )
                activeValues.append( ',' );

              activeValues.append( quotedValue );
            }
          }
        }
      }

      const QString attr = isExpression ? attributeName : u"\"%1\""_s.arg( attributeName );

      if ( allActive && hasDefault )
      {
        return QString();
      }
      if ( noneActive )
      {
        return u"FALSE"_s;
      }
      if ( defaultActive )
      {
        return u"(%1) NOT IN (%2) OR (%1) IS NULL"_s.arg( attr, inactiveValues );
      }
      return u"(%1) IN (%2)"_s.arg( attr, activeValues );
    }
};
