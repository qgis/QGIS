/******************************************************************************
 * Project:  libsidx - A C API wrapper around libspatialindex
 * Purpose:	 C API.
 * Author:   Howard Butler, hobu.inc@gmail.com
 ******************************************************************************
 * Copyright (c) 2009, Howard Butler
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

#ifndef SIDX_API_H_INCLUDED
#define SIDX_API_H_INCLUDED

#define SIDX_C_API 1

#include "sidx_config.h"

#include <stddef.h>

IDX_C_START

SIDX_DLL IndexH Index_Create(IndexPropertyH properties);

SIDX_DLL IndexH Index_CreateWithStream( IndexPropertyH properties,
										int (*readNext)(int64_t *id, double **pMin, double **pMax, uint32_t *nDimension, const uint8_t **pData, size_t *nDataLength)
									   );

SIDX_DLL void Index_Destroy(IndexH index);
SIDX_DLL IndexPropertyH Index_GetProperties(IndexH index);

SIDX_DLL RTError Index_DeleteData(	IndexH index,
									int64_t id,
									double* pdMin,
									double* pdMax,
									uint32_t nDimension);

SIDX_C_DLL RTError Index_DeleteTPData( IndexH index,
                  int64_t id,
                  double* pdMin,
                  double* pdMax,
                  double* pdVMin,
                  double* pdVMax,
                  double tStart,
                  double tEnd,
                  uint32_t nDimension
                  );

SIDX_C_DLL RTError Index_DeleteMVRData( IndexH index,
									int64_t id,
									double* pdMin,
									double* pdMax,
									double tStart,
									double tEnd,
									uint32_t nDimension
									);

SIDX_DLL RTError Index_InsertData(	IndexH index,
									int64_t id,
									double* pdMin,
									double* pdMax,
									uint32_t nDimension,
									const uint8_t* pData,
									size_t nDataLength);

SIDX_C_DLL RTError Index_InsertTPData( IndexH index,
  int64_t id,
  double* pdMin,
  double* pdMax,
  double* pdVMin,
  double* pdVMax,
  double tStart,
  double tEnd,
  uint32_t nDimension,
  const uint8_t* pData,
  size_t nDataLength);

SIDX_C_DLL RTError Index_InsertMVRData( IndexH index,
	int64_t id,
	double* pdMin,
	double* pdMax,
	double tStart,
	double tEnd,
	uint32_t nDimension,
	const uint8_t* pData,
	size_t nDataLength);

SIDX_DLL uint32_t Index_IsValid(IndexH index);

SIDX_C_DLL RTError Index_TPIntersects_obj(  IndexH index,
                    double* pdMin,
                    double* pdMax,
                    double* pdVMin,
                    double* pdVMax,
                    double tStart,
                    double tEnd,
                    uint32_t nDimension,
                    IndexItemH** items,
                    uint64_t* nResults);

SIDX_C_DLL RTError Index_MVRIntersects_obj(  IndexH index,
										double* pdMin,
										double* pdMax,
										double tStart,
										double tEnd,
										uint32_t nDimension,
										IndexItemH** items,
										uint64_t* nResults);

SIDX_DLL RTError Index_Intersects_obj(	IndexH index,
										double* pdMin,
										double* pdMax,
										uint32_t nDimension,
										IndexItemH** items,
										uint64_t* nResults);

SIDX_C_DLL RTError Index_Contains_obj(  IndexH index,
                                        double* pdMin,
                                        double* pdMax,
                                        uint32_t nDimension,
                                        IndexItemH** items,
                                        uint64_t* nResults);

SIDX_C_DLL RTError Index_TPIntersects_id(  IndexH index,
                    double* pdMin,
                    double* pdMax,
                    double* pdVMin,
                    double* pdVMax,
                    double tStart,
                    double tEnd,
                    uint32_t nDimension,
                    int64_t** ids,
                    uint64_t* nResults);

SIDX_C_DLL RTError Index_MVRIntersects_id(  IndexH index,
										double* pdMin,
										double* pdMax,
										double tStart,
										double tEnd,
										uint32_t nDimension,
										int64_t** ids,
										uint64_t* nResults);

SIDX_DLL RTError Index_Intersects_id(	IndexH index,
										double* pdMin,
										double* pdMax,
										uint32_t nDimension,
										int64_t** items,
										uint64_t* nResults);

SIDX_C_DLL RTError Index_Contains_id(IndexH index,
                                     double *pdMin,
                                     double *pdMax,
                                     uint32_t nDimension,
                                     int64_t **ids,
                                     uint64_t *nResults);

SIDX_C_DLL RTError Index_TPIntersects_count(	  IndexH index,
                    double* pdMin,
                    double* pdMax,
                    double* pdVMin,
                    double* pdVMax,
                    double tStart,
                    double tEnd,
                    uint32_t nDimension,
                    uint64_t* nResults);

SIDX_C_DLL RTError Index_MVRIntersects_count(	  IndexH index,
										double* pdMin,
										double* pdMax,
										double tStart,
										double tEnd,
										uint32_t nDimension,
										uint64_t* nResults);

SIDX_DLL RTError Index_Intersects_count(	IndexH index,
										double* pdMin,
										double* pdMax,
										uint32_t nDimension,
										uint64_t* nResults);

SIDX_C_DLL RTError Index_Contains_count( IndexH index,
                                         double* pdMin,
                                         double* pdMax,
                                         uint32_t nDimension,
                                         uint64_t* nResults);

SIDX_C_DLL RTError Index_TPNearestNeighbors_obj(IndexH index,
                      double* pdMin,
                      double* pdMax,
                      double* pdVMin,
                      double* pdVMax,
                      double tStart,
                      double tEnd,
                      uint32_t nDimension,
                      IndexItemH** items,
                      uint64_t* nResults);

SIDX_C_DLL RTError Index_MVRNearestNeighbors_obj(IndexH index,
											double* pdMin,
											double* pdMax,
											double tStart,
											double tEnd,
											uint32_t nDimension,
											IndexItemH** items,
											uint64_t* nResults);

SIDX_DLL RTError Index_NearestNeighbors_obj(IndexH index,
											double* pdMin,
											double* pdMax,
											uint32_t nDimension,
											IndexItemH** items,
											uint64_t* nResults);

SIDX_C_DLL RTError Index_TPNearestNeighbors_id(IndexH index,
                      double* pdMin,
                      double* pdMax,
                      double* pdVMin,
                      double* pdVMax,
                      double tStart,
                      double tEnd,
                      uint32_t nDimension,
                      int64_t** ids,
                      uint64_t* nResults);

SIDX_C_DLL RTError Index_MVRNearestNeighbors_id(IndexH index,
											double* pdMin,
											double* pdMax,
											double tStart,
											double tEnd,
											uint32_t nDimension,
											int64_t** ids,
											uint64_t* nResults);

SIDX_DLL RTError Index_NearestNeighbors_id( IndexH index,
											double* pdMin,
											double* pdMax,
											uint32_t nDimension,
											int64_t** items,
											uint64_t* nResults);

SIDX_DLL RTError Index_Intersects_internal(	IndexH index,
											double* pdMin,
											double* pdMax,
											uint32_t nDimension,
											IndexItemH** items,
											uint64_t* nResults);

SIDX_DLL RTError Index_GetBounds(	IndexH index,
									double** ppdMin,
									double** ppdMax,
									uint32_t* nDimension);


SIDX_C_DLL RTError Index_GetLeaves( IndexH index,
									uint32_t* nLeafNodes,
									uint32_t** nLeafSizes,
									int64_t** nLeafIDs,
									int64_t*** nLeafChildIDs,
									double*** pppdMin,
									double*** pppdMax,
									uint32_t* nDimension);

SIDX_DLL RTError Index_SetResultSetOffset(IndexH index, int64_t value);
SIDX_DLL int64_t Index_GetResultSetOffset(IndexH index);

SIDX_DLL RTError Index_SetResultSetLimit(IndexH index, int64_t value);
SIDX_DLL int64_t Index_GetResultSetLimit(IndexH index);

SIDX_DLL void Index_DestroyObjResults(IndexItemH* results, uint32_t nResults);
SIDX_DLL void Index_ClearBuffer(IndexH index);
SIDX_DLL void Index_Free(void* object);
SIDX_DLL void Index_Flush(IndexH index);

SIDX_DLL void IndexItem_Destroy(IndexItemH item);
SIDX_DLL int64_t IndexItem_GetID(IndexItemH item);

SIDX_DLL RTError IndexItem_GetData(IndexItemH item, uint8_t** data, uint64_t* length);

SIDX_DLL RTError IndexItem_GetBounds(	IndexItemH item,
										double** ppdMin,
										double** ppdMax,
										uint32_t* nDimension);

SIDX_DLL IndexPropertyH IndexProperty_Create(void);
SIDX_DLL void IndexProperty_Destroy(IndexPropertyH hProp);

SIDX_DLL RTError IndexProperty_SetIndexType(IndexPropertyH iprop, RTIndexType value);
SIDX_DLL RTIndexType IndexProperty_GetIndexType(IndexPropertyH iprop);

SIDX_DLL RTError IndexProperty_SetDimension(IndexPropertyH iprop, uint32_t value);
SIDX_DLL uint32_t IndexProperty_GetDimension(IndexPropertyH iprop);

SIDX_DLL RTError IndexProperty_SetIndexVariant(IndexPropertyH iprop, RTIndexVariant value);
SIDX_DLL RTIndexVariant IndexProperty_GetIndexVariant(IndexPropertyH iprop);

SIDX_DLL RTError IndexProperty_SetIndexStorage(IndexPropertyH iprop, RTStorageType value);
SIDX_DLL RTStorageType IndexProperty_GetIndexStorage(IndexPropertyH iprop);

SIDX_DLL RTError IndexProperty_SetPagesize(IndexPropertyH iprop, uint32_t value);
SIDX_DLL uint32_t IndexProperty_GetPagesize(IndexPropertyH iprop);

SIDX_DLL RTError IndexProperty_SetIndexCapacity(IndexPropertyH iprop, uint32_t value);
SIDX_DLL uint32_t IndexProperty_GetIndexCapacity(IndexPropertyH iprop);

SIDX_DLL RTError IndexProperty_SetLeafCapacity(IndexPropertyH iprop, uint32_t value);
SIDX_DLL uint32_t IndexProperty_GetLeafCapacity(IndexPropertyH iprop);

SIDX_DLL RTError IndexProperty_SetLeafPoolCapacity(IndexPropertyH iprop, uint32_t value);
SIDX_DLL uint32_t IndexProperty_GetLeafPoolCapacity(IndexPropertyH iprop);

SIDX_DLL RTError IndexProperty_SetIndexPoolCapacity(IndexPropertyH iprop, uint32_t value);
SIDX_DLL uint32_t IndexProperty_GetIndexPoolCapacity(IndexPropertyH iprop);

SIDX_DLL RTError IndexProperty_SetRegionPoolCapacity(IndexPropertyH iprop, uint32_t value);
SIDX_DLL uint32_t IndexProperty_GetRegionPoolCapacity(IndexPropertyH iprop);

SIDX_DLL RTError IndexProperty_SetPointPoolCapacity(IndexPropertyH iprop, uint32_t value);
SIDX_DLL uint32_t IndexProperty_GetPointPoolCapacity(IndexPropertyH iprop);

SIDX_DLL RTError IndexProperty_SetBufferingCapacity(IndexPropertyH iprop, uint32_t value);
SIDX_DLL uint32_t IndexProperty_GetBufferingCapacity(IndexPropertyH iprop);

SIDX_DLL RTError IndexProperty_SetEnsureTightMBRs(IndexPropertyH iprop, uint32_t value);
SIDX_DLL uint32_t IndexProperty_GetEnsureTightMBRs(IndexPropertyH iprop);

SIDX_DLL RTError IndexProperty_SetOverwrite(IndexPropertyH iprop, uint32_t value);
SIDX_DLL uint32_t IndexProperty_GetOverwrite(IndexPropertyH iprop);

SIDX_DLL RTError IndexProperty_SetNearMinimumOverlapFactor(IndexPropertyH iprop, uint32_t value);
SIDX_DLL uint32_t IndexProperty_GetNearMinimumOverlapFactor(IndexPropertyH iprop);

SIDX_DLL RTError IndexProperty_SetWriteThrough(IndexPropertyH iprop, uint32_t value);
SIDX_DLL uint32_t IndexProperty_GetWriteThrough(IndexPropertyH iprop);

SIDX_DLL RTError IndexProperty_SetFillFactor(IndexPropertyH iprop, double value);
SIDX_DLL double IndexProperty_GetFillFactor(IndexPropertyH iprop);

SIDX_DLL RTError IndexProperty_SetSplitDistributionFactor(IndexPropertyH iprop, double value);
SIDX_DLL double IndexProperty_GetSplitDistributionFactor(IndexPropertyH iprop);

SIDX_DLL RTError IndexProperty_SetTPRHorizon(IndexPropertyH iprop, double value);
SIDX_DLL double IndexProperty_GetTPRHorizon(IndexPropertyH iprop);

SIDX_DLL RTError IndexProperty_SetReinsertFactor(IndexPropertyH iprop, double value);
SIDX_DLL double IndexProperty_GetReinsertFactor(IndexPropertyH iprop);

SIDX_DLL RTError IndexProperty_SetFileName(IndexPropertyH iprop, const char* value);
SIDX_DLL char* IndexProperty_GetFileName(IndexPropertyH iprop);

SIDX_DLL RTError IndexProperty_SetFileNameExtensionDat(IndexPropertyH iprop, const char* value);
SIDX_DLL char* IndexProperty_GetFileNameExtensionDat(IndexPropertyH iprop);

SIDX_DLL RTError IndexProperty_SetFileNameExtensionIdx(IndexPropertyH iprop, const char* value);
SIDX_DLL char* IndexProperty_GetFileNameExtensionIdx(IndexPropertyH iprop);

SIDX_DLL RTError IndexProperty_SetCustomStorageCallbacksSize(IndexPropertyH iprop, uint32_t value);
SIDX_DLL uint32_t IndexProperty_GetCustomStorageCallbacksSize(IndexPropertyH iprop);

SIDX_DLL RTError IndexProperty_SetCustomStorageCallbacks(IndexPropertyH iprop, const void* value);
SIDX_DLL void* IndexProperty_GetCustomStorageCallbacks(IndexPropertyH iprop);

SIDX_DLL RTError IndexProperty_SetIndexID(IndexPropertyH iprop, int64_t value);
SIDX_DLL int64_t IndexProperty_GetIndexID(IndexPropertyH iprop);

SIDX_C_DLL void* SIDX_NewBuffer(size_t bytes);
SIDX_C_DLL void  SIDX_DeleteBuffer(void* buffer);

SIDX_DLL RTError IndexProperty_SetResultSetLimit(IndexPropertyH iprop, uint64_t value);
SIDX_DLL uint64_t IndexProperty_GetResultSetLimit(IndexPropertyH iprop);

SIDX_C_DLL char* SIDX_Version(void);

SIDX_C_DLL char* Error_GetLastErrorMsg(void);

IDX_C_END

#endif
