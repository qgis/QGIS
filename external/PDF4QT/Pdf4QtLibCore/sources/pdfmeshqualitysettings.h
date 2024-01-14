//    Copyright (C) 2019-2021 Jakub Melka
//
//    This file is part of PDF4QT.
//
//    PDF4QT is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    with the written consent of the copyright owner, any later version.
//
//    PDF4QT is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with PDF4QT.  If not, see <https://www.gnu.org/licenses/>.

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
