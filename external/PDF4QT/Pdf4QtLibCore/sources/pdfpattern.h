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

#ifndef PDFPATTERN_H
#define PDFPATTERN_H

#include "pdfobject.h"
#include "pdffunction.h"
#include "pdfcolorspaces.h"
#include "pdfmeshqualitysettings.h"
#include "pdfcolorconvertor.h"

#include <QTransform>
#include <QPainterPath>

#include <memory>

namespace pdf
{
class PDFPattern;
class PDFTilingPattern;
class PDFShadingPattern;

using PDFPatternPtr = std::shared_ptr<PDFPattern>;

enum class PatternType
{
    Invalid = 0,
    Tiling = 1,
    Shading = 2,
};

enum class ShadingType
{
    Invalid = 0,
    Function = 1,
    Axial = 2,
    Radial = 3,
    FreeFormGouradTriangle = 4,
    LatticeFormGouradTriangle = 5,
    CoonsPatchMesh = 6,
    TensorProductPatchMesh = 7
};

/// Mesh consisting of triangles
class PDFMesh
{
public:
    explicit PDFMesh() = default;

    struct Triangle
    {
        uint32_t v1 = 0;
        uint32_t v2 = 0;
        uint32_t v3 = 0;

        QRgb color;
    };

    /// Adds vertex. Returns index of added vertex.
    /// \param vertex Vertex to be added
    /// \returns Index of the added vertex
    inline uint32_t addVertex(const QPointF& vertex) { const size_t index = m_vertices.size(); m_vertices.emplace_back(vertex); return static_cast<uint32_t>(index); }

    /// Adds triangle. Returns index of added triangle.
    /// \param triangle Triangle to be added
    /// \returns Index of the added vertex
    inline uint32_t addTriangle(const Triangle& triangle) { const size_t index = m_triangles.size(); m_triangles.emplace_back(triangle); return static_cast<uint32_t>(index); }

    /// Adds quad. Vertices are in clockwise order (so, we have edges v1-v2, v2-v3, v3-v4, v4-v1).
    /// \param v1 First vertex (for example, topleft)
    /// \param v2 Second vertex (for example, topright)
    /// \param v3 Third vertex (for example, bottomright)
    /// \param v4 Fourth vertex (for example, bottomleft)
    /// \param color Color of the quad.
    inline void addQuad(uint32_t v1, uint32_t v2, uint32_t v3, uint32_t v4, QRgb color) { addTriangle({v1, v2, v3, color}); addTriangle({ v1, v3, v4, color}); }

    /// Paints the mesh on the painter
    /// \param painter Painter, onto which is mesh drawn
    /// \param alpha Opacity factor
    void paint(QPainter* painter, PDFReal alpha) const;

    /// Transforms the mesh according to the matrix transform
    /// \param matrix Matrix transform to be performed
    void transform(const QTransform& matrix);

    /// Reserves memory for meshing - both number of vertices and triangles.
    /// Use this function, if number of vertices and triangles is known.
    /// \param vertexCount Vertex count
    /// \param triangleCount Triangle count
    void reserve(size_t vertexCount, size_t triangleCount) { m_vertices.reserve(vertexCount); m_triangles.reserve(triangleCount); }

    const QPainterPath& getBoundingPath() const { return m_boundingPath; }
    void setBoundingPath(const QPainterPath& path) { m_boundingPath = path; }

    /// Sets the vertex array to the mesh
    /// \param vertices New vertex array
    void setVertices(std::vector<QPointF>&& vertices) { m_vertices = qMove(vertices); }

    /// Sets the triangle array to the mesh
    /// \param triangles New triangle array
    void setTriangles(std::vector<Triangle>&& triangles) { m_triangles = qMove(triangles); }

    /// Merges the vertices/triangles (renumbers triangle indices) to this mesh.
    /// Algorithm assumes that vertices/triangles are numbered from zero.
    /// \param vertices Added vertex array
    /// \param triangles Added triangle array
    void addMesh(std::vector<QPointF>&& vertices, std::vector<Triangle>&& triangles);

    /// Returns vertex at given index
    /// \param index Index of the vertex
    const QPointF& getVertex(size_t index) const { return m_vertices[index]; }

    /// Returns triangle center. Triangles vertice indices must be valid.
    /// \param triangle Triangle
    QPointF getTriangleCenter(const Triangle& triangle) const;

    /// Sets the background path. In order to draw background properly, the background
    /// color must be set to a valid color.
    /// \param path Background path
    void setBackgroundPath(QPainterPath path) { m_backgroundPath = qMove(path); }

    /// Sets the background color (background path is then painted with this color, if it is not
    /// empty), if color is invalid, it turns off background painting.
    /// \param backgroundColor Background color
    void setBackgroundColor(QColor backgroundColor) { m_backgroundColor = backgroundColor; }

    /// Returns true, if mesh is empty
    bool isEmpty() const { return m_vertices.empty(); }

    /// Returns estimate of number of bytes, which this mesh occupies in memory
    qint64 getMemoryConsumptionEstimate() const;

    /// Apply color conversion
    void convertColors(const PDFColorConvertor& colorConvertor);

private:
    std::vector<QPointF> m_vertices;
    std::vector<Triangle> m_triangles;
    QPainterPath m_boundingPath;
    QPainterPath m_backgroundPath;
    QColor m_backgroundColor;
};

/// Represents tiling/shading pattern
class PDFPattern
{
public:
    explicit PDFPattern() = default;
    virtual ~PDFPattern() = default;

    virtual PatternType getType() const = 0;
    virtual const PDFShadingPattern* getShadingPattern() const = 0;
    virtual const PDFTilingPattern* getTilingPattern() const = 0;

    /// Returns bounding box in the shadings target coordinate system (not in
    /// pattern coordinate system).
    const QRectF& getBoundingBox() const { return m_boundingBox; }

    /// Returns transformation matrix from pattern space to the default
    /// target space.
    const QTransform& getMatrix() const { return m_matrix; }

    /// Create pattern from the object. If error occurs, exception is thrown
    /// \param colorSpaceDictionary Color space dictionary
    /// \param document Document, owning the pdf object
    /// \param object Object defining the pattern
    /// \param cms Color management system
    /// \param intent Rendering intent
    /// \param reporter Error reporter
    static PDFPatternPtr createPattern(const PDFDictionary* colorSpaceDictionary,
                                       const PDFDocument* document,
                                       const PDFObject& object,
                                       const PDFCMS* cms,
                                       RenderingIntent intent,
                                       PDFRenderErrorReporter* reporter);

    /// Create shading pattern from the object. If error occurs, exception is thrown
    /// \param colorSpaceDictionary Color space dictionary
    /// \param document Document, owning the pdf object
    /// \param object Object defining the shading
    /// \param matrix Matrix converting reference coordinate system to the device coordinate system
    /// \param patternGraphicState Pattern graphic state
    /// \param cms Color management system
    /// \param intent Rendering intent
    /// \param reporter Error reporter
    /// \param ignoreBackgroundColor If set, then ignores background color, even if it is present
    static PDFPatternPtr createShadingPattern(const PDFDictionary* colorSpaceDictionary,
                                              const PDFDocument* document,
                                              const PDFObject& shadingObject,
                                              const QTransform& matrix,
                                              const PDFObject& patternGraphicState,
                                              const PDFCMS* cms,
                                              RenderingIntent intent,
                                              PDFRenderErrorReporter* reporter,
                                              bool ignoreBackgroundColor);

protected:
    QRectF m_boundingBox;
    QTransform m_matrix;
};

class PDFInvalidPattern : public PDFPattern
{
public:
    explicit PDFInvalidPattern() = default;

    virtual PatternType getType() const override { return PatternType::Invalid; }
    virtual const PDFShadingPattern* getShadingPattern() const override { return nullptr; }
    virtual const PDFTilingPattern* getTilingPattern() const override { return nullptr; }
};

class PDFTilingPattern : public PDFPattern
{
public:
    explicit PDFTilingPattern() = default;

    virtual PatternType getType() const override { return PatternType::Tiling; }
    virtual const PDFShadingPattern* getShadingPattern() const override { return nullptr; }
    virtual const PDFTilingPattern* getTilingPattern() const override { return this; }

    enum class PaintType
    {
        Colored = 1,
        Uncolored = 2,
        Invalid = 3
    };

    enum class TilingType
    {
        ConstantSpacing = 1,
        NoDistortion = 2,
        ConstantSpacingAndFasterTiling = 3,
        Invalid
    };

    PaintType getPaintingType() const { return m_paintType; }
    TilingType getTilingType() const { return m_tilingType; }
    PDFReal getXStep() const { return m_xStep; }
    PDFReal getYStep() const { return m_yStep; }
    const PDFObject& getResources() const { return m_resources; }
    const QByteArray& getContent() const { return m_content; }

private:
    friend class PDFPattern;

    PaintType m_paintType = PaintType::Colored;
    TilingType m_tilingType = TilingType::ConstantSpacing;
    PDFReal m_xStep = 0.0;
    PDFReal m_yStep = 0.0;
    PDFObject m_resources;
    QByteArray m_content;
};

/// Compute color of sample points from shading pattern. Some sampler implementation
/// uses numerical algorithms (such as Newton-Raphson method for type 6/7 shading), so
/// calculation can be very slow for some types of shadings.
class PDFShadingSampler
{
public:
    explicit inline PDFShadingSampler(const PDFShadingPattern* pattern) :
        m_pattern(pattern)
    {

    }
    virtual ~PDFShadingSampler() = default;

    /// Try to compute color of the point in device space coordinates in the shading. If color
    /// can't be computed, then false is returned, otherwise true is returned.
    /// \param devicePoint Point in device space coordinates
    /// \param outputBuffer Color output buffer (where computed color is stored)
    /// \param limit Maximal number of the steps of numerical calculation algorithms (for type 6/7 shading only)
    virtual bool sample(const QPointF& devicePoint, PDFColorBuffer outputBuffer, int limit) const = 0;

    /// Fill background color to the output buffer. If the background color is not filled,
    /// or is invalid, then false is returned, otherwise true is returned.
    bool fillBackgroundColor(PDFColorBuffer outputBuffer) const;

protected:
    const PDFShadingPattern* m_pattern;
};

/// Shading pattern - smooth color distribution along the pattern's space
class PDFShadingPattern : public PDFPattern
{
public:
    explicit PDFShadingPattern() = default;

    virtual PatternType getType() const override;
    virtual ShadingType getShadingType() const = 0;
    virtual const PDFShadingPattern* getShadingPattern() const override { return this; }
    virtual const PDFTilingPattern* getTilingPattern() const override { return nullptr; }

    /// Creates a colored mesh using settings. Mesh is generated in device space
    /// coordinate system. You must transform the mesh, if you want to
    /// use it in another coordinate system.
    /// \param settings Meshing settings
    /// \param cms Color management system
    /// \param intent Rendering intent
    /// \param reporter Error reporter
    /// \param operationControl Operation control
    virtual PDFMesh createMesh(const PDFMeshQualitySettings& settings,
                               const PDFCMS* cms, RenderingIntent intent,
                               PDFRenderErrorReporter* reporter,
                               const PDFOperationControl* operationControl) const = 0;

    /// Returns patterns graphic state. This state must be applied before
    /// the shading pattern is painted to the target device.
    const PDFObject& getPatternGraphicState() const { return m_patternGraphicState; }

    /// Returns color space of the pattern.
    const PDFAbstractColorSpace* getColorSpace() const;

    /// Returns color space pointer
    const PDFColorSpacePointer& getColorSpacePtr() const { return m_colorSpace; }

    /// Returns patterns background color (if pattern has background color).
    /// If pattern has not background color, then invalid color is returned.
    const QColor& getBackgroundColor() const { return m_backgroundColor; }

    /// Returns original background color (in color space of the shading pattern)
    const PDFColor& getOriginalBackgroundColor() const { return m_originalBackgroundColor; }

    /// Returns true, if shading pattern should be anti-aliased
    bool isAntialiasing() const { return m_antiAlias; }

    /// Returns matrix transforming pattern space to device space
    QTransform getPatternSpaceToDeviceSpaceMatrix(const PDFMeshQualitySettings& settings) const;

    /// Returns matrix transforming pattern space to device space
    QTransform getPatternSpaceToDeviceSpaceMatrix(const QTransform& userSpaceToDeviceSpaceMatrix) const;

    /// Create sampler which can compute shading colors in device space coordinates. If sampler can't
    /// be created (or shading is invalid), then nullptr is returned.
    /// \param userSpaceToDeviceSpaceMatrix Matrix, which transforms user space points
    ///        (user space is target space of the shading) to the device space of the paint device.
    virtual PDFShadingSampler* createSampler(QTransform userSpaceToDeviceSpaceMatrix) const;

protected:
    friend class PDFPattern;

    PDFObject m_patternGraphicState;
    PDFColorSpacePointer m_colorSpace;
    QColor m_backgroundColor;
    PDFColor m_originalBackgroundColor;
    bool m_antiAlias = false;
};

class PDFSingleDimensionShading : public PDFShadingPattern
{
public:
    explicit PDFSingleDimensionShading() = default;

    const std::vector<PDFFunctionPtr>& getFunctions() const { return m_functions; }
    const QPointF& getStartPoint() const { return m_startPoint; }
    const QPointF& getEndPoint() const { return m_endPoint; }
    PDFReal getDomainStart() const { return m_domainStart; }
    PDFReal getDomainEnd() const { return m_domainEnd; }
    bool isExtendStart() const { return m_extendStart; }
    bool isExtendEnd() const { return m_extendEnd; }

protected:
    friend class PDFPattern;

    std::vector<PDFFunctionPtr> m_functions;
    QPointF m_startPoint;
    QPointF m_endPoint;
    PDFReal m_domainStart = 0.0;
    PDFReal m_domainEnd = 1.0;
    bool m_extendStart = false;
    bool m_extendEnd = false;
};

class PDFFunctionShading : public PDFShadingPattern
{
public:
    explicit PDFFunctionShading() = default;

    virtual ShadingType getShadingType() const override;
    virtual PDFMesh createMesh(const PDFMeshQualitySettings& settings,
                               const PDFCMS* cms,
                               RenderingIntent intent,
                               PDFRenderErrorReporter* reporter,
                               const PDFOperationControl* operationControl) const override;
    virtual PDFShadingSampler* createSampler(QTransform userSpaceToDeviceSpaceMatrix) const override;

    const QRectF& getDomain() const { return m_domain; }
    const QTransform& getDomainToTargetTransform() const { return m_domainToTargetTransform; }
    const std::vector<PDFFunctionPtr>& getFunctions() const { return m_functions; }

private:
    friend class PDFPattern;

    QRectF m_domain; ///< Domain of the color function
    QTransform m_domainToTargetTransform; ///< Transformation mapping from domain to shading coordinate space
    std::vector<PDFFunctionPtr> m_functions; ///< Color functions
};

class PDFAxialShading : public PDFSingleDimensionShading
{
public:
    explicit PDFAxialShading() = default;

    virtual ShadingType getShadingType() const override;
    virtual PDFMesh createMesh(const PDFMeshQualitySettings& settings,
                               const PDFCMS* cms,
                               RenderingIntent intent,
                               PDFRenderErrorReporter* reporter,
                               const PDFOperationControl* operationControl) const override;
    virtual PDFShadingSampler* createSampler(QTransform userSpaceToDeviceSpaceMatrix) const override;

private:
    friend class PDFPattern;
};

class PDFRadialShading : public PDFSingleDimensionShading
{
public:
    explicit PDFRadialShading() = default;

    virtual ShadingType getShadingType() const override;
    virtual PDFMesh createMesh(const PDFMeshQualitySettings& settings,
                               const PDFCMS* cms,
                               RenderingIntent intent,
                               PDFRenderErrorReporter* reporter,
                               const PDFOperationControl* operationControl) const override;
    virtual PDFShadingSampler* createSampler(QTransform userSpaceToDeviceSpaceMatrix) const override;

    PDFReal getR0() const { return m_r0; }
    PDFReal getR1() const { return m_r1; }

private:
    friend class PDFPattern;

    PDFReal m_r0 = 0.0;
    PDFReal m_r1 = 0.0;
};

class PDFType4567Shading : public PDFShadingPattern
{
public:
    explicit PDFType4567Shading() = default;

    /// Returns color for given color or function parameter
    PDFColor getColor(PDFColor colorOrFunctionParameter) const;

protected:
    friend class PDFPattern;

    void addSubdividedTriangles(const PDFMeshQualitySettings& settings,
                                PDFMesh& mesh,
                                uint32_t v1,
                                uint32_t v2,
                                uint32_t v3,
                                PDFColor c1,
                                PDFColor c2,
                                PDFColor c3,
                                const PDFCMS* cms,
                                RenderingIntent intent,
                                PDFRenderErrorReporter* reporter) const;

    uint8_t m_bitsPerCoordinate = 0;
    uint8_t m_bitsPerComponent = 0;
    uint8_t m_bitsPerFlag = 0;
    PDFReal m_xmin = 0.0;
    PDFReal m_xmax = 0.0;
    PDFReal m_ymin = 0.0;
    PDFReal m_ymax = 0.0;
    std::vector<PDFReal> m_limits;
    size_t m_colorComponentCount = 0;

    /// Color functions. This array can be empty. If it is empty,
    /// then colors should be determined directly from color space.
    std::vector<PDFFunctionPtr> m_functions;

    /// Data of the shading, containing triangles and colors
    QByteArray m_data;
};

class PDFFreeFormGouradTriangleShading : public PDFType4567Shading
{
public:
    explicit PDFFreeFormGouradTriangleShading() = default;

    virtual ShadingType getShadingType() const override;
    virtual PDFMesh createMesh(const PDFMeshQualitySettings& settings,
                               const PDFCMS* cms,
                               RenderingIntent intent,
                               PDFRenderErrorReporter* reporter,
                               const PDFOperationControl* operationControl) const override;
    virtual PDFShadingSampler* createSampler(QTransform userSpaceToDeviceSpaceMatrix) const override;

private:
    struct VertexData
    {
        uint32_t index = 0;
        uint8_t flags = 0;
        QPointF position;
        PDFColor color;
    };

    friend class PDFPattern;

    using InitializeFunction = std::function<void(std::vector<QPointF>&&, size_t)>;
    using AddTriangleFunction = std::function<void(const VertexData*, const VertexData*, const VertexData*)>;

    bool processTriangles(InitializeFunction initializeMeshFunction,
                          AddTriangleFunction addTriangle,
                          const QTransform& userSpaceToDeviceSpaceMatrix,
                          bool convertColors) const;
};

class PDFLatticeFormGouradTriangleShading : public PDFType4567Shading
{
public:
    explicit PDFLatticeFormGouradTriangleShading() = default;

    virtual ShadingType getShadingType() const override;
    virtual PDFMesh createMesh(const PDFMeshQualitySettings& settings,
                               const PDFCMS* cms,
                               RenderingIntent intent,
                               PDFRenderErrorReporter* reporter,
                               const PDFOperationControl* operationControl) const override;
    virtual PDFShadingSampler* createSampler(QTransform userSpaceToDeviceSpaceMatrix) const override;

private:
    friend class PDFPattern;

    struct VertexData
    {
        uint32_t index = 0;
        QPointF position;
        PDFColor color;
    };

    using InitializeFunction = std::function<void(std::vector<QPointF>&&, size_t)>;
    using AddTriangleFunction = std::function<void(const VertexData*, const VertexData*, const VertexData*)>;

    bool processTriangles(InitializeFunction initializeMeshFunction,
                          AddTriangleFunction addTriangle,
                          const QTransform& userSpaceToDeviceSpaceMatrix,
                          bool convertColors) const;

    PDFInteger m_verticesPerRow = 0;
};

class PDFTensorPatch
{
public:
    enum ColorIndex
    {
        C_00 = 0,
        C_03 = 1,
        C_33 = 2,
        C_30 = 3
    };

    using PointMatrix = std::array<std::array<QPointF, 4>, 4>;
    using Colors = std::array<PDFColor, 4>;

    explicit inline PDFTensorPatch(PointMatrix P, Colors colors) : m_P(P), m_colors(colors) { computeBoundingRectangle(); }

    /// Calculates value at point in the patch.
    /// \param u Horizontal coordinate of the patch, must be in range [0, 1]
    /// \param v Vertical coordinate of the patch, must be in range [0, 1]
    QPointF getValue(PDFReal u, PDFReal v) const;

    /// Calculates value at point in the patch, possibly derivation, if derivation
    /// variables are positive.
    /// \param u Horizontal coordinate of the patch, must be in range [0, 1]
    /// \param v Vertical coordinate of the patch, must be in range [0, 1]
    /// \param derivativeOrderU Derivation order in direction u (0 means no derivation)
    /// \param derivativeOrderV Derivation order in direction v (0 means no derivation)
    QPointF getValue(PDFReal u, PDFReal v, int derivativeOrderU, int derivativeOrderV) const;

    /// Tries to find value (u,v) using Newton-Raphson numerical algorithm method.
    /// This can fail and when it fails, false is returned. As initial point, input uv
    /// is considered. Also, if maximal number of steps is reached, false is returned.
    /// \param[in,out] u Output value of u (passed value is used as a seed)
    /// \param[in,out] v Output value of v (passed value is used as a seed)
    /// \param x Input x coordinate, for which we want to compute (u, v)
    /// \param y Input y coordinate, for which we want to compute (u, v)
    /// \param epsilon Epsilon, when distance from the getValue(u, v) and (x, y)
    ///                is less than epsilon, convergence is reached
    /// \param maximalNumberOfSteps Maximal number of steps of Newton-Raphson algorithm
    bool getUV(PDFReal& u, PDFReal& v, PDFReal x, PDFReal y, PDFReal epsilon, int maximalNumberOfSteps) const;

    /// Calculates first derivate in the surface, in the direction of variable u.
    /// \param u Horizontal coordinate of the patch, must be in range [0, 1]
    /// \param v Vertical coordinate of the patch, must be in range [0, 1]
    QPointF getDerivative_u(PDFReal u, PDFReal v) const { return getValue(u, v, 1, 0); }

    /// Calculates second derivate in the surface, in the direction of variable u.
    /// \param u Horizontal coordinate of the patch, must be in range [0, 1]
    /// \param v Vertical coordinate of the patch, must be in range [0, 1]
    QPointF getDerivative_uu(PDFReal u, PDFReal v) const { return getValue(u, v, 2, 0); }

    /// Calculates first derivate in the surface, in the direction of variable v.
    /// \param u Horizontal coordinate of the patch, must be in range [0, 1]
    /// \param v Vertical coordinate of the patch, must be in range [0, 1]
    QPointF getDerivative_v(PDFReal u, PDFReal v) const { return getValue(u, v, 0, 1); }

    /// Calculates second derivate in the surface, in the direction of variable v.
    /// \param u Horizontal coordinate of the patch, must be in range [0, 1]
    /// \param v Vertical coordinate of the patch, must be in range [0, 1]
    QPointF getDerivative_vv(PDFReal u, PDFReal v) const { return getValue(u, v, 0, 2); }

    /// Calculates curvature of the given point in the surface, in the direction of u.
    /// \param u Horizontal coordinate of the patch, must be in range [0, 1]
    /// \param v Vertical coordinate of the patch, must be in range [0, 1]
    PDFReal getCurvature_u(PDFReal u, PDFReal v) const;

    /// Calculates curvature of the given point in the surface, in the direction of v.
    /// \param u Horizontal coordinate of the patch, must be in range [0, 1]
    /// \param v Vertical coordinate of the patch, must be in range [0, 1]
    PDFReal getCurvature_v(PDFReal u, PDFReal v) const;

    /// Returns matrix of control points
    const PointMatrix& getP() const { return m_P; }

    /// Returns colors of corner points
    const Colors& getColors() const { return m_colors; }

private:
    /// Computes Bernstein polynomial B0, B1, B2, B3, for parameter t.
    /// If \p derivative is zero, then it evaluates polynomial's value,
    /// otherwise it computes value of the derivation of Bx, up to degree 3
    /// derivation.
    /// \param index Index of polynomial, from 0 to 3 (B0, B1, B2, B3)
    /// \param t Parameter of polynomial function
    /// \param derivativeOrder Derivative order (0 - value, 1 - first derivation, 2 - second derivation, 3 - third derivation)
    static constexpr PDFReal B(int index, PDFReal t, int derivativeOrder);

    /// Computes Bernstein polynomial B0 for parameter t.
    /// If \p derivative is zero, then it evaluates polynomial's value,
    /// otherwise it computes value of the derivation of B0, up to degree 3
    /// derivation.
    /// \param t Parameter of polynomial function
    /// \param derivativeOrder Derivative order (0 - value, 1 - first derivation, 2 - second derivation, 3 - third derivation)
    static constexpr PDFReal B0(PDFReal t, int derivative);

    /// Computes Bernstein polynomial B1 for parameter t.
    /// If \p derivative is zero, then it evaluates polynomial's value,
    /// otherwise it computes value of the derivation of B1, up to degree 3
    /// derivation.
    /// \param t Parameter of polynomial function
    /// \param derivativeOrder Derivative order (0 - value, 1 - first derivation, 2 - second derivation, 3 - third derivation)
    static constexpr PDFReal B1(PDFReal t, int derivative);

    /// Computes Bernstein polynomial B2 for parameter t.
    /// If \p derivative is zero, then it evaluates polynomial's value,
    /// otherwise it computes value of the derivation of B2, up to degree 3
    /// derivation.
    /// \param t Parameter of polynomial function
    /// \param derivativeOrder Derivative order (0 - value, 1 - first derivation, 2 - second derivation, 3 - third derivation)
    static constexpr PDFReal B2(PDFReal t, int derivative);

    /// Computes Bernstein polynomial B3 for parameter t.
    /// If \p derivative is zero, then it evaluates polynomial's value,
    /// otherwise it computes value of the derivation of B3, up to degree 3
    /// derivation.
    /// \param t Parameter of polynomial function
    /// \param derivativeOrder Derivative order (0 - value, 1 - first derivation, 2 - second derivation, 3 - third derivation)
    static constexpr PDFReal B3(PDFReal t, int derivative);

    static constexpr PDFReal pow2(PDFReal x) { return x * x; }
    static constexpr PDFReal pow3(PDFReal x) { return x * x * x; }

    void computeBoundingRectangle();

    PointMatrix m_P = { };
    Colors m_colors = { };
    QRectF m_boundingBox; ///< Bounding box of the tensor product patch
};

using PDFTensorPatches = std::vector<PDFTensorPatch>;

class PDFTensorProductPatchShadingBase : public PDFType4567Shading
{
public:
    explicit inline PDFTensorProductPatchShadingBase() = default;

    virtual PDFShadingSampler* createSampler(QTransform userSpaceToDeviceSpaceMatrix) const override;
    virtual PDFTensorPatches createPatches(QTransform userSpaceToDeviceSpaceMatrix, bool transformColor) const = 0;

protected:
    struct Triangle;

    void fillMesh(PDFMesh& mesh, const PDFMeshQualitySettings& settings, const PDFTensorPatch& patch, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter, bool fastAlgorithm, const PDFOperationControl* operationControl) const;
    void fillMesh(PDFMesh& mesh, const QTransform& patternSpaceToDeviceSpaceMatrix, const PDFMeshQualitySettings& settings, const PDFTensorPatches& patches, const PDFCMS* cms, RenderingIntent intent, PDFRenderErrorReporter* reporter, const PDFOperationControl* operationControl) const;
    static void addTriangle(std::vector<Triangle>& triangles, const PDFTensorPatch& patch, std::array<QPointF, 3> uvCoordinates);

private:
    friend class PDFPattern;
};

class PDFCoonsPatchShading : public PDFTensorProductPatchShadingBase
{
public:
    explicit PDFCoonsPatchShading() = default;

    virtual ShadingType getShadingType() const override;
    virtual PDFMesh createMesh(const PDFMeshQualitySettings& settings,
                               const PDFCMS* cms,
                               RenderingIntent intent,
                               PDFRenderErrorReporter* reporter,
                               const PDFOperationControl* operationControl) const override;
    virtual PDFTensorPatches createPatches(QTransform userSpaceToDeviceSpaceMatrix, bool transformColor) const override;

private:
    friend class PDFPattern;
};

class PDFTensorProductPatchShading : public PDFTensorProductPatchShadingBase
{
public:
    explicit PDFTensorProductPatchShading() = default;

    virtual ShadingType getShadingType() const override;
    virtual PDFMesh createMesh(const PDFMeshQualitySettings& settings,
                               const PDFCMS* cms,
                               RenderingIntent intent,
                               PDFRenderErrorReporter* reporter,
                               const PDFOperationControl* operationControl) const override;
    virtual PDFTensorPatches createPatches(QTransform userSpaceToDeviceSpaceMatrix, bool transformColor) const override;

private:
    friend class PDFPattern;
};

}   // namespace pdf

#endif // PDFPATTERN_H
