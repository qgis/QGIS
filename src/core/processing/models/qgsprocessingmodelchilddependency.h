/***************************************************************************
                         qgsprocessingmodelchilddependency.h
                         -----------------------------
    begin                : April 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGMODELCHILDDEPENDENCY_H
#define QGSPROCESSINGMODELCHILDDEPENDENCY_H

#include "qgis_core.h"
#include "qgis.h"


///@cond NOT_STABLE

/**
 * \class QgsProcessingModelChildDependency
 * \ingroup core
 * \brief Contains details of a child algorithm dependency.
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsProcessingModelChildDependency
{
  public:

    /**
     * Constructor for QgsProcessingModelChildDependency, with the specified \a childId.
     */
    QgsProcessingModelChildDependency( const QString &childId = QString(), const QString &conditionalBranch = QString() )
      : childId( childId )
      , conditionalBranch( conditionalBranch )
    {
    }

    //! Child algorithm ID
    QString childId;

    //! Conditional branch output name, if applicable.
    QString conditionalBranch;

    /**
     * Saves this dependency to a QVariant.
     * \see loadVariant()
     */
    QVariant toVariant() const
    {
      QVariantMap res;
      res.insert( QStringLiteral( "child_id" ), childId );
      res.insert( QStringLiteral( "conditional_branch" ), conditionalBranch );
      return res;
    }

    /**
     * Loads this dependency from a QVariantMap.
     * \see toVariant()
     */
    bool loadVariant( const QVariantMap &map )
    {
      childId = map.value( QStringLiteral( "child_id" ) ).toString();
      conditionalBranch = map.value( QStringLiteral( "conditional_branch" ) ).toString();
      return true;
    }

    bool operator==( const QgsProcessingModelChildDependency &other ) const
    {
      return childId == other.childId && conditionalBranch == other.conditionalBranch;
    }

    bool operator!=( const QgsProcessingModelChildDependency &other ) const
    {
      return !( *this == other );
    }

    bool operator<( const QgsProcessingModelChildDependency &other ) const
    {
      return childId == other.childId ? conditionalBranch < other.conditionalBranch : childId < other.childId;
    }
};

Q_DECLARE_METATYPE( QgsProcessingModelChildDependency )

///@endcond

#endif // QGSPROCESSINGMODELCHILDDEPENDENCY_H
