/***************************************************************************
                             qgsprojutils.h
                             -------------------
    begin                : March 2019
    copyright            : (C) 2019 by Nyall Dawson
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
#include "qgsprojutils.h"

#if PROJ_VERSION_MAJOR>=6
#include <proj.h>
#else
#include <proj_api.h>
#endif


#if defined(USE_THREAD_LOCAL) && !defined(Q_OS_WIN)
thread_local QgsProjContext QgsProjContext::sProjContext;
#else
QThreadStorage< QgsProjContext * > QgsProjContext::sProjContext;
#endif

QgsProjContext::QgsProjContext()
{
#if PROJ_VERSION_MAJOR>=6
  mContext = proj_context_create();
#else
  mContext = pj_ctx_alloc();
#endif
}

QgsProjContext::~QgsProjContext()
{
#if PROJ_VERSION_MAJOR>=6
  proj_context_destroy( mContext );
#else
  pj_ctx_free( mContext );
#endif
}

PJ_CONTEXT *QgsProjContext::get()
{
#if defined(USE_THREAD_LOCAL) && !defined(Q_OS_WIN)
  return sProjContext.mContext;
#else
  PJ_CONTEXT *pContext = nullptr;
  if ( sProjContext.hasLocalData() )
  {
    pContext = sProjContext.localData()->mContext;
  }
  else
  {
    sProjContext.setLocalData( new QgsProjContext() );
    pContext = sProjContext.localData()->mContext;
  }
  return pContext;
#endif
}
