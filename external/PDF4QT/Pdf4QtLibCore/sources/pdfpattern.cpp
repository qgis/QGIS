//    Copyright (C) 2019-2022 Jakub Melka
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

#include "pdfpattern.h"
#include "pdfdocument.h"
#include "pdfexception.h"
#include "pdfutils.h"
#include "pdfcolorspaces.h"
#include "pdfexecutionpolicy.h"
#include "pdfconstants.h"

#include <QMutex>
#include <QPainter>

#include "pdfdbgheap.h"

#include <execution>

namespace pdf
{

PatternType PDFShadingPattern::getType() const
{
    return PatternType::Shading;
}

const PDFAbstractColorSpace* PDFShadingPattern::getColorSpace() const
{
    return m_colorSpace.data();
}

QTransform PDFShadingPattern::getPatternSpaceToDeviceSpaceMatrix(const PDFMeshQualitySettings& settings) const
{
    return m_matrix * settings.userSpaceToDeviceSpaceMatrix;
}

QTransform PDFShadingPattern::getPatternSpaceToDeviceSpaceMatrix(const QTransform& userSpaceToDeviceSpaceMatrix) const
{
    return m_matrix * userSpaceToDeviceSpaceMatrix;
}

PDFShadingSampler* PDFShadingPattern::createSampler(QTransform userSpaceToDeviceSpaceMatrix) const
{
    Q_UNUSED(userSpaceToDeviceSpaceMatrix);

    return nullptr;
}

ShadingType PDFAxialShading::getShadingType() const
{
    return ShadingType::Axial;
}

PDFPatternPtr PDFPattern::createPattern(const PDFDictionary* colorSpaceDictionary,
                                        const PDFDocument* document,
                                        const PDFObject& object,
                                        const PDFCMS* cms,
                                        RenderingIntent intent,
                                        PDFRenderErrorReporter* reporter)
{
    const PDFObject& dereferencedObject = document->getObject(object);
    const PDFDictionary* patternDictionary = nullptr;
    QByteArray streamData;

    if (dereferencedObject.isDictionary())
    {
        patternDictionary = dereferencedObject.getDictionary();
    }
    else if (dereferencedObject.isStream())
    {
        const PDFStream* stream = dereferencedObject.getStream();
        patternDictionary = stream->getDictionary();
        streamData = document->getDecodedStream(stream);
    }

    if (patternDictionary)
    {
        PDFDocumentDataLoaderDecorator loader(document);

        const PatternType patternType = static_cast<PatternType>(loader.readIntegerFromDictionary(patternDictionary, "PatternType", static_cast<PDFInteger>(PatternType::Invalid)));
        switch (patternType)
        {
            case PatternType::Tiling:
            {
                const PDFTilingPattern::PaintType paintType = static_cast<PDFTilingPattern::PaintType>(loader.readIntegerFromDictionary(patternDictionary, "PaintType", static_cast<PDFInteger>(PDFTilingPattern::PaintType::Invalid)));
                const PDFTilingPattern::TilingType tilingType = static_cast<PDFTilingPattern::TilingType>(loader.readIntegerFromDictionary(patternDictionary, "TilingType", static_cast<PDFInteger>(PDFTilingPattern::TilingType::Invalid)));
                const QRectF boundingBox = loader.readRectangle(patternDictionary->get("BBox"), QRectF());
                const PDFReal xStep = loader.readNumberFromDictionary(patternDictionary, "XStep", 0.0);
                const PDFReal yStep = loader.readNumberFromDictionary(patternDictionary, "YStep", 0.0);
                PDFObject resources = document->getObject(patternDictionary->get("Resources"));
                QTransform matrix = loader.readMatrixFromDictionary(patternDictionary, "Matrix", QTransform());

                // Verify the data
                if (paintType != PDFTilingPattern::PaintType::Colored && paintType != PDFTilingPattern::PaintType::Uncolored)
                {
                    throw PDFException(PDFTranslationContext::tr("Invalid tiling pattern - wrong paint type %1.").arg(static_cast<PDFInteger>(paintType)));
                }
                if (tilingType != PDFTilingPattern::TilingType::ConstantSpacing && tilingType != PDFTilingPattern::TilingType::NoDistortion && tilingType != PDFTilingPattern::TilingType::ConstantSpacingAndFasterTiling)
                {
                    throw PDFException(PDFTranslationContext::tr("Invalid tiling pattern - wrong tiling type %1.").arg(static_cast<PDFInteger>(tilingType)));
                }
                if (!boundingBox.isValid())
                {
                    throw PDFException(PDFTranslationContext::tr("Invalid tiling pattern - bounding box is invalid.").arg(static_cast<PDFInteger>(paintType)));
                }
                if (isZero(xStep) || isZero(yStep))
                {
                    throw PDFException(PDFTranslationContext::tr("Invalid tiling pattern - steps are invalid.").arg(static_cast<PDFInteger>(paintType)));
                }

                PDFTilingPattern* pattern = new PDFTilingPattern();
                pattern->m_boundingBox = boundingBox;
                pattern->m_matrix = matrix;
                pattern->m_paintType = paintType;
                pattern->m_tilingType = tilingType;
                pattern->m_xStep = xStep;
                pattern->m_yStep = yStep;
                pattern->m_resources = resources;
                pattern->m_content = qMove(streamData);

                return PDFPatternPtr(pattern);
            }

            case PatternType::Shading:
            {
                PDFObject patternGraphicState = document->getObject(patternDictionary->get("ExtGState"));
                QTransform matrix = loader.readMatrixFromDictionary(patternDictionary, "Matrix", QTransform());
                return createShadingPattern(colorSpaceDictionary, document, patternDictionary->get("Shading"), matrix, patternGraphicState, cms, intent, reporter, false);
            }

            default:
                throw PDFException(PDFTranslationContext::tr("Invalid pattern."));
        }
    }

    throw PDFException(PDFTranslationContext::tr("Invalid pattern."));
}

PDFPatternPtr PDFPattern::createShadingPattern(const PDFDictionary* colorSpaceDictionary,
                                               const PDFDocument* document,
                                               const PDFObject& shadingObject,
                                               const QTransform& matrix,
                                               const PDFObject& patternGraphicState,
                                               const PDFCMS* cms,
                                               RenderingIntent intent,
                                               PDFRenderErrorReporter* reporter,
                                               bool ignoreBackgroundColor)
{
    const PDFObject& dereferencedShadingObject = document->getObject(shadingObject);
    if (!dereferencedShadingObject.isDictionary() && !dereferencedShadingObject.isStream())
    {
        throw PDFException(PDFTranslationContext::tr("Invalid shading."));
    }

    PDFDocumentDataLoaderDecorator loader(document);
    const PDFDictionary* shadingDictionary = nullptr;
    const PDFStream* stream = nullptr;

    if (dereferencedShadingObject.isDictionary())
    {
        shadingDictionary = dereferencedShadingObject.getDictionary();
    }
    else if (dereferencedShadingObject.isStream())
    {
        stream = dereferencedShadingObject.getStream();
        shadingDictionary = stream->getDictionary();
    }

    // Parse common data for all shadings
    PDFColorSpacePointer colorSpace = PDFAbstractColorSpace::createColorSpace(colorSpaceDictionary, document, document->getObject(shadingDictionary->get("ColorSpace")));

    if (colorSpace->asPatternColorSpace())
    {
        throw PDFException(PDFTranslationContext::tr("Pattern color space is not valid for shading patterns."));
    }

    QColor backgroundColor;
    PDFColor originalBackgroundColor;
    if (!ignoreBackgroundColor)
    {
        std::vector<PDFReal> backgroundColorValues = loader.readNumberArrayFromDictionary(shadingDictionary, "Background");
        if (!backgroundColorValues.empty())
        {
            backgroundColor = colorSpace->getCheckedColor(PDFAbstractColorSpace::convertToColor(backgroundColorValues), cms, intent, reporter);
        }

        originalBackgroundColor.resize(backgroundColorValues.size());
        for (size_t i = 0; i < backgroundColorValues.size(); ++i)
        {
            originalBackgroundColor[i] = backgroundColorValues[i];
        }
    }
    QRectF boundingBox = loader.readRectangle(shadingDictionary->get("BBox"), QRectF());
    bool antialias = loader.readBooleanFromDictionary(shadingDictionary, "AntiAlias", false);
    const PDFObject& extendObject = document->getObject(shadingDictionary->get("Extend"));
    bool extendStart = false;
    bool extendEnd = false;
    if (extendObject.isArray())
    {
        const PDFArray* array = extendObject.getArray();
        if (array->getCount() != 2)
        {
            throw PDFException(PDFTranslationContext::tr("Invalid shading pattern extends. Expected 2, but %1 provided.").arg(array->getCount()));
        }

        extendStart = loader.readBoolean(array->getItem(0), false);
        extendEnd = loader.readBoolean(array->getItem(1), false);
    }
    std::vector<PDFFunctionPtr> functions;
    const PDFObject& functionsObject = document->getObject(shadingDictionary->get("Function"));
    if (functionsObject.isArray())
    {
        const PDFArray* functionsArray = functionsObject.getArray();
        functions.reserve(functionsArray->getCount());
        for (size_t i = 0, functionCount = functionsArray->getCount(); i < functionCount; ++i)
        {
            functions.push_back(PDFFunction::createFunction(document, functionsArray->getItem(i)));
        }
    }
    else if (!functionsObject.isNull())
    {
        functions.push_back(PDFFunction::createFunction(document, functionsObject));
    }

    const ShadingType shadingType = static_cast<ShadingType>(loader.readIntegerFromDictionary(shadingDictionary, "ShadingType", static_cast<PDFInteger>(ShadingType::Invalid)));
    switch (shadingType)
    {
        case ShadingType::Function:
        {
            PDFFunctionShading* functionShading = new PDFFunctionShading();
            PDFPatternPtr result(functionShading);

            std::vector<PDFReal> functionDomain = loader.readNumberArrayFromDictionary(shadingDictionary, "Domain", { 0.0, 1.0, 0.0, 1.0 });
            if (functionDomain.size() != 4)
            {
                throw PDFException(PDFTranslationContext::tr("Invalid function shading pattern domain. Expected 4 values, but %1 provided.").arg(functionDomain.size()));
            }
            if (functionDomain[1] < functionDomain[0] || functionDomain[3] < functionDomain[2])
            {
                throw PDFException(PDFTranslationContext::tr("Invalid function shading pattern domain. Invalid domain ranges."));
            }

            QTransform domainToTargetTransform = loader.readMatrixFromDictionary(shadingDictionary, "Matrix", QTransform());

            size_t colorComponentCount = colorSpace->getColorComponentCount();
            if (functions.size() > 1 && colorComponentCount != functions.size())
            {
                throw PDFException(PDFTranslationContext::tr("Invalid axial shading pattern color functions. Expected %1 functions, but %2 provided.").arg(int(colorComponentCount)).arg(int(functions.size())));
            }

            // Load items for function shading
            functionShading->m_antiAlias = antialias;
            functionShading->m_backgroundColor = backgroundColor;
            functionShading->m_originalBackgroundColor = qMove(originalBackgroundColor);
            functionShading->m_colorSpace = colorSpace;
            functionShading->m_boundingBox = boundingBox;
            functionShading->m_domain = QRectF(functionDomain[0], functionDomain[2], functionDomain[1] - functionDomain[0], functionDomain[3] - functionDomain[2]);
            functionShading->m_domainToTargetTransform = domainToTargetTransform;
            functionShading->m_functions = qMove(functions);
            functionShading->m_matrix = matrix;
            functionShading->m_patternGraphicState = patternGraphicState;

            return result;
        }

        case ShadingType::Axial:
        {
            PDFAxialShading* axialShading = new PDFAxialShading();
            PDFPatternPtr result(axialShading);

            std::vector<PDFReal> coordinates = loader.readNumberArrayFromDictionary(shadingDictionary, "Coords");
            if (coordinates.size() != 4)
            {
                throw PDFException(PDFTranslationContext::tr("Invalid axial shading pattern coordinates. Expected 4, but %1 provided.").arg(coordinates.size()));
            }

            std::vector<PDFReal> domain = loader.readNumberArrayFromDictionary(shadingDictionary, "Domain");
            if (domain.empty())
            {
                domain = { 0.0, 1.0 };
            }
            if (domain.size() != 2)
            {
                throw PDFException(PDFTranslationContext::tr("Invalid axial shading pattern domain. Expected 2, but %1 provided.").arg(domain.size()));
            }

            size_t colorComponentCount = colorSpace->getColorComponentCount();
            if (functions.size() > 1 && colorComponentCount != functions.size())
            {
                throw PDFException(PDFTranslationContext::tr("Invalid axial shading pattern color functions. Expected %1 functions, but %2 provided.").arg(int(colorComponentCount)).arg(int(functions.size())));
            }

            // Load items for axial shading
            axialShading->m_antiAlias = antialias;
            axialShading->m_backgroundColor = backgroundColor;
            axialShading->m_originalBackgroundColor = qMove(originalBackgroundColor);
            axialShading->m_colorSpace = colorSpace;
            axialShading->m_boundingBox = boundingBox;
            axialShading->m_domainStart = domain[0];
            axialShading->m_domainEnd = domain[1];
            axialShading->m_startPoint = QPointF(coordinates[0], coordinates[1]);
            axialShading->m_endPoint = QPointF(coordinates[2], coordinates[3]);
            axialShading->m_extendStart = extendStart;
            axialShading->m_extendEnd = extendEnd;
            axialShading->m_functions = qMove(functions);
            axialShading->m_matrix = matrix;
            axialShading->m_patternGraphicState = patternGraphicState;

            return result;
        }

        case ShadingType::Radial:
        {
            PDFRadialShading* radialShading = new PDFRadialShading();
            PDFPatternPtr result(radialShading);

            std::vector<PDFReal> coordinates = loader.readNumberArrayFromDictionary(shadingDictionary, "Coords");
            if (coordinates.size() != 6)
            {
                throw PDFException(PDFTranslationContext::tr("Invalid radial shading pattern coordinates. Expected 6, but %1 provided.").arg(coordinates.size()));
            }

            std::vector<PDFReal> domain = loader.readNumberArrayFromDictionary(shadingDictionary, "Domain");
            if (domain.empty())
            {
                domain = { 0.0, 1.0 };
            }
            if (domain.size() != 2)
            {
                throw PDFException(PDFTranslationContext::tr("Invalid radial shading pattern domain. Expected 2, but %1 provided.").arg(domain.size()));
            }

            size_t colorComponentCount = colorSpace->getColorComponentCount();
            if (functions.size() > 1 && colorComponentCount != functions.size())
            {
                throw PDFException(PDFTranslationContext::tr("Invalid radial shading pattern color functions. Expected %1 functions, but %2 provided.").arg(int(colorComponentCount)).arg(int(functions.size())));
            }

            if (coordinates[2] < 0.0 || coordinates[5] < 0.0)
            {
                throw PDFException(PDFTranslationContext::tr("Radial shading cannot have negative circle radius."));
            }

            // Load items for axial shading
            radialShading->m_antiAlias = antialias;
            radialShading->m_backgroundColor = backgroundColor;
            radialShading->m_originalBackgroundColor = qMove(originalBackgroundColor);
            radialShading->m_colorSpace = colorSpace;
            radialShading->m_boundingBox = boundingBox;
            radialShading->m_domainStart = domain[0];
            radialShading->m_domainEnd = domain[1];
            radialShading->m_startPoint = QPointF(coordinates[0], coordinates[1]);
            radialShading->m_r0 = coordinates[2];
            radialShading->m_endPoint = QPointF(coordinates[3], coordinates[4]);
            radialShading->m_r1 = coordinates[5];
            radialShading->m_extendStart = extendStart;
            radialShading->m_extendEnd = extendEnd;
            radialShading->m_functions = qMove(functions);
            radialShading->m_matrix = matrix;
            radialShading->m_patternGraphicState = patternGraphicState;

            return result;
        }

        case ShadingType::FreeFormGouradTriangle:
        case ShadingType::LatticeFormGouradTriangle:
        case ShadingType::CoonsPatchMesh:
        case ShadingType::TensorProductPatchMesh:
        {
            PDFLatticeFormGouradTriangleShading* latticeFormGouradTriangleShading = nullptr;
            PDFType4567Shading* type4567Shading = nullptr;

            switch (shadingType)
            {
                case ShadingType::FreeFormGouradTriangle:
                {
                    type4567Shading = new PDFFreeFormGouradTriangleShading();
                    break;
                }

                case ShadingType::LatticeFormGouradTriangle:
                {
                    latticeFormGouradTriangleShading = new PDFLatticeFormGouradTriangleShading();
                    type4567Shading = latticeFormGouradTriangleShading;
                    break;
                }

                case ShadingType::CoonsPatchMesh:
                {
                    type4567Shading = new PDFCoonsPatchShading;
                    break;
                }

                case ShadingType::TensorProductPatchMesh:
                {
                    type4567Shading = new PDFTensorProductPatchShading;
                    break;
                }

                default:
                {
                    Q_ASSERT(false);
                    break;
                }
            }

            PDFPatternPtr result(type4567Shading);

            PDFInteger bitsPerCoordinate = loader.readIntegerFromDictionary(shadingDictionary, "BitsPerCoordinate", -1);
            if (!contains(bitsPerCoordinate, std::initializer_list<PDFInteger>{ 1, 2, 4, 8, 12, 16, 24, 32 }))
            {
                throw PDFException(PDFTranslationContext::tr("Invalid bits per coordinate (%1) for shading.").arg(bitsPerCoordinate));
            }
            PDFInteger bitsPerComponent = loader.readIntegerFromDictionary(shadingDictionary, "BitsPerComponent", -1);
            if (!contains(bitsPerComponent, std::initializer_list<PDFInteger>{ 1, 2, 4, 8, 12, 16 }))
            {
                throw PDFException(PDFTranslationContext::tr("Invalid bits per component (%1) for shading.").arg(bitsPerComponent));
            }

            std::vector<PDFReal> decode = loader.readNumberArrayFromDictionary(shadingDictionary, "Decode");
            if (!functions.empty())
            {
                if (decode.size() != 6)
                {
                    throw PDFException(PDFTranslationContext::tr("Invalid domain for shading. Expected size is 6, actual size is %1.").arg(decode.size()));
                }
            }
            else
            {
                const size_t expectedSize = colorSpace->getColorComponentCount() * 2 + 4;
                if (decode.size() != expectedSize)
                {
                    throw PDFException(PDFTranslationContext::tr("Invalid domain for shading. Expected size is %1, actual size is %2.").arg(expectedSize).arg(decode.size()));
                }
            }

            type4567Shading->m_antiAlias = antialias;
            type4567Shading->m_backgroundColor = backgroundColor;
            type4567Shading->m_originalBackgroundColor = qMove(originalBackgroundColor);
            type4567Shading->m_colorSpace = colorSpace;
            type4567Shading->m_matrix = matrix;
            type4567Shading->m_patternGraphicState = patternGraphicState;
            type4567Shading->m_bitsPerCoordinate = static_cast<uint8_t>(bitsPerCoordinate);
            type4567Shading->m_bitsPerComponent = static_cast<uint8_t>(bitsPerComponent);
            type4567Shading->m_xmin = decode[0];
            type4567Shading->m_xmax = decode[1];
            type4567Shading->m_ymin = decode[2];
            type4567Shading->m_ymax = decode[3];
            type4567Shading->m_limits = std::vector<PDFReal>(std::next(decode.cbegin(), 4), decode.cend());
            type4567Shading->m_colorComponentCount = !functions.empty() ? 1 : colorSpace->getColorComponentCount();
            type4567Shading->m_functions = qMove(functions);
            type4567Shading->m_data = document->getDecodedStream(stream);

            switch (shadingType)
            {
                case ShadingType::FreeFormGouradTriangle:
                case ShadingType::CoonsPatchMesh:
                case ShadingType::TensorProductPatchMesh:
                {
                    PDFInteger bitsPerFlag = loader.readIntegerFromDictionary(shadingDictionary, "BitsPerFlag", -1);
                    if (!contains(bitsPerFlag, std::initializer_list<PDFInteger>{ 2, 4, 8 }))
                    {
                        throw PDFException(PDFTranslationContext::tr("Invalid bits per flag (%1) for shading.").arg(bitsPerFlag));
                    }
                    type4567Shading->m_bitsPerFlag = bitsPerFlag;
                    break;
                }

                case ShadingType::LatticeFormGouradTriangle:
                {
                    latticeFormGouradTriangleShading->m_verticesPerRow = loader.readIntegerFromDictionary(shadingDictionary, "VerticesPerRow", -1);
                    if (latticeFormGouradTriangleShading->m_verticesPerRow < 2)
                    {
                        throw PDFException(PDFTranslationContext::tr("Invalid vertices per row (%1) for lattice-form gourad triangle meshing.").arg(latticeFormGouradTriangleShading->m_verticesPerRow));
                    }
                    break;
                }

                default:
                    break;
            }

            return result;
        }

        default:
        {
            throw PDFException(PDFTranslationContext::tr("Invalid shading pattern type (%1).").arg(static_cast<PDFInteger>(shadingType)));
        }
    }
}

class PDFFunctionShadingSampler : public PDFShadingSampler
{
public:
    PDFFunctionShadingSampler(const PDFFunctionShading* functionShadingPattern, QTransform userSpaceToDeviceSpaceMatrix) :
        PDFShadingSampler(functionShadingPattern),
        m_functionShadingPattern(functionShadingPattern),
        m_domain(functionShadingPattern->getDomain())
    {
        QTransform patternSpaceToDeviceSpaceMatrix = functionShadingPattern->getMatrix() * userSpaceToDeviceSpaceMatrix;
        QTransform domainToDeviceSpaceMatrix = functionShadingPattern->getDomainToTargetTransform() * patternSpaceToDeviceSpaceMatrix;

        if (domainToDeviceSpaceMatrix.isInvertible())
        {
            m_deviceSpaceToDomainMatrix = domainToDeviceSpaceMatrix.inverted();
        }
        else
        {
            m_deviceSpaceToDomainMatrix = QTransform();
        }
    }

    virtual bool sample(const QPointF& devicePoint, PDFColorBuffer outputBuffer, int limit) const override
    {
        Q_UNUSED(limit);

        if (!m_pattern->getColorSpace() || m_pattern->getColorSpace()->getColorComponentCount() != outputBuffer.size())
        {
            // Invalid color space, or invalid color buffer
            return false;
        }

        QPointF domainPoint = m_deviceSpaceToDomainMatrix.map(devicePoint);

        if (!m_domain.contains(domainPoint))
        {
            return fillBackgroundColor(outputBuffer);
        }

        const auto& functions = m_functionShadingPattern->getFunctions();
        std::array<PDFReal, PDF_MAX_COLOR_COMPONENTS> colorBuffer = { };

        if (colorBuffer.size() < outputBuffer.size())
        {
            // Jakub Melka: Too much colors - we cant process it
            return false;
        }

        std::array<PDFReal, 2> input = { domainPoint.x(), domainPoint.y() };

        if (functions.size() == 1)
        {
            Q_ASSERT(outputBuffer.size() <= colorBuffer.size());
            PDFFunction::FunctionResult result = functions.front()->apply(input.data(), input.data() + input.size(), colorBuffer.data(), colorBuffer.data() + outputBuffer.size());

            if (!result)
            {
                // Function call failed
                return false;
            }
        }
        else
        {
            if (functions.size() != outputBuffer.size())
            {
                // Invalid number of functions
                return false;
            }

            Q_ASSERT(outputBuffer.size() <= colorBuffer.size());
            for (size_t i = 0, count = outputBuffer.size(); i < count; ++i)
            {
                PDFFunction::FunctionResult result = functions[i]->apply(input.data(), input.data() + input.size(), colorBuffer.data() + i, colorBuffer.data() + i + 1);

                if (!result)
                {
                    // Function call failed
                    return false;
                }
            }
        }

        for (size_t i = 0, count = outputBuffer.size(); i < count; ++i)
        {
            outputBuffer[i] = colorBuffer[i];
        }

        return true;
    }

private:
    const PDFFunctionShading* m_functionShadingPattern;
    QRectF m_domain;
    QTransform m_deviceSpaceToDomainMatrix;
};

ShadingType PDFFunctionShading::getShadingType() const
{
    return ShadingType::Function;
}

PDFMesh PDFFunctionShading::createMesh(const PDFMeshQualitySettings& settings,
                                       const PDFCMS* cms,
                                       RenderingIntent intent,
                                       PDFRenderErrorReporter* reporter,
                                       const PDFOperationControl* operationControl) const
{
    PDFMesh mesh;

    Q_UNUSED(operationControl);

    QTransform patternSpaceToDeviceSpaceMatrix = getPatternSpaceToDeviceSpaceMatrix(settings);
    QTransform domainToDeviceSpaceMatrix = m_domainToTargetTransform * patternSpaceToDeviceSpaceMatrix;
    QLineF topLine(m_domain.topLeft(), m_domain.topRight());
    QLineF leftLine(m_domain.topLeft(), m_domain.bottomLeft());

    Q_ASSERT(domainToDeviceSpaceMatrix.isInvertible());
    QTransform deviceSpaceToDomainMatrix = domainToDeviceSpaceMatrix.inverted();

    QLineF topLineDS = domainToDeviceSpaceMatrix.map(topLine);
    QLineF leftLineDS = domainToDeviceSpaceMatrix.map(leftLine);

    const size_t colorComponents = m_colorSpace->getColorComponentCount();
    auto resolutions = { settings.preferredMeshResolution,
                         interpolate(0.25, 0.0, 1.0, settings.preferredMeshResolution, settings.minimalMeshResolution),
                         interpolate(0.50, 0.0, 1.0, settings.preferredMeshResolution, settings.minimalMeshResolution),
                         interpolate(0.75, 0.0, 1.0, settings.preferredMeshResolution, settings.minimalMeshResolution),
                         settings.minimalMeshResolution
                       };

    for (PDFReal resolution : resolutions)
    {
        const PDFReal xSteps = qMax(std::floor(topLineDS.length() / resolution), 2.0);
        const PDFReal ySteps = qMax(std::floor(leftLineDS.length() / resolution), 2.0);
        const PDFReal xStep = 1.0 / xSteps;
        const PDFReal yStep = 1.0 / ySteps;

        // Prepare x/y ordinates array for given resolution
        std::vector<PDFReal> xOrdinates;
        std::vector<PDFReal> yOrdinates;
        xOrdinates.reserve(xSteps + 1);
        yOrdinates.reserve(ySteps + 1);

        for (PDFReal x = 0.0; x <= 1.0; x += xStep)
        {
            xOrdinates.push_back(x);
        }
        if (xOrdinates.back() + PDF_EPSILON >= 1.0)
        {
            xOrdinates.pop_back();
        }
        xOrdinates.push_back(1.0);

        for (PDFReal y = 0.0; y <= 1.0; y += yStep)
        {
            yOrdinates.push_back(y);
        }
        if (yOrdinates.back() + PDF_EPSILON >= 1.0)
        {
            yOrdinates.pop_back();
        }
        yOrdinates.push_back(1.0);

        // We have determined x/y ordinates. Now we must create result array with colors,
        // which for each x/y ordinate tells us, what color in the given position is.
        const size_t rowCount = yOrdinates.size();
        const size_t columnCount = xOrdinates.size();
        const size_t nodesCount = rowCount * columnCount;
        const size_t stride = columnCount * colorComponents;

        std::vector<size_t> indices;
        indices.resize(nodesCount, 0);
        std::iota(indices.begin(), indices.end(), 0);

        auto indexToRowColumn = [columnCount](size_t index) -> std::pair<size_t, size_t>
        {
            return std::make_pair(index / columnCount, index % columnCount);
        };

        auto rowColumnToIndex = [columnCount](size_t row, size_t column) -> size_t
        {
            return row * columnCount + column;
        };

        auto rowColumnToFirstColorComponent = [stride, colorComponents](size_t row, size_t column) -> size_t
        {
            return row * stride + column * colorComponents;
        };

        const bool isSingleFunction = m_functions.size() == 1;
        std::vector<PDFReal> sourceColorBuffer;
        sourceColorBuffer.resize(indices.size() * colorComponents, 0.0);

        std::vector<QPointF> gridPoints;
        gridPoints.resize(nodesCount);

        QMutex functionErrorMutex;
        PDFFunction::FunctionResult functionError(true);

        auto setColor = [&](size_t index)
        {
            auto [row, column] = indexToRowColumn(index);
            QPointF nodeDS = topLineDS.pointAt(xOrdinates[column]) + leftLineDS.pointAt(yOrdinates[row]) - topLineDS.p1();
            QPointF node = deviceSpaceToDomainMatrix.map(nodeDS);
            const size_t colorComponentIndex = rowColumnToFirstColorComponent(row, column);
            Q_ASSERT(colorComponentIndex <= sourceColorBuffer.size());

            gridPoints[index] = nodeDS;

            PDFReal* sourceColorBegin = sourceColorBuffer.data() + colorComponentIndex;
            PDFReal* sourceColorEnd = sourceColorBegin + colorComponents;

            std::array<PDFReal, 2> uv = { node.x(), node.y() };

            if (isSingleFunction)
            {
                PDFFunction::FunctionResult result = m_functions.front()->apply(uv.data(), uv.data() + uv.size(), sourceColorBegin, sourceColorEnd);
                if (!result)
                {
                    QMutexLocker lock(&functionErrorMutex);
                    if (!functionError)
                    {
                        functionError = result;
                    }
                }
            }
            else
            {
                for (size_t i = 0, count = colorComponents; i < count; ++i)
                {
                    PDFFunction::FunctionResult result = m_functions[i]->apply(uv.data(), uv.data() + uv.size(), sourceColorBegin + i, sourceColorBegin + i + 1);
                    if (!result)
                    {
                        QMutexLocker lock(&functionErrorMutex);
                        if (!functionError)
                        {
                            functionError = result;
                        }
                    }
                }
            }
        };

        PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Content, indices.cbegin(), indices.cend(), setColor);

        if (!functionError)
        {
            throw PDFRendererException(RenderErrorType::Error, PDFTranslationContext::tr("Error occured during mesh generation of shading: %1").arg(functionError.errorMessage));
        }

        // Check the colors, if mesh is bad, then refine it
        std::atomic_bool isMeshOK = true;
        auto validateMesh = [&](size_t index)
        {
            if (!isMeshOK.load(std::memory_order_relaxed))
            {
                return;
            }

            auto [row, column] = indexToRowColumn(index);
            const size_t colorComponentIndex = rowColumnToFirstColorComponent(row, column);

            // Check, if color left doesn't differ too much
            if (column > 0)
            {
                const size_t colorOtherComponentIndex = rowColumnToFirstColorComponent(row, column - 1);
                for (size_t i = 0; i < colorComponents; ++i)
                {
                    if (std::fabs(sourceColorBuffer[colorComponentIndex + i] - sourceColorBuffer[colorOtherComponentIndex + i]) > settings.tolerance)
                    {
                        isMeshOK.store(false, std::memory_order_relaxed);
                        return;
                    }
                }
            }

            if (row > 0)
            {
                const size_t colorOtherComponentIndex = rowColumnToFirstColorComponent(row - 1, column);
                for (size_t i = 0; i < colorComponents; ++i)
                {
                    if (std::fabs(sourceColorBuffer[colorComponentIndex + i] - sourceColorBuffer[colorOtherComponentIndex + i]) > settings.tolerance)
                    {
                        isMeshOK.store(false, std::memory_order_relaxed);
                        return;
                    }
                }
            }
        };
        PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Content, indices.cbegin(), indices.cend(), validateMesh);
        if (!isMeshOK && resolution != settings.minimalMeshResolution)
        {
            continue;
        }

        // Now, we are ready to generate the mesh
        std::vector<QRgb> colors;
        colors.resize(rowCount * columnCount, QRgb());

        mesh.setVertices(qMove(gridPoints));
        std::vector<PDFMesh::Triangle> triangles;
        triangles.resize((rowCount - 1) * (columnCount - 1) * 2);

        auto generateTriangle = [&](size_t index)
        {
            auto [row, column] = indexToRowColumn(index);
                    if (row == 0 || column == 0)
            {
                return;
            }

            Q_ASSERT(index == rowColumnToIndex(row, column));

            const size_t triangleIndex1 = ((row - 1) * (columnCount - 1) + column - 1) * 2;
            const size_t triangleIndex2 = triangleIndex1 + 1;
            const size_t v1 = rowColumnToIndex(row - 1, column - 1);
            const size_t v2 = rowColumnToIndex(row - 1, column);
            const size_t v3 = index;
            const size_t v4 = rowColumnToIndex(row, column - 1);
            std::vector<PDFReal> colorBuffer;
            colorBuffer.resize(colorComponents, 0.0);

            auto calculateColor = [&](const PDFMesh::Triangle& triangle)
            {
                QPointF centerDS = mesh.getTriangleCenter(triangle);
                QPointF center = deviceSpaceToDomainMatrix.map(centerDS);

                std::array<PDFReal, 2> uv = { center.x(), center.y() };

                if (isSingleFunction)
                {
                    PDFFunction::FunctionResult result = m_functions.front()->apply(uv.data(), uv.data() + uv.size(), colorBuffer.data(), colorBuffer.data() + colorBuffer.size());
                    if (!result)
                    {
                        QMutexLocker lock(&functionErrorMutex);
                        if (!functionError)
                        {
                            functionError = result;
                        }
                    }
                }
                else
                {
                    for (size_t i = 0, count = colorComponents; i < count; ++i)
                    {
                        PDFFunction::FunctionResult result = m_functions[i]->apply(uv.data(), uv.data() + uv.size(), colorBuffer.data() + i, colorBuffer.data() + i + 1);
                        if (!result)
                        {
                            QMutexLocker lock(&functionErrorMutex);
                            if (!functionError)
                            {
                                functionError = result;
                            }
                        }
                    }
                }

                return m_colorSpace->getColor(PDFAbstractColorSpace::convertToColor(colorBuffer), cms, intent, reporter, true);
            };

            PDFMesh::Triangle triangle1;
            triangle1.v1 = static_cast<uint32_t>(v1);
            triangle1.v2 = static_cast<uint32_t>(v2);
            triangle1.v3 = static_cast<uint32_t>(v3);
            triangle1.color = calculateColor(triangle1).rgb();

            PDFMesh::Triangle triangle2;
            triangle2.v1 = static_cast<uint32_t>(v3);
            triangle2.v2 = static_cast<uint32_t>(v4);
            triangle2.v3 = static_cast<uint32_t>(v1);
            triangle2.color = calculateColor(triangle2).rgb();

            triangles[triangleIndex1] = triangle1;
            triangles[triangleIndex2] = triangle2;
        };
        PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Content, indices.cbegin(), indices.cend(), generateTriangle);
        mesh.setTriangles(qMove(triangles));

        if (!functionError)
        {
            throw PDFRendererException(RenderErrorType::Error, PDFTranslationContext::tr("Error occured during mesh generation of shading: %1").arg(functionError.errorMessage));
        }

        break;
    }

    if (m_backgroundColor.isValid())
    {
        QPainterPath backgroundPath;
        backgroundPath.addRect(settings.deviceSpaceMeshingArea);

        QPainterPath paintedPath;
        paintedPath.addPolygon(domainToDeviceSpaceMatrix.map(m_domain));

        backgroundPath = backgroundPath.subtracted(paintedPath);
        mesh.setBackgroundPath(backgroundPath);
        mesh.setBackgroundColor(m_backgroundColor);
    }

    // Create bounding path
    if (m_boundingBox.isValid())
    {
        QPainterPath boundingPath;
        boundingPath.addPolygon(patternSpaceToDeviceSpaceMatrix.map(m_boundingBox));
        mesh.setBoundingPath(boundingPath);
    }

    return mesh;
}

PDFShadingSampler* PDFFunctionShading::createSampler(QTransform userSpaceToDeviceSpaceMatrix) const
{
    return new PDFFunctionShadingSampler(this, userSpaceToDeviceSpaceMatrix);
}

PDFMesh PDFAxialShading::createMesh(const PDFMeshQualitySettings& settings,
                                    const PDFCMS* cms,
                                    RenderingIntent intent,
                                    PDFRenderErrorReporter* reporter,
                                    const PDFOperationControl* operationControl) const
{
    PDFMesh mesh;

    Q_UNUSED(operationControl);

    QTransform patternSpaceToDeviceSpaceMatrix = getPatternSpaceToDeviceSpaceMatrix(settings);
    QPointF p1 = patternSpaceToDeviceSpaceMatrix.map(m_startPoint);
    QPointF p2 = patternSpaceToDeviceSpaceMatrix.map(m_endPoint);

    // Strategy: for simplification, we rotate the line clockwise so we will
    // get the shading axis equal to the x-axis. Then we will determine the shading
    // area and create mesh according the settings.
    QLineF line(p1, p2);
    const double angle = line.angleTo(QLineF(0, 0, 1, 0));

    // Matrix p1p2LCS is local coordinate system of line p1-p2. It transforms
    // points on the line to the global coordinate system. So, point (0, 0) will
    // map onto p1 and point (length(p1-p2), 0) will map onto p2.
    QTransform p1p2LCS;
    p1p2LCS.translate(p1.x(), p1.y());
    p1p2LCS.rotate(angle);
    QTransform p1p2GCS = p1p2LCS.inverted();

    QPointF p1m = p1p2GCS.map(p1);
    QPointF p2m = p1p2GCS.map(p2);

    Q_ASSERT(isZero(p1m.y()));
    Q_ASSERT(isZero(p2m.y()));
    Q_ASSERT(p1m.x() <= p2m.x());

    QPainterPath meshingArea;
    meshingArea.addPolygon(p1p2GCS.map(settings.deviceSpaceMeshingArea));
    meshingArea.addRect(p1m.x(), p1m.y() - settings.preferredMeshResolution * 0.5, p2m.x() - p1m.x(), settings.preferredMeshResolution);
    QRectF meshingRectangle = meshingArea.boundingRect();

    PDFReal xl = meshingRectangle.left();
    PDFReal xr = meshingRectangle.right();
    PDFReal yt = meshingRectangle.top();
    PDFReal yb = meshingRectangle.bottom();

    // Create coordinate array filled with stops, where we will determine the color
    std::vector<PDFReal> xCoords;
    xCoords.reserve((xr - xl) / settings.minimalMeshResolution + 3);
    xCoords.push_back(xl);
    for (PDFReal x = p1m.x(); x <= p2m.x(); x += settings.minimalMeshResolution)
    {
        if (!qFuzzyCompare(xCoords.back(), x))
        {
            xCoords.push_back(x);
        }
    }

    if (xCoords.back() + PDF_EPSILON < p2m.x())
    {
        xCoords.push_back(p2m.x());
    }

    if (!qFuzzyCompare(xCoords.back(), xr))
    {
        xCoords.push_back(xr);
    }

    const PDFReal tAtStart = m_domainStart;
    const PDFReal tAtEnd = m_domainEnd;
    const PDFReal tMin = qMin(tAtStart, tAtEnd);
    const PDFReal tMax = qMax(tAtStart, tAtEnd);

    const bool isSingleFunction = m_functions.size() == 1;
    std::vector<PDFReal> colorBuffer(m_colorSpace->getColorComponentCount(), 0.0);
    auto getColor = [this, isSingleFunction, &colorBuffer](PDFReal t) -> PDFColor
    {
        if (isSingleFunction)
        {
            PDFFunction::FunctionResult result = m_functions.front()->apply(&t, &t + 1, colorBuffer.data(), colorBuffer.data() + colorBuffer.size());
            if (!result)
            {
                throw PDFRendererException(RenderErrorType::Error, PDFTranslationContext::tr("Error occured during mesh creation of shading: %1").arg(result.errorMessage));
            }
        }
        else
        {
            for (size_t i = 0, count = colorBuffer.size(); i < count; ++i)
            {
                PDFFunction::FunctionResult result = m_functions[i]->apply(&t, &t + 1, colorBuffer.data() + i, colorBuffer.data() + i + 1);
                if (!result)
                {
                    throw PDFRendererException(RenderErrorType::Error, PDFTranslationContext::tr("Error occured during mesh creation of shading: %1").arg(result.errorMessage));
                }
            }
        }

        return PDFAbstractColorSpace::convertToColor(colorBuffer);
    };

    // Determine color of each coordinate
    std::vector<std::pair<PDFReal, PDFColor>> coloredCoordinates;
    coloredCoordinates.reserve(xCoords.size());

    for (PDFReal x : xCoords)
    {
        if (x < p1m.x() - PDF_EPSILON && !m_extendStart)
        {
            // Move to the next coordinate, this is skipped
            continue;
        }

        if (x > p2m.x() + PDF_EPSILON && !m_extendEnd)
        {
            // We are finished no more triangles will occur
            break;
        }

        // Determine current parameter t
        const PDFReal t = interpolate(x, p1m.x(), p2m.x(), tAtStart, tAtEnd);
        const PDFReal tBounded = qBound(tMin, t, tMax);
        const PDFColor color = getColor(tBounded);
        coloredCoordinates.emplace_back(x, color);
    }

    // Filter coordinates according the meshing criteria
    std::vector<std::pair<PDFReal, PDFColor>> filteredCoordinates;
    filteredCoordinates.reserve(coloredCoordinates.size());

    for (auto it = coloredCoordinates.cbegin(); it != coloredCoordinates.cend(); ++it)
    {
        // We will skip this coordinate, if both of meshing criteria have been met:
        //  1) Color difference is small (lesser than tolerance)
        //  2) Distance from previous and next point is less than preferred meshing resolution OR colors are equal

        if (it != coloredCoordinates.cbegin() && std::next(it) != coloredCoordinates.cend())
        {
            auto itNext = std::next(it);

            const std::pair<PDFReal, PDFColor>& prevItem = filteredCoordinates.back();
            const std::pair<PDFReal, PDFColor>& currentItem = *it;
            const std::pair<PDFReal, PDFColor>& nextItem = *itNext;

            if (currentItem.first != p1m.x() && currentItem.first != p2m.x())
            {
                if (prevItem.second == currentItem.second && currentItem.second == nextItem.second)
                {
                    // Colors are same, skip the test
                    continue;
                }

                if (PDFAbstractColorSpace::isColorEqual(prevItem.second, currentItem.second, settings.tolerance) &&
                        PDFAbstractColorSpace::isColorEqual(currentItem.second, nextItem.second, settings.tolerance) &&
                        PDFAbstractColorSpace::isColorEqual(prevItem.second, nextItem.second, settings.tolerance) &&
                        (nextItem.first - prevItem.first < settings.preferredMeshResolution))
                {
                    continue;
                }
            }
        }

        filteredCoordinates.push_back(*it);
    }

    if (!filteredCoordinates.empty())
    {
        size_t vertexCount = filteredCoordinates.size() * 2;
        size_t triangleCount = filteredCoordinates.size() * 2 - 2;

        if (m_backgroundColor.isValid())
        {
            vertexCount += 8;
            triangleCount += 4;
        }
        mesh.reserve(vertexCount, triangleCount);

        PDFColor previousColor = filteredCoordinates.front().second;
        uint32_t topLeft = mesh.addVertex(QPointF(filteredCoordinates.front().first, yt));
        uint32_t bottomLeft = mesh.addVertex(QPointF(filteredCoordinates.front().first, yb));
        for (auto it = std::next(filteredCoordinates.cbegin()); it != filteredCoordinates.cend(); ++it)
        {
            const std::pair<PDFReal, PDFColor>& item = *it;

            uint32_t topRight = mesh.addVertex(QPointF(item.first, yt));
            uint32_t bottomRight = mesh.addVertex(QPointF(item.first, yb));

            PDFColor mixedColor = PDFAbstractColorSpace::mixColors(previousColor, item.second, 0.5);
            QColor color = m_colorSpace->getColor(mixedColor, cms, intent, reporter, true);
            mesh.addQuad(topLeft, topRight, bottomRight, bottomLeft, color.rgb());

            topLeft = topRight;
            bottomLeft = bottomRight;
            previousColor = item.second;
        }
    }

    // Create background color triangles
    if (m_backgroundColor.isValid() && (!m_extendStart || !m_extendEnd))
    {
        if (!m_extendStart && xl + PDF_EPSILON < p1m.x())
        {
            uint32_t topLeft = mesh.addVertex(QPointF(xl, yt));
            uint32_t topRight = mesh.addVertex(QPointF(p1m.x(), yt));
            uint32_t bottomLeft = mesh.addVertex(QPointF(xl, yb));
            uint32_t bottomRight = mesh.addVertex(QPointF(p1m.x(), yb));
            mesh.addQuad(topLeft, topRight, bottomRight, bottomLeft, m_backgroundColor.rgb());
        }

        if (!m_extendEnd && p2m.x() + PDF_EPSILON < xr)
        {
            uint32_t topRight = mesh.addVertex(QPointF(xr, yt));
            uint32_t topLeft = mesh.addVertex(QPointF(p2m.x(), yt));
            uint32_t bottomRight = mesh.addVertex(QPointF(xr, yb));
            uint32_t bottomLeft = mesh.addVertex(QPointF(p2m.x(), yb));
            mesh.addQuad(topLeft, topRight, bottomRight, bottomLeft, m_backgroundColor.rgb());
        }
    }

    // Transform mesh to the device space coordinates
    mesh.transform(p1p2LCS);

    // Create bounding path
    if (m_boundingBox.isValid())
    {
        QPainterPath boundingPath;
        boundingPath.addPolygon(patternSpaceToDeviceSpaceMatrix.map(m_boundingBox));
        mesh.setBoundingPath(boundingPath);
    }

    return mesh;
}

class PDFAxialShadingSampler : public PDFShadingSampler
{
public:
    PDFAxialShadingSampler(const PDFAxialShading* axialShadingPattern, QTransform userSpaceToDeviceSpaceMatrix) :
        PDFShadingSampler(axialShadingPattern),
        m_axialShadingPattern(axialShadingPattern),
        m_xStart(0.0),
        m_xEnd(0.0),
        m_tAtStart(0.0),
        m_tAtEnd(0.0),
        m_tMin(0.0),
        m_tMax(0.0)
    {
        QTransform patternSpaceToDeviceSpace = axialShadingPattern->getMatrix() * userSpaceToDeviceSpaceMatrix;

        QPointF p1 = patternSpaceToDeviceSpace.map(axialShadingPattern->getStartPoint());
        QPointF p2 = patternSpaceToDeviceSpace.map(axialShadingPattern->getEndPoint());

        // Strategy: for simplification, we rotate the line clockwise so we will
        // get the shading axis equal to the x-axis. Then we will determine the shading
        // area and create mesh according the settings.
        QLineF line(p1, p2);
        const double angle = line.angleTo(QLineF(0, 0, 1, 0));

        // Matrix p1p2LCS is local coordinate system of line p1-p2. It transforms
        // points on the line to the global coordinate system. So, point (0, 0) will
        // map onto p1 and point (length(p1-p2), 0) will map onto p2.
        QTransform p1p2LCS;
        p1p2LCS.translate(p1.x(), p1.y());
        p1p2LCS.rotate(angle);
        QTransform p1p2GCS = p1p2LCS.inverted();

        QPointF p1m = p1p2GCS.map(p1);
        QPointF p2m = p1p2GCS.map(p2);

        Q_ASSERT(isZero(p1m.y()));
        Q_ASSERT(isZero(p2m.y()));
        Q_ASSERT(p1m.x() <= p2m.x());

        m_xStart = p1m.x();
        m_xEnd = p2m.x();

        m_tAtStart = axialShadingPattern->getDomainStart();
        m_tAtEnd = axialShadingPattern->getDomainEnd();
        m_tMin = qMin(m_tAtStart, m_tAtEnd);
        m_tMax = qMax(m_tAtStart, m_tAtEnd);

        m_p1p2GCS = p1p2GCS;
    }

    virtual bool sample(const QPointF& devicePoint, PDFColorBuffer outputBuffer, int limit) const override
    {
        Q_UNUSED(limit);

        if (!m_pattern->getColorSpace() || m_pattern->getColorSpace()->getColorComponentCount() != outputBuffer.size())
        {
            // Invalid color space, or invalid color buffer
            return false;
        }

        QPointF mappedPoint = m_p1p2GCS.map(devicePoint);
        const PDFReal x = mappedPoint.x();

        PDFReal t = m_tAtStart;

        if (x < m_xStart)
        {
            if (!m_axialShadingPattern->isExtendStart())
            {
                return false;
            }

            if (fillBackgroundColor(outputBuffer))
            {
                return true;
            }

            t = m_tAtStart;
        }
        else if (x > m_xEnd)
        {
            if (!m_axialShadingPattern->isExtendEnd())
            {
                return false;
            }

            if (fillBackgroundColor(outputBuffer))
            {
                return true;
            }

            t = m_tAtEnd;
        }
        else
        {
            t = interpolate(x, m_xStart, m_xEnd, m_tAtStart, m_tAtEnd);
            t = qBound(m_tMin, t, m_tMax);
        }

        const auto& functions = m_axialShadingPattern->getFunctions();
        std::array<PDFReal, PDF_MAX_COLOR_COMPONENTS> colorBuffer = { };

        if (colorBuffer.size() < outputBuffer.size())
        {
            // Jakub Melka: Too much colors - we cant process it
            return false;
        }

        if (functions.size() == 1)
        {
            Q_ASSERT(outputBuffer.size() <= colorBuffer.size());
            PDFFunction::FunctionResult result = functions.front()->apply(&t, &t + 1, colorBuffer.data(), colorBuffer.data() + outputBuffer.size());

            if (!result)
            {
                // Function call failed
                return false;
            }
        }
        else
        {
            if (functions.size() != outputBuffer.size())
            {
                // Invalid number of functions
                return false;
            }

            Q_ASSERT(outputBuffer.size() <= colorBuffer.size());
            for (size_t i = 0, count = outputBuffer.size(); i < count; ++i)
            {
                PDFFunction::FunctionResult result = functions[i]->apply(&t, &t + 1, colorBuffer.data() + i, colorBuffer.data() + i + 1);

                if (!result)
                {
                    // Function call failed
                    return false;
                }
            }
        }

        for (size_t i = 0, count = outputBuffer.size(); i < count; ++i)
        {
            outputBuffer[i] = colorBuffer[i];
        }

        return true;
    }

private:
    const PDFAxialShading* m_axialShadingPattern;
    QTransform m_p1p2GCS;
    PDFReal m_xStart;
    PDFReal m_xEnd;
    PDFReal m_tAtStart;
    PDFReal m_tAtEnd;
    PDFReal m_tMin;
    PDFReal m_tMax;
};

PDFShadingSampler* PDFAxialShading::createSampler(QTransform userSpaceToDeviceSpaceMatrix) const
{
    return new PDFAxialShadingSampler(this, userSpaceToDeviceSpaceMatrix);
}

void PDFMesh::paint(QPainter* painter, PDFReal alpha) const
{
    if (m_triangles.empty())
    {
        return;
    }

    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setRenderHint(QPainter::Antialiasing, true);

    // Set the clipping area, if we have it
    if (!m_boundingPath.isEmpty())
    {
        painter->setClipPath(m_boundingPath, Qt::IntersectClip);
    }

    if (!m_backgroundPath.isEmpty() && m_backgroundColor.isValid())
    {
        QColor backgroundColor = m_backgroundColor;
        backgroundColor.setAlphaF(alpha);
        painter->setBrush(QBrush(backgroundColor, Qt::SolidPattern));
        painter->drawPath(m_backgroundPath);
    }

    QColor color;

    // Draw all triangles
    for (const Triangle& triangle : m_triangles)
    {
        if (color != triangle.color)
        {
            QColor newColor(triangle.color);
            newColor.setAlphaF(alpha);
            painter->setPen(newColor);
            painter->setBrush(QBrush(newColor, Qt::SolidPattern));
            color = newColor;
        }

        std::array<QPointF, 3> triangleCorners = { m_vertices[triangle.v1], m_vertices[triangle.v2], m_vertices[triangle.v3] };
        painter->drawConvexPolygon(triangleCorners.data(), static_cast<int>(triangleCorners.size()));
    }

    painter->restore();
}

void PDFMesh::transform(const QTransform& matrix)
{
    for (QPointF& vertex : m_vertices)
    {
        vertex = matrix.map(vertex);
    }

    m_boundingPath = matrix.map(m_boundingPath);
    m_backgroundPath = matrix.map(m_backgroundPath);
}

void PDFMesh::addMesh(std::vector<QPointF>&& vertices, std::vector<PDFMesh::Triangle>&& triangles)
{
    if (isEmpty())
    {
        m_vertices = qMove(vertices);
        m_triangles = qMove(triangles);
    }
    else
    {
        size_t offset = m_vertices.size();
        m_vertices.insert(m_vertices.cend(), vertices.cbegin(), vertices.cend());

        for (Triangle& triangle : triangles)
        {
            triangle.v1 += static_cast<uint32_t>(offset);
            triangle.v2 += static_cast<uint32_t>(offset);
            triangle.v3 += static_cast<uint32_t>(offset);
        }

        m_triangles.insert(m_triangles.cend(), triangles.cbegin(), triangles.cend());
    }
}

QPointF PDFMesh::getTriangleCenter(const PDFMesh::Triangle& triangle) const
{
    return (m_vertices[triangle.v1] + m_vertices[triangle.v2] + m_vertices[triangle.v3]) / 3.0;
}

qint64 PDFMesh::getMemoryConsumptionEstimate() const
{
    qint64 memoryConsumption = sizeof(*this);
    memoryConsumption += sizeof(QPointF) * m_vertices.capacity();
    memoryConsumption += sizeof(Triangle) * m_triangles.capacity();
    memoryConsumption += sizeof(QPainterPath::Element) * m_boundingPath.capacity();
    memoryConsumption += sizeof(QPainterPath::Element) * m_backgroundPath.capacity();
    return memoryConsumption;
}

void PDFMesh::convertColors(const PDFColorConvertor& colorConvertor)
{
    for (Triangle& triangle : m_triangles)
    {
        QColor color = QColor::fromRgb(triangle.color);
        QColor adjustedColor = colorConvertor.convert(color, false, false);
        triangle.color = adjustedColor.rgb();
    }

    m_backgroundColor = colorConvertor.convert(m_backgroundColor, true, false);
}

void PDFMeshQualitySettings::initResolution()
{
    Q_ASSERT(deviceSpaceMeshingArea.isValid());
    PDFReal size = qMax(deviceSpaceMeshingArea.width(), deviceSpaceMeshingArea.height());
    minimalMeshResolution = size * minimalMeshResolutionRatio;
    preferredMeshResolution = size * qMax(preferredMeshResolutionRatio, minimalMeshResolutionRatio);
}

ShadingType PDFRadialShading::getShadingType() const
{
    return ShadingType::Radial;
}

PDFMesh PDFRadialShading::createMesh(const PDFMeshQualitySettings& settings,
                                     const PDFCMS* cms,
                                     RenderingIntent intent,
                                     PDFRenderErrorReporter* reporter,
                                     const PDFOperationControl* operationControl) const
{
    PDFMesh mesh;

    Q_UNUSED(operationControl);

    QTransform patternSpaceToDeviceSpaceMatrix = getPatternSpaceToDeviceSpaceMatrix(settings);
    QPointF p1 = patternSpaceToDeviceSpaceMatrix.map(m_startPoint);
    QPointF p2 = patternSpaceToDeviceSpaceMatrix.map(m_endPoint);

    QPointF r1TestPoint = patternSpaceToDeviceSpaceMatrix.map(QPointF(m_startPoint.x(), m_startPoint.y() + m_r0));
    QPointF r2TestPoint = patternSpaceToDeviceSpaceMatrix.map(QPointF(m_endPoint.x(), m_endPoint.y() + m_r1));

    const PDFReal r1 = QLineF(p1, r1TestPoint).length();
    const PDFReal r2 = QLineF(p2, r2TestPoint).length();

    // Strategy: for simplification, we rotate the line clockwise so we will
    // get the shading axis equal to the x-axis. Then we will determine the shading
    // area and create mesh according the settings.
    QLineF line(p1, p2);
    const double angle = line.angleTo(QLineF(0, 0, 1, 0));

    // Matrix p1p2LCS is local coordinate system of line p1-p2. It transforms
    // points on the line to the global coordinate system. So, point (0, 0) will
    // map onto p1 and point (length(p1-p2), 0) will map onto p2.
    QTransform p1p2LCS;
    p1p2LCS.translate(p1.x(), p1.y());
    p1p2LCS.rotate(angle);
    QTransform p1p2GCS = p1p2LCS.inverted();

    QPointF p1m = p1p2GCS.map(p1);
    QPointF p2m = p1p2GCS.map(p2);

    Q_ASSERT(isZero(p1m.y()));
    Q_ASSERT(isZero(p2m.y()));
    Q_ASSERT(p1m.x() <= p2m.x());

    QPainterPath meshingArea;
    meshingArea.addPolygon(p1p2GCS.map(settings.deviceSpaceMeshingArea));
    QRectF meshingRectangle = meshingArea.boundingRect();

    PDFReal xl = p1m.x();
    PDFReal xr = p2m.x();

    if (m_extendStart)
    {
        // Well, we must calculate the "zero" point, i.e. when starting radius become zero.
        // It will happen, when r1 < r2, if r1 >= r2, then radius never become zero. We also
        // bound the start by target draw area. We have line between points:
        //
        //  Line: (x1, r1) to (x2, r2)
        // and we will calculate intersection with x axis. If we found intersection points, which
        // is on the left side, then we

        if (r1 > r2)
        {
            xl = meshingRectangle.left() - 2 * r1;
        }
        else
        {
            QLineF radiusInterpolationLine(p1m.x(), r1, p2m.x(), r2);
            QLineF xAxisLine(p1m.x(), 0, p2m.x(), 0);

            QPointF intersectionPoint;
            if (radiusInterpolationLine.intersects(xAxisLine, &intersectionPoint) != QLineF::NoIntersection)
            {
                xl = qBound(meshingRectangle.left() - r1, intersectionPoint.x(), xl);
            }
            else
            {
                xl = meshingRectangle.left() - 2 * r1;
            }
        }
    }

    if (m_extendEnd)
    {
        // Similar as in previous case, find the "zero" point, i.e. when ending radius become zero.

        if (r1 < r2)
        {
            xr = meshingRectangle.right() + 2 * r2;
        }
        else
        {
            QLineF radiusInterpolationLine(p1m.x(), r1, p2m.x(), r2);
            QLineF xAxisLine(p1m.x(), 0, p2m.x(), 0);

            QPointF intersectionPoint;
            if (radiusInterpolationLine.intersects(xAxisLine, &intersectionPoint) != QLineF::NoIntersection)
            {
                xr = qBound(xr, intersectionPoint.x(), meshingRectangle.right() + r2);
            }
            else
            {
                xr = meshingRectangle.right() + 2 * r2;
            }
        }
    }

    // Create coordinate array filled with stops, where we will determine the color
    std::vector<PDFReal> xCoords;
    xCoords.reserve((xr - xl) / settings.minimalMeshResolution + 3);
    xCoords.push_back(xl);
    for (PDFReal x = p1m.x(); x <= p2m.x(); x += settings.minimalMeshResolution)
    {
        if (!qFuzzyCompare(xCoords.back(), x))
        {
            xCoords.push_back(x);
        }
    }

    if (xCoords.back() + PDF_EPSILON < p2m.x())
    {
        xCoords.push_back(p2m.x());
    }

    if (!qFuzzyCompare(xCoords.back(), xr))
    {
        xCoords.push_back(xr);
    }

    const PDFReal tAtStart = m_domainStart;
    const PDFReal tAtEnd = m_domainEnd;
    const PDFReal tMin = qMin(tAtStart, tAtEnd);
    const PDFReal tMax = qMax(tAtStart, tAtEnd);

    const bool isSingleFunction = m_functions.size() == 1;
    std::vector<PDFReal> colorBuffer(m_colorSpace->getColorComponentCount(), 0.0);
    auto getColor = [this, isSingleFunction, &colorBuffer](PDFReal t) -> PDFColor
    {
        if (isSingleFunction)
        {
            PDFFunction::FunctionResult result = m_functions.front()->apply(&t, &t + 1, colorBuffer.data(), colorBuffer.data() + colorBuffer.size());
            if (!result)
            {
                throw PDFRendererException(RenderErrorType::Error, PDFTranslationContext::tr("Error occured during mesh creation of shading: %1").arg(result.errorMessage));
            }
        }
        else
        {
            for (size_t i = 0, count = colorBuffer.size(); i < count; ++i)
            {
                PDFFunction::FunctionResult result = m_functions[i]->apply(&t, &t + 1, colorBuffer.data() + i, colorBuffer.data() + i + 1);
                if (!result)
                {
                    throw PDFRendererException(RenderErrorType::Error, PDFTranslationContext::tr("Error occured during mesh creation of shading: %1").arg(result.errorMessage));
                }
            }
        }

        return PDFAbstractColorSpace::convertToColor(colorBuffer);
    };

    // Determine color of each coordinate
    std::vector<std::pair<PDFReal, PDFColor>> coloredCoordinates;
    coloredCoordinates.reserve(xCoords.size());

    for (PDFReal x : xCoords)
    {
        // Determine current parameter t
        const PDFReal t = interpolate(x, p1m.x(), p2m.x(), tAtStart, tAtEnd);
        const PDFReal tBounded = qBound(tMin, t, tMax);
        const PDFColor color = getColor(tBounded);
        coloredCoordinates.emplace_back(x, color);
    }

    // Filter coordinates according the meshing criteria
    std::vector<std::pair<PDFReal, PDFColor>> filteredCoordinates;
    filteredCoordinates.reserve(coloredCoordinates.size());

    for (auto it = coloredCoordinates.cbegin(); it != coloredCoordinates.cend(); ++it)
    {
        // We will skip this coordinate, if both of meshing criteria have been met:
        //  1) Color difference is small (lesser than tolerance)
        //  2) Distance from previous and next point is less than preferred meshing resolution OR colors are equal

        if (it != coloredCoordinates.cbegin() && std::next(it) != coloredCoordinates.cend())
        {
            auto itNext = std::next(it);

            const std::pair<PDFReal, PDFColor>& prevItem = filteredCoordinates.back();
            const std::pair<PDFReal, PDFColor>& currentItem = *it;
            const std::pair<PDFReal, PDFColor>& nextItem = *itNext;

            if (currentItem.first != p1m.x() && currentItem.first != p2m.x())
            {
                if (prevItem.second == currentItem.second && currentItem.second == nextItem.second)
                {
                    // Colors are same, skip the test
                    continue;
                }

                if (PDFAbstractColorSpace::isColorEqual(prevItem.second, currentItem.second, settings.tolerance) &&
                        PDFAbstractColorSpace::isColorEqual(currentItem.second, nextItem.second, settings.tolerance) &&
                        PDFAbstractColorSpace::isColorEqual(prevItem.second, nextItem.second, settings.tolerance) &&
                        (nextItem.first - prevItem.first < settings.preferredMeshResolution))
                {
                    continue;
                }
            }
        }

        filteredCoordinates.push_back(*it);
    }

    if (!filteredCoordinates.empty())
    {
        constexpr const int SLICES = 120;

        size_t vertexCount = filteredCoordinates.size() * SLICES * 4;
        size_t triangleCount = filteredCoordinates.size() * SLICES * 2;

        if (m_backgroundColor.isValid())
        {
            vertexCount += 4;
            triangleCount += 2;
        }
        mesh.reserve(vertexCount, triangleCount);

        // Create background color triangles
        if (m_backgroundColor.isValid())
        {
            uint32_t topLeft = mesh.addVertex(meshingRectangle.topLeft());
            uint32_t topRight = mesh.addVertex(meshingRectangle.topRight());
            uint32_t bottomLeft = mesh.addVertex(meshingRectangle.bottomRight());
            uint32_t bottomRight = mesh.addVertex(meshingRectangle.bottomLeft());
            mesh.addQuad(topLeft, topRight, bottomRight, bottomLeft, m_backgroundColor.rgb());
        }

        // Create radial shading triangles
        QLineF rLine(QPointF(p1m.x(), r1), QPointF(p2m.x(), r2));
        const PDFReal rlength = rLine.length();

        for (auto it = std::next(filteredCoordinates.cbegin()); it != filteredCoordinates.cend(); ++it)
        {
            const std::pair<PDFReal, PDFColor>& leftItem = *std::prev(it);
            const std::pair<PDFReal, PDFColor>& rightItem = *it;

            const PDFReal x0 = leftItem.first;
            const PDFReal x1 = rightItem.first;
            const PDFColor mixedColor = PDFAbstractColorSpace::mixColors(leftItem.second, rightItem.second, 0.5);
            const PDFReal angleStep = 2 * M_PI / SLICES;
            const PDFReal cr0 = rLine.pointAt((x0 - p1m.x()) / rlength).y();
            const PDFReal cr1 = rLine.pointAt((x1 - p1m.x()) / rlength).y();

            PDFReal angle0 = 0;
            for (int i = 0; i < SLICES; ++i)
            {
                const PDFReal angle1 = angle0 + angleStep;
                const PDFReal cos0 = std::cos(angle0);
                const PDFReal sin0 = std::sin(angle0);
                const PDFReal cos1 = std::cos(angle1);
                const PDFReal sin1 = std::sin(angle1);

                QPointF cp1(x0 + cos0 * cr0, sin0 * cr0);
                QPointF cp2(x1 + cos0 * cr1, sin0 * cr1);
                QPointF cp3(x1 + cos1 * cr1, sin1 * cr1);
                QPointF cp4(x0 + cos1 * cr0, sin1 * cr0);

                uint32_t v1 = mesh.addVertex(cp1);
                uint32_t v2 = mesh.addVertex(cp2);
                uint32_t v3 = mesh.addVertex(cp3);
                uint32_t v4 = mesh.addVertex(cp4);

                QColor color = m_colorSpace->getColor(mixedColor, cms, intent, reporter, true);
                mesh.addQuad(v1, v2, v3, v4, color.rgb());

                angle0 = angle1;
            }
        }
    }

    // Transform mesh to the device space coordinates
    mesh.transform(p1p2LCS);

    // Create bounding path
    if (m_boundingBox.isValid())
    {
        QPainterPath boundingPath;
        boundingPath.addPolygon(patternSpaceToDeviceSpaceMatrix.map(m_boundingBox));
        mesh.setBoundingPath(boundingPath);
    }

    return mesh;
}

class PDFRadialShadingSampler : public PDFShadingSampler
{
public:
    PDFRadialShadingSampler(const PDFRadialShading* radialShadingPattern, QTransform userSpaceToDeviceSpaceMatrix) :
        PDFShadingSampler(radialShadingPattern),
        m_radialShadingPattern(radialShadingPattern),
        m_xStart(0.0),
        m_xEnd(0.0),
        m_tAtStart(0.0),
        m_tAtEnd(0.0),
        m_tMin(0.0),
        m_tMax(0.0),
        m_r0(0.0),
        m_r1(0.0)
    {
        QTransform patternSpaceToDeviceSpace = radialShadingPattern->getMatrix() * userSpaceToDeviceSpaceMatrix;

        QPointF p1 = patternSpaceToDeviceSpace.map(radialShadingPattern->getStartPoint());
        QPointF p2 = patternSpaceToDeviceSpace.map(radialShadingPattern->getEndPoint());

        QPointF r0TestPoint = patternSpaceToDeviceSpace.map(radialShadingPattern->getStartPoint() + QPointF(0.0, radialShadingPattern->getR0()));
        QPointF r1TestPoint = patternSpaceToDeviceSpace.map(radialShadingPattern->getEndPoint() + QPointF(0.0, radialShadingPattern->getR1()));

        const PDFReal r0 = QLineF(p1, r0TestPoint).length();
        const PDFReal r1 = QLineF(p2, r1TestPoint).length();

        // Strategy: for simplification, we rotate the line clockwise so we will
        // get the shading axis equal to the x-axis.
        QLineF line(p1, p2);
        const double angle = line.angleTo(QLineF(0, 0, 1, 0));

        // Matrix p1p2LCS is local coordinate system of line p1-p2. It transforms
        // points on the line to the global coordinate system. So, point (0, 0) will
        // map onto p1 and point (length(p1-p2), 0) will map onto p2.
        QTransform p1p2LCS;
        p1p2LCS.translate(p1.x(), p1.y());
        p1p2LCS.rotate(angle);
        QTransform p1p2GCS = p1p2LCS.inverted();

        QPointF p1m = p1p2GCS.map(p1);
        QPointF p2m = p1p2GCS.map(p2);

        Q_ASSERT(isZero(p1m.y()));
        Q_ASSERT(isZero(p2m.y()));
        Q_ASSERT(p1m.x() <= p2m.x());

        m_xStart = p1m.x();
        m_xEnd = p2m.x();

        m_tAtStart = radialShadingPattern->getDomainStart();
        m_tAtEnd = radialShadingPattern->getDomainEnd();
        m_tMin = qMin(m_tAtStart, m_tAtEnd);
        m_tMax = qMax(m_tAtStart, m_tAtEnd);

        m_r0 = r0;
        m_r1 = r1;

        m_p1p2GCS = p1p2GCS;
    }

    virtual bool sample(const QPointF& devicePoint, PDFColorBuffer outputBuffer, int limit) const override
    {
        Q_UNUSED(limit);

        if (!m_pattern->getColorSpace() || m_pattern->getColorSpace()->getColorComponentCount() != outputBuffer.size())
        {
            // Invalid color space, or invalid color buffer
            return false;
        }

        QPointF mappedPoint = m_p1p2GCS.map(devicePoint);

        // Well, how to proceed with sampling? We would like to find parameter s for point (x_p, y_p),
        // where (x_p, y_p) is mappedPoint. According to the formulas in the PDF 2.0 specification, we want
        // to find variable s:
        //
        //    x_c = x_0 + s * (x_1 - x_0)
        //    y_c = y_0 + s * (y_1 - y_0)
        //      r = r_0 + s * (r_1 - r_0)
        //
        // Where (x_c, y_c) is center of the circle. We assume this simplification: we translate the pattern
        // to horizontal axis, this implies y_0 = y_1 = 0, so y_c will be always zero. This will allow us to use
        // simplification.
        //
        // This is general equation, which we want to solve:
        //
        // (x_p - x_c)^2 + (y_p - y_c)^2 = r^2,
        // where (x_p, y_p) is sample point, (x_c, y_c) is coordinate of the circle center and r is radius.
        // If we use y_c = 0, then we get following equation:
        //
        // (x_p - x_c)^2 + y_p^2 = r^2,
        //
        // If we substitute x_c and r with formulas above, we get:
        //
        // (x_p - x_0 - s * (x_1 - x_0))^2 + y_p^2 = (r_0 + s * (r_1 - r_0))^2,
        //
        // We also have x_0 = 0, because we have origin at (0, 0), so we get following final equation:
        //
        // (x_p - s * x_1)^2 + y_p^2 = (r_0 + s * (r_1 - r_0))^2,
        //
        // which is easily solvable quadratic equation in variable s. Using wxMaxima, we get following formula
        // for our variable s:
        //
        // a.s^2 + b.s + c = 0,
        //
        // where:
        //
        // a = x_1 * x_1 - r_1 * r_1 + 2.0 * r_0 * r_1 - r_0 * r_0 = (x_1 - r_1 + r_0) * (x_1 + r_1 - r_0)
        // b = 2.0 * (-x_1 * x_p - r_0 * r_1 + r_0 * r_0)
        // c = y_p * y_p + x_p * x_p - r_0 * r_0
        //

        Q_ASSERT(qIsNull(m_xStart));

        const PDFReal x_p = mappedPoint.x();
        const PDFReal y_p = mappedPoint.y();
        const PDFReal x_1 = m_xEnd;
        const PDFReal r_0 = m_r0;
        const PDFReal r_1 = m_r1;
        const PDFReal r_1_0 = r_1 - r_0;
        const PDFReal a = x_1 * x_1 - r_1_0 * r_1_0;
        const PDFReal b = 2.0 * (-x_1 * x_p - r_0 * r_1 + r_0 * r_0);
        const PDFReal c = y_p * y_p + x_p * x_p - r_0 * r_0;
        const PDFReal Dsqr = b * b - 4.0 * a * c;

        if (Dsqr < 0.0)
        {
            return false;
        }

        PDFReal s1 = 0.0;
        PDFReal s2 = 0.0;

        if (qFuzzyIsNull(a))
        {
            // We have equation b.s + c = 0
            if (qFuzzyIsNull(b))
            {
                return false;
            }

            const PDFReal solution = -c / b;
            s1 = solution;
            s2 = solution;
        }
        else
        {
            const PDFReal D = std::sqrt(Dsqr);
            s1 = (-b - D) / (2.0 * a);
            s2 = (-b + D) / (2.0 * a);
        }
        PDFReal s = 0.0;

        while (true)
        {
            const PDFReal radius2 = r_0 + s2 * r_1_0;
            if (radius2 >= 0.0)
            {
                if (m_radialShadingPattern->isExtendStart())
                {
                    s2 = qMax(s2, 0.0);
                }
                if (m_radialShadingPattern->isExtendEnd())
                {
                    s2 = qMin(s2, 1.0);
                }

                if (s2 >= 0.0 && s2 <= 1.0)
                {
                    s = s2;
                    break;
                }
            }

            const PDFReal radius1 = r_0 + s1 * r_1_0;
            if (radius1 >= 0.0)
            {
                if (m_radialShadingPattern->isExtendStart())
                {
                    s1 = qMax(s1, 0.0);
                }
                if (m_radialShadingPattern->isExtendEnd())
                {
                    s1 = qMin(s1, 1.0);
                }

                if (s1 >= 0.0 && s1 <= 1.0)
                {
                    s = s1;
                    break;
                }
            }

            return false;
        }

        PDFReal t = interpolate(s, 0.0, 1.0, m_tAtStart, m_tAtEnd);
        t = qBound(m_tMin, t, m_tMax);

        const auto& functions = m_radialShadingPattern->getFunctions();
        std::array<PDFReal, PDF_MAX_COLOR_COMPONENTS> colorBuffer = { };

        if (colorBuffer.size() < outputBuffer.size())
        {
            // Jakub Melka: Too much colors - we cant process it
            return false;
        }

        if (functions.size() == 1)
        {
            Q_ASSERT(outputBuffer.size() <= colorBuffer.size());
            PDFFunction::FunctionResult result = functions.front()->apply(&t, &t + 1, colorBuffer.data(), colorBuffer.data() + outputBuffer.size());

            if (!result)
            {
                // Function call failed
                return false;
            }
        }
        else
        {
            if (functions.size() != outputBuffer.size())
            {
                // Invalid number of functions
                return false;
            }

            Q_ASSERT(outputBuffer.size() <= colorBuffer.size());
            for (size_t i = 0, count = outputBuffer.size(); i < count; ++i)
            {
                PDFFunction::FunctionResult result = functions[i]->apply(&t, &t + 1, colorBuffer.data() + i, colorBuffer.data() + i + 1);

                if (!result)
                {
                    // Function call failed
                    return false;
                }
            }
        }

        for (size_t i = 0, count = outputBuffer.size(); i < count; ++i)
        {
            outputBuffer[i] = colorBuffer[i];
        }

        return true;
    }

private:
    const PDFRadialShading* m_radialShadingPattern;
    QTransform m_p1p2GCS;
    PDFReal m_xStart;
    PDFReal m_xEnd;
    PDFReal m_tAtStart;
    PDFReal m_tAtEnd;
    PDFReal m_tMin;
    PDFReal m_tMax;
    PDFReal m_r0;
    PDFReal m_r1;
};

PDFShadingSampler* PDFRadialShading::createSampler(QTransform userSpaceToDeviceSpaceMatrix) const
{
    return new PDFRadialShadingSampler(this, userSpaceToDeviceSpaceMatrix);
}

class PDFTriangleShadingSampler : public PDFShadingSampler
{
private:
    struct Triangle
    {
        std::array<uint32_t, 3> vertexIndices = { };
        std::array<PDFColor, 3> vertexColors;
        QTransform barycentricCoordinateMatrix;
    };

public:
    PDFTriangleShadingSampler(const PDFType4567Shading* shadingPattern, QTransform userSpaceToDeviceSpaceMatrix) :
        PDFShadingSampler(shadingPattern),
        m_type4567ShadingPattern(shadingPattern)
    {
        Q_UNUSED(userSpaceToDeviceSpaceMatrix);
    }

    virtual bool sample(const QPointF& devicePoint, PDFColorBuffer outputBuffer, int limit) const override
    {
        Q_UNUSED(limit);

        for (const Triangle& triangle : m_triangles)
        {
            // Calculate barycentric coordinates
            QPointF b1b2 = triangle.barycentricCoordinateMatrix.map(devicePoint);

            const qreal b1 = b1b2.x();
            const qreal b2 = b1b2.y();
            const qreal b3 = 1.0 - b1 - b2;

            if (b1 >= 0.0 && b2 >= 0.0 && b3 >= 0.0 && qFuzzyCompare(b1 + b2 + b3, 1.0))
            {
                // Jakub Melka: we got hit, we are in the triangle. Using the barycentric
                // coordinates, we can calculate result color.

                const PDFColor& c1 = triangle.vertexColors[0];
                const PDFColor& c2 = triangle.vertexColors[1];
                const PDFColor& c3 = triangle.vertexColors[2];

                Q_ASSERT(c1.size() == c2.size());
                Q_ASSERT(c2.size() == c3.size());

                const size_t inputColorSize = c1.size();
                PDFColor interpolatedColor;
                interpolatedColor.resize(inputColorSize);
                for (size_t i = 0; i < inputColorSize; ++i)
                {
                    interpolatedColor[i] = c1[i] * b1 + c2[i] * b2 + c3[i] * b3;
                }

                interpolatedColor = m_type4567ShadingPattern->getColor(interpolatedColor);

                if (interpolatedColor.size() != outputBuffer.size())
                {
                    return false;
                }

                for (size_t i = 0; i < outputBuffer.size(); ++i)
                {
                    outputBuffer[i] = interpolatedColor[i];
                }

                return true;
            }
        }

        return false;
    }

    void addTriangle(std::array<uint32_t, 3> vertexIndices, std::array<PDFColor, 3> vertexColors)
    {
        Triangle triangle;
        triangle.vertexIndices = qMove(vertexIndices);
        triangle.vertexColors = qMove(vertexColors);

        // Compute barycentric coordinate matrix, which will tranform cartesian coordinates of given
        // point in the plane into the barycentric coordinates in the triangle. Barycentric coordinate system
        // is three point coordinates (b1, b2, b3), where b1,b2,b3 >= 0 and b1 + b2 + b3 = 1.0, such that
        //
        // (x, y) = b1 * p1 + b2 * p2 + b3 * p3, where
        // triangle consists of vertices p1, p2, p3 and (x, y) is point inside triangle. If requirements
        // of b1, b2, b3 are not met, then point doesn't lie in the triangle.
        //
        // We will use following transformation from caresian plane to barycentric coordinate system:
        // Usign equation b1 + b2 + b3 = 1.0 we get b3 = 1.0 - b1 - b2, so we will get following system
        // of equations:
        //
        // x = b1 * x1 + b2 * x2 + (1.0 - b1 - b2) * x3
        // y = b1 * y1 + b2 * y2 + (1.0 - b1 - b2) * y3
        //
        // b1 * (x1 - x3) + b2 * (x2 - x3) = x - x3
        // b1 * (y1 - y3) + b2 * (y2 - y3) = y - y3
        //
        // Now, we have system of two linear equation of two variables (b1, b2) and b3 can be computed
        // easily from equation b1 + b2 + b3 = 1.0. Now, we will introduce matrix B:
        //
        // B = ( x1 - x3, x2 - x3)
        //     ( y1 - y3, y2 - y3)
        //
        // And we will have final equation:
        //
        // (b1, b2) = B^-1 * (p - p3)
        //

        QPointF p1 = m_vertices[triangle.vertexIndices[0]];
        QPointF p2 = m_vertices[triangle.vertexIndices[1]];
        QPointF p3 = m_vertices[triangle.vertexIndices[2]];

        QPointF p1p3 = p1 - p3;
        QPointF p2p3 = p2 - p3;

        QTransform B(p1p3.x(), p1p3.y(), p2p3.x(), p2p3.y(), 0.0, 0.0);

        if (!B.isInvertible())
        {
            // Jakub Melka: B is is not invertible, triangle is degenerated
            return;
        }

        // We precalculate B^-1 * (-p3), so we do not have it to compute it
        // in each iteration.
        QTransform Binv = B.inverted();
        QPointF pt = Binv.map(-p3);
        Binv = QTransform(Binv.m11(), Binv.m12(), Binv.m21(), Binv.m22(), pt.x(), pt.y());

        triangle.barycentricCoordinateMatrix = Binv;
        m_triangles.emplace_back(qMove(triangle));
    }

    void setVertexArray(std::vector<QPointF>&& vertices) { m_vertices = qMove(vertices); }
    void reserveSpaceForTriangles(size_t triangleCount) { m_triangles.reserve(triangleCount); }

private:
    const PDFType4567Shading* m_type4567ShadingPattern;
    std::vector<QPointF> m_vertices;
    std::vector<Triangle> m_triangles;
};

ShadingType PDFFreeFormGouradTriangleShading::getShadingType() const
{
    return ShadingType::FreeFormGouradTriangle;
}

bool PDFFreeFormGouradTriangleShading::processTriangles(InitializeFunction initializeMeshFunction,
                                                        AddTriangleFunction addTriangle,
                                                        const QTransform& userSpaceToDeviceSpaceMatrix,
                                                        bool convertColors) const
{
    QTransform patternSpaceToDeviceSpaceMatrix = getPatternSpaceToDeviceSpaceMatrix(userSpaceToDeviceSpaceMatrix);
    size_t bitsPerVertex = m_bitsPerFlag + 2 * m_bitsPerCoordinate + m_colorComponentCount * m_bitsPerComponent;
    size_t remainder = (8 - (bitsPerVertex % 8)) % 8;
    bitsPerVertex += remainder;
    size_t bytesPerVertex = bitsPerVertex / 8;
    size_t vertexCount = m_data.size() / bytesPerVertex;

    if (vertexCount < 3)
    {
        // No mesh produced
        return true;
    }

    // We have 3 vertices for start triangle, then for each new vertex, we get
    // a new triangle, or, based on flags, no triangle (if new triangle is processed)
    size_t triangleCount = vertexCount - 2;

    const PDFReal vertexScaleRatio = 1.0 / double((static_cast<uint64_t>(1) << m_bitsPerCoordinate) - 1);
    const PDFReal xScaleRatio = (m_xmax - m_xmin) * vertexScaleRatio;
    const PDFReal yScaleRatio = (m_ymax - m_ymin) * vertexScaleRatio;
    const PDFReal colorScaleRatio = 1.0 / double((static_cast<uint64_t>(1) << m_bitsPerComponent) - 1);

    std::vector<VertexData> vertices;
    vertices.resize(vertexCount);
    std::vector<QPointF> meshVertices;
    meshVertices.resize(vertexCount);

    auto readVertex = [this, &vertices, &patternSpaceToDeviceSpaceMatrix, &meshVertices, bytesPerVertex, xScaleRatio, yScaleRatio, colorScaleRatio, convertColors](size_t index)
    {
        PDFBitReader reader(&m_data, 8);
        reader.seek(index * bytesPerVertex);

        VertexData data;
        data.index = static_cast<uint32_t>(index);
        data.flags = reader.read(m_bitsPerFlag);
        const PDFReal x = m_xmin + (reader.read(m_bitsPerCoordinate)) * xScaleRatio;
        const PDFReal y = m_ymin + (reader.read(m_bitsPerCoordinate)) * yScaleRatio;
        data.position = patternSpaceToDeviceSpaceMatrix.map(QPointF(x, y));
        data.color.resize(m_colorComponentCount);
        meshVertices[index] = data.position;

        for (size_t i = 0; i < m_colorComponentCount; ++i)
        {
            const double cMin = m_limits[2 * i + 0];
            const double cMax = m_limits[2 * i + 1];
            data.color[i] = cMin + (reader.read(m_bitsPerComponent)) * (cMax - cMin) * colorScaleRatio;
        }

        if (convertColors)
        {
            data.color = getColor(data.color);
        }

        vertices[index] = qMove(data);
    };

    PDFIntegerRange indices(size_t(0), vertexCount);
    PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Content, indices.begin(), indices.end(), readVertex);
    initializeMeshFunction(qMove(meshVertices), triangleCount);

    vertices.front().flags = 0;

    const VertexData* va = nullptr;
    const VertexData* vb = nullptr;
    const VertexData* vc = nullptr;
    const VertexData* vd = nullptr;

    for (size_t i = 0; i < vertexCount;)
    {
        vd = &vertices[i];

        switch (vd->flags)
        {
            case 0:
            {
                if (i + 2 >= vertexCount)
                {
                    return false;
                }
                va = vd;
                vb = &vertices[i + 1];
                vc = &vertices[i + 2];
                i += 3;
                addTriangle(va, vb, vc);
                break;
            }

            case 1:
            {
                // Triangle vb, vc, vd
                va = vb;
                vb = vc;
                vc = vd;
                ++i;
                addTriangle(va, vb, vc);
                break;
            }

            case 2:
            {
                // Triangle va, vc, vd
                vb = vc;
                vc = vd;
                ++i;
                addTriangle(va, vb, vc);
                break;
            }

            default:
                return false;
        }
    }

    return true;
}

PDFMesh PDFFreeFormGouradTriangleShading::createMesh(const PDFMeshQualitySettings& settings,
                                                     const PDFCMS* cms,
                                                     RenderingIntent intent,
                                                     PDFRenderErrorReporter* reporter,
                                                     const PDFOperationControl* operationControl) const
{
    PDFMesh mesh;

    Q_UNUSED(operationControl);

    auto addTriangle = [this, &settings, &mesh, cms, intent, reporter](const VertexData* va, const VertexData* vb, const VertexData* vc)
    {
        const uint32_t via = va->index;
        const uint32_t vib = vb->index;
        const uint32_t vic = vc->index;

        addSubdividedTriangles(settings, mesh, via, vib, vic, va->color, vb->color, vc->color, cms, intent, reporter);
    };

    auto initializeMeshFunction = [&mesh](std::vector<QPointF>&& vertices, size_t triangleCount)
    {
        mesh.reserve(0, triangleCount);
        mesh.setVertices(qMove(vertices));
    };

    if (!processTriangles(initializeMeshFunction, addTriangle, settings.userSpaceToDeviceSpaceMatrix, true))
    {
        throw PDFRendererException(RenderErrorType::Error, PDFTranslationContext::tr("Invalid free form gourad triangle data stream."));
    }

    if (m_backgroundColor.isValid())
    {
        QPainterPath path;
        path.addRect(settings.deviceSpaceMeshingArea);
        mesh.setBackgroundPath(path);
        mesh.setBackgroundColor(m_backgroundColor);
    }

    return mesh;
}

PDFShadingSampler* PDFFreeFormGouradTriangleShading::createSampler(QTransform userSpaceToDeviceSpaceMatrix) const
{
    PDFTriangleShadingSampler* sampler = new PDFTriangleShadingSampler(this, userSpaceToDeviceSpaceMatrix);

    auto addTriangle = [sampler](const VertexData* va, const VertexData* vb, const VertexData* vc)
    {
        const uint32_t via = va->index;
        const uint32_t vib = vb->index;
        const uint32_t vic = vc->index;

        sampler->addTriangle({ via, vib, vic }, { va->color, vb->color, vc->color });
    };

    auto initializeMeshFunction = [sampler](std::vector<QPointF>&& vertices, size_t triangleCount)
    {
        sampler->setVertexArray(qMove(vertices));
        sampler->reserveSpaceForTriangles(triangleCount);
    };

    if (!processTriangles(initializeMeshFunction, addTriangle, userSpaceToDeviceSpaceMatrix, false))
    {
        // Just delete the sampler, data are invalid
        delete sampler;
        sampler = nullptr;
    }

    return sampler;
}

ShadingType PDFLatticeFormGouradTriangleShading::getShadingType() const
{
    return ShadingType::LatticeFormGouradTriangle;
}

bool PDFLatticeFormGouradTriangleShading::processTriangles(InitializeFunction initializeMeshFunction,
                                                           AddTriangleFunction addTriangle,
                                                           const QTransform& userSpaceToDeviceSpaceMatrix,
                                                           bool convertColors) const
{
    QTransform patternSpaceToDeviceSpaceMatrix = getPatternSpaceToDeviceSpaceMatrix(userSpaceToDeviceSpaceMatrix);
    size_t bitsPerVertex = 2 * m_bitsPerCoordinate + m_colorComponentCount * m_bitsPerComponent;
    size_t remainder = (8 - (bitsPerVertex % 8)) % 8;
    bitsPerVertex += remainder;
    size_t bytesPerVertex = bitsPerVertex / 8;
    size_t vertexCount = m_data.size() / bytesPerVertex;
    size_t columnCount = static_cast<size_t>(m_verticesPerRow);
    size_t rowCount = vertexCount / columnCount;

    if (rowCount < 2)
    {
        // No mesh produced
        return false;
    }

    // We have 2 triangles for each quad. We have (columnCount - 1) quads
    // in single line and we have (rowCount - 1) lines.
    size_t triangleCount = (rowCount - 1) * (columnCount - 1) * 2;

    const PDFReal vertexScaleRatio = 1.0 / double((static_cast<uint64_t>(1) << m_bitsPerCoordinate) - 1);
    const PDFReal xScaleRatio = (m_xmax - m_xmin) * vertexScaleRatio;
    const PDFReal yScaleRatio = (m_ymax - m_ymin) * vertexScaleRatio;
    const PDFReal colorScaleRatio = 1.0 / double((static_cast<uint64_t>(1) << m_bitsPerComponent) - 1);

    std::vector<VertexData> vertices;
    vertices.resize(vertexCount);
    std::vector<QPointF> meshVertices;
    meshVertices.resize(vertexCount);

    auto readVertex = [this, &vertices, &patternSpaceToDeviceSpaceMatrix, &meshVertices, bytesPerVertex, xScaleRatio, yScaleRatio, colorScaleRatio, convertColors](size_t index)
    {
        PDFBitReader reader(&m_data, 8);
        reader.seek(index * bytesPerVertex);

        VertexData data;
        data.index = static_cast<uint32_t>(index);
        const PDFReal x = m_xmin + (reader.read(m_bitsPerCoordinate)) * xScaleRatio;
        const PDFReal y = m_ymin + (reader.read(m_bitsPerCoordinate)) * yScaleRatio;
        data.position = patternSpaceToDeviceSpaceMatrix.map(QPointF(x, y));
        data.color.resize(m_colorComponentCount);
        meshVertices[index] = data.position;

        for (size_t i = 0; i < m_colorComponentCount; ++i)
        {
            const double cMin = m_limits[2 * i + 0];
            const double cMax = m_limits[2 * i + 1];
            data.color[i] = cMin + (reader.read(m_bitsPerComponent)) * (cMax - cMin) * colorScaleRatio;
        }

        if (convertColors)
        {
            data.color = getColor(data.color);
        }

        vertices[index] = qMove(data);
    };

    PDFIntegerRange indices(size_t(0), vertexCount);
    PDFExecutionPolicy::execute(PDFExecutionPolicy::Scope::Content, indices.begin(), indices.end(), readVertex);
    initializeMeshFunction(qMove(meshVertices), triangleCount);

    auto getVertexIndex = [columnCount](size_t row, size_t column) -> size_t
    {
        return row * columnCount + column;
    };

    for (size_t row = 1; row < rowCount; ++row)
    {
        for (size_t column = 1; column < columnCount; ++column)
        {
            const size_t vTopLeft = getVertexIndex(row - 1, column - 1);
            const size_t vTopRight = getVertexIndex(row - 1, column);
            const size_t vBottomRight = getVertexIndex(row, column);
            const size_t vBottomLeft = getVertexIndex(row, column - 1);

            const VertexData& vertexTopLeft = vertices[vTopLeft];
            const VertexData& vertexTopRight = vertices[vTopRight];
            const VertexData& vertexBottomRight = vertices[vBottomRight];
            const VertexData& vertexBottomLeft = vertices[vBottomLeft];

            addTriangle(&vertexTopLeft, &vertexTopRight, &vertexBottomRight);
            addTriangle(&vertexBottomRight, &vertexBottomLeft, &vertexTopLeft);
        }
    }

    return true;
}

PDFMesh PDFLatticeFormGouradTriangleShading::createMesh(const PDFMeshQualitySettings& settings,
                                                        const PDFCMS* cms,
                                                        RenderingIntent intent,
                                                        PDFRenderErrorReporter* reporter,
                                                        const PDFOperationControl* operationControl) const
{
    PDFMesh mesh;

    Q_UNUSED(operationControl);

    auto addTriangle = [this, &settings, &mesh, cms, intent, reporter](const VertexData* va, const VertexData* vb, const VertexData* vc)
    {
        const uint32_t via = va->index;
        const uint32_t vib = vb->index;
        const uint32_t vic = vc->index;

        addSubdividedTriangles(settings, mesh, via, vib, vic, va->color, vb->color, vc->color, cms, intent, reporter);
    };

    auto initializeMeshFunction = [&mesh](std::vector<QPointF>&& vertices, size_t triangleCount)
    {
        mesh.reserve(0, triangleCount);
        mesh.setVertices(qMove(vertices));
    };

    if (!processTriangles(initializeMeshFunction, addTriangle, settings.userSpaceToDeviceSpaceMatrix, true))
    {
        throw PDFRendererException(RenderErrorType::Error, PDFTranslationContext::tr("Invalid lattice form gourad triangle data stream."));
    }

    if (m_backgroundColor.isValid())
    {
        QPainterPath path;
        path.addRect(settings.deviceSpaceMeshingArea);
        mesh.setBackgroundPath(path);
        mesh.setBackgroundColor(m_backgroundColor);
    }

    return mesh;
}

PDFShadingSampler* PDFLatticeFormGouradTriangleShading::createSampler(QTransform userSpaceToDeviceSpaceMatrix) const
{
    PDFTriangleShadingSampler* sampler = new PDFTriangleShadingSampler(this, userSpaceToDeviceSpaceMatrix);

    auto addTriangle = [sampler](const VertexData* va, const VertexData* vb, const VertexData* vc)
    {
        const uint32_t via = va->index;
        const uint32_t vib = vb->index;
        const uint32_t vic = vc->index;

        sampler->addTriangle({ via, vib, vic }, { va->color, vb->color, vc->color });
    };

    auto initializeMeshFunction = [sampler](std::vector<QPointF>&& vertices, size_t triangleCount)
    {
        sampler->setVertexArray(qMove(vertices));
        sampler->reserveSpaceForTriangles(triangleCount);
    };

    if (!processTriangles(initializeMeshFunction, addTriangle, userSpaceToDeviceSpaceMatrix, false))
    {
        // Just delete the sampler, data are invalid
        delete sampler;
        sampler = nullptr;
    }

    return sampler;
}

PDFColor PDFType4567Shading::getColor(PDFColor colorOrFunctionParameter) const
{
    if (!m_functions.empty())
    {
        const PDFReal t = colorOrFunctionParameter[0];

        Q_ASSERT(m_colorComponentCount == 1);
        std::vector<PDFReal> colorBuffer(m_colorSpace->getColorComponentCount(), 0.0);
        if (m_functions.size() == 1)
        {
            PDFFunction::FunctionResult result = m_functions.front()->apply(&t, &t + 1, colorBuffer.data(), colorBuffer.data() + colorBuffer.size());
            if (!result)
            {
                throw PDFRendererException(RenderErrorType::Error, PDFTranslationContext::tr("Error occured during mesh creation of shading: %1").arg(result.errorMessage));
            }
        }
        else
        {
            for (size_t i = 0, count = colorBuffer.size(); i < count; ++i)
            {
                PDFFunction::FunctionResult result = m_functions[i]->apply(&t, &t + 1, colorBuffer.data() + i, colorBuffer.data() + i + 1);
                if (!result)
                {
                    throw PDFRendererException(RenderErrorType::Error, PDFTranslationContext::tr("Error occured during mesh creation of shading: %1").arg(result.errorMessage));
                }
            }
        }

        return PDFAbstractColorSpace::convertToColor(colorBuffer);
    }

    return colorOrFunctionParameter;
}

void PDFType4567Shading::addSubdividedTriangles(const PDFMeshQualitySettings& settings,
                                                PDFMesh& mesh, uint32_t v1, uint32_t v2, uint32_t v3,
                                                PDFColor c1, PDFColor c2, PDFColor c3,
                                                const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter) const
{
    // First, verify, if we can subdivide the triangle
    QLineF v12(mesh.getVertex(v1), mesh.getVertex(v2));
    QLineF v13(mesh.getVertex(v1), mesh.getVertex(v3));
    QLineF v23(mesh.getVertex(v2), mesh.getVertex(v3));

    const qreal length12 = v12.length();
    const qreal length13 = v13.length();
    const qreal length23 = v23.length();
    const qreal maxLength = qMax(length12, qMax(length13, length23));

    const bool isColorEqual = PDFAbstractColorSpace::isColorEqual(c1, c2, settings.tolerance) &&
                              PDFAbstractColorSpace::isColorEqual(c1, c3, settings.tolerance) &&
                              PDFAbstractColorSpace::isColorEqual(c2, c3, settings.tolerance);
    const bool canSubdivide = maxLength >= settings.minimalMeshResolution * 2.0; // If we subdivide, we will have length at least settings.minimalMeshResolution


    if (!isColorEqual && canSubdivide)
    {
        if (length23 == maxLength)
        {
            // We split line (v2, v3), create two triangles, (v1, v2, vx) and (v1, v3, vx), where
            // vx is centerpoint of line (v2, v3). We also interpolate colors.
            QPointF x = v23.center();
            PDFColor cx = PDFAbstractColorSpace::mixColors(c2, c3, 0.5);
            const uint32_t vx = mesh.addVertex(x);

            addSubdividedTriangles(settings, mesh, v1, v2, vx, c1, c2, cx, cms, intent, reporter);
            addSubdividedTriangles(settings, mesh, v1, v3, vx, c1, c3, cx, cms, intent, reporter);
        }
        else if (length13 == maxLength)
        {
            // We split line (v1, v3), create two triangles, (v1, v2, vx) and (v2, v3, vx), where
            // vx is centerpoint of line (v1, v3). We also interpolate colors.
            QPointF x = v13.center();
            PDFColor cx = PDFAbstractColorSpace::mixColors(c1, c3, 0.5);
            const uint32_t vx = mesh.addVertex(x);

            addSubdividedTriangles(settings, mesh, v1, v2, vx, c1, c2, cx, cms, intent, reporter);
            addSubdividedTriangles(settings, mesh, v2, v3, vx, c2, c3, cx, cms, intent, reporter);
        }
        else
        {
            Q_ASSERT(length12 == maxLength);

            // We split line (v1, v2), create two triangles, (v1, v3, vx) and (v2, v3, vx), where
            // vx is centerpoint of line (v1, v2). We also interpolate colors.
            QPointF x = v12.center();
            PDFColor cx = PDFAbstractColorSpace::mixColors(c1, c2, 0.5);
            const uint32_t vx = mesh.addVertex(x);

            addSubdividedTriangles(settings, mesh, v1, v3, vx, c1, c3, cx, cms, intent, reporter);
            addSubdividedTriangles(settings, mesh, v2, v3, vx, c2, c3, cx, cms, intent, reporter);
        }
    }
    else
    {
        const size_t colorComponents = c1.size();

        // Calculate color - interpolate 3 vertex colors
        PDFColor color;
        color.resize(colorComponents);

        constexpr const PDFReal coefficient = 1.0 / 3.0;
        for (size_t i = 0; i < colorComponents; ++i)
        {
            color[i] = (c1[i] + c2[i] + c3[i]) * coefficient;
        }
        Q_ASSERT(colorComponents == m_colorSpace->getColorComponentCount());
        QColor transformedColor = m_colorSpace->getColor(color, cms, intent, reporter, true);

        PDFMesh::Triangle triangle;
        triangle.v1 = v1;
        triangle.v2 = v2;
        triangle.v3 = v3;
        triangle.color = transformedColor.rgb();
        mesh.addTriangle(triangle);
    }
}

QPointF PDFTensorPatch::getValue(PDFReal u, PDFReal v) const
{
    return getValue(u, v, 0, 0);
}

QPointF PDFTensorPatch::getValue(PDFReal u, PDFReal v, int derivativeOrderU, int derivativeOrderV) const
{
    QPointF result(0.0, 0.0);

    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            result += m_P[i][j] * B(i, u, derivativeOrderU) * B(j, v, derivativeOrderV);
        }
    }

    return result;
}

bool PDFTensorPatch::getUV(PDFReal& u, PDFReal& v, PDFReal x, PDFReal y, PDFReal epsilon, int maximalNumberOfSteps) const
{
    // First we will text, if point (x, y) is in bounding rectangle of the patch.
    // If it isn't, then return false immediately, because point is not in tensor patch.
    if (!m_boundingBox.contains(x, y))
    {
        return false;
    }

    int i = 0;

    // Jakub Melka: We are finding root of function F(u, v) defined as:
    //
    //            F(u, v) = getValue(u, v) - (x, y)
    //
    // And using Newton-Raphson method to find the root
    //   v_n+1 = v_n - J^-1(v_n) * F(v_n)
    //
    // Where J^-1 is inverse of Jacobi matrix of the function F(u, v), defined as:
    //   dF1/du   dF1/dv
    //   dF2/du   dF2/dv

    QPointF v_n(u, v);
    QPointF p_xy(x, y);

    while (i++ < maximalNumberOfSteps)
    {
        // Evaluate function at pivot
        QPointF value_F_v_n = getValue(v_n.x(), v_n.y(), 0, 0) - p_xy;

        // Do we actually converge?
        if (qAbs(value_F_v_n.x()) < epsilon && qAbs(value_F_v_n.y()) < epsilon)
        {
            u = v_n.x();
            v = v_n.y();

            const bool uValid = u >= 0.0 && u <= 1.0;
            const bool vValid = v >= 0.0 && v <= 1.0;

            return uValid && vValid;
        }

        // Evaluate Jacobi matrix
        QPointF dfdu = getValue(v_n.x(), v_n.y(), 1, 0);
        QPointF dfdv = getValue(v_n.x(), v_n.y(), 0, 1);

        const PDFReal m11 = dfdu.x();
        const PDFReal m12 = dfdv.x();
        const PDFReal m21 = dfdu.y();
        const PDFReal m22 = dfdv.y();

        // Create inverse of Jacobi matrix
        const PDFReal determinant = m11 * m22 - m12 * m21;

        if (qFuzzyIsNull(determinant))
        {
            // We did not converge, unfortunately, we are probably,
            // in a stationary point.
            return false;
        }

        const PDFReal inverseDeterminant = 1.0 / determinant;
        const PDFReal im11 = m22 * inverseDeterminant;
        const PDFReal im12 = -m12 * inverseDeterminant;
        const PDFReal im21 = -m21 * inverseDeterminant;
        const PDFReal im22 = m11 * inverseDeterminant;

        QPointF imFirstRow(im11, im12);
        QPointF imSecondRow(im21, im22);
        QPointF delta(QPointF::dotProduct(imFirstRow, value_F_v_n), QPointF::dotProduct(imSecondRow, value_F_v_n));

        v_n = v_n - delta;
    }

    return false;
}

PDFReal PDFTensorPatch::getCurvature_u(PDFReal u, PDFReal v) const
{
    QPointF dSdu = getDerivative_u(u, v);
    QPointF dSduu = getDerivative_uu(u, v);

    PDFReal squaredLengthOfdSdu = QPointF::dotProduct(dSdu, dSdu);

    if (qFuzzyIsNull(squaredLengthOfdSdu))
    {
        // We assume, that curvature, due to zero length of the tangent vector, is also zero
        return 0.0;
    }

    // Well known formula, how to compute curvature of curve f(x):
    //  K = ( df/dx * df/dyy - df/dxx * df/dy ) / ( (df/dx)^2 + (df/dy)^2 ) ^ (3/2)
    PDFReal curvature = std::fabs(dSdu.x() * dSduu.y() - dSdu.y() * dSduu.x()) / std::pow(squaredLengthOfdSdu, 1.5);
    return curvature;
}

PDFReal PDFTensorPatch::getCurvature_v(PDFReal u, PDFReal v) const
{
    QPointF dSdv = getDerivative_v(u, v);
    QPointF dSdvv = getDerivative_vv(u, v);

    PDFReal squaredLengthOfdSdv = QPointF::dotProduct(dSdv, dSdv);

    if (qFuzzyIsNull(squaredLengthOfdSdv))
    {
        // We assume, that curvature, due to zero length of the tangent vector, is also zero
        return 0.0;
    }

    // Well known formula, how to compute curvature of curve f(x):
    //  K = ( df/dx * df/dyy - df/dxx * df/dy ) / ( (df/dx)^2 + (df/dy)^2 ) ^ (3/2)
    PDFReal curvature = std::fabs(dSdv.x() * dSdvv.y() - dSdv.y() * dSdvv.x()) / std::pow(squaredLengthOfdSdv, 1.5);
    return curvature;
}

constexpr PDFReal PDFTensorPatch::B(int index, PDFReal t, int derivativeOrder)
{
    switch (index)
    {
        case 0:
            return B0(t, derivativeOrder);

        case 1:
            return B1(t, derivativeOrder);

        case 2:
            return B2(t, derivativeOrder);

        case 3:
            return B3(t, derivativeOrder);

        default:
            break;
    }

    return std::numeric_limits<PDFReal>::signaling_NaN();
}

constexpr PDFReal PDFTensorPatch::B0(PDFReal t, int derivative)
{
    switch (derivative)
    {
        case 0:
            return pow3(1.0 - t);

        case 1:
            return -3.0 * pow2(1.0 - t);

        case 2:
            return 6.0 * (1.0 - t);

        case 3:
            return -6.0;

        default:
            break;
    }

    return std::numeric_limits<PDFReal>::signaling_NaN();
}

constexpr PDFReal PDFTensorPatch::B1(PDFReal t, int derivative)
{
    switch (derivative)
    {
        case 0:
            return 3.0 * t * pow2(1.0 - t);

        case 1:
            return 9.0 * pow2(t) - 12.0 * t + 3.0;

        case 2:
            return 18.0 * t - 12.0;

        case 3:
            return 18.0;
    }

    return std::numeric_limits<PDFReal>::signaling_NaN();
}

constexpr PDFReal PDFTensorPatch::B2(PDFReal t, int derivative)
{
    switch (derivative)
    {
        case 0:
            return 3.0 * pow2(t) * (1.0 - t);

        case 1:
            return -9.0 * pow2(t) + 6.0 * t;

        case 2:
            return -18.0 * t + 6.0;

        case 3:
            return -18.0;
    }

    return std::numeric_limits<PDFReal>::signaling_NaN();
}

constexpr PDFReal PDFTensorPatch::B3(PDFReal t, int derivative)
{
    switch (derivative)
    {
        case 0:
            return pow3(t);

        case 1:
            return 3.0 * pow2(t);

        case 2:
            return 6.0 * t;

        case 3:
            return 6.0;
    }

    return std::numeric_limits<PDFReal>::signaling_NaN();
}

void PDFTensorPatch::computeBoundingRectangle()
{
    PDFReal xMin = std::numeric_limits<PDFReal>::infinity();
    PDFReal xMax = -xMin;
    PDFReal yMin = xMin;
    PDFReal yMax = xMax;

    for (const auto& row : m_P)
    {
        for (const auto& point : row)
        {
            xMin = qMin(xMin, point.x());
            xMax = qMax(xMax, point.x());
            yMin = qMin(yMin, point.y());
            yMax = qMax(yMax, point.y());
        }
    }

    m_boundingBox = QRectF(xMin, yMin, xMax - xMin, yMax - yMin);
}

ShadingType PDFTensorProductPatchShading::getShadingType() const
{
    return ShadingType::TensorProductPatchMesh;
}

PDFTensorPatches PDFTensorProductPatchShading::createPatches(QTransform userSpaceToDeviceSpaceMatrix, bool transformColor) const
{
    QTransform patternSpaceToDeviceSpaceMatrix = getPatternSpaceToDeviceSpaceMatrix(userSpaceToDeviceSpaceMatrix);

    size_t bitsPerPatch = m_bitsPerFlag + 16 * 2 * m_bitsPerCoordinate + 4 * m_colorComponentCount * m_bitsPerComponent;
    size_t remainder = (8 - (bitsPerPatch % 8)) % 8;
    bitsPerPatch += remainder;
    size_t bytesPerPatch = bitsPerPatch / 8;
    size_t patchCountEstimate = m_data.size() / bytesPerPatch;

    const PDFReal vertexScaleRatio = 1.0 / double((static_cast<uint64_t>(1) << m_bitsPerCoordinate) - 1);
    const PDFReal xScaleRatio = (m_xmax - m_xmin) * vertexScaleRatio;
    const PDFReal yScaleRatio = (m_ymax - m_ymin) * vertexScaleRatio;
    const PDFReal colorScaleRatio = 1.0 / double((static_cast<uint64_t>(1) << m_bitsPerComponent) - 1);

    PDFTensorPatches patches;
    patches.reserve(patchCountEstimate);

    PDFBitReader reader(&m_data, 8);

    auto readFlags = [this, &reader]() -> uint8_t
    {
        return reader.read(m_bitsPerFlag);
    };

    auto readPoint = [this, &reader, &patternSpaceToDeviceSpaceMatrix, xScaleRatio, yScaleRatio]() -> QPointF
    {
        const PDFReal x = m_xmin + (reader.read(m_bitsPerCoordinate)) * xScaleRatio;
        const PDFReal y = m_ymin + (reader.read(m_bitsPerCoordinate)) * yScaleRatio;
        return patternSpaceToDeviceSpaceMatrix.map(QPointF(x, y));
    };

    auto readColor = [this, &reader, colorScaleRatio, transformColor]() -> PDFColor
    {
        PDFColor color;
        color.resize(m_colorComponentCount);

        for (size_t i = 0; i < m_colorComponentCount; ++i)
        {
            const double cMin = m_limits[2 * i + 0];
            const double cMax = m_limits[2 * i + 1];
            color[i] = cMin + (reader.read(m_bitsPerComponent)) * (cMax - cMin) * colorScaleRatio;
        }

        return transformColor ? getColor(color) : color;
    };

    while (!reader.isAtEnd())
    {
        const uint8_t flags = readFlags();
        switch (flags)
        {
            case 0:
            {
                PDFTensorPatch::PointMatrix P = { };
                PDFTensorPatch::Colors colors = { };

                P[0][0] = readPoint();
                P[0][1] = readPoint();
                P[0][2] = readPoint();
                P[0][3] = readPoint();
                P[1][3] = readPoint();
                P[2][3] = readPoint();
                P[3][3] = readPoint();
                P[3][2] = readPoint();
                P[3][1] = readPoint();
                P[3][0] = readPoint();
                P[2][0] = readPoint();
                P[1][0] = readPoint();
                P[1][1] = readPoint();
                P[1][2] = readPoint();
                P[2][2] = readPoint();
                P[2][1] = readPoint();

                colors[PDFTensorPatch::C_00] = readColor();
                colors[PDFTensorPatch::C_03] = readColor();
                colors[PDFTensorPatch::C_33] = readColor();
                colors[PDFTensorPatch::C_30] = readColor();

                patches.emplace_back(P, colors);
                break;
            }

            case 1:
            {
                if (patches.empty())
                {
                    throw PDFException(PDFTranslationContext::tr("Nonzero flag for first patch (flags = %1).").arg(flags));
                }

                const PDFTensorPatch& previousPatch = patches.back();
                const PDFTensorPatch::PointMatrix& PPrevious = previousPatch.getP();
                const PDFTensorPatch::Colors& colorsPrevious = previousPatch.getColors();

                PDFTensorPatch::PointMatrix P = { };
                PDFTensorPatch::Colors colors = { };

                P[1][3] = readPoint();
                P[2][3] = readPoint();
                P[3][3] = readPoint();
                P[3][2] = readPoint();
                P[3][1] = readPoint();
                P[3][0] = readPoint();
                P[2][0] = readPoint();
                P[1][0] = readPoint();
                P[1][1] = readPoint();
                P[1][2] = readPoint();
                P[2][2] = readPoint();
                P[2][1] = readPoint();

                colors[PDFTensorPatch::C_33] = readColor();
                colors[PDFTensorPatch::C_30] = readColor();

                // Copy data from previous patch according the PDF specification:
                P[0][0] = PPrevious[0][3];
                P[0][1] = PPrevious[1][3];
                P[0][2] = PPrevious[2][3];
                P[0][3] = PPrevious[3][3];

                colors[PDFTensorPatch::C_00] = colorsPrevious[PDFTensorPatch::C_03];
                colors[PDFTensorPatch::C_03] = colorsPrevious[PDFTensorPatch::C_33];

                patches.emplace_back(P, colors);
                break;
            }

            case 2:
            {
                if (patches.empty())
                {
                    throw PDFException(PDFTranslationContext::tr("Nonzero flag for first patch (flags = %1).").arg(flags));
                }

                const PDFTensorPatch& previousPatch = patches.back();
                const PDFTensorPatch::PointMatrix& PPrevious = previousPatch.getP();
                const PDFTensorPatch::Colors& colorsPrevious = previousPatch.getColors();

                PDFTensorPatch::PointMatrix P = { };
                PDFTensorPatch::Colors colors = { };

                P[1][3] = readPoint();
                P[2][3] = readPoint();
                P[3][3] = readPoint();
                P[3][2] = readPoint();
                P[3][1] = readPoint();
                P[3][0] = readPoint();
                P[2][0] = readPoint();
                P[1][0] = readPoint();
                P[1][1] = readPoint();
                P[1][2] = readPoint();
                P[2][2] = readPoint();
                P[2][1] = readPoint();

                colors[PDFTensorPatch::C_33] = readColor();
                colors[PDFTensorPatch::C_30] = readColor();

                // Copy data from previous patch according the PDF specification:
                P[0][0] = PPrevious[3][3];
                P[0][1] = PPrevious[3][2];
                P[0][2] = PPrevious[3][1];
                P[0][3] = PPrevious[3][0];

                colors[PDFTensorPatch::C_00] = colorsPrevious[PDFTensorPatch::C_33];
                colors[PDFTensorPatch::C_03] = colorsPrevious[PDFTensorPatch::C_30];

                patches.emplace_back(P, colors);
                break;
            }

            case 3:
            {
                if (patches.empty())
                {
                    throw PDFException(PDFTranslationContext::tr("Nonzero flag for first patch (flags = %1).").arg(flags));
                }

                const PDFTensorPatch& previousPatch = patches.back();
                const PDFTensorPatch::PointMatrix& PPrevious = previousPatch.getP();
                const PDFTensorPatch::Colors& colorsPrevious = previousPatch.getColors();

                PDFTensorPatch::PointMatrix P = { };
                PDFTensorPatch::Colors colors = { };

                P[1][3] = readPoint();
                P[2][3] = readPoint();
                P[3][3] = readPoint();
                P[3][2] = readPoint();
                P[3][1] = readPoint();
                P[3][0] = readPoint();
                P[2][0] = readPoint();
                P[1][0] = readPoint();
                P[1][1] = readPoint();
                P[1][2] = readPoint();
                P[2][2] = readPoint();
                P[2][1] = readPoint();

                colors[PDFTensorPatch::C_33] = readColor();
                colors[PDFTensorPatch::C_30] = readColor();

                // Copy data from previous patch according the PDF specification:
                P[0][0] = PPrevious[3][0];
                P[0][1] = PPrevious[2][0];
                P[0][2] = PPrevious[1][0];
                P[0][3] = PPrevious[0][0];

                colors[PDFTensorPatch::C_00] = colorsPrevious[PDFTensorPatch::C_30];
                colors[PDFTensorPatch::C_03] = colorsPrevious[PDFTensorPatch::C_00];

                patches.emplace_back(P, colors);
                break;
            }

            default:
                patches.clear();
                return patches;
        }
    }

    return patches;
}

PDFMesh PDFTensorProductPatchShading::createMesh(const PDFMeshQualitySettings& settings,
                                                 const PDFCMS* cms,
                                                 RenderingIntent intent,
                                                 PDFRenderErrorReporter* reporter,
                                                 const PDFOperationControl* operationControl) const
{
    PDFMesh mesh;

    PDFTensorPatches patches = createPatches(settings.userSpaceToDeviceSpaceMatrix, true);

    if (patches.empty())
    {
        throw PDFException(PDFTranslationContext::tr("Invalid data in tensor product patch shading."));
    }

    fillMesh(mesh, getPatternSpaceToDeviceSpaceMatrix(settings.userSpaceToDeviceSpaceMatrix), settings, patches, cms, intent, reporter, operationControl);
    return mesh;
}

struct PDFTensorProductPatchShadingBase::Triangle
{
    std::array<QPointF, 3> uvCoordinates;
    std::array<QPointF, 3> devicePoints;

    QPointF getCenter() const
    {
        constexpr PDFReal coefficient = 1.0 / 3.0;
        return (uvCoordinates[0] + uvCoordinates[1] + uvCoordinates[2]) * coefficient;
    }

    PDFReal getCurvature(const PDFTensorPatch& patch) const
    {
        QPointF uv = getCenter();
        return patch.getCurvature_u(uv.x(), uv.y()) + patch.getCurvature_v(uv.x(), uv.y());
    }

    void fillTriangleDevicePoints(const PDFTensorPatch& patch)
    {
        Q_ASSERT(uvCoordinates.size() == devicePoints.size());
        for (size_t i = 0; i < uvCoordinates.size(); ++i)
        {
            devicePoints[i] = patch.getValue(uvCoordinates[i].x(), uvCoordinates[i].y());
        }
    }

    PDFReal getArea() const
    {
        const PDFReal x1 = devicePoints[0].x();
        const PDFReal y1 = devicePoints[0].y();
        const PDFReal x2 = devicePoints[1].x();
        const PDFReal y2 = devicePoints[1].y();
        const PDFReal x3 = devicePoints[2].x();
        const PDFReal y3 = devicePoints[2].y();

        // Use shoelace formula to determine the triangle area, see
        // https://en.wikipedia.org/wiki/Shoelace_formula
        return std::fabs(0.5 * (x1 * y2 + x2 * y3 + x3 * y1 - x2 * y1 - x3 * y2 - x1 * y3));
    }
};

class PDFTensorPatchesSample : public PDFShadingSampler
{
public:
    PDFTensorPatchesSample(const PDFTensorProductPatchShadingBase* shadingPattern, QTransform userSpaceToDeviceSpaceMatrix) :
        PDFShadingSampler(shadingPattern),
        m_tensorProductShadingPattern(shadingPattern)
    {
        m_patches = shadingPattern->createPatches(userSpaceToDeviceSpaceMatrix, false);
        std::reverse(m_patches.begin(), m_patches.end());
    }

    virtual bool sample(const QPointF& devicePoint, PDFColorBuffer outputBuffer, int limit) const override
    {
        constexpr PDFReal epsilon = 0.001;
        std::array initialSamples = { QPointF(0.5, 0.5), // Middle of the patch
                                      QPointF(0.0, 0.0), QPointF(1.0, 0.0), QPointF(0.0, 1.0), QPointF(1.0, 1.0), // Four corners
                                      QPointF(0.5, 0.0), QPointF(0.5, 1.0), QPointF(0.0, 0.5), QPointF(1.0, 0.5) }; // Middle point of edges

        for (const PDFTensorPatch& patch : m_patches)
        {
            PDFReal u = -1.0;
            PDFReal v = -1.0;

            for (const QPointF& initialSample : initialSamples)
            {
                PDFReal uSample = initialSample.x();
                PDFReal vSample = initialSample.y();

                if (patch.getUV(uSample, vSample, devicePoint.x(), devicePoint.y(), epsilon, limit))
                {
                    // We have successfully retrieved u,v source for the target x,y point. But is it actually
                    // better than previous sample?
                    if (vSample > v || (qAbs(vSample - v) < epsilon && uSample > u))
                    {
                        u = uSample;
                        v = vSample;
                    }
                }
            }

            if (u >= 0.0 && v >= 0.0)
            {
                const PDFTensorPatch::Colors& colors = patch.getColors();
                const PDFColor& topLeft = colors[PDFTensorPatch::C_00];
                const PDFColor& topRight = colors[PDFTensorPatch::C_30];
                const PDFColor& bottomLeft = colors[PDFTensorPatch::C_03];
                const PDFColor& bottomRight = colors[PDFTensorPatch::C_33];

                PDFColor color;
                color.resize(topLeft.size());

                const size_t colorComponentCount = color.size();
                for (size_t i = 0; i < colorComponentCount; ++i)
                {
                    color[i] = topLeft[i] * (1.0 - u) * (1.0 - v) +
                               topRight[i] * u * (1.0 - v) +
                               bottomLeft[i] * (1.0 - u) * v +
                               bottomRight[i] * u * v;
                }

                PDFColor finalColor = m_tensorProductShadingPattern->getColor(color);
                if (finalColor.size() != outputBuffer.size())
                {
                    return false;
                }

                for (size_t i = 0; i < finalColor.size(); ++i)
                {
                    outputBuffer[i] = finalColor[i];
                }

                return true;
            }
        }

        return false;
    }

private:
    const PDFTensorProductPatchShadingBase* m_tensorProductShadingPattern;
    PDFTensorPatches m_patches;
};

PDFShadingSampler* PDFTensorProductPatchShadingBase::createSampler(QTransform userSpaceToDeviceSpaceMatrix) const
{
    PDFTensorPatches patches = createPatches(userSpaceToDeviceSpaceMatrix, false);

    if (patches.empty())
    {
        return nullptr;
    }

    return new PDFTensorPatchesSample(this, userSpaceToDeviceSpaceMatrix);
}

void PDFTensorProductPatchShadingBase::fillMesh(PDFMesh& mesh,
                                                const PDFMeshQualitySettings& settings,
                                                const PDFTensorPatch& patch,
                                                const PDFCMS* cms,
                                                RenderingIntent intent,
                                                PDFRenderErrorReporter* reporter,
                                                bool fastAlgorithm,
                                                const PDFOperationControl* operationControl) const
{
    // We implement algorithm similar to Ruppert's algorithm (see https://en.wikipedia.org/wiki/Ruppert%27s_algorithm), but
    // we do not need a mesh for FEM calculation, so we do not care about quality of the triangles (we can have triangles with
    // very small angles). We just need to meet these conditions:
    //
    //      1) Mesh is dense enough (to satisfy at least preferred mesh resolution)
    //      2) Mesh is more dense, where it is deformed (high surface curvature)
    //      3) Mesh will also correctly consider color interpolation
    //
    // We will determine maximum surface curvature of the surface (by evaluating test points - this is not reliable, but
    // it will suffice), then start meshing. We must also handle case, when surface maximal curvature is almost zero - then it is
    // probably a rectangle (or something like that). We cannot assume, that directions u,v are principal directions of the surface.
    // So, we will use the sum of curvatures in two perpendicular directions - u,v and we will hope, that it will be OK and will be
    // around mean surface curvature.

    std::atomic<PDFReal> maximalCurvature(0.0);

    if (!fastAlgorithm)
    {
        Q_ASSERT(settings.patchTestPoints > 2);
        const PDFReal testPointScale = 1.0 / (settings.patchTestPoints - 1.0);
        PDFIntegerRange<PDFInteger> range(0, settings.patchTestPoints * settings.patchTestPoints);
        auto updateCurvature = [&](PDFInteger i)
        {
            PDFInteger row = i / settings.patchTestPoints;
            PDFInteger column = i % settings.patchTestPoints;

            const PDFReal u = column * testPointScale;
            const PDFReal v = row * testPointScale;

            const PDFReal curvature = patch.getCurvature_u(u, v) + patch.getCurvature_v(u, v);

            // Atomically update the maximum curvature
            PDFReal previousCurvature = maximalCurvature;
            while (previousCurvature < curvature && !maximalCurvature.compare_exchange_weak(previousCurvature, curvature)) { }
        };
        std::for_each(range.begin(), range.end(), updateCurvature);
    }
    else
    {
        maximalCurvature = std::numeric_limits<PDFReal>::infinity();
    }

    auto getColorForUV = [&](PDFReal u, PDFReal v)
    {
        // Perform bilinear interpolation of colors, u is column, v is row
        const PDFTensorPatch::Colors& colors = patch.getColors();

        const PDFColor& topLeft = colors[PDFTensorPatch::C_00];
        const PDFColor& topRight = colors[PDFTensorPatch::C_30];
        const PDFColor& bottomLeft = colors[PDFTensorPatch::C_03];
        const PDFColor& bottomRight = colors[PDFTensorPatch::C_33];

        PDFColor top = PDFAbstractColorSpace::mixColors(topLeft, topRight, u);
        PDFColor bottom = PDFAbstractColorSpace::mixColors(bottomLeft, bottomRight, u);
        PDFColor interpolated = PDFAbstractColorSpace::mixColors(top, bottom, v);

        return interpolated;
    };

    Triangle workStartA;
    workStartA.uvCoordinates = { QPointF(0.0, 0.0), QPointF(1.0, 0.0), QPointF(0.0, 1.0) };
    workStartA.fillTriangleDevicePoints(patch);
    Triangle workStartB;
    workStartB.uvCoordinates = { QPointF(1.0, 0.0), QPointF(1.0, 1.0), QPointF(0.0, 1.0) };
    workStartB.fillTriangleDevicePoints(patch);

    std::vector<Triangle> unfinishedTriangles = { workStartA, workStartB };
    std::vector<Triangle> finishedTriangles;

    while (!unfinishedTriangles.empty())
    {
        // Mesh generation is cancelled
        if (PDFOperationControl::isOperationCancelled(operationControl))
        {
            mesh = PDFMesh();
            return;
        }

        Triangle triangle = unfinishedTriangles.back();
        unfinishedTriangles.pop_back();

        // Should we divide the triangle? These conditions should be verified:
        //  1) Largest edge of triangle in device space exceeds preferred size of mesh
        //  2) Curvature of the triangle is too high (and largest edge doesn't exceed minimal size of mesh)
        //  3) Color difference is too high (and largest edge doesn't exceed minimal size of mesh)

        // First, verify, if we can subdivide the triangle
        QLineF deviceLine01(triangle.devicePoints[0], triangle.devicePoints[1]);
        QLineF deviceLine02(triangle.devicePoints[0], triangle.devicePoints[2]);
        QLineF deviceLine12(triangle.devicePoints[1], triangle.devicePoints[2]);

        const qreal length01 = deviceLine01.length();
        const qreal length02 = deviceLine02.length();
        const qreal length12 = deviceLine12.length();
        const qreal maxLength = qMax(length01, qMax(length02, length12));

        const PDFReal curvature = triangle.getCurvature(patch);
        const PDFReal curvatureRatio = curvature / maximalCurvature;

        // Calculate target length
        PDFReal targetLength = settings.preferredMeshResolution;
        if (curvatureRatio <= settings.patchResolutionMappingRatioLow)
        {
            Q_ASSERT(targetLength == settings.preferredMeshResolution);
        }
        else if (curvatureRatio >= settings.patchResolutionMappingRatioHigh)
        {
            targetLength = settings.minimalMeshResolution;
        }
        else
        {
            targetLength = interpolate(curvatureRatio, settings.patchResolutionMappingRatioLow, settings.patchResolutionMappingRatioHigh, settings.preferredMeshResolution, settings.minimalMeshResolution);
        }

        const PDFColor c0 = getColorForUV(triangle.uvCoordinates[0].x(), triangle.uvCoordinates[0].y());
        const PDFColor c1 = getColorForUV(triangle.uvCoordinates[1].x(), triangle.uvCoordinates[1].y());
        const PDFColor c2 = getColorForUV(triangle.uvCoordinates[2].x(), triangle.uvCoordinates[2].y());

        const bool isColorEqual = PDFAbstractColorSpace::isColorEqual(c0, c1, settings.tolerance) &&
                                  PDFAbstractColorSpace::isColorEqual(c0, c2, settings.tolerance) &&
                                  PDFAbstractColorSpace::isColorEqual(c1, c2, settings.tolerance);
        const bool canSubdivide = maxLength >= settings.minimalMeshResolution * 2.0; // If we subdivide, we will have length at least settings.minimalMeshResolution
        const bool shouldSubdivide = maxLength >= targetLength;

        if ((!isColorEqual || shouldSubdivide) && canSubdivide)
        {
            QPointF v0 = triangle.uvCoordinates[0];
            QPointF v1 = triangle.uvCoordinates[1];
            QPointF v2 = triangle.uvCoordinates[2];

            QLineF v12uv(v1, v2);
            QLineF v02uv(v0, v2);
            QLineF v01uv(v0, v1);

            QPointF v12 = v12uv.center();
            QPointF v02 = v02uv.center();
            QPointF v01 = v01uv.center();

            addTriangle(unfinishedTriangles, patch, { v0, v01, v02 });
            addTriangle(unfinishedTriangles, patch, { v1, v01, v12 });
            addTriangle(unfinishedTriangles, patch, { v2, v02, v12 });
            addTriangle(unfinishedTriangles, patch, { v01, v02, v12 });
        }
        else
        {
            finishedTriangles.emplace_back(qMove(triangle));
        }
    }

    Q_ASSERT(unfinishedTriangles.empty());

    // Sort the triangles according the standard (first is v direction, then u direction)
    auto comparator = [](const Triangle& left, const Triangle& right)
    {
        QPointF leftCenter = left.getCenter();
        QPointF rightCenter = right.getCenter();
        return std::pair(leftCenter.y(), leftCenter.x()) < std::pair(rightCenter.y(), rightCenter.x());
    };
    PDFExecutionPolicy::sort(PDFExecutionPolicy::Scope::Content, finishedTriangles.begin(), finishedTriangles.end(), comparator);

    std::vector<QPointF> vertices;
    std::vector<PDFMesh::Triangle> triangles;

    vertices.reserve(finishedTriangles.size() * 3);
    triangles.reserve(finishedTriangles.size());

    size_t vertexIndex = 0;
    for (const Triangle& triangle : finishedTriangles)
    {
        vertices.push_back(triangle.devicePoints[0]);
        vertices.push_back(triangle.devicePoints[1]);
        vertices.push_back(triangle.devicePoints[2]);

        QPointF center = triangle.getCenter();
        PDFColor color = getColorForUV(center.x(), center.y());
        QRgb rgbColor = m_colorSpace->getColor(color, cms, intent, reporter, true).rgb();

        PDFMesh::Triangle meshTriangle;
        meshTriangle.v1 = static_cast<uint32_t>(vertexIndex++);
        meshTriangle.v2 = static_cast<uint32_t>(vertexIndex++);
        meshTriangle.v3 = static_cast<uint32_t>(vertexIndex++);
        meshTriangle.color = rgbColor;
        triangles.push_back(meshTriangle);
    }

    mesh.addMesh(qMove(vertices), qMove(triangles));
}

void PDFTensorProductPatchShadingBase::fillMesh(PDFMesh& mesh,
                                                const QTransform& patternSpaceToDeviceSpaceMatrix,
                                                const PDFMeshQualitySettings& settings,
                                                const PDFTensorPatches& patches,
                                                const PDFCMS* cms,
                                                RenderingIntent intent,
                                                PDFRenderErrorReporter* reporter,
                                                const PDFOperationControl* operationControl) const
{
    const bool fastAlgorithm = patches.size() > 16;
    for (const auto& patch : patches)
    {
        fillMesh(mesh, settings, patch, cms, intent, reporter, fastAlgorithm, operationControl);
    }

    // Create bounding path
    if (m_boundingBox.isValid())
    {
        QPainterPath boundingPath;
        boundingPath.addPolygon(patternSpaceToDeviceSpaceMatrix.map(m_boundingBox));
        mesh.setBoundingPath(boundingPath);
    }

    if (m_backgroundColor.isValid())
    {
        QPainterPath path;
        path.addRect(settings.deviceSpaceMeshingArea);
        mesh.setBackgroundPath(path);
        mesh.setBackgroundColor(m_backgroundColor);
    }
}

void PDFTensorProductPatchShadingBase::addTriangle(std::vector<Triangle>& triangles, const PDFTensorPatch& patch, std::array<QPointF, 3> uvCoordinates)
{
    Q_ASSERT(uvCoordinates[0] != uvCoordinates[1] && uvCoordinates[1] != uvCoordinates[2]);

    Triangle triangle;
    triangle.uvCoordinates = uvCoordinates;
    triangle.fillTriangleDevicePoints(patch);

    triangles.push_back(triangle);
}

ShadingType PDFCoonsPatchShading::getShadingType() const
{
    return ShadingType::CoonsPatchMesh;
}

PDFTensorPatches PDFCoonsPatchShading::createPatches(QTransform userSpaceToDeviceSpaceMatrix, bool transformColor) const
{
    QTransform patternSpaceToDeviceSpaceMatrix = getPatternSpaceToDeviceSpaceMatrix(userSpaceToDeviceSpaceMatrix);

    size_t bitsPerPatch = m_bitsPerFlag + 16 * 2 * m_bitsPerCoordinate + 4 * m_colorComponentCount * m_bitsPerComponent;
    size_t remainder = (8 - (bitsPerPatch % 8)) % 8;
    bitsPerPatch += remainder;
    size_t bytesPerPatch = bitsPerPatch / 8;
    size_t patchCountEstimate = m_data.size() / bytesPerPatch;

    const PDFReal vertexScaleRatio = 1.0 / double((static_cast<uint64_t>(1) << m_bitsPerCoordinate) - 1);
    const PDFReal xScaleRatio = (m_xmax - m_xmin) * vertexScaleRatio;
    const PDFReal yScaleRatio = (m_ymax - m_ymin) * vertexScaleRatio;
    const PDFReal colorScaleRatio = 1.0 / double((static_cast<uint64_t>(1) << m_bitsPerComponent) - 1);

    PDFTensorPatches patches;
    patches.reserve(patchCountEstimate);

    PDFBitReader reader(&m_data, 8);

    auto readFlags = [this, &reader]() -> uint8_t
    {
        return reader.read(m_bitsPerFlag);
    };

    auto readPoint = [this, &reader, &patternSpaceToDeviceSpaceMatrix, xScaleRatio, yScaleRatio]() -> QPointF
    {
        const PDFReal x = m_xmin + (reader.read(m_bitsPerCoordinate)) * xScaleRatio;
        const PDFReal y = m_ymin + (reader.read(m_bitsPerCoordinate)) * yScaleRatio;
        return patternSpaceToDeviceSpaceMatrix.map(QPointF(x, y));
    };

    auto readColor = [this, &reader, colorScaleRatio, transformColor]() -> PDFColor
    {
        PDFColor color;
        color.resize(m_colorComponentCount);

        for (size_t i = 0; i < m_colorComponentCount; ++i)
        {
            const double cMin = m_limits[2 * i + 0];
            const double cMax = m_limits[2 * i + 1];
            color[i] = cMin + (reader.read(m_bitsPerComponent)) * (cMax - cMin) * colorScaleRatio;
        }

        return transformColor ? getColor(color) : color;
    };

    std::array<QPointF, 12> vertices;
    std::array<PDFColor, 4> colors;

    auto createTensorPatch = [&]
    {
        // Jakub Melka: please see following pictures, in PDF 1.7 specification, figures 4.22 and 4.24.
        // We copy the control points to the tensor patch in the appropriate order.
        //
        //              P_13               P_23                               V_5                 V_6
        //             /                       \                             /                       \
        //        P_03/                         \ P_33                   V_4/                         \ V_7
        //          |-----------------------------|                       |-----------------------------|
        //        / |                             |\                    / | C_2                    C_3  |\
        //       /  |                             | \                  /  |                             | \
        //    P_02  |                             |  P_32            V_3  |                             |  V_8
        //          |        P_12     P_22        |                       |                             |
        //          |                             |                       |                             |
        //          |                             |                       |                             |
        //          |                             |                       |                             |
        //          |        P_11     P_21        |                       |                             |
        //          |                             |                       |                             |
        //    P_01  |                             |  P_31           V_2   |                             |  V_9
        //       \  |                             | /                  \  |                             | /
        //        \ |                             |/                    \ | C_1                    C_4  |/
        //          |-----------------------------|                       |-----------------------------|
        //      P_00  \                         / P_30                V_1   \                         / V_10
        //             \                       /                             \                       /
        //             P_10                 P_20                             V_12                 V_11

        PDFTensorPatch::PointMatrix P;
        PDFTensorPatch::Colors tensorColors;

        P[0][0] = vertices[0];
        P[0][1] = vertices[1];
        P[0][2] = vertices[2];
        P[0][3] = vertices[3];
        P[1][3] = vertices[4];
        P[2][3] = vertices[5];
        P[3][3] = vertices[6];
        P[3][2] = vertices[7];
        P[3][1] = vertices[8];
        P[3][0] = vertices[9];
        P[2][0] = vertices[10];
        P[1][0] = vertices[11];

        auto computeTensorInterior = [](QPointF p1, QPointF p2, QPointF p3, QPointF p4, QPointF p5, QPointF p6, QPointF p7, QPointF p8)
        {
            return (-4.0 * p1 + 6.0 * (p2 + p3) - 2.0 * (p4 + p5) + 3.0 * (p6 + p7) - p8) / 9.0;
        };

        P[1][1] = computeTensorInterior(P[0][0], P[0][1], P[1][0], P[0][3], P[3][0], P[3][1], P[1][3], P[3][3]);
        P[1][2] = computeTensorInterior(P[0][3], P[0][2], P[1][3], P[0][0], P[3][3], P[3][2], P[1][0], P[3][0]);
        P[2][1] = computeTensorInterior(P[3][0], P[3][1], P[2][0], P[3][3], P[3][0], P[0][1], P[2][3], P[0][3]);
        P[2][2] = computeTensorInterior(P[3][3], P[3][2], P[2][3], P[3][0], P[0][3], P[0][2], P[2][0], P[0][0]);

        tensorColors[PDFTensorPatch::C_00] = colors[0];
        tensorColors[PDFTensorPatch::C_03] = colors[1];
        tensorColors[PDFTensorPatch::C_33] = colors[2];
        tensorColors[PDFTensorPatch::C_30] = colors[3];

        patches.emplace_back(P, tensorColors);
    };

    auto readPatchesFlag123 = [&]
    {
        for (size_t i = 4; i < vertices.size(); ++i)
        {
            vertices[i] = readPoint();
        }

        colors[2] = readColor();
        colors[3] = readColor();
    };

    while (!reader.isAtEnd())
    {
        const uint8_t flags = readFlags();
        switch (flags)
        {
            case 0:
            {
                // New Coons patch
                for (size_t i = 0; i < vertices.size(); ++i)
                {
                    vertices[i] = readPoint();
                }
                for (size_t i = 0; i < colors.size(); ++i)
                {
                    colors[i] = readColor();
                }

                createTensorPatch();
                break;
            }

            case 1:
            {
                vertices[0] = vertices[3];
                vertices[1] = vertices[4];
                vertices[2] = vertices[5];
                vertices[3] = vertices[6];

                colors[0] = colors[1];
                colors[1] = colors[2];

                readPatchesFlag123();
                createTensorPatch();
                break;
            }

            case 2:
            {
                vertices[0] = vertices[6];
                vertices[1] = vertices[7];
                vertices[2] = vertices[8];
                vertices[3] = vertices[9];

                colors[0] = colors[2];
                colors[1] = colors[3];

                readPatchesFlag123();
                createTensorPatch();
                break;
            }

            case 3:
            {
                vertices[3] = vertices[0];
                vertices[0] = vertices[9];
                vertices[1] = vertices[10];
                vertices[2] = vertices[11];

                colors[1] = colors[0];
                colors[0] = colors[3];

                readPatchesFlag123();
                createTensorPatch();
                break;
            }

            default:
                // This is error, clear patches and return
                patches.clear();
                return patches;
        }
    }

    return patches;
}

PDFMesh PDFCoonsPatchShading::createMesh(const PDFMeshQualitySettings& settings,
                                         const PDFCMS* cms,
                                         RenderingIntent intent,
                                         PDFRenderErrorReporter* reporter,
                                         const PDFOperationControl* operationControl) const
{
    PDFMesh mesh;
    PDFTensorPatches patches = createPatches(settings.userSpaceToDeviceSpaceMatrix, true);

    if (patches.empty())
    {
        throw PDFException(PDFTranslationContext::tr("Invalid data in coons patch shading."));
    }

    fillMesh(mesh, getPatternSpaceToDeviceSpaceMatrix(settings), settings, patches, cms, intent, reporter, operationControl);
    return mesh;
}

bool PDFShadingSampler::fillBackgroundColor(PDFColorBuffer outputBuffer) const
{
    const auto& originalBackgroundColor = m_pattern->getOriginalBackgroundColor();

    if (originalBackgroundColor.size() == outputBuffer.size())
    {
        for (size_t i = 0; i < outputBuffer.size(); ++i)
        {
            outputBuffer[i] = originalBackgroundColor[i];
        }

        return true;
    }

    return false;
}

}   // namespace pdf
