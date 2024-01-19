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

#ifndef PDFPAINTER_H
#define PDFPAINTER_H

#include "pdfutils.h"
#include "pdfpattern.h"
#include "pdfrenderer.h"
#include "pdfpagecontentprocessor.h"
#include "pdftextlayout.h"
#include "pdfcolorconvertor.h"
#include "pdfsnapper.h"

#include <QPen>
#include <QBrush>
#include <QElapsedTimer>

namespace pdf
{

/// Base painter, encapsulating common functionality for all PDF painters (for example,
/// direct painter, or painter, which generates list of graphic commands).
class PDFPainterBase : public PDFPageContentProcessor
{
    using BaseClass = PDFPageContentProcessor;

public:
    explicit PDFPainterBase(PDFRenderer::Features features,
                            const PDFPage* page,
                            const PDFDocument* document,
                            const PDFFontCache* fontCache,
                            const PDFCMS* cms,
                            const PDFOptionalContentActivity* optionalContentActivity,
                            QTransform pagePointToDevicePointMatrix,
                            const PDFMeshQualitySettings& meshQualitySettings);

    virtual bool isContentSuppressedByOC(PDFObjectReference ocgOrOcmd) override;

protected:
    virtual void performUpdateGraphicsState(const PDFPageContentProcessorState& state) override;
    virtual void performBeginTransparencyGroup(ProcessOrder order, const PDFTransparencyGroup& transparencyGroup) override;
    virtual void performEndTransparencyGroup(ProcessOrder order, const PDFTransparencyGroup& transparencyGroup) override;
    virtual void setWorldMatrix(const QTransform& matrix) = 0;
    virtual void setCompositionMode(QPainter::CompositionMode mode) = 0;

    /// Returns current pen
    const QPen& getCurrentPen() { return m_currentPen.get(this, &PDFPainterBase::getCurrentPenImpl); }

    /// Returns current brush
    const QBrush& getCurrentBrush() { return m_currentBrush.get(this, &PDFPainterBase::getCurrentBrushImpl); }

    /// Returns effective stroking alpha from transparency groups and current graphic state
    PDFReal getEffectiveStrokingAlpha() const;

    /// Returns effective filling alpha from transparency groups and current graphic state
    PDFReal getEffectiveFillingAlpha() const;

    /// Returns true, if blend mode can be set according the transparency group stack
    bool canSetBlendMode(BlendMode mode) const;

    /// Returns, if feature is turned on
    bool hasFeature(PDFRenderer::Feature feature) const { return m_features.testFlag(feature); }

    /// Is transparency group active?
    bool isTransparencyGroupActive() const { return !m_transparencyGroupDataStack.empty(); }

private:
    /// Returns current pen (implementation)
    QPen getCurrentPenImpl() const;

    /// Returns current brush (implementation)
    QBrush getCurrentBrushImpl() const;

    struct PDFTransparencyGroupPainterData
    {
        PDFTransparencyGroup group;
        PDFReal alphaStroke = 1.0;
        PDFReal alphaFill = 1.0;
        BlendMode blendMode = BlendMode::Normal;
    };

    PDFRenderer::Features m_features;
    PDFCachedItem<QPen> m_currentPen;
    PDFCachedItem<QBrush> m_currentBrush;
    std::vector<PDFTransparencyGroupPainterData> m_transparencyGroupDataStack;
};

/// Processor, which processes PDF's page commands on the QPainter. It works with QPainter
/// and with transformation matrix, which translates page points to the device points.
/// Only basic transparency is supported, advanced transparency, such as transparency groups,
/// are not supported. Painter will try to emulate them so painting will not fail completely.
class PDFPainter : public PDFPainterBase
{
    using BaseClass = PDFPainterBase;

public:
    /// Constructs new PDFPainter object, with default parameters.
    /// \param painter Painter, on which page content is drawn
    /// \param features Features of the painter
    /// \param pagePointToDevicePointMatrix Matrix, which translates page points to device points
    /// \param page Page, which will be drawn
    /// \param document Document owning the page
    /// \param fontCache Font cache
    /// \param cms Color management system
    /// \param optionalContentActivity Activity of optional content
    /// \param meshQualitySettings Mesh quality settings
    explicit PDFPainter(QPainter* painter,
                        PDFRenderer::Features features,
                        QTransform pagePointToDevicePointMatrix,
                        const PDFPage* page,
                        const PDFDocument* document,
                        const PDFFontCache* fontCache,
                        const PDFCMS* cms,
                        const PDFOptionalContentActivity* optionalContentActivity,
                        const PDFMeshQualitySettings& meshQualitySettings);
    virtual ~PDFPainter() override;

protected:
    virtual void performPathPainting(const QPainterPath& path, bool stroke, bool fill, bool text, Qt::FillRule fillRule) override;
    virtual void performClipping(const QPainterPath& path, Qt::FillRule fillRule) override;
    virtual void performImagePainting(const QImage& image) override;
    virtual void performMeshPainting(const PDFMesh& mesh) override;
    virtual void performSaveGraphicState(ProcessOrder order) override;
    virtual void performRestoreGraphicState(ProcessOrder order) override;
    virtual void setWorldMatrix(const QTransform& matrix) override;
    virtual void setCompositionMode(QPainter::CompositionMode mode) override;

private:
    QPainter* m_painter;
};

/// Precompiled page contains precompiled graphic instructions of a PDF page to draw it quickly
/// on the target painter. It enables very fast drawing, because instructions are not decoded
/// and interpreted from the PDF stream, but they are just "played" on the painter.
class PDF4QTLIBCORESHARED_EXPORT PDFPrecompiledPage
{
public:
    explicit inline PDFPrecompiledPage() = default;

    inline PDFPrecompiledPage(const PDFPrecompiledPage&) = default;
    inline PDFPrecompiledPage(PDFPrecompiledPage&&) = default;
    inline PDFPrecompiledPage& operator=(const PDFPrecompiledPage&) = default;
    inline PDFPrecompiledPage& operator=(PDFPrecompiledPage&&) = default;

    enum class InstructionType
    {
        Invalid,
        DrawPath,
        DrawImage,
        DrawMesh,
        Clip,
        SaveGraphicState,
        RestoreGraphicState,
        SetWorldMatrix,
        SetCompositionMode
    };

    struct Instruction
    {
        inline Instruction() = default;
        inline Instruction(InstructionType type, size_t dataIndex) :
            type(type),
            dataIndex(dataIndex)
        {

        }

        InstructionType type = InstructionType::Invalid;
        size_t dataIndex = 0;
    };

    /// Paints page onto the painter using matrix
    /// \param painter Painter, onto which is page drawn
    /// \param cropBox Page's crop box
    /// \param pagePointToDevicePointMatrix Page point to device point transformation matrix
    /// \param features Renderer features
    /// \param opacity Opacity of page graphics
    void draw(QPainter* painter,
              const QRectF& cropBox,
              const QTransform& pagePointToDevicePointMatrix,
              PDFRenderer::Features features,
              PDFReal opacity) const;

    /// Redact path - remove all content intersecting given path,
    /// and fill redact path with given color.
    /// \param redactPath Redaction path in page coordinates
    /// \param color Redaction color (if invalid, nothing is being drawn)
    void redact(QPainterPath redactPath, const QTransform& matrix, QColor color);

    void addPath(QPen pen, QBrush brush, QPainterPath path, bool isText);
    void addClip(QPainterPath path);
    void addImage(QImage image);
    void addMesh(PDFMesh mesh, PDFReal alpha);
    void addSaveGraphicState() { m_instructions.emplace_back(InstructionType::SaveGraphicState, 0); }
    void addRestoreGraphicState() { m_instructions.emplace_back(InstructionType::RestoreGraphicState, 0); }
    void addSetWorldMatrix(const QTransform& matrix);
    void addSetCompositionMode(QPainter::CompositionMode compositionMode);

    /// Optimizes page memory allocation to contain less space
    void optimize();

    /// Converts all colors
    void convertColors(const PDFColorConvertor& colorConvertor);

    /// Finalizes precompiled page
    /// \param compilingTimeNS Compiling time in nanoseconds
    /// \param errors List of rendering errors
    void finalize(qint64 compilingTimeNS, QList<PDFRenderError> errors);

    /// Returns compiling time in nanoseconds
    qint64 getCompilingTimeNS() const { return m_compilingTimeNS; }

    /// Returns a list of rendering errors
    const QList<PDFRenderError>& getErrors() const { return m_errors; }

    /// Returns true, if page is valid (i.e. has nonzero instruction count)
    bool isValid() const { return !m_instructions.empty(); }

    /// Returns memory consumption estimate
    qint64 getMemoryConsumptionEstimate() const { return m_memoryConsumptionEstimate; }

    /// Returns paper color
    QColor getPaperColor() const { return m_paperColor; }
    void setPaperColor(QColor paperColor) { m_paperColor = paperColor; }

    PDFSnapInfo* getSnapInfo() { return &m_snapInfo; }
    const PDFSnapInfo* getSnapInfo() const { return &m_snapInfo; }

    /// Mark this precompiled page as accessed at a current time
    void markAccessed() { m_expirationTimer.start(); }

    /// Has page content expired with given timeout? This function
    /// is used together with function \p markAccessed to control
    /// cached pages expiration policy. Pages can be marked as accessed,
    /// and too old accessed pages can be removed.
    /// \sa markAccessed
    bool hasExpired(qint64 timeout) const { return m_expirationTimer.hasExpired(timeout); }

    struct GraphicPieceInfo
    {
        enum class Type
        {
            Unknown,
            Text,
            VectorGraphics,
            Image,
            Shading
        };

        bool operator<(const GraphicPieceInfo& other) const
        {
            return std::tie(type, hash) < std::tie(other.type, other.hash);
        }

        bool isText() const { return type == Type::Text; }
        bool isVectorGraphics() const { return type == Type::VectorGraphics; }
        bool isImage() const { return type == Type::Image; }
        bool isShading() const { return type == Type::Shading; }

        Type type = Type::Unknown;
        QRectF boundingRect;
        std::array<uint8_t, 64> hash = { }; ///< Hash of all data
        std::array<uint8_t, 64> imageHash = { }; ///< Hash of the image only
        QPainterPath pagePath;
    };

    using GraphicPieceInfos = std::vector<GraphicPieceInfo>;

    /// Creates information about piece of graphic in this page,
    /// for example, for comparation reasons. Parameter \p epsilon
    /// is for numerical precision - values under epsilon are considered
    /// as equal.
    /// \param mediaBox Page's media box
    /// \param epsilon Epsilon
    GraphicPieceInfos calculateGraphicPieceInfos(QRectF mediaBox,
                                                 PDFReal epsilon) const;

private:
    struct PathPaintData
    {
        inline PathPaintData() = default;
        inline PathPaintData(QPen pen, QBrush brush, QPainterPath path, bool isText) :
            pen(qMove(pen)),
            brush(qMove(brush)),
            path(qMove(path)),
            isText(isText)
        {

        }

        QPen pen;
        QBrush brush;
        QPainterPath path;
        bool isText = false;
    };

    struct ClipData
    {
        inline ClipData() = default;
        inline ClipData(QPainterPath path) :
            clipPath(qMove(path))
        {

        }

        QPainterPath clipPath;
    };

    struct ImageData
    {
        inline ImageData() = default;
        inline ImageData(QImage image) :
            image(qMove(image))
        {

        }

        QImage image;
    };

    struct MeshPaintData
    {
        inline MeshPaintData() = default;
        inline MeshPaintData(PDFMesh mesh, PDFReal alpha) :
            mesh(qMove(mesh)),
            alpha(alpha)
        {

        }

        PDFMesh mesh;
        PDFReal alpha = 1.0;
    };

    qint64 m_compilingTimeNS = 0;
    qint64 m_memoryConsumptionEstimate = 0;
    QColor m_paperColor = QColor(Qt::white);
    std::vector<Instruction> m_instructions;
    std::vector<PathPaintData> m_paths;
    std::vector<ClipData> m_clips;
    std::vector<ImageData> m_images;
    std::vector<MeshPaintData> m_meshes;
    std::vector<QTransform> m_matrices;
    std::vector<QPainter::CompositionMode> m_compositionModes;
    QList<PDFRenderError> m_errors;
    PDFSnapInfo m_snapInfo;
    QElapsedTimer m_expirationTimer;
};

/// Processor, which processes PDF's page commands and writes them to the precompiled page.
/// Precompiled page then can be used to execute these commands on QPainter.
class PDF4QTLIBCORESHARED_EXPORT PDFPrecompiledPageGenerator : public PDFPainterBase
{
    using BaseClass = PDFPainterBase;

public:
    explicit PDFPrecompiledPageGenerator(PDFPrecompiledPage* precompiledPage,
                                         PDFRenderer::Features features,
                                         const PDFPage* page,
                                         const PDFDocument* document,
                                         const PDFFontCache* fontCache,
                                         const PDFCMS* cms,
                                         const PDFOptionalContentActivity* optionalContentActivity,
                                         const PDFMeshQualitySettings& meshQualitySettings);

protected:
    virtual void performPathPainting(const QPainterPath& path, bool stroke, bool fill, bool text, Qt::FillRule fillRule) override;
    virtual void performClipping(const QPainterPath& path, Qt::FillRule fillRule) override;
    virtual void performImagePainting(const QImage& image) override;
    virtual void performMeshPainting(const PDFMesh& mesh) override;
    virtual void performSaveGraphicState(ProcessOrder order) override;
    virtual void performRestoreGraphicState(ProcessOrder order) override;
    virtual void setWorldMatrix(const QTransform& matrix) override;
    virtual void setCompositionMode(QPainter::CompositionMode mode) override;

private:
    PDFPrecompiledPage* m_precompiledPage;
};

}   // namespace pdf

#endif // PDFPAINTER_H
