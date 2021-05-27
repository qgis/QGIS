// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#include <Graphics/GTGraphicsPCH.h>
#include <Graphics/Terrain.h>
#include <Mathematics/Logger.h>
using namespace gte;

Terrain::Terrain(size_t numRows, size_t numCols, size_t size, float minElevation,
    float maxElevation, float spacing, VertexFormat const& vformat,
    std::shared_ptr<Camera> camera)
    :
    mNumRows(numRows),
    mNumCols(numCols),
    mSize(size),
    mMinElevation(minElevation),
    mMaxElevation(maxElevation),
    mSpacing(spacing),
    mLength(mSpacing * (static_cast<float>(size) - 1.0f)),
    mCameraRow(std::numeric_limits<size_t>::max()),
    mCameraCol(std::numeric_limits<size_t>::max()),
    mCamera(camera)
{
    LogAssert(numRows > 0 && numCols > 0, "Invalid number of rows or columns.");
    LogAssert(mSize == 3 || mSize == 5 || mSize == 9 || mSize == 17
        || mSize == 33 || mSize == 65 || mSize == 129, "Invalid page size.");
    LogAssert(minElevation <= maxElevation, "Invalid ordering of elevation extremes.");
    LogAssert(spacing > 0.0f, "Spacing must be positive.");

    int index = vformat.GetIndex(VA_POSITION, 0);
    LogAssert(index >= 0, "Vertex format does not have VA_POSITION.");

    DFType type = vformat.GetType(index);
    LogAssert(type == DF_R32G32B32_FLOAT || type == DF_R32G32B32A32_FLOAT,
        "VertexFormat type is not supported.");

    unsigned int offset = vformat.GetOffset(index);
    LogAssert(offset == 0, "VertexFormat offset must be 0.");

    LogAssert(mCamera != nullptr, "Camera must exist.");

    // The child array of the terrain is resized to store the maximum
    // number of terrain pages.  The heights of the pages themselves
    // must be loaded with calls to CreatePage for each row and column.
    // The default heights are all the minimum elevation.
    mChild.resize(mNumRows * mNumCols);
    for (size_t row = 0; row < mNumRows; ++row)
    {
        for (size_t col = 0; col < mNumCols; ++col)
        {
            Vector2<float> origin{ col * mLength, row * mLength };
            auto page = std::make_shared<Page>(mSize, mMinElevation,
                mMaxElevation, mSpacing, mLength, origin, vformat);
            AttachChild(page);
        }
    }
}

std::shared_ptr<Visual> Terrain::GetPage(size_t row, size_t col) const
{
    LogAssert(row < mNumRows && col < mNumCols, "Invalid input to GetPage.");
    auto child = mChild[col + mNumCols * row];
    auto page = std::dynamic_pointer_cast<Visual>(child);
    return page;
}

void Terrain::SetHeights(size_t row, size_t col, std::vector<unsigned short> const& heights)
{
    LogAssert(row < mNumRows && col < mNumCols && heights.size() >= mSize * mSize,
        "Invalid input to SetHeights.");
    auto child = mChild[col + mNumCols * row];
    auto page = std::dynamic_pointer_cast<Page>(child);
    page->SetHeights(heights);
}

std::vector<unsigned short> const& Terrain::GetHeights(size_t row, size_t col) const
{
    LogAssert(row < mNumRows && col < mNumCols, "Invalid input to GetHeights.");
    auto child = mChild[col + mNumCols * row];
    auto page = std::dynamic_pointer_cast<Page>(child);
    return page->GetHeights();
}

float Terrain::GetHeight(float x, float y) const
{
    // Subtract off the translation due to wrap-around.
    auto page = GetPage(x, y);
    Vector3<float> trn = page->localTransform.GetTranslation();
    return page->GetHeight(x - trn[0], y - trn[1]);
}

Vector3<float> Terrain::GetNormal(float x, float y) const
{
    float xp = x + mSpacing;
    float xm = x - mSpacing;
    float yp = y + mSpacing;
    float ym = y - mSpacing;

    auto page = GetPage(xp, y);
    Vector3<float> trn = page->localTransform.GetTranslation();
    float hpz = page->GetHeight(xp - trn[0], y - trn[1]);

    page = GetPage(xm, y);
    trn = page->localTransform.GetTranslation();
    float hmz = page->GetHeight(xm - trn[0], y - trn[1]);

    page = GetPage(x, yp);
    trn = page->localTransform.GetTranslation();
    float hzp = page->GetHeight(x - trn[0], yp - trn[1]);

    page = GetPage(x, ym);
    trn = page->localTransform.GetTranslation();
    float hzm = page->GetHeight(x - trn[0], ym - trn[1]);

    Vector3<float> normal{ hmz - hpz, hzm - hzp, 1.0f };
    Normalize(normal);
    return normal;
}

void Terrain::OnCameraMotion()
{
    // Get the camera location/direction in the model space of the terrain.
    Vector4<float> worldEye = mCamera->GetPosition();
#if defined(GTE_USE_MAT_VEC)
    Vector4<float> modelEye = worldTransform.Inverse() * worldEye;
#else
    Vector4<float> modelEye = worldEye * worldTransform.Inverse();
#endif

    // Update the model-space origins of the terrain pages.  Start the
    // process by locating the page that contains the camera.
    size_t newCameraCol = static_cast<size_t>(std::floor(modelEye[0] / mLength));
    size_t newCameraRow = static_cast<size_t>(std::floor(modelEye[1] / mLength));
    if (newCameraCol != mCameraCol || newCameraRow != mCameraRow)
    {
        mCameraCol = newCameraCol;
        mCameraRow = newCameraRow;

        // Translate page origins for toroidal wraparound.
        int cminO = static_cast<int>(mCameraCol) - static_cast<int>(mNumCols / 2);
        int cminP = (cminO + static_cast<int>(mNumCols)) % static_cast<int>(mNumCols);
        int rminO = static_cast<int>(mCameraRow) - static_cast<int>(mNumRows / 2);
        int rminP = (rminO + static_cast<int>(mNumRows)) % static_cast<int>(mNumRows);

        int rO = rminO, rP = rminP;
        for (size_t row = 0; row < mNumRows; ++row)
        {
            int cO = cminO, cP = cminP;
            for (size_t col = 0; col < mNumCols; ++col)
            {
                auto child = mChild[cP + mNumCols * rP];
                auto page = std::dynamic_pointer_cast<Page>(child);
                Vector2<float> oldOrigin = page->GetOrigin();
                Vector2<float> newOrigin{ cO * mLength, rO * mLength };
                Vector3<float> pageTrn
                {
                    newOrigin[0] - oldOrigin[0],
                    newOrigin[1] - oldOrigin[1],
                    page->localTransform.GetTranslation()[2]
                };
                page->localTransform.SetTranslation(pageTrn);

                ++cO;
                if (++cP == static_cast<int>(mNumCols))
                {
                    cP = 0;
                }
            }

            ++rO;
            if (++rP == static_cast<int>(mNumRows))
            {
                rP = 0;
            }
        }
        Update();
    }
}

Terrain::Page::Page(size_t size, float minElevation, float maxElevation,
    float spacing, float length, Vector2<float> const& origin, VertexFormat const& vformat)
    :
    mSize(size),
    mMinElevation(minElevation),
    mMaxElevation(maxElevation),
    mSpacing(spacing),
    mOrigin(origin),
    mHeights(mSize * mSize, 0)
{
    // Create a mesh for the page.  The vertices are initialized using the
    // SetHeights(...) function.
    MeshFactory mf;
    mf.SetVertexFormat(vformat);
    unsigned int numSamples = static_cast<unsigned int>(mSize);
    auto rectangle = mf.CreateRectangle(numSamples, numSamples, length, length);
    mVBuffer = rectangle->GetVertexBuffer();
    mIBuffer = rectangle->GetIndexBuffer();
}

void Terrain::Page::SetHeights(std::vector<unsigned short> const& heights)
{
    char* vertices = mVBuffer->GetData();
    size_t vertexSize = static_cast<size_t>(mVBuffer->GetFormat().GetVertexSize());
    for (size_t row = 0, i = 0; row < mSize; ++row)
    {
        float y = mOrigin[1] + mSpacing * static_cast<float>(row);
        for (size_t col = 0; col < mSize; ++col, ++i)
        {
            mHeights[i] = heights[i];
            Vector3<float>& vertex = *reinterpret_cast<Vector3<float>*>(vertices);
            vertex[0] = mOrigin[0] + mSpacing * static_cast<float>(col);
            vertex[1] = y;
            vertex[2] = GetHeight(i);
            vertices += vertexSize;
        }
    }

    UpdateModelBound();
    UpdateModelNormals();
}

float Terrain::Page::GetHeight(float x, float y) const
{
    float xGrid = (x - mOrigin[0]) / mSpacing;
    if (xGrid < 0.0f || xGrid + 1.0f >= static_cast<float>(mSize))
    {
        // Location not in page.
        return std::numeric_limits<float>::max();
    }

    float yGrid = (y - mOrigin[1]) / mSpacing;
    if (yGrid < 0.0f || yGrid + 1.0f >= static_cast<float>(mSize))
    {
        // Location not in page.
        return std::numeric_limits<float>::max();
    }

    float fCol = std::floor(xGrid);
    size_t col = static_cast<size_t>(fCol);
    float fRow = std::floor(yGrid);
    size_t row = static_cast<size_t>(fRow);

    float dx = xGrid - fCol;
    float dy = yGrid - fRow;
    float h00, h10, h01, h11, height;

    if ((col & 1) == (row & 1))
    {
        float diff = dx - dy;
        h00 = GetHeight(row, col);
        h11 = GetHeight(row + 1, col);
        if (diff > 0.0f)
        {
            h10 = GetHeight(row, col + 1);
            height = (1.0f - diff - dy) * h00 + diff * h10 + dy * h11;
        }
        else
        {
            h01 = GetHeight(row + 1, col);
            height = (1.0f + diff - dx) * h00 - diff * h01 + dx * h11;
        }
    }
    else
    {
        float sum = dx + dy;
        h10 = GetHeight(row, col + 1);
        h01 = GetHeight(row + 1, col);
        if (sum <= 1.0f)
        {
            h00 = GetHeight(row, col);
            height = (1.0f - sum) * h00 + dx * h10 + dy * h01;
        }
        else
        {
            h11 = GetHeight(row + 1, col + 1);
            height = (sum - 1.0f) * h11 + (1.0f - dy) * h10 + (1.0f - dx) * h01;
        }
    }

    return height;
}

float Terrain::Page::GetHeight(size_t i) const
{
    // The t-value is in [0,1].
    float t = static_cast<float>(mHeights[i]) / 65535.0f;
    return (1.0f - t) * mMinElevation + t * mMaxElevation;
}

float Terrain::Page::GetHeight(size_t row, size_t col) const
{
    return GetHeight(col + mSize * row);
}

std::shared_ptr<Terrain::Page> Terrain::GetPage(float x, float y) const
{
    size_t col = static_cast<size_t>(std::floor(x / mLength));
    col = (col + mNumCols) % mNumCols;
    size_t row = static_cast<size_t>(std::floor(y / mLength));
    row = (row + mNumRows) % mNumRows;
    auto child = mChild[col + mNumCols * row];
    auto page = std::dynamic_pointer_cast<Page>(child);
    return page;
}
