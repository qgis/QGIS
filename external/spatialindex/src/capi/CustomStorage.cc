/******************************************************************************
 * Project:  libsidx - A C API wrapper around libspatialindex
 * Purpose:  C++ object to implement the custom storage manager.
 * Author:   Matthias (nitro), nitro@dr-code.org
 ******************************************************************************
 * Copyright (c) 2010, Matthias (nitro)
 *
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
******************************************************************************/

#include <spatialindex/capi/sidx_impl.h>

using namespace SpatialIndex;
using namespace SpatialIndex::StorageManager;


IStorageManager* SpatialIndex::StorageManager::returnCustomStorageManager(Tools::PropertySet& ps)
{
	IStorageManager* sm = new CustomStorageManager(ps);
	return sm;
}

CustomStorageManager::CustomStorageManager(Tools::PropertySet& ps)
{
	Tools::Variant var;
	var = ps.getProperty("CustomStorageCallbacks");

	if (var.m_varType != Tools::VT_EMPTY)
	{
		if (var.m_varType != Tools::VT_PVOID)
			throw Tools::IllegalArgumentException("CustomStorageManager: Property CustomStorageCallbacks must be Tools::VT_PVOID");

        if (!var.m_val.pvVal)
			throw Tools::IllegalArgumentException("CustomStorageManager: Property CustomStorageCallbacks must not be 0.");

        // we already checked for validity in IndexProperty_SetCustomStorageCallbacks
        CustomStorageManagerCallbacks* callbackArray = static_cast<CustomStorageManagerCallbacks*>(var.m_val.pvVal);
		callbacks = *callbackArray;
	}

    int errorCode( NoError );
    if ( callbacks.createCallback ) callbacks.createCallback( callbacks.context, &errorCode );
    processErrorCode( errorCode, NewPage );
}

CustomStorageManager::~CustomStorageManager()
{
    int errorCode( NoError );
    if ( callbacks.destroyCallback ) callbacks.destroyCallback( callbacks.context, &errorCode );
    processErrorCode( errorCode, NewPage );
}

void CustomStorageManager::flush()
{
    int errorCode( NoError );
    if ( callbacks.flushCallback ) callbacks.flushCallback( callbacks.context, &errorCode );
    processErrorCode( errorCode, NewPage );
}

void CustomStorageManager::loadByteArray(const id_type page, uint32_t& len, uint8_t** data)
{
    int errorCode( NoError );
    if ( callbacks.loadByteArrayCallback ) callbacks.loadByteArrayCallback( callbacks.context, page, &len, data, &errorCode );
    processErrorCode( errorCode, page );
}

void CustomStorageManager::storeByteArray(id_type& page, const uint32_t len, const uint8_t* const data)
{
    int errorCode( NoError );
    if ( callbacks.storeByteArrayCallback ) callbacks.storeByteArrayCallback( callbacks.context, &page, len, data, &errorCode );
    processErrorCode( errorCode, page );
}

void CustomStorageManager::deleteByteArray(const id_type page)
{
    int errorCode( NoError );
    if ( callbacks.deleteByteArrayCallback ) callbacks.deleteByteArrayCallback( callbacks.context, page, &errorCode );
    processErrorCode( errorCode, page );
}

inline void CustomStorageManager::processErrorCode(int errorCode, const id_type page)
{
    switch (errorCode)
    {
    case NoError:
    break;

    case InvalidPageError:
        throw InvalidPageException( page );
    break;

    case IllegalStateError:
        throw Tools::IllegalStateException( "CustomStorageManager: Error in user implementation." );
    break;

    default:
        throw Tools::IllegalStateException( "CustomStorageManager: Unknown error." );
    }
}
