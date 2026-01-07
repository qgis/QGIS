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

#include "qgis.h"
#include "qgis_core.h"

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
      res.insert( u"child_id"_s, childId );
      res.insert( u"conditional_branch"_s, conditionalBranch );
      return res;
    }

    /**
     * Loads this dependency from a QVariantMap.
     * \see toVariant()
     */
    bool loadVariant( const QVariantMap &map )
    {
      childId = map.value( u"child_id"_s ).toString();
      conditionalBranch = map.value( u"conditional_branch"_s ).toString();
      return true;
    }

    // TODO c++20 - replace with = default
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
