/***************************************************************************
  qgsstacktrace.cpp - QgsStackTrace

 ---------------------
 begin                : 24.4.2017
 copyright            : (C) 2017 by Nathan Woodrow
 email                : woodrow.nathan@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsstacktrace.h"

#include <QVector>

#ifdef WIN32
#include <windows.h>
#include <dbghelp.h>
#endif

#include "qgis.h"

///@cond PRIVATE

#ifdef _MSC_VER
QVector<QgsStackTrace::StackLine> QgsStackTrace::trace( _EXCEPTION_POINTERS *ExceptionInfo )
{
  QgsStackLines stack;
#ifndef QGISDEBUG
  return stack;
#endif

  HANDLE process = GetCurrentProcess();
  SymSetOptions( SYMOPT_DEFERRED_LOADS | SYMOPT_INCLUDE_32BIT_MODULES | SYMOPT_UNDNAME );

  PCSTR paths;
  if ( QgsStackTrace::mSymbolPaths.isEmpty() )
  {
    paths = NULL;
  }
  else
  {
    paths = QgsStackTrace::mSymbolPaths.toStdString().c_str();
  }

  SymInitialize( process, paths, TRUE );

  // StackWalk64() may modify context record passed to it, so we will
  // use a copy.
  CONTEXT context_record;
  if ( ExceptionInfo )
    context_record = *ExceptionInfo->ContextRecord;
  else
    RtlCaptureContext( &context_record );

  // Initialize stack walking.
  STACKFRAME64 stack_frame;
  memset( &stack_frame, 0, sizeof( stack_frame ) );
#if defined(_WIN64)
  int machine_type = IMAGE_FILE_MACHINE_AMD64;
  stack_frame.AddrPC.Offset = context_record.Rip;
  stack_frame.AddrFrame.Offset = context_record.Rbp;
  stack_frame.AddrStack.Offset = context_record.Rsp;
#else
  int machine_type = IMAGE_FILE_MACHINE_I386;
  stack_frame.AddrPC.Offset = context_record.Eip;
  stack_frame.AddrFrame.Offset = context_record.Ebp;
  stack_frame.AddrStack.Offset = context_record.Esp;
#endif
  stack_frame.AddrPC.Mode = AddrModeFlat;
  stack_frame.AddrFrame.Mode = AddrModeFlat;
  stack_frame.AddrStack.Mode = AddrModeFlat;

  SYMBOL_INFO *symbol = ( SYMBOL_INFO * ) qgsMalloc( sizeof( SYMBOL_INFO ) + MAX_SYM_NAME );
  symbol->SizeOfStruct = sizeof( SYMBOL_INFO );
  symbol->MaxNameLen = MAX_SYM_NAME;

  IMAGEHLP_LINE *line = ( IMAGEHLP_LINE * ) qgsMalloc( sizeof( IMAGEHLP_LINE ) );
  line->SizeOfStruct = sizeof( IMAGEHLP_LINE );

  IMAGEHLP_MODULE *module = ( IMAGEHLP_MODULE * ) qgsMalloc( sizeof( IMAGEHLP_MODULE ) );
  module->SizeOfStruct = sizeof( IMAGEHLP_MODULE );

  while ( StackWalk64( machine_type,
                       GetCurrentProcess(),
                       GetCurrentThread(),
                       &stack_frame,
                       &context_record,
                       NULL,
                       &SymFunctionTableAccess64,
                       &SymGetModuleBase64,
                       NULL ) )
  {

    DWORD64 displacement = 0;

    if ( SymFromAddr( process, ( DWORD64 )stack_frame.AddrPC.Offset, &displacement, symbol ) )
    {
      DWORD dwDisplacement;

      QgsStackTrace::StackLine stackline;
      stackline.symbolName = QString( symbol->Name );

      if ( SymGetLineFromAddr( process, ( DWORD64 ) stack_frame.AddrPC.Offset, &dwDisplacement, line ) )
      {
        stackline.fileName = QString( line->FileName );
        stackline.lineNumber = QString::number( line->LineNumber );
      }
      else
      {
        stackline.fileName = "(unknown file)";
        stackline.lineNumber = "(unknown line)";
      }

      if ( SymGetModuleInfo( process, ( DWORD64 ) stack_frame.AddrPC.Offset, module ) )
      {
        stackline.moduleName = module->ModuleName;
      }
      else
      {
        stackline.moduleName = "(unknown module)";
      }

      stack.append( stackline );
    }
  }

  qgsFree( symbol );
  qgsFree( line );
  qgsFree( module );
  SymCleanup( process );
  return stack;
}

QString QgsStackTrace::mSymbolPaths;

void QgsStackTrace::setSymbolPath( QString symbolPaths )
{
  mSymbolPaths = symbolPaths;
}

#endif // _MSC_VER

#ifdef Q_OS_LINUX
QVector<QgsStackTrace::StackLine> QgsStackTrace::trace( unsigned int maxFrames )
{
  Q_UNUSED( maxFrames )
  QgsStackLines stack;
#ifndef QGISDEBUG
  return stack;
#endif

  // TODO Add linux stack trace support. Pull it from main.cpp
  return stack;
}
#endif

bool QgsStackTrace::StackLine::isQgisModule() const
{
  return moduleName.contains( QLatin1String( "qgis" ), Qt::CaseInsensitive );
}

bool QgsStackTrace::StackLine::isValid() const
{
  return !( fileName.contains( QLatin1String( "exe_common" ), Qt::CaseInsensitive ) ||
            fileName.contains( QLatin1String( "unknown" ), Qt::CaseInsensitive ) ||
            lineNumber.contains( QLatin1String( "unknown" ), Qt::CaseInsensitive ) );

}
///@endcond
