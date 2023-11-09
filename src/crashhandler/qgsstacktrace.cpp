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
#include <iostream>

#ifdef _MSC_VER
#define _NO_CVCONST_H
#define _CRT_STDIO_ISO_WIDE_SPECIFIERS
#endif

#include "qgsstacktrace.h"

#include <QDir>
#include <QVector>
#include <QFile>
#include <QTextStream>

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef _MSC_VER
#include <tlhelp32.h>

#include <Windows.h>
#include <DbgHelp.h>
#endif


///@cond PRIVATE

#ifdef Q_OS_WIN
enum BasicType
{
  btNoType = 0,
  btVoid = 1,
  btChar = 2,
  btWChar = 3,
  btInt = 6,
  btUInt = 7,
  btFloat = 8,
  btBCD = 9,
  btBool = 10,
  btLong = 13,
  btULong = 14,
  btCurrency = 25,
  btDate = 26,
  btVariant = 27,
  btComplex = 28,
  btBit = 29,
  btBSTR = 30,
  btHresult = 31,
};

#define WIDEN_(x) L ## x
#define WIDEN(x) WIDEN_(x)

typedef struct _StackTrace
{
  wchar_t message[2 * 1024 * 1024];
  int written;
  HANDLE process;
  HANDLE thread;
  PCONTEXT contextRecord;
  STACKFRAME64 currentStackFrame;
  bool isFirstParameter;
  LPVOID scratchSpace;
} StackTrace;

bool printBasicType( StackTrace *stackTrace, PSYMBOL_INFOW pSymInfo, ULONG typeIndex, void *valueLocation, bool whilePrintingPointer )
{
  enum BasicType basicType;
  if ( !SymGetTypeInfo( stackTrace->process, pSymInfo->ModBase, typeIndex, TI_GET_BASETYPE, &basicType ) )
  {
    return false;
  }

  switch ( basicType )
  {
    case btChar:
    {
      if ( !whilePrintingPointer )
      {
        char value;
        ReadProcessMemory( stackTrace->process, valueLocation, &value, sizeof( value ), NULL );

        stackTrace->written +=
          swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                      L"'%c'", value );
      }
      else
      {
        char *value = ( char * )valueLocation;

        MEMORY_BASIC_INFORMATION pageInfo = { 0 };
        if ( VirtualQueryEx( stackTrace->process, value, &pageInfo, sizeof( pageInfo ) ) == 0 )
        {
          return false;
        }

        PVOID pageEndAddress = ( char * )pageInfo.BaseAddress + pageInfo.RegionSize;

        stackTrace->written +=
          swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                      L"\"" );

        for ( int charsWritten = 0; charsWritten < 100; )
        {
          if ( ( void * )value < pageEndAddress )
          {
            char next;
            ReadProcessMemory( stackTrace->process, value, &next, sizeof( next ), NULL );

            if ( next != '\0' )
            {
              if ( charsWritten == 100 - 1 )
              {
                stackTrace->written +=
                  swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                              L"..." );
                break;
              }
              else
              {
                stackTrace->written +=
                  swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                              L"%c", next );
                charsWritten++;
                value++;
              }
            }
            else
            {
              break;
            }
          }
          else
          {
            if ( VirtualQueryEx( stackTrace->process, pageEndAddress, &pageInfo, sizeof( pageInfo ) ) == 0 )
            {
              stackTrace->written +=
                swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                            L"<Bad memory>" );
              break;
            }

            pageEndAddress = ( char * )pageInfo.BaseAddress + pageInfo.RegionSize;
          }
        }

        stackTrace->written +=
          swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                      L"\"" );
      }

      break;
    }

    case btWChar:
    {
      if ( !whilePrintingPointer )
      {
        wchar_t value;
        ReadProcessMemory( stackTrace->process, valueLocation, &value, sizeof( value ), NULL );

        stackTrace->written +=
          swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                      L"'%lc'", value );
      }
      else
      {
        wchar_t *value = ( wchar_t * )valueLocation;

        MEMORY_BASIC_INFORMATION pageInfo = { 0 };
        if ( VirtualQueryEx( stackTrace->process, value, &pageInfo, sizeof( pageInfo ) ) == 0 )
        {
          return false;
        }

        PVOID pageEndAddress = ( char * )pageInfo.BaseAddress + pageInfo.RegionSize;

        stackTrace->written +=
          swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                      L"L\"" );

        for ( int charsWritten = 0; charsWritten < 100; )
        {
          if ( ( void * )( ( char * )value + 1 ) < pageEndAddress )
          {
            wchar_t next;
            ReadProcessMemory( stackTrace->process, value, &next, sizeof( next ), NULL );

            if ( next != L'\0' )
            {
              if ( charsWritten == 100 - 1 )
              {
                stackTrace->written +=
                  swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                              L"..." );
                break;
              }
              else
              {
                stackTrace->written +=
                  swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                              L"%lc", next );
                charsWritten++;
                value++;
              }
            }
            else
            {
              break;
            }
          }
          else
          {
            if ( VirtualQueryEx( stackTrace->process, pageEndAddress, &pageInfo, sizeof( pageInfo ) ) == 0 )
            {
              stackTrace->written +=
                swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                            L"<Bad memory>" );
              break;
            }

            pageEndAddress = ( char * )pageInfo.BaseAddress + pageInfo.RegionSize;
          }
        }

        stackTrace->written +=
          swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                      L"\"" );
      }

      break;
    }

    case btInt:
    {
      ULONG64 length;
      if ( !SymGetTypeInfo( stackTrace->process, pSymInfo->ModBase, typeIndex, TI_GET_LENGTH, &length ) )
      {
        return false;
      }

      switch ( length )
      {
        case sizeof( int8_t ) :
        {
          int8_t value;
          ReadProcessMemory( stackTrace->process, valueLocation, &value, sizeof( value ), NULL );

          stackTrace->written +=
            swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                        L"%" WIDEN( PRId8 ), value );
          break;
        }

        case sizeof( int16_t ) :
        {
          int16_t value;
          ReadProcessMemory( stackTrace->process, valueLocation, &value, sizeof( value ), NULL );

          stackTrace->written +=
            swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                        L"%" WIDEN( PRId16 ), value );
          break;
        }

        case sizeof( int32_t ) :
        {
          int32_t value;
          ReadProcessMemory( stackTrace->process, valueLocation, &value, sizeof( value ), NULL );

          stackTrace->written +=
            swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                        L"%" WIDEN( PRId32 ), value );
          break;
        }

        case sizeof( int64_t ) :
        {
          int64_t value;
          ReadProcessMemory( stackTrace->process, valueLocation, &value, sizeof( value ), NULL );

          stackTrace->written +=
            swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                        L"%" WIDEN( PRId64 ), value );
          break;
        }

        default:
        {
          return false;
          break;
        }
      }

      break;
    }

    case btUInt:
    {
      ULONG64 length;
      if ( !SymGetTypeInfo( stackTrace->process, pSymInfo->ModBase, typeIndex, TI_GET_LENGTH, &length ) )
      {
        return false;
      }

      switch ( length )
      {
        case sizeof( uint8_t ) :
        {
          uint8_t value;
          ReadProcessMemory( stackTrace->process, valueLocation, &value, sizeof( value ), NULL );

          stackTrace->written +=
            swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                        L"%" WIDEN( PRIu8 ), value );
          break;
        }

        case sizeof( uint16_t ) :
        {
          uint16_t value;
          ReadProcessMemory( stackTrace->process, valueLocation, &value, sizeof( value ), NULL );

          stackTrace->written +=
            swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                        L"%" WIDEN( PRIu16 ), value );
          break;
        }

        case sizeof( uint32_t ) :
        {
          uint32_t value;
          ReadProcessMemory( stackTrace->process, valueLocation, &value, sizeof( value ), NULL );

          stackTrace->written +=
            swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                        L"%" WIDEN( PRIu32 ), value );
          break;
        }

        case sizeof( uint64_t ) :
        {
          uint64_t value;
          ReadProcessMemory( stackTrace->process, valueLocation, &value, sizeof( value ), NULL );

          stackTrace->written +=
            swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                        L"%" WIDEN( PRIu64 ), value );
          break;
        }

        default:
        {
          return false;
          break;
        }
      }

      break;
    }

    case btFloat:
    {
      float value;
      ReadProcessMemory( stackTrace->process, valueLocation, &value, sizeof( value ), NULL );

      stackTrace->written +=
        swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                    L"%f", value );
      break;
    }

    case btLong:
    {
      long value;
      ReadProcessMemory( stackTrace->process, valueLocation, &value, sizeof( value ), NULL );

      stackTrace->written +=
        swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                    L"%ld", value );
      break;
    }

    case btULong:
    {
      unsigned long value;
      ReadProcessMemory( stackTrace->process, valueLocation, &value, sizeof( value ), NULL );

      stackTrace->written +=
        swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                    L"%lu", value );
      break;
    }

    default:
    {
      return false;
      break;
    }
  }

  return true;
}

bool printGivenType( StackTrace *stackTrace, PSYMBOL_INFOW pSymInfo, enum SymTagEnum symbolTag, ULONG typeIndex, void *valueLocation, bool whilePrintingPointer )
{
  switch ( symbolTag )
  {
    case SymTagBaseType:
    {
      if ( !printBasicType( stackTrace, pSymInfo, typeIndex, valueLocation, whilePrintingPointer ) )
      {
        return false;
      }

      break;
    }

    case SymTagPointerType:
    {
      void *pointedValueLocation;
      ReadProcessMemory( stackTrace->process, valueLocation, &pointedValueLocation, sizeof( pointedValueLocation ), NULL );


      stackTrace->written +=
        swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                    L"0x%p -> ", pointedValueLocation );

      if ( pointedValueLocation < ( void * )0x1000 )
      {
        stackTrace->written +=
          swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                      L"?" );
        break;
      }

      DWORD pointedTypeIndex;
      if ( !SymGetTypeInfo( stackTrace->process, pSymInfo->ModBase, typeIndex, TI_GET_TYPE, &pointedTypeIndex ) )
      {
        return false;
      }

      ULONG64 pointedTypeLength;
      if ( !SymGetTypeInfo( stackTrace->process, pSymInfo->ModBase, pointedTypeIndex, TI_GET_LENGTH, &pointedTypeLength ) )
      {
        return false;
      }

      MEMORY_BASIC_INFORMATION pageInfo = { 0 };
      if ( VirtualQueryEx( stackTrace->process, pointedValueLocation, &pageInfo, sizeof( pageInfo ) ) == 0 )
      {
        return false;
      }

      for ( ;; )
      {
        PVOID pageEndAddress = ( char * )pageInfo.BaseAddress + pageInfo.RegionSize;

        if ( ( void * )( ( char * )pointedValueLocation + pointedTypeLength ) >= pageEndAddress )
        {
          if ( VirtualQueryEx( stackTrace->process, pageEndAddress, &pageInfo, sizeof( pageInfo ) ) == 0 )
          {
            return false;
          }
        }
        else
        {
          break;
        }
      }

      enum SymTagEnum pointedSymbolTag;
      if ( !SymGetTypeInfo( stackTrace->process, pSymInfo->ModBase, pointedTypeIndex, TI_GET_SYMTAG, &pointedSymbolTag ) )
      {
        return false;
      }

      return printGivenType( stackTrace, pSymInfo, pointedSymbolTag, pointedTypeIndex, pointedValueLocation, true );
    }

    default:
    {
      return false;
    }
  }

  return true;
}

bool printType( StackTrace *stackTrace, PSYMBOL_INFOW pSymInfo, void *valueLocation )
{
  enum SymTagEnum symbolTag;
  if ( !SymGetTypeInfo( stackTrace->process, pSymInfo->ModBase, pSymInfo->TypeIndex, TI_GET_SYMTAG, &symbolTag ) )
  {
    return false;
  }

  return printGivenType( stackTrace, pSymInfo, symbolTag, pSymInfo->TypeIndex, valueLocation, false );
}


BOOL CALLBACK enumParams(
  _In_ PSYMBOL_INFOW pSymInfo,
  _In_ ULONG SymbolSize,
  _In_opt_ PVOID UserContext )
{
  if ( ( pSymInfo->Flags & SYMFLAG_LOCAL ) == 0 )
  {
    return TRUE;
  }

  StackTrace *stackTrace = ( StackTrace * )UserContext;

  stackTrace->written +=
    swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                L"\n" );

  if ( pSymInfo->Flags & SYMFLAG_PARAMETER )
  {
    stackTrace->written +=
      swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                  L"Parm: " );
  }
  else if ( pSymInfo->Flags & SYMFLAG_LOCAL )
  {
    stackTrace->written +=
      swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                  L"Local: " );
  }

  stackTrace->written +=
    swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                L"%ls = ", pSymInfo->Name );

  void *valueLocation = NULL;
  if ( pSymInfo->Flags & SYMFLAG_REGISTER )
  {
    if ( stackTrace->scratchSpace == NULL )
    {
      stackTrace->written +=
        swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                    L"<Could not allocate memory to write register value>" );
      return TRUE;
    }

    valueLocation = stackTrace->scratchSpace;

    switch ( pSymInfo->Register )
    {
#ifndef _WIN64
      case 17:
        WriteProcessMemory( stackTrace->process, valueLocation, &stackTrace->contextRecord->Eax, sizeof( stackTrace->contextRecord->Eax ), NULL );
        break;
      case 18:
        WriteProcessMemory( stackTrace->process, valueLocation, &stackTrace->contextRecord->Ecx, sizeof( stackTrace->contextRecord->Ecx ), NULL );
        break;
      case 19:
        WriteProcessMemory( stackTrace->process, valueLocation, &stackTrace->contextRecord->Edx, sizeof( stackTrace->contextRecord->Edx ), NULL );
        break;
      case 20:
        WriteProcessMemory( stackTrace->process, valueLocation, &stackTrace->contextRecord->Ebx, sizeof( stackTrace->contextRecord->Ebx ), NULL );
        break;
#else
      case 328:
        WriteProcessMemory( stackTrace->process, valueLocation, &stackTrace->contextRecord->Rax, sizeof( stackTrace->contextRecord->Rax ), NULL );
        break;

      case 329:
        WriteProcessMemory( stackTrace->process, valueLocation, &stackTrace->contextRecord->Rbx, sizeof( stackTrace->contextRecord->Rbx ), NULL );
        break;

      case 330:
        WriteProcessMemory( stackTrace->process, valueLocation, &stackTrace->contextRecord->Rcx, sizeof( stackTrace->contextRecord->Rcx ), NULL );
        break;

      case 331:
        WriteProcessMemory( stackTrace->process, valueLocation, &stackTrace->contextRecord->Rdx, sizeof( stackTrace->contextRecord->Rdx ), NULL );
        break;

      case 336:
        WriteProcessMemory( stackTrace->process, valueLocation, &stackTrace->contextRecord->R8, sizeof( stackTrace->contextRecord->R8 ), NULL );
        break;

      case 337:
        WriteProcessMemory( stackTrace->process, valueLocation, &stackTrace->contextRecord->R9, sizeof( stackTrace->contextRecord->R9 ), NULL );
        break;

      case 154:
        WriteProcessMemory( stackTrace->process, valueLocation, &stackTrace->contextRecord->Xmm0, sizeof( stackTrace->contextRecord->Xmm0 ), NULL );
        break;

      case 155:
        WriteProcessMemory( stackTrace->process, valueLocation, &stackTrace->contextRecord->Xmm1, sizeof( stackTrace->contextRecord->Xmm1 ), NULL );
        break;

      case 156:
        WriteProcessMemory( stackTrace->process, valueLocation, &stackTrace->contextRecord->Xmm2, sizeof( stackTrace->contextRecord->Xmm2 ), NULL );
        break;

      case 157:
        WriteProcessMemory( stackTrace->process, valueLocation, &stackTrace->contextRecord->Xmm3, sizeof( stackTrace->contextRecord->Xmm3 ), NULL );
        break;
#endif

      default:
        stackTrace->written +=
          swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                      L"<Unknown register %lu>", pSymInfo->Register );
        return TRUE;
    }
  }
  else if ( pSymInfo->Flags & SYMFLAG_LOCAL )
  {
#ifndef _WIN64
    valueLocation = ( ( char * )stackTrace->contextRecord->Ebp ) + pSymInfo->Address;
#else
    valueLocation = ( ( char * )stackTrace->contextRecord->Rbp ) + pSymInfo->Address;
#endif
  }
  else if ( pSymInfo->Flags & SYMFLAG_REGREL )
  {
    switch ( pSymInfo->Register )
    {
#ifndef _WIN64
      case 22:
        valueLocation = ( ( char * )stackTrace->contextRecord->Ebp ) + pSymInfo->Address;
        break;
#else
      case 334:
        valueLocation = ( ( char * )stackTrace->contextRecord->Rbp ) + pSymInfo->Address;
        break;
      case 335:
        valueLocation = ( ( char * )stackTrace->contextRecord->Rsp ) + 0x20 + pSymInfo->Address;
        break;
#endif

      default:
        stackTrace->written +=
          swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                      L"<Relative to unknown register %lu>", pSymInfo->Register );
        return TRUE;
    }
  }
  else
  {
    valueLocation = ( void * )( stackTrace->currentStackFrame.AddrFrame.Offset + pSymInfo->Address );
  }

  if ( !printType( stackTrace, pSymInfo, valueLocation ) )
  {
    stackTrace->written +=
      swprintf_s( stackTrace->message + stackTrace->written, sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                  L"?" );
  }

  return TRUE;
}

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
void GetLastErrorAsString()
{
  //Get the error message, if any.
  DWORD errorMessageID = ::GetLastError();

  LPSTR messageBuffer = nullptr;
  size_t size = FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                NULL, errorMessageID, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), ( LPSTR )&messageBuffer, 0, NULL );

  std::string message( messageBuffer, size );

  std::cout << message << std::endl;

  //Free the buffer.
  LocalFree( messageBuffer );
}

void getStackTrace( StackTrace *stackTrace, QString symbolPath, QgsStackTrace *trace )
{
  SymSetOptions( SYMOPT_DEFERRED_LOADS | SYMOPT_INCLUDE_32BIT_MODULES | SYMOPT_UNDNAME );
  if ( !SymInitialize( stackTrace->process, symbolPath.toStdString().c_str(), TRUE ) )
  {
    trace->symbolsLoaded = false;
    return;
  }
  trace->symbolsLoaded = true;

  SYMBOL_INFOW *symbol = ( SYMBOL_INFOW * )calloc( sizeof( *symbol ) + 256 * sizeof( wchar_t ), 1 );
  symbol->MaxNameLen = 255;
  symbol->SizeOfStruct = sizeof( SYMBOL_INFOW );

#ifndef _WIN64
  DWORD machineType = IMAGE_FILE_MACHINE_I386;
#else
  DWORD machineType = IMAGE_FILE_MACHINE_AMD64;
#endif

  for ( int i = 0; ; i++ )
  {
    if ( !StackWalk64( machineType,
                       stackTrace->process,
                       stackTrace->thread,
                       &stackTrace->currentStackFrame,
                       stackTrace->contextRecord,
                       NULL,
                       SymFunctionTableAccess64,
                       SymGetModuleBase64,
                       NULL ) )
    {
      break;
    }

    if ( SymFromAddrW( stackTrace->process, stackTrace->currentStackFrame.AddrPC.Offset, NULL, symbol ) )
    {
      QgsStackTrace::StackLine stackline;
      stackline.symbolName = QString::fromWCharArray( symbol->Name );

      stackTrace->written +=
        swprintf_s( &stackTrace->message[stackTrace->written], sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written - 1,
                    L">%02i: 0x%08llX %ls", i, symbol->Address, symbol->Name );

      IMAGEHLP_STACK_FRAME stackFrame = { 0 };
      stackFrame.InstructionOffset = symbol->Address;
      if ( SymSetContext( stackTrace->process, &stackFrame, NULL ) )
      {
        stackTrace->isFirstParameter = true;
        SymEnumSymbolsW( stackTrace->process, 0, NULL, enumParams, stackTrace );
      }

      DWORD pos;
      IMAGEHLP_LINEW64 lineInfo = { 0 };
      lineInfo.SizeOfStruct = sizeof( lineInfo );
      if ( SymGetLineFromAddrW64( stackTrace->process, stackTrace->currentStackFrame.AddrPC.Offset, &pos, &lineInfo ) )
      {
        stackline.fileName = QString::fromWCharArray( lineInfo.FileName );
        stackline.lineNumber = QString::number( lineInfo.LineNumber );

        stackTrace->written +=
          swprintf_s( &stackTrace->message[stackTrace->written], sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                      L"\nSource: %ls:%lu", lineInfo.FileName, lineInfo.LineNumber );
      }

      stackTrace->written +=
        swprintf_s( &stackTrace->message[stackTrace->written], sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                    L"\n" );

      trace->lines.append( stackline );
    }
    else
    {
      stackTrace->written +=
        swprintf_s( &stackTrace->message[stackTrace->written], sizeof( stackTrace->message ) / sizeof( stackTrace->message[0] ) - stackTrace->written,
                    L">%02i: 0x%08llX ?\n", i, stackTrace->currentStackFrame.AddrPC.Offset );
    }
  }

  free( symbol );

  SymCleanup( stackTrace->process );
}

QgsStackTrace *QgsStackTrace::trace( DWORD processId, DWORD threadId, LPEXCEPTION_POINTERS exception, QString symbolPath )
{
  QgsStackTrace *trace = new QgsStackTrace();
  EXCEPTION_POINTERS remoteException = { 0 };
  CONTEXT remoteContextRecord = { 0 };

  StackTrace *stackTrace = ( StackTrace * )calloc( sizeof( *stackTrace ), 1 );
  stackTrace->process = OpenProcess( PROCESS_ALL_ACCESS, FALSE, processId );
  stackTrace->thread = OpenThread( THREAD_ALL_ACCESS, FALSE, threadId );
  trace->process = stackTrace->process;
  trace->thread = stackTrace->thread;

  HANDLE h = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 );
  if ( h != INVALID_HANDLE_VALUE )
  {
    THREADENTRY32 te;
    te.dwSize = sizeof( te );
    if ( Thread32First( h, &te ) )
    {
      do
      {
        if ( te.dwSize >= FIELD_OFFSET( THREADENTRY32, th32OwnerProcessID ) +
             sizeof( te.th32OwnerProcessID ) )
        {
          if ( te.th32OwnerProcessID == processId )
          {
            HANDLE threadHandle = OpenThread( THREAD_ALL_ACCESS, FALSE, te.th32ThreadID );
            trace->threads.push_back( threadHandle );
            SuspendThread( threadHandle );
          }
        }
        te.dwSize = sizeof( te );
      }
      while ( Thread32Next( h, &te ) );
    }
    CloseHandle( h );
  }

  ReadProcessMemory( stackTrace->process, exception, &remoteException, sizeof( remoteException ), NULL );
  ReadProcessMemory( stackTrace->process, remoteException.ContextRecord, &remoteContextRecord, sizeof( remoteContextRecord ), NULL );

#ifndef _WIN64
  stackTrace->currentStackFrame.AddrPC.Offset = remoteContextRecord.Eip;
  stackTrace->currentStackFrame.AddrPC.Mode = AddrModeFlat;
  stackTrace->currentStackFrame.AddrFrame.Offset = remoteContextRecord.Ebp;
  stackTrace->currentStackFrame.AddrFrame.Mode = AddrModeFlat;
  stackTrace->currentStackFrame.AddrStack.Offset = remoteContextRecord.Esp;
  stackTrace->currentStackFrame.AddrStack.Mode = AddrModeFlat;
#else
  stackTrace->currentStackFrame.AddrPC.Offset = remoteContextRecord.Rip;
  stackTrace->currentStackFrame.AddrPC.Mode = AddrModeFlat;
  stackTrace->currentStackFrame.AddrFrame.Offset = remoteContextRecord.Rbp;
  stackTrace->currentStackFrame.AddrFrame.Mode = AddrModeFlat;
  stackTrace->currentStackFrame.AddrStack.Offset = remoteContextRecord.Rsp;
  stackTrace->currentStackFrame.AddrStack.Mode = AddrModeFlat;
#endif

  stackTrace->contextRecord = &remoteContextRecord;

  // The biggest registers are the XMM registers (128 bits), so reserve enough space for them.
  stackTrace->scratchSpace = VirtualAllocEx( stackTrace->process, NULL, 128 / 8, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE );

  getStackTrace( stackTrace, symbolPath, trace );
  trace->fullStack = QString::fromWCharArray( stackTrace->message );

  return trace;
}

#endif // Q_OS_WIN

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
