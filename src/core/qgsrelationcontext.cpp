/***************************************************************************
    qgsrelationcontext.cpp
     --------------------------------------
    Date                 : 20.12.2019
    Copyright            : (C) 2019 David Marteau
    Email                : dmarteau at 3liz dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/
#include "qgsproject.h"
#include "qgsrelationcontext.h"

QgsRelationContext::QgsRelationContext( QgsProject *project )
  : mProject( project )
{
}

QgsRelationContext::~QgsRelationContext() = default;

QgsRelationContext::QgsRelationContext( const QgsRelationContext &other )
  : mProject( other.mProject )
{
}

QgsRelationContext &QgsRelationContext::operator=( const QgsRelationContext &other )
{
  mProject = other.mProject;
  return *this;
}

const QgsProject *QgsRelationContext::project() const
{
  if ( mProject )
  {
    return mProject;
  }

  // Fallback to qgis instance
  return QgsProject::instance();
}

