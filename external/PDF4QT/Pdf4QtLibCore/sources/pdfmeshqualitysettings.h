// MIT License
//
// Copyright (c) 2018-2025 Jakub Melka and Contributors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef PDFMESHQUALITYSETTINGS_H
#define PDFMESHQUALITYSETTINGS_H

#include "pdfglobal.h"

#include <QTransform>

namespace pdf
{

struct PDFMeshQualitySettings
{
    /// Initializes default resolution
    void initResolution();

    /// Value used to compute minimal pxiel size resolution (it is multiplied by whole shading area size)
    PDFReal minimalMeshResolutionRatio = 0.005;

    /// Value used to compute pixel size resolution (it is multiplied by whole shading area size)
    PDFReal preferredMeshResolutionRatio = 0.02;

    /// Matrix, which transforms user space points (user space is target space of the shading)
    /// to the device space of the paint device.
    QTransform userSpaceToDeviceSpaceMatrix;

    /// Rectangle in device space coordinate system, onto which is area meshed.
    QRectF deviceSpaceMeshingArea;

    /// Preferred mesh resolution in device space pixels. Mesh will be created in this
    /// resolution, if it is smooth enough (no jumps in colors occurs).
    PDFReal preferredMeshResolution = 1.0;

    /// Minimal mesh resolution in device space pixels. If jumps in colors occurs (jump
    /// is two colors, that differ more than \p color tolerance), then mesh is meshed to
    /// minimal mesh resolution.
    PDFReal minimalMeshResolution = 1.0;

    /// Color tolerance - 1% by default
    PDFReal tolerance = 0.01;

    /// Test points to determine maximal curvature of the tensor product patch meshes
    PDFInteger patchTestPoints = 64;

    /// Lower value of the surface curvature meshing resolution mapping. When ratio between
    /// current curvature at the center of meshed triangle and maximal curvature is below
    /// this value, then prefered mesh resolution is used. If ratio is higher than this value
    /// and lower than \p patchResolutionMappingRatioHigh, then target length is linearly mapped.
    /// If value is higher, than \p patchResolutionMappingRatioHigh, then minimal mesh resolution
    /// is used when generating triangles on the patch.
    PDFReal patchResolutionMappingRatioLow = 0.3;

    /// Highter value of the surface curvature meshing resolution mapping. \sa patchResolutionMappingRatioLow
    PDFReal patchResolutionMappingRatioHigh = 0.9;
};

}   // namespace pdf

#endif // PDFMESHQUALITYSETTINGS_H
