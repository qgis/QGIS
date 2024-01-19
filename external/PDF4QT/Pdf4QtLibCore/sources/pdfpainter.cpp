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

#include "pdfpainter.h"
#include "pdfpattern.h"
#include "pdfcms.h"

#include <QPainter>
#include <QCryptographicHash>
#include <QtMath>

#include "pdfdbgheap.h"

namespace pdf
{

PDFPainterBase::PDFPainterBase(PDFRenderer::Features features,
                               const PDFPage* page,
                               const PDFDocument* document,
                               const PDFFontCache* fontCache,
                               const PDFCMS* cms,
                               const PDFOptionalContentActivity* optionalContentActivity,
                               QTransform pagePointToDevicePointMatrix,
                               const PDFMeshQualitySettings& meshQualitySettings) :
    BaseClass(page, document, fontCache, cms, optionalContentActivity, pagePointToDevicePointMatrix, meshQualitySettings),
    m_features(features)
{

}

void PDFPainterBase::performUpdateGraphicsState(const PDFPageContentProcessorState& state)
{
    const PDFPageContentProcessorState::StateFlags flags = state.getStateFlags();

    // If current transformation matrix has changed, then update it
    if (flags.testFlag(PDFPageContentProcessorState::StateCurrentTransformationMatrix))
    {
        setWorldMatrix(getCurrentWorldMatrix());
    }

    if (flags.testFlag(PDFPageContentProcessorState::StateStrokeColor) ||
        flags.testFlag(PDFPageContentProcessorState::StateLineWidth) ||
        flags.testFlag(PDFPageContentProcessorState::StateLineCapStyle) ||
        flags.testFlag(PDFPageContentProcessorState::StateLineJoinStyle) ||
        flags.testFlag(PDFPageContentProcessorState::StateMitterLimit) ||
        flags.testFlag(PDFPageContentProcessorState::StateLineDashPattern) ||
        flags.testFlag(PDFPageContentProcessorState::StateAlphaStroking))
    {
        m_currentPen.dirty();
    }

    if (flags.testFlag(PDFPageContentProcessorState::StateFillColor) ||
        flags.testFlag(PDFPageContentProcessorState::StateAlphaFilling))
    {
        m_currentBrush.dirty();
    }

    // If current blend mode has changed, then update it
    if (flags.testFlag(PDFPageContentProcessorState::StateBlendMode))
    {
        // Try to simulate transparency groups. Use only first composition mode,
        // outside the transparency groups (so we are on pages main transparency
        // groups).

        const BlendMode blendMode = state.getBlendMode();
        if (canSetBlendMode(blendMode))
        {
            if (!PDFBlendModeInfo::isSupportedByQt(blendMode))
            {
                reportRenderErrorOnce(RenderErrorType::NotSupported, PDFTranslationContext::tr("Blend mode '%1' not supported.").arg(PDFBlendModeInfo::getBlendModeName(blendMode)));
            }

            const QPainter::CompositionMode compositionMode = PDFBlendModeInfo::getCompositionModeFromBlendMode(blendMode);
            setCompositionMode(compositionMode);
        }
        else if (blendMode != BlendMode::Normal && blendMode != BlendMode::Compatible)
        {
            reportRenderErrorOnce(RenderErrorType::NotSupported, PDFTranslationContext::tr("Blend mode '%1' is in transparency group, which is not supported.").arg(PDFBlendModeInfo::getBlendModeName(blendMode)));
        }
    }

    if (flags.testFlag(PDFPageContentProcessorState::StateSoftMask) && state.getSoftMask())
    {
        reportRenderErrorOnce(RenderErrorType::NotSupported, PDFTranslationContext::tr("Soft masks not supported."));
    }

    BaseClass::performUpdateGraphicsState(state);
}

bool PDFPainterBase::isContentSuppressedByOC(PDFObjectReference ocgOrOcmd)
{
    if (m_features.testFlag(PDFRenderer::IgnoreOptionalContent))
    {
        return false;
    }

    return PDFPageContentProcessor::isContentSuppressedByOC(ocgOrOcmd);
}

QPen PDFPainterBase::getCurrentPenImpl() const
{
    const PDFPageContentProcessorState* graphicState = getGraphicState();
    QColor color = graphicState->getStrokeColor();
    if (color.isValid())
    {
        color.setAlphaF(getEffectiveStrokingAlpha());
        const PDFReal lineWidth = graphicState->getLineWidth();
        Qt::PenCapStyle penCapStyle = graphicState->getLineCapStyle();
        Qt::PenJoinStyle penJoinStyle = graphicState->getLineJoinStyle();
        const PDFLineDashPattern& lineDashPattern = graphicState->getLineDashPattern();
        const PDFReal mitterLimit = graphicState->getMitterLimit();

        QPen pen(color);

        pen.setWidthF(lineWidth);
        pen.setCapStyle(penCapStyle);
        pen.setJoinStyle(penJoinStyle);
        pen.setMiterLimit(mitterLimit);

        if (lineDashPattern.isSolid())
        {
            pen.setStyle(Qt::SolidLine);
        }
        else
        {
            pen.setStyle(Qt::CustomDashLine);
            pen.setDashPattern(lineDashPattern.createForQPen(pen.widthF()));
            pen.setDashOffset(lineDashPattern.getDashOffset());
        }

        return pen;
    }
    else
    {
        return QPen(Qt::NoPen);
    }
}

QBrush PDFPainterBase::getCurrentBrushImpl() const
{
    const PDFPageContentProcessorState* graphicState = getGraphicState();
    QColor color = graphicState->getFillColor();
    if (color.isValid())
    {
        color.setAlphaF(getEffectiveFillingAlpha());
        return QBrush(color, Qt::SolidPattern);
    }
    else
    {
        return QBrush(Qt::NoBrush);
    }
}

PDFReal PDFPainterBase::getEffectiveStrokingAlpha() const
{
    PDFReal alpha = getGraphicState()->getAlphaStroking();

    auto it = m_transparencyGroupDataStack.crbegin();
    auto itEnd = m_transparencyGroupDataStack.crend();
    for (; it != itEnd; ++it)
    {
        const PDFTransparencyGroupPainterData& transparencyGroup = *it;
        alpha *= transparencyGroup.alphaStroke;

        if (transparencyGroup.group.isolated)
        {
            break;
        }
    }

    return alpha;
}

PDFReal PDFPainterBase::getEffectiveFillingAlpha() const
{
    PDFReal alpha = getGraphicState()->getAlphaFilling();

    auto it = m_transparencyGroupDataStack.crbegin();
    auto itEnd = m_transparencyGroupDataStack.crend();
    for (; it != itEnd; ++it)
    {
        const PDFTransparencyGroupPainterData& transparencyGroup = *it;
        alpha *= transparencyGroup.alphaFill;

        if (transparencyGroup.group.isolated)
        {
            break;
        }
    }

    return alpha;
}

bool PDFPainterBase::canSetBlendMode(BlendMode mode) const
{
    // We will assume, that we can set blend mode, when
    // all other blend modes on transparency stack are normal,
    // or compatible. It should work.

    Q_UNUSED(mode);
    return std::all_of(m_transparencyGroupDataStack.cbegin(), m_transparencyGroupDataStack.cend(), [](const PDFTransparencyGroupPainterData& group) { return group.blendMode == BlendMode::Normal || group.blendMode == BlendMode::Compatible; });
}

void PDFPainterBase::performBeginTransparencyGroup(ProcessOrder order, const PDFTransparencyGroup& transparencyGroup)
{
    if (order == ProcessOrder::BeforeOperation)
    {
        PDFTransparencyGroupPainterData data;
        data.group = transparencyGroup;
        data.alphaFill = getGraphicState()->getAlphaFilling();
        data.alphaStroke = getGraphicState()->getAlphaStroking();
        data.blendMode = getGraphicState()->getBlendMode();
        m_transparencyGroupDataStack.emplace_back(qMove(data));
    }
}

void PDFPainterBase::performEndTransparencyGroup(ProcessOrder order, const PDFTransparencyGroup& transparencyGroup)
{
    Q_UNUSED(transparencyGroup);

    if (order == ProcessOrder::AfterOperation)
    {
        m_transparencyGroupDataStack.pop_back();
    }
}

PDFPainter::PDFPainter(QPainter* painter,
                       PDFRenderer::Features features,
                       QTransform pagePointToDevicePointMatrix,
                       const PDFPage* page,
                       const PDFDocument* document,
                       const PDFFontCache* fontCache,
                       const PDFCMS* cms,
                       const PDFOptionalContentActivity* optionalContentActivity,
                       const PDFMeshQualitySettings& meshQualitySettings) :
    BaseClass(features, page, document, fontCache, cms, optionalContentActivity, pagePointToDevicePointMatrix, meshQualitySettings),
    m_painter(painter)
{
    Q_ASSERT(painter);
    Q_ASSERT(pagePointToDevicePointMatrix.isInvertible());

    m_painter->save();

    if (features.testFlag(PDFRenderer::ClipToCropBox))
    {
        QRectF cropBox = page->getCropBox();
        if (cropBox.isValid())
        {
            QPainterPath path;
            path.addPolygon(pagePointToDevicePointMatrix.map(cropBox));

            m_painter->setClipPath(path, Qt::IntersectClip);
        }
    }

    m_painter->setRenderHint(QPainter::SmoothPixmapTransform, features.testFlag(PDFRenderer::SmoothImages));
}

PDFPainter::~PDFPainter()
{
    m_painter->restore();
}

void PDFPainter::performPathPainting(const QPainterPath& path, bool stroke, bool fill, bool text, Qt::FillRule fillRule)
{
    Q_ASSERT(stroke || fill);

    // Set antialiasing
    const bool antialiasing = (text && hasFeature(PDFRenderer::TextAntialiasing)) || (!text && hasFeature(PDFRenderer::Antialiasing));
    m_painter->setRenderHint(QPainter::Antialiasing, antialiasing);

    if (stroke)
    {
        m_painter->setPen(getCurrentPen());
    }
    else
    {
        m_painter->setPen(Qt::NoPen);
    }

    if (fill)
    {
        m_painter->setBrush(getCurrentBrush());
    }
    else
    {
        m_painter->setBrush(Qt::NoBrush);
    }

    Q_ASSERT(path.fillRule() == fillRule);
    m_painter->drawPath(path);
}

void PDFPainter::performClipping(const QPainterPath& path, Qt::FillRule fillRule)
{
    Q_ASSERT(path.fillRule() == fillRule);
    m_painter->setClipPath(path, Qt::IntersectClip);
}

void PDFPainter::performImagePainting(const QImage& image)
{
    if (isContentSuppressed())
    {
        // Content is suppressed, do not paint anything
        return;
    }

    m_painter->save();

    QImage adjustedImage = image;

    if (hasFeature(PDFRenderer::SmoothImages))
    {
        // Test, if we can use smooth images. We can use them under following conditions:
        //  1) Transformed rectangle is not skewed or deformed (so vectors (0, 1) and (1, 0) are orthogonal)
        //  2) We are shrinking the image

        QTransform transform = m_painter->worldTransform();
        QLineF mappedWidthVector = transform.map(QLineF(0, 0, 1, 0));
        QLineF mappedHeightVector = transform.map(QLineF(0, 0, 0, 1));
        qreal angle = mappedWidthVector.angleTo(mappedHeightVector);
        if (qFuzzyCompare(angle, 90.0))
        {
            // Image is not skewed, so we test enlargement factor
            const int newWidth = mappedWidthVector.length();
            const int newHeight = mappedHeightVector.length();

            const int newPixels = newWidth * newHeight;
            const int oldPixels = image.width() * image.height();

            if (newPixels < oldPixels)
            {
                QSize size = adjustedImage.size();
                QSize adjustedImageSize = size.scaled(newWidth, newHeight, Qt::KeepAspectRatio);
                adjustedImage = adjustedImage.scaled(adjustedImageSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }
        }
    }

    QTransform imageTransform(1.0 / adjustedImage.width(), 0, 0, 1.0 / adjustedImage.height(), 0, 0);
    QTransform worldTransform = imageTransform * m_painter->worldTransform();

    // Because Qt uses opposite axis direction than PDF, then we must transform the y-axis
    // to the opposite (so the image is then unchanged)
    worldTransform.translate(0, adjustedImage.height());
    worldTransform.scale(1, -1);

    m_painter->setWorldTransform(worldTransform);
    m_painter->drawImage(0, 0, adjustedImage);

    m_painter->restore();
}

void PDFPainter::performMeshPainting(const PDFMesh& mesh)
{
    m_painter->save();
    m_painter->setWorldTransform(QTransform());
    mesh.paint(m_painter, getEffectiveFillingAlpha());
    m_painter->restore();
}

void PDFPainter::performSaveGraphicState(ProcessOrder order)
{
    if (order == ProcessOrder::AfterOperation)
    {
        m_painter->save();
    }
}

void PDFPainter::performRestoreGraphicState(ProcessOrder order)
{
    if (order == ProcessOrder::BeforeOperation)
    {
        m_painter->restore();
    }
}

void PDFPainter::setWorldMatrix(const QTransform& matrix)
{
    m_painter->setWorldTransform(QTransform(matrix), false);
}

void PDFPainter::setCompositionMode(QPainter::CompositionMode mode)
{
    m_painter->setCompositionMode(mode);
}

PDFPrecompiledPageGenerator::PDFPrecompiledPageGenerator(PDFPrecompiledPage* precompiledPage,
                                                         PDFRenderer::Features features,
                                                         const PDFPage* page,
                                                         const PDFDocument* document,
                                                         const PDFFontCache* fontCache,
                                                         const PDFCMS* cms,
                                                         const PDFOptionalContentActivity* optionalContentActivity,
                                                         const PDFMeshQualitySettings& meshQualitySettings) :
    BaseClass(features, page, document, fontCache, cms, optionalContentActivity, QTransform(), meshQualitySettings),
    m_precompiledPage(precompiledPage)
{
    m_precompiledPage->setPaperColor(cms->getPaperColor());
    m_precompiledPage->getSnapInfo()->addPageMediaBox(page->getRotatedMediaBox());
}

void PDFPrecompiledPageGenerator::performPathPainting(const QPainterPath& path, bool stroke, bool fill, bool text, Qt::FillRule fillRule)
{
    Q_ASSERT(stroke || fill);
    Q_ASSERT(path.fillRule() == fillRule);

    QPen pen = stroke ? getCurrentPen() : QPen(Qt::NoPen);
    QBrush brush = fill ? getCurrentBrush() : QBrush(Qt::NoBrush);
    m_precompiledPage->addPath(qMove(pen), qMove(brush), path, text);
}

void PDFPrecompiledPageGenerator::performClipping(const QPainterPath& path, Qt::FillRule fillRule)
{
    Q_ASSERT(path.fillRule() == fillRule);
    m_precompiledPage->addClip(path);
}

void PDFPrecompiledPageGenerator::performImagePainting(const QImage& image)
{
    if (isContentSuppressed())
    {
        // Content is suppressed, do not paint anything
        return;
    }

    // Add snap info for image to the snapper
    QTransform matrix = getCurrentWorldMatrix();
    PDFSnapInfo* snapInfo = m_precompiledPage->getSnapInfo();
    snapInfo->addImage({
                           matrix.map(QPointF(0.0, 0.0)),
                           matrix.map(QPointF(1.0, 0.0)),
                           matrix.map(QPointF(1.0, 1.0)),
                           matrix.map(QPointF(0.0, 1.0)),
                           matrix.map(QPointF(0.5, 0.5)),
                       }, image);

    if (isTransparencyGroupActive())
    {
        PDFReal alpha = getEffectiveFillingAlpha();
        if (alpha != 1.0)
        {
            // Try to approximate transparency group using alpha channel
            QImage imageWithAlpha = image;
            QImage alphaChannel = imageWithAlpha.convertToFormat(QImage::Format_Alpha8);
            uchar* bits = alphaChannel.bits();

            for (qsizetype i = 0, sizeInBytes = alphaChannel.sizeInBytes(); i < sizeInBytes; ++i)
            {
                bits[i] *= alpha;
            }

            imageWithAlpha.setAlphaChannel(alphaChannel);
            m_precompiledPage->addImage(imageWithAlpha);
            return;
        }
    }

    m_precompiledPage->addImage(image);
}

void PDFPrecompiledPageGenerator::performMeshPainting(const PDFMesh& mesh)
{
    m_precompiledPage->addMesh(mesh, getEffectiveFillingAlpha());
}

void PDFPrecompiledPageGenerator::performSaveGraphicState(PDFPageContentProcessor::ProcessOrder order)
{
    if (order == ProcessOrder::AfterOperation)
    {
        m_precompiledPage->addSaveGraphicState();
    }
}

void PDFPrecompiledPageGenerator::performRestoreGraphicState(PDFPageContentProcessor::ProcessOrder order)
{
    if (order == ProcessOrder::BeforeOperation)
    {
        m_precompiledPage->addRestoreGraphicState();
    }
}

void PDFPrecompiledPageGenerator::setWorldMatrix(const QTransform& matrix)
{
    m_precompiledPage->addSetWorldMatrix(matrix);
}

void PDFPrecompiledPageGenerator::setCompositionMode(QPainter::CompositionMode mode)
{
    m_precompiledPage->addSetCompositionMode(mode);
}

void PDFPrecompiledPage::draw(QPainter* painter,
                              const QRectF& cropBox,
                              const QTransform& pagePointToDevicePointMatrix,
                              PDFRenderer::Features features,
                              PDFReal opacity) const
{
    Q_ASSERT(painter);
    Q_ASSERT(pagePointToDevicePointMatrix.isInvertible());

    painter->save();
    painter->setWorldTransform(QTransform());
    painter->setOpacity(opacity);

    if (features.testFlag(PDFRenderer::ClipToCropBox))
    {
        if (cropBox.isValid())
        {
            QPainterPath path;
            path.addPolygon(pagePointToDevicePointMatrix.map(cropBox));
            painter->setClipPath(path, Qt::IntersectClip);
        }
    }

    painter->setRenderHint(QPainter::SmoothPixmapTransform, features.testFlag(PDFRenderer::SmoothImages));

    // Process all instructions
    for (const Instruction& instruction : m_instructions)
    {
        switch (instruction.type)
        {
            case InstructionType::DrawPath:
            {
                const PathPaintData& data = m_paths[instruction.dataIndex];

                // Set antialiasing
                const bool antialiasing = (data.isText && features.testFlag(PDFRenderer::TextAntialiasing)) || (!data.isText && features.testFlag(PDFRenderer::Antialiasing));
                painter->setRenderHint(QPainter::Antialiasing, antialiasing);
                painter->setPen(data.pen);
                painter->setBrush(data.brush);
                painter->drawPath(data.path);
                break;
            }

            case InstructionType::DrawImage:
            {
                const ImageData& data = m_images[instruction.dataIndex];
                const QImage& image = data.image;

                painter->save();

                QTransform imageTransform(1.0 / image.width(), 0, 0, 1.0 / image.height(), 0, 0);
                QTransform worldTransform = imageTransform * painter->worldTransform();

                // Jakub Melka: Because Qt uses opposite axis direction than PDF, then we must transform the y-axis
                // to the opposite (so the image is then unchanged)
                worldTransform.translate(0, image.height());
                worldTransform.scale(1, -1);

                painter->setWorldTransform(worldTransform);
                painter->drawImage(0, 0, image);
                painter->restore();
                break;
            }

            case InstructionType::DrawMesh:
            {
                const MeshPaintData& data = m_meshes[instruction.dataIndex];

                painter->save();
                painter->setWorldTransform(QTransform(pagePointToDevicePointMatrix));
                data.mesh.paint(painter, data.alpha);
                painter->restore();
                break;
            }

            case InstructionType::Clip:
            {
                painter->setClipPath(m_clips[instruction.dataIndex].clipPath, Qt::IntersectClip);
                break;
            }

            case InstructionType::SaveGraphicState:
            {
                painter->save();
                break;
            }

            case InstructionType::RestoreGraphicState:
            {
                painter->restore();
                break;
            }

            case InstructionType::SetWorldMatrix:
            {
                painter->setWorldTransform(QTransform(m_matrices[instruction.dataIndex] * pagePointToDevicePointMatrix));
                break;
            }

            case InstructionType::SetCompositionMode:
            {
                painter->setCompositionMode(m_compositionModes[instruction.dataIndex]);
                break;
            }

            default:
            {
                Q_ASSERT(false);
                break;
            }
        }
    }

    painter->restore();
}

void PDFPrecompiledPage::redact(QPainterPath redactPath, const QTransform& matrix, QColor color)
{
    if (redactPath.isEmpty())
    {
        // Nothing to be redacted
        return;
    }

    std::stack<QTransform> worldMatrixStack;
    worldMatrixStack.push(matrix);

    if (color.isValid())
    {
        m_instructions.insert(m_instructions.begin(), Instruction(InstructionType::SaveGraphicState, 0));
    }

    // Process all instructions
    for (const Instruction& instruction : m_instructions)
    {
        switch (instruction.type)
        {
            case InstructionType::DrawPath:
            {
                QTransform currentMatrix = worldMatrixStack.top().inverted();
                QPainterPath mappedRedactPath = currentMatrix.map(redactPath);
                PathPaintData& path = m_paths[instruction.dataIndex];
                path.path = path.path.subtracted(mappedRedactPath);
                break;
            }

            case InstructionType::DrawImage:
            {
                ImageData& data = m_images[instruction.dataIndex];
                QImage& image = data.image;

                QTransform imageTransform(1.0 / image.width(), 0, 0, 1.0 / image.height(), 0, 0);
                QTransform worldTransform = imageTransform * QTransform(worldMatrixStack.top());

                // Jakub Melka: Because Qt uses opposite axis direction than PDF, then we must transform the y-axis
                // to the opposite (so the image is then unchanged)
                worldTransform.translate(0, image.height());
                worldTransform.scale(1, -1);

                QPainter painter(&image);
                painter.setWorldTransform(worldTransform.inverted());
                painter.drawPath(redactPath);
                painter.end();
                break;
            }

            case InstructionType::DrawMesh:
                // We do not redact mesh
                break;

            case InstructionType::Clip:
            {
                QTransform currentMatrix = worldMatrixStack.top().inverted();
                QPainterPath mappedRedactPath = currentMatrix.map(redactPath);
                m_clips[instruction.dataIndex].clipPath = m_clips[instruction.dataIndex].clipPath.subtracted(mappedRedactPath);
                break;
            }

            case InstructionType::SaveGraphicState:
                worldMatrixStack.push(worldMatrixStack.top());
                break;

            case InstructionType::RestoreGraphicState:
                worldMatrixStack.pop();
                break;

            case InstructionType::SetWorldMatrix:
                worldMatrixStack.top() = m_matrices[instruction.dataIndex];
                break;

            case InstructionType::SetCompositionMode:
                break;

            default:
            {
                Q_ASSERT(false);
                break;
            }
        }
    }

    if (color.isValid())
    {
        addRestoreGraphicState();
        addPath(Qt::NoPen, QBrush(color), matrix.map(redactPath), false);
    }
}

void PDFPrecompiledPage::addPath(QPen pen, QBrush brush, QPainterPath path, bool isText)
{
    m_instructions.emplace_back(InstructionType::DrawPath, m_paths.size());
    m_paths.emplace_back(qMove(pen), qMove(brush), qMove(path), isText);
}

void PDFPrecompiledPage::addClip(QPainterPath path)
{
    m_instructions.emplace_back(InstructionType::Clip, m_clips.size());
    m_clips.emplace_back(qMove(path));
}

void PDFPrecompiledPage::addImage(QImage image)
{
    // Convert the image into format Format_ARGB32_Premultiplied for fast drawing.
    // If this format is used, then no image conversion is performed while drawing.
    if (image.format() != QImage::Format_ARGB32_Premultiplied)
    {
        image.convertTo(QImage::Format_ARGB32_Premultiplied);
    }

    m_instructions.emplace_back(InstructionType::DrawImage, m_images.size());
    m_images.emplace_back(qMove(image));
}

void PDFPrecompiledPage::addMesh(PDFMesh mesh, PDFReal alpha)
{
    m_instructions.emplace_back(InstructionType::DrawMesh, m_meshes.size());
    m_meshes.emplace_back(qMove(mesh), alpha);
}

void PDFPrecompiledPage::addSetWorldMatrix(const QTransform& matrix)
{
    m_instructions.emplace_back(InstructionType::SetWorldMatrix, m_matrices.size());
    m_matrices.push_back(matrix);
}

void PDFPrecompiledPage::addSetCompositionMode(QPainter::CompositionMode compositionMode)
{
    m_instructions.emplace_back(InstructionType::SetCompositionMode, m_compositionModes.size());
    m_compositionModes.push_back(compositionMode);
}

void PDFPrecompiledPage::optimize()
{
    m_instructions.shrink_to_fit();
    m_paths.shrink_to_fit();
    m_clips.shrink_to_fit();
    m_images.shrink_to_fit();
    m_meshes.shrink_to_fit();
    m_matrices.shrink_to_fit();
    m_compositionModes.shrink_to_fit();
}

void PDFPrecompiledPage::convertColors(const PDFColorConvertor& colorConvertor)
{
    // Jakub Melka: we must apply color convertor in following areas:
    //     - painter paths
    //     - images
    //     - meshes

    if (!colorConvertor.isActive())
    {
        return;
    }

    for (PathPaintData& pathData : m_paths)
    {
        if (pathData.pen.style() != Qt::NoPen)
        {
            pathData.pen.setColor(colorConvertor.convert(pathData.pen.color(), false, pathData.isText));
        }
        if (pathData.brush.style() == Qt::SolidPattern)
        {
            pathData.brush.setColor(colorConvertor.convert(pathData.brush.color(), false, pathData.isText));
        }
    }

    for (ImageData& imageData : m_images)
    {
        imageData.image = colorConvertor.convert(imageData.image);
    }

    for (MeshPaintData& meshPaintData : m_meshes)
    {
        meshPaintData.mesh.convertColors(colorConvertor);
    }

    m_paperColor = colorConvertor.convert(m_paperColor, true, false);
}

void PDFPrecompiledPage::finalize(qint64 compilingTimeNS, QList<PDFRenderError> errors)
{
    m_compilingTimeNS = compilingTimeNS;
    m_errors = qMove(errors);

    // Determine memory consumption
    m_memoryConsumptionEstimate = sizeof(*this);
    m_memoryConsumptionEstimate += sizeof(Instruction) * m_instructions.capacity();
    m_memoryConsumptionEstimate += sizeof(PathPaintData) * m_paths.capacity();
    m_memoryConsumptionEstimate += sizeof(ClipData) * m_clips.capacity();
    m_memoryConsumptionEstimate += sizeof(ImageData) * m_images.capacity();
    m_memoryConsumptionEstimate += sizeof(MeshPaintData) * m_meshes.capacity();
    m_memoryConsumptionEstimate += sizeof(QTransform) * m_matrices.capacity();
    m_memoryConsumptionEstimate += sizeof(QPainter::CompositionMode) * m_compositionModes.capacity();
    m_memoryConsumptionEstimate += sizeof(PDFRenderError) * m_errors.size();

    auto calculateQPathMemoryConsumption = [](const QPainterPath& path)
    {
        return sizeof(QPainterPath::Element) * path.capacity();
    };
    for (const PathPaintData& data : m_paths)
    {
        m_memoryConsumptionEstimate += calculateQPathMemoryConsumption(data.path);
    }
    for (const ClipData& data : m_clips)
    {
        m_memoryConsumptionEstimate += calculateQPathMemoryConsumption(data.clipPath);
    }
    for (const ImageData& data : m_images)
    {
        m_memoryConsumptionEstimate += data.image.sizeInBytes();
    }
    for (const MeshPaintData& data : m_meshes)
    {
        m_memoryConsumptionEstimate += data.mesh.getMemoryConsumptionEstimate();
    }
}

PDFPrecompiledPage::GraphicPieceInfos PDFPrecompiledPage::calculateGraphicPieceInfos(QRectF mediaBox,
                                                                                     PDFReal epsilon) const
{
    GraphicPieceInfos infos;

    struct State
    {
        QTransform matrix;
    };
    std::stack<State> stateStack;
    stateStack.emplace();

    // Check, if epsilon is not too small
    if (qFuzzyIsNull(epsilon))
    {
        epsilon = 0.000001;
    }
    PDFReal factor = 1.0 / epsilon;

    QImage shadingTestImage;

    // Process all instructions
    for (const Instruction& instruction : m_instructions)
    {
        switch (instruction.type)
        {
            case InstructionType::DrawPath:
            {
                const PathPaintData& data = m_paths[instruction.dataIndex];

                GraphicPieceInfo info;
                QByteArray serializedPath;

                // Serialize data
                if (true)
                {
                    QDataStream stream(&serializedPath, QIODevice::WriteOnly);

                    stream << data.isText;
                    stream << data.pen;
                    stream << data.brush;

                    // Translate map to page coordinates
                    QPainterPath pagePath = stateStack.top().matrix.map(data.path);

                    info.type = data.isText ? GraphicPieceInfo::Type::Text : GraphicPieceInfo::Type::VectorGraphics;
                    info.boundingRect = pagePath.controlPointRect();
                    info.pagePath = pagePath;

                    const int elementCount = pagePath.elementCount();
                    for (int i = 0; i < elementCount; ++i)
                    {
                        QPainterPath::Element element = pagePath.elementAt(i);

                        PDFReal roundedX = qFloor(element.x * factor);
                        PDFReal roundedY = qFloor(element.y * factor);

                        stream << roundedX;
                        stream << roundedY;
                        stream << element.type;
                    }
                }

                QByteArray hash = QCryptographicHash::hash(serializedPath, QCryptographicHash::Sha512);
                Q_ASSERT(QCryptographicHash::hashLength(QCryptographicHash::Sha512) == 64);

                size_t size = qMin<size_t>(hash.length(), info.hash.size());
                std::copy(hash.data(), hash.data() + size, info.hash.data());

                infos.emplace_back(std::move(info));
                break;
            }

            case InstructionType::DrawImage:
            {
                const ImageData& data = m_images[instruction.dataIndex];
                const QImage& image = data.image;

                GraphicPieceInfo info;
                QByteArray serializedPath;
                QByteArray serializedImage;

                // Serialize data
                if (true)
                {
                    QDataStream stream(&serializedPath, QIODevice::WriteOnly);
                    QDataStream streamImage(&serializedImage, QIODevice::WriteOnly);

                    // Jakub Melka: serialize image position
                    QTransform worldMatrix = stateStack.top().matrix;

                    QPainterPath pagePath;
                    pagePath.addRect(0, 0, 1, 1);
                    pagePath = worldMatrix.map(pagePath);

                    info.type = GraphicPieceInfo::Type::Image;
                    info.boundingRect = pagePath.controlPointRect();
                    info.pagePath = pagePath;

                    const int elementCount = pagePath.elementCount();
                    for (int i = 0; i < elementCount; ++i)
                    {
                        QPainterPath::Element element = pagePath.elementAt(i);

                        PDFReal roundedX = qRound(element.x * factor);
                        PDFReal roundedY = qRound(element.y * factor);

                        stream << roundedX;
                        stream << roundedY;
                        stream << element.type;
                    }

                    // serialize image data
                    stream.writeBytes(reinterpret_cast<const char*>(image.bits()), image.sizeInBytes());
                    streamImage.writeBytes(reinterpret_cast<const char*>(image.bits()), image.sizeInBytes());
                }

                QByteArray hash = QCryptographicHash::hash(serializedPath, QCryptographicHash::Sha512);
                Q_ASSERT(QCryptographicHash::hashLength(QCryptographicHash::Sha512) == 64);

                QByteArray imageHash = QCryptographicHash::hash(serializedImage, QCryptographicHash::Sha512);

                size_t size = qMin<size_t>(hash.length(), info.hash.size());
                std::copy(hash.data(), hash.data() + size, info.hash.data());

                size_t sizeImage = qMin<size_t>(imageHash.length(), info.imageHash.size());
                std::copy(imageHash.data(), imageHash.data() + sizeImage, info.imageHash.data());

                infos.emplace_back(std::move(info));
                break;
            }

            case InstructionType::DrawMesh:
            {
                const MeshPaintData& data = m_meshes[instruction.dataIndex];

                if (shadingTestImage.isNull())
                {
                    QSizeF mediaBoxSize = mediaBox.size();
                    mediaBoxSize = mediaBoxSize.scaled(256, 256, Qt::KeepAspectRatio);
                    QSize imageSize = mediaBoxSize.toSize();
                    shadingTestImage = QImage(imageSize, QImage::Format_ARGB32);
                }

                shadingTestImage.fill(Qt::transparent);

                QTransform pagePointToDevicePointMatrix;
                pagePointToDevicePointMatrix.scale(shadingTestImage.width() / mediaBox.width(), -shadingTestImage.height() / mediaBox.height());

                {
                    QPainter painter(&shadingTestImage);
                    painter.setWorldTransform(pagePointToDevicePointMatrix);
                    data.mesh.paint(&painter, data.alpha);
                }

                GraphicPieceInfo info;
                QByteArray serializedMesh;

                // Serialize data
                if (true)
                {
                    QDataStream stream(&serializedMesh, QIODevice::WriteOnly);

                    // serialize image data
                    stream.writeBytes(reinterpret_cast<const char*>(shadingTestImage.bits()), shadingTestImage.sizeInBytes());
                }

                QByteArray hash = QCryptographicHash::hash(serializedMesh, QCryptographicHash::Sha512);
                Q_ASSERT(QCryptographicHash::hashLength(QCryptographicHash::Sha512) == 64);

                size_t size = qMin<size_t>(hash.length(), info.hash.size());
                std::copy(hash.data(), hash.data() + size, info.hash.data());

                info.boundingRect = QRectF();
                info.type = GraphicPieceInfo::Type::Shading;
                infos.emplace_back(std::move(info));
                break;
            }

            case InstructionType::Clip:
            {
                // Do nothing, we are just collecting information
                break;
            }

            case InstructionType::SaveGraphicState:
            {
                stateStack.push(stateStack.top());
                break;
            }

            case InstructionType::RestoreGraphicState:
            {
                stateStack.pop();
                break;
            }

            case InstructionType::SetWorldMatrix:
            {
                stateStack.top().matrix = m_matrices[instruction.dataIndex];
                break;
            }

            case InstructionType::SetCompositionMode:
            {
                // Do nothing, we are just collecting information
                break;
            }

            default:
            {
                Q_ASSERT(false);
                break;
            }
        }
    }

    return infos;
}

}   // namespace pdf
