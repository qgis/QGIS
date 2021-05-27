// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2021
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
// https://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// Version: 4.0.2019.08.13

#pragma once

#include <Mathematics/Image2.h>
#include <cmath>
#include <functional>
#include <limits>

// Image utilities for Image2<int> objects.  TODO: Extend this to a template
// class to allow the pixel type to be int*_t and uint*_t for * in
// {8,16,32,64}.
//
// All but the Draw* functions are operations on binary images.  Let the image
// have d0 columns and d1 rows.  The input image must have zeros on its
// boundaries x = 0, x = d0-1, y = 0 and y = d1-1.  The 0-valued pixels are
// considered to be background.  The 1-valued pixels are considered to be
// foreground.  In some of the operations, to save memory and time the input
// image is modified by the algorithms.  If you need to preserve the input
// image, make a copy of it before calling these functions.  Dilation and
// erosion functions do not have the requirement that the boundary pixels of
// the binary image inputs be zero.

namespace gte
{
    class ImageUtility2
    {
    public:
        // Compute the 4-connected components of a binary image.  The input
        // image is modified to avoid the cost of making a copy.  On output,
        // the image values are the labels for the components.  The array
        // components[k], k >= 1, contains the indices for the k-th component.
        static void GetComponents4(Image2<int>& image,
            std::vector<std::vector<size_t>>& components)
        {
            std::array<int, 4> neighbors;
            image.GetNeighborhood(neighbors);
            GetComponents(4, &neighbors[0], image, components);
        }

        // Compute the 8-connected components of a binary image.  The input
        // image is modified to avoid the cost of making a copy.  On output,
        // the image values are the labels for the components.  The array
        // components[k], k >= 1, contains the indices for the k-th component.
        static void GetComponents8(Image2<int>& image,
            std::vector<std::vector<size_t>>& components)
        {
            std::array<int, 8> neighbors;
            image.GetNeighborhood(neighbors);
            GetComponents(8, &neighbors[0], image, components);
        }

        // Compute a dilation with a structuring element consisting of the
        // 4-connected neighbors of each pixel.  The input image is binary
        // with 0 for background and 1 for foreground.  The output image must
        // be an object different from the input image.
        static void Dilate4(Image2<int> const& input, Image2<int>& output)
        {
            std::array<std::array<int, 2>, 4> neighbors;
            input.GetNeighborhood(neighbors);
            Dilate(input, 4, &neighbors[0], output);
        }

        // Compute a dilation with a structuring element consisting of the
        // 8-connected neighbors of each pixel.  The input image is binary
        // with 0 for background and 1 for foreground.  The output image must
        // be an object different from the input image.
        static void Dilate8(Image2<int> const& input, Image2<int>& output)
        {
            std::array<std::array<int, 2>, 8> neighbors;
            input.GetNeighborhood(neighbors);
            Dilate(input, 8, &neighbors[0], output);
        }

        // Compute a dilation with a structing element consisting of neighbors
        // specified by offsets relative to the pixel.  The input image is
        // binary with 0 for background and 1 for foreground.  The output
        // image must be an object different from the input image.
        static void Dilate(Image2<int> const& input, int numNeighbors,
            std::array<int, 2> const* neighbors, Image2<int>& output)
        {
            LogAssert(&output != &input, "Input and output must be different.");

            output = input;

            // If the pixel at (x,y) is 1, then the pixels at (x+dx,y+dy) are
            // set to 1 where (dx,dy) is in the 'neighbors' array.  Boundary
            // testing is used to avoid accessing out-of-range pixels.
            int const dim0 = input.GetDimension(0);
            int const dim1 = input.GetDimension(1);
            for (int y = 0; y < dim1; ++y)
            {
                for (int x = 0; x < dim0; ++x)
                {
                    if (input(x, y) == 1)
                    {
                        for (int j = 0; j < numNeighbors; ++j)
                        {
                            int xNbr = x + neighbors[j][0];
                            int yNbr = y + neighbors[j][1];
                            if (0 <= xNbr && xNbr < dim0 && 0 <= yNbr && yNbr < dim1)
                            {
                                output(xNbr, yNbr) = 1;
                            }
                        }
                    }
                }
            }
        }

        // Compute an erosion with a structuring element consisting of the
        // 4-connected neighbors of each pixel.  The input image is binary 
        // with 0 for background and 1 for foreground.  The output image must
        // be an object different from the input image.  If zeroExterior is
        // true, the image exterior is assumed to be 0, so 1-valued boundary
        // pixels are set to 0; otherwise, boundary pixels are set to 0 only
        // when they have neighboring image pixels that are 0.
        static void Erode4(Image2<int> const& input, bool zeroExterior, Image2<int>& output)
        {
            std::array<std::array<int, 2>, 4> neighbors;
            input.GetNeighborhood(neighbors);
            Erode(input, zeroExterior, 4, &neighbors[0], output);
        }

        // Compute an erosion with a structuring element consisting of the
        // 8-connected neighbors of each pixel.  The input image is binary
        // with 0 for background and 1 for foreground.  The output image must
        // be an object different from the input image.  If zeroExterior is
        // true, the image exterior is assumed to be 0, so 1-valued boundary
        // pixels are set to 0; otherwise, boundary pixels are set to 0 only
        // when they have neighboring image pixels that are 0.
        static void Erode8(Image2<int> const& input, bool zeroExterior, Image2<int>& output)
        {
            std::array<std::array<int, 2>, 8> neighbors;
            input.GetNeighborhood(neighbors);
            Erode(input, zeroExterior, 8, &neighbors[0], output);
        }

        // Compute an erosion with a structuring element consisting of
        // neighbors specified by offsets relative to the pixel.  The input
        // image is binary with 0 for background and 1 for foreground.  The
        // output image must be an object different from the input image.  If
        // zeroExterior is true, the image exterior is assumed to be 0, so
        // 1-valued boundary pixels are set to 0; otherwise, boundary pixels
        // are set to 0 only when they have neighboring image pixels that
        // are 0.
        static void Erode(Image2<int> const& input, bool zeroExterior,
            int numNeighbors, std::array<int, 2> const* neighbors, Image2<int>& output)
        {
            LogAssert(&output != &input, "Input and output must be different.");

            output = input;

            // If the pixel at (x,y) is 1, it is changed to 0 when at least
            // one neighbor (x+dx,y+dy) is 0, where (dx,dy) is in the
            // 'neighbors' array.
            int const dim0 = input.GetDimension(0);
            int const dim1 = input.GetDimension(1);
            for (int y = 0; y < dim1; ++y)
            {
                for (int x = 0; x < dim0; ++x)
                {
                    if (input(x, y) == 1)
                    {
                        for (int j = 0; j < numNeighbors; ++j)
                        {
                            int xNbr = x + neighbors[j][0];
                            int yNbr = y + neighbors[j][1];
                            if (0 <= xNbr && xNbr < dim0 && 0 <= yNbr && yNbr < dim1)
                            {
                                if (input(xNbr, yNbr) == 0)
                                {
                                    output(x, y) = 0;
                                    break;
                                }
                            }
                            else if (zeroExterior)
                            {
                                output(x, y) = 0;
                                break;
                            }
                        }
                    }
                }
            }
        }

        // Compute an opening with a structuring element consisting of the
        // 4-connected neighbors of each pixel.  The input image is binary
        // with 0 for background and 1 for foreground.  The output image must
        // be an object different from the input image.  If zeroExterior is
        // true, the image exterior is assumed to consist of 0-valued pixels;
        // otherwise, the image exterior is assumed to consist of 1-valued
        // pixels.
        static void Open4(Image2<int> const& input, bool zeroExterior, Image2<int>& output)
        {
            Image2<int> temp(input.GetDimension(0), input.GetDimension(1));
            Erode4(input, zeroExterior, temp);
            Dilate4(temp, output);
        }

        // Compute an opening with a structuring element consisting of the
        // 8-connected neighbors of each pixel.  The input image is binary
        // with 0 for background and 1 for foreground.  The output image must
        // be an object different from the input image.  If zeroExterior is
        // true, the image exterior is assumed to consist of 0-valued pixels;
        // otherwise, the image exterior is assumed to consist of 1-valued
        // pixels.
        static void Open8(Image2<int> const& input, bool zeroExterior, Image2<int>& output)
        {
            Image2<int> temp(input.GetDimension(0), input.GetDimension(1));
            Erode8(input, zeroExterior, temp);
            Dilate8(temp, output);
        }

        // Compute an opening with a structuring element consisting of
        // neighbors specified by offsets relative to the pixel.  The input
        // image is binary with 0 for background and 1 for foreground.  The
        // output image must be an object different from the input image.  If
        // zeroExterior is true, the image exterior is assumed to consist of
        // 0-valued pixels; otherwise, the image exterior is assumed to
        // consist of 1-valued pixels.
        static void Open(Image2<int> const& input, bool zeroExterior,
            int numNeighbors, std::array<int, 2> const* neighbors, Image2<int>& output)
        {
            Image2<int> temp(input.GetDimension(0), input.GetDimension(1));
            Erode(input, zeroExterior, numNeighbors, neighbors, temp);
            Dilate(temp, numNeighbors, neighbors, output);
        }

        // Compute a closing with a structuring element consisting of the
        // 4-connected neighbors of each pixel.  The input image is binary
        // with 0 for background and 1 for foreground.  The output image must
        // be an object different from the input image.  If zeroExterior is
        // true, the image exterior is assumed to consist of 0-valued pixels;
        // otherwise, the image exterior is assumed to consist of 1-valued
        // pixels.
        static void Close4(Image2<int> const& input, bool zeroExterior, Image2<int>& output)
        {
            Image2<int> temp(input.GetDimension(0), input.GetDimension(1));
            Dilate4(input, temp);
            Erode4(temp, zeroExterior, output);
        }

        // Compute a closing with a structuring element consisting of the
        // 8-connected neighbors of each pixel.  The input image is binary
        // with 0 for background and 1 for foreground.  The output image must
        // be an object different from the input image.  If zeroExterior is
        // true, the image exterior is assumed to consist of 0-valued pixels;
        // otherwise, the image exterior is assumed to consist of 1-valued
        // pixels.
        static void Close8(Image2<int> const& input, bool zeroExterior, Image2<int>& output)
        {
            Image2<int> temp(input.GetDimension(0), input.GetDimension(1));
            Dilate8(input, temp);
            Erode8(temp, zeroExterior, output);
        }

        // Compute a closing with a structuring element consisting of
        // neighbors specified by offsets relative to the pixel.  The input
        // image is binary with 0 for background and 1 for foreground.  The
        // output image must be an object different from the input image.  If
        // zeroExterior is true, the image exterior is assumed to consist of
        // 0-valued pixels; otherwise, the image exterior is assumed to
        // consist of 1-valued pixels.
        static void Close(Image2<int> const& input, bool zeroExterior,
            int numNeighbors, std::array<int, 2> const* neighbors, Image2<int>& output)
        {
            Image2<int> temp(input.GetDimension(0), input.GetDimension(1));
            Dilate(input, numNeighbors, neighbors, temp);
            Erode(temp, zeroExterior, numNeighbors, neighbors, output);
        }

        // Locate a pixel and walk around the edge of a component.  The input
        // (x,y) is where the search starts for a nonzero pixel.  If (x,y) is
        // outside the component, the walk is around the outside the
        // component.  If the component has a hole and (x,y) is inside that
        // hole, the walk is around the boundary surrounding the hole.  The
        // function returns 'true' on a success walk.  The return value is
        // 'false' when no boundary was found from the starting (x,y).
        static bool ExtractBoundary(int x, int y, Image2<int>& image, std::vector<size_t>& boundary)
        {
            // Find a first boundary pixel.
            size_t const numPixels = image.GetNumPixels();
            size_t i;
            for (i = image.GetIndex(x, y); i < numPixels; ++i)
            {
                if (image[i])
                {
                    break;
                }
            }
            if (i == numPixels)
            {
                // No boundary pixel found.
                return false;
            }

            std::array<int, 8> const dx = { -1,  0, +1, +1, +1,  0, -1, -1 };
            std::array<int, 8> const dy = { -1, -1, -1,  0, +1, +1, +1,  0 };

            // Create a new point list that contains the first boundary point.
            boundary.push_back(i);

            // The direction from background 0 to boundary pixel 1 is
            // (dx[7],dy[7]).
            std::array<int, 2> coord = image.GetCoordinates(i);
            int x0 = coord[0], y0 = coord[1];
            int cx = x0, cy = y0;
            int nx = x0 - 1, ny = y0, dir = 7;

            // Traverse the boundary in clockwise order.  Mark visited pixels
            // as 2.
            image(cx, cy) = 2;
            bool notDone = true;
            while (notDone)
            {
                int j, nbr;
                for (j = 0, nbr = dir; j < 8; ++j, nbr = (nbr + 1) % 8)
                {
                    nx = cx + dx[nbr];
                    ny = cy + dy[nbr];
                    if (image(nx, ny))  // next boundary pixel found
                    {
                        break;
                    }
                }

                if (j == 8)
                {
                    // (cx,cy) is isolated
                    notDone = false;
                    continue;
                }

                if (nx == x0 && ny == y0)
                {
                    // boundary traversal completed
                    notDone = false;
                    continue;
                }

                // (nx,ny) is next boundary point, add point to list.  Note
                // that the index for the pixel is computed for the original
                // image, not for the larger temporary image.
                boundary.push_back(image.GetIndex(nx, ny));

                // Mark visited pixels as 2.
                image(nx, ny) = 2;

                // Start search for next point.
                cx = nx;
                cy = ny;
                dir = (j + 5 + dir) % 8;
            }

            return true;
        }

        // Use a depth-first search for filling a 4-connected region.  This is
        // nonrecursive, simulated by using a heap-allocated "stack".  The
        // input (x,y) is the seed point that starts the fill.
        template <typename PixelType>
        static void FloodFill4(Image2<PixelType>& image, int x, int y,
            PixelType foreColor, PixelType backColor)
        {
            // Test for a valid seed.
            int const dim0 = image.GetDimension(0);
            int const dim1 = image.GetDimension(1);
            if (x < 0 || x >= dim0 || y < 0 || y >= dim1)
            {
                // The seed point is outside the image domain, so there is
                // nothing to fill.
                return;
            }

            // Allocate the maximum amount of space needed for the stack.
            // An empty stack has top == -1.
            size_t const numPixels = image.GetNumPixels();
            std::vector<int> xStack(numPixels), yStack(numPixels);

            // Push seed point onto stack if it has the background color.  All
            // points pushed onto stack have background color backColor.
            int top = 0;
            xStack[top] = x;
            yStack[top] = y;

            while (top >= 0)  // stack is not empty
            {
                // Read top of stack.  Do not pop since we need to return to
                // this top value later to restart the fill in a different
                // direction.
                x = xStack[top];
                y = yStack[top];

                // Fill the pixel.
                image(x, y) = foreColor;

                int xp1 = x + 1;
                if (xp1 < dim0 && image(xp1, y) == backColor)
                {
                    // Push pixel with background color.
                    ++top;
                    xStack[top] = xp1;
                    yStack[top] = y;
                    continue;
                }

                int xm1 = x - 1;
                if (0 <= xm1 && image(xm1, y) == backColor)
                {
                    // Push pixel with background color.
                    ++top;
                    xStack[top] = xm1;
                    yStack[top] = y;
                    continue;
                }

                int yp1 = y + 1;
                if (yp1 < dim1 && image(x, yp1) == backColor)
                {
                    // Push pixel with background color.
                    ++top;
                    xStack[top] = x;
                    yStack[top] = yp1;
                    continue;
                }

                int ym1 = y - 1;
                if (0 <= ym1 && image(x, ym1) == backColor)
                {
                    // Push pixel with background color.
                    ++top;
                    xStack[top] = x;
                    yStack[top] = ym1;
                    continue;
                }

                // Done in all directions, pop and return to search other
                // directions of predecessor.
                --top;
            }
        }

        // Compute the L1-distance transform of the binary image. The function
        // returns the maximum distance and a point at which the maximum
        // distance is attained.
        static void GetL1Distance(Image2<int>& image, int& maxDistance, int& xMax, int& yMax)
        {
            int const dim0 = image.GetDimension(0);
            int const dim1 = image.GetDimension(1);
            int const dim0m1 = dim0 - 1;
            int const dim1m1 = dim1 - 1;

            // Use a grass-fire approach, computing distance from boundary to
            // interior one pass at a time.
            bool changeMade = true;
            int distance;
            for (distance = 1, xMax = 0, yMax = 0; changeMade; ++distance)
            {
                changeMade = false;
                int distanceP1 = distance + 1;
                for (int y = 1; y < dim1m1; ++y)
                {
                    for (int x = 1; x < dim0m1; ++x)
                    {
                        if (image(x, y) == distance)
                        {
                            if (image(x - 1, y) >= distance
                                && image(x + 1, y) >= distance
                                && image(x, y - 1) >= distance
                                && image(x, y + 1) >= distance)
                            {
                                image(x, y) = distanceP1;
                                xMax = x;
                                yMax = y;
                                changeMade = true;
                            }
                        }
                    }
                }
            }

            maxDistance = --distance;
        }

        // Compute the L2-distance transform of the binary image.  The maximum
        // distance should not be larger than 100, so you have to ensure this
        // is the case for the input image.  The function returns the maximum
        // distance and a point at which the maximum distance is attained.
        // Comments about the algorithm are in the source file.
        static void GetL2Distance(Image2<int> const& image, float& maxDistance,
            int& xMax, int& yMax, Image2<float>& transform)
        {
            // This program calculates the Euclidean distance transform of a
            // binary input image.  The adaptive algorithm is guaranteed to
            // give exact distances for all distances < 100.  The algorithm
            // was provided John Gauch at University of Kansas.  The following
            // is a quote:
            ///
            // The basic idea is similar to a EDT described recently in PAMI
            // by Laymarie from McGill.  By keeping the dx and dy offset to
            // the nearest edge (feature) point in the image, we can search to
            // see which dx dy is closest to a given point by examining a set
            // of neighbors.  The Laymarie method (and Borgfors) look at a
            // fixed 3x3 or 5x5 neighborhood and call it a day.  What we did
            // was calculate (painfully) what neighborhoods you need to look
            // at to guarentee that the exact distance is obtained.  Thus,
            // you will see in the code, that we L2Check the current distance
            // and depending on what we have so far, we extend the search
            // region.  Since our algorithm for L2Checking the exactness of
            // each neighborhood is on the order N^4, we have only gone to
            // N=100.  In theory, you could make this large enough to get all
            // distances exact.  We have implemented the algorithm to get all
            // distances < 100 to be exact. 
            int const dim0 = image.GetDimension(0);
            int const dim1 = image.GetDimension(1);
            int const dim0m1 = dim0 - 1;
            int const dim1m1 = dim1 - 1;
            int x, y, distance;

            // Create and initialize intermediate images.
            Image2<int> xNear(dim0, dim1);
            Image2<int> yNear(dim0, dim1);
            Image2<int> dist(dim0, dim1);
            for (y = 0; y < dim1; ++y)
            {
                for (x = 0; x < dim0; ++x)
                {
                    if (image(x, y) != 0)
                    {
                        xNear(x, y) = 0;
                        yNear(x, y) = 0;
                        dist(x, y) = std::numeric_limits<int>::max();
                    }
                    else
                    {
                        xNear(x, y) = x;
                        yNear(x, y) = y;
                        dist(x, y) = 0;
                    }
                }
            }

            int const K1 = 1;
            int const K2 = 169;   // 13^2
            int const K3 = 961;   // 31^2
            int const K4 = 2401;  // 49^2
            int const K5 = 5184;  // 72^2

            // Pass in the ++ direction.
            for (y = 0; y < dim1; ++y)
            {
                for (x = 0; x < dim0; ++x)
                {
                    distance = dist(x, y);
                    if (distance > K1)
                    {
                        L2Check(x, y, -1, 0, xNear, yNear, dist);
                        L2Check(x, y, -1, -1, xNear, yNear, dist);
                        L2Check(x, y, 0, -1, xNear, yNear, dist);
                    }
                    if (distance > K2)
                    {
                        L2Check(x, y, -2, -1, xNear, yNear, dist);
                        L2Check(x, y, -1, -2, xNear, yNear, dist);
                    }
                    if (distance > K3)
                    {
                        L2Check(x, y, -3, -1, xNear, yNear, dist);
                        L2Check(x, y, -3, -2, xNear, yNear, dist);
                        L2Check(x, y, -2, -3, xNear, yNear, dist);
                        L2Check(x, y, -1, -3, xNear, yNear, dist);
                    }
                    if (distance > K4)
                    {
                        L2Check(x, y, -4, -1, xNear, yNear, dist);
                        L2Check(x, y, -4, -3, xNear, yNear, dist);
                        L2Check(x, y, -3, -4, xNear, yNear, dist);
                        L2Check(x, y, -1, -4, xNear, yNear, dist);
                    }
                    if (distance > K5)
                    {
                        L2Check(x, y, -5, -1, xNear, yNear, dist);
                        L2Check(x, y, -5, -2, xNear, yNear, dist);
                        L2Check(x, y, -5, -3, xNear, yNear, dist);
                        L2Check(x, y, -5, -4, xNear, yNear, dist);
                        L2Check(x, y, -4, -5, xNear, yNear, dist);
                        L2Check(x, y, -2, -5, xNear, yNear, dist);
                        L2Check(x, y, -3, -5, xNear, yNear, dist);
                        L2Check(x, y, -1, -5, xNear, yNear, dist);
                    }
                }
            }

            // Pass in -- direction.
            for (y = dim1m1; y >= 0; --y)
            {
                for (x = dim0m1; x >= 0; --x)
                {
                    distance = dist(x, y);
                    if (distance > K1)
                    {
                        L2Check(x, y, 1, 0, xNear, yNear, dist);
                        L2Check(x, y, 1, 1, xNear, yNear, dist);
                        L2Check(x, y, 0, 1, xNear, yNear, dist);
                    }
                    if (distance > K2)
                    {
                        L2Check(x, y, 2, 1, xNear, yNear, dist);
                        L2Check(x, y, 1, 2, xNear, yNear, dist);
                    }
                    if (distance > K3)
                    {
                        L2Check(x, y, 3, 1, xNear, yNear, dist);
                        L2Check(x, y, 3, 2, xNear, yNear, dist);
                        L2Check(x, y, 2, 3, xNear, yNear, dist);
                        L2Check(x, y, 1, 3, xNear, yNear, dist);
                    }
                    if (distance > K4)
                    {
                        L2Check(x, y, 4, 1, xNear, yNear, dist);
                        L2Check(x, y, 4, 3, xNear, yNear, dist);
                        L2Check(x, y, 3, 4, xNear, yNear, dist);
                        L2Check(x, y, 1, 4, xNear, yNear, dist);
                    }
                    if (distance > K5)
                    {
                        L2Check(x, y, 5, 1, xNear, yNear, dist);
                        L2Check(x, y, 5, 2, xNear, yNear, dist);
                        L2Check(x, y, 5, 3, xNear, yNear, dist);
                        L2Check(x, y, 5, 4, xNear, yNear, dist);
                        L2Check(x, y, 4, 5, xNear, yNear, dist);
                        L2Check(x, y, 2, 5, xNear, yNear, dist);
                        L2Check(x, y, 3, 5, xNear, yNear, dist);
                        L2Check(x, y, 1, 5, xNear, yNear, dist);
                    }
                }
            }

            // Pass in the +- direction.
            for (y = dim1m1; y >= 0; --y)
            {
                for (x = 0; x < dim0; ++x)
                {
                    distance = dist(x, y);
                    if (distance > K1)
                    {
                        L2Check(x, y, -1, 0, xNear, yNear, dist);
                        L2Check(x, y, -1, 1, xNear, yNear, dist);
                        L2Check(x, y, 0, 1, xNear, yNear, dist);
                    }
                    if (distance > K2)
                    {
                        L2Check(x, y, -2, 1, xNear, yNear, dist);
                        L2Check(x, y, -1, 2, xNear, yNear, dist);
                    }
                    if (distance > K3)
                    {
                        L2Check(x, y, -3, 1, xNear, yNear, dist);
                        L2Check(x, y, -3, 2, xNear, yNear, dist);
                        L2Check(x, y, -2, 3, xNear, yNear, dist);
                        L2Check(x, y, -1, 3, xNear, yNear, dist);
                    }
                    if (distance > K4)
                    {
                        L2Check(x, y, -4, 1, xNear, yNear, dist);
                        L2Check(x, y, -4, 3, xNear, yNear, dist);
                        L2Check(x, y, -3, 4, xNear, yNear, dist);
                        L2Check(x, y, -1, 4, xNear, yNear, dist);
                    }
                    if (distance > K5)
                    {
                        L2Check(x, y, -5, 1, xNear, yNear, dist);
                        L2Check(x, y, -5, 2, xNear, yNear, dist);
                        L2Check(x, y, -5, 3, xNear, yNear, dist);
                        L2Check(x, y, -5, 4, xNear, yNear, dist);
                        L2Check(x, y, -4, 5, xNear, yNear, dist);
                        L2Check(x, y, -2, 5, xNear, yNear, dist);
                        L2Check(x, y, -3, 5, xNear, yNear, dist);
                        L2Check(x, y, -1, 5, xNear, yNear, dist);
                    }
                }
            }

            // Pass in the -+ direction.
            for (y = 0; y < dim1; ++y)
            {
                for (x = dim0m1; x >= 0; --x)
                {
                    distance = dist(x, y);
                    if (distance > K1)
                    {
                        L2Check(x, y, 1, 0, xNear, yNear, dist);
                        L2Check(x, y, 1, -1, xNear, yNear, dist);
                        L2Check(x, y, 0, -1, xNear, yNear, dist);
                    }
                    if (distance > K2)
                    {
                        L2Check(x, y, 2, -1, xNear, yNear, dist);
                        L2Check(x, y, 1, -2, xNear, yNear, dist);
                    }
                    if (distance > K3)
                    {
                        L2Check(x, y, 3, -1, xNear, yNear, dist);
                        L2Check(x, y, 3, -2, xNear, yNear, dist);
                        L2Check(x, y, 2, -3, xNear, yNear, dist);
                        L2Check(x, y, 1, -3, xNear, yNear, dist);
                    }
                    if (distance > K4)
                    {
                        L2Check(x, y, 4, -1, xNear, yNear, dist);
                        L2Check(x, y, 4, -3, xNear, yNear, dist);
                        L2Check(x, y, 3, -4, xNear, yNear, dist);
                        L2Check(x, y, 1, -4, xNear, yNear, dist);
                    }
                    if (distance > K5)
                    {
                        L2Check(x, y, 5, -1, xNear, yNear, dist);
                        L2Check(x, y, 5, -2, xNear, yNear, dist);
                        L2Check(x, y, 5, -3, xNear, yNear, dist);
                        L2Check(x, y, 5, -4, xNear, yNear, dist);
                        L2Check(x, y, 4, -5, xNear, yNear, dist);
                        L2Check(x, y, 2, -5, xNear, yNear, dist);
                        L2Check(x, y, 3, -5, xNear, yNear, dist);
                        L2Check(x, y, 1, -5, xNear, yNear, dist);
                    }
                }
            }

            xMax = 0;
            yMax = 0;
            maxDistance = 0.0f;
            for (y = 0; y < dim1; ++y)
            {
                for (x = 0; x < dim0; ++x)
                {
                    float fdistance = std::sqrt((float)dist(x, y));
                    if (fdistance > maxDistance)
                    {
                        maxDistance = fdistance;
                        xMax = x;
                        yMax = y;
                    }
                    transform(x, y) = fdistance;
                }
            }
        }

        // Compute a skeleton of a binary image.  Boundary pixels are trimmed
        // from the object one layer at a time based on their adjacency to
        // interior pixels.  At each step the connectivity and cycles of the
        // object are preserved.  The skeleton overwrites the contents of the
        // input image.
        static void GetSkeleton(Image2<int>& image)
        {
            int const dim0 = image.GetDimension(0);
            int const dim1 = image.GetDimension(1);

            // Trim pixels, mark interior as 4.
            bool notDone = true;
            while (notDone)
            {
                if (MarkInterior(image, 4, Interior4))
                {
                    // No interior pixels, trimmed set is at most 2-pixels
                    // thick.
                    notDone = false;
                    continue;
                }

                if (ClearInteriorAdjacent(image, 4))
                {
                    // All remaining interior pixels are either articulation
                    // points or part of blobs whose boundary pixels are all
                    // articulation points.  An example of the latter case is
                    // shown below.  The background pixels are marked with '.'
                    // rather than '0' for readability.  The interior pixels
                    // are marked with '4' and the boundary pixels are marked
                    // with '1'.
                    //
                    //   .........
                    //   .....1...
                    //   ..1.1.1..
                    //   .1.141...
                    //   ..14441..
                    //   ..1441.1.
                    //   .1.11.1..
                    //   ..1..1...
                    //   .........
                    //
                    // This is a pathological problem where there are many
                    // small holes (0-pixel with north, south, west, and east
                    // neighbors all 1-pixels) that your application can try
                    // to avoid by an initial pass over the image to fill in
                    // such holes.  Of course, you do have problems with
                    // checkerboard patterns...
                    notDone = false;
                    continue;
                }
            }

            // Trim pixels, mark interior as 3.
            notDone = true;
            while (notDone)
            {
                if (MarkInterior(image, 3, Interior3))
                {
                    // No interior pixels, trimmed set is at most 2-pixels
                    // thick.
                    notDone = false;
                    continue;
                }

                if (ClearInteriorAdjacent(image, 3))
                {
                    // All remaining 3-values can be safely removed since they
                    // are not articulation points and the removal will not
                    // cause new holes.
                    for (int y = 0; y < dim1; ++y)
                    {
                        for (int x = 0; x < dim0; ++x)
                        {
                            if (image(x, y) == 3 && !IsArticulation(image, x, y))
                            {
                                image(x, y) = 0;
                            }
                        }
                    }
                    notDone = false;
                    continue;
                }
            }

            // Trim pixels, mark interior as 2.
            notDone = true;
            while (notDone)
            {
                if (MarkInterior(image, 2, Interior2))
                {
                    // No interior pixels, trimmed set is at most 1-pixel
                    // thick.  Call it a skeleton.
                    notDone = false;
                    continue;
                }

                if (ClearInteriorAdjacent(image, 2))
                {
                    // Removes 2-values that are not articulation points.
                    for (int y = 0; y < dim1; ++y)
                    {
                        for (int x = 0; x < dim0; ++x)
                        {
                            if (image(x, y) == 2 && !IsArticulation(image, x, y))
                            {
                                image(x, y) = 0;
                            }
                        }
                    }
                    notDone = false;
                    continue;
                }
            }

            // Make the skeleton a binary image.
            size_t const numPixels = image.GetNumPixels();
            for (size_t i = 0; i < numPixels; ++i)
            {
                if (image[i] != 0)
                {
                    image[i] = 1;
                }
            }
        }

        // In the remaining public member functions, the callback represents
        // the action you want applied to each pixel as it is visited.

        // Visit pixels in a (2*thick+1)x(2*thick+1) square centered at (x,y).
        static void DrawThickPixel(int x, int y, int thick,
            std::function<void(int, int)> const& callback)
        {
            for (int dy = -thick; dy <= thick; ++dy)
            {
                for (int dx = -thick; dx <= thick; ++dx)
                {
                    callback(x + dx, y + dy);
                }
            }
        }

        // Visit pixels using Bresenham's line drawing algorithm.
        static void DrawLine(int x0, int y0, int x1, int y1,
            std::function<void(int, int)> const& callback)
        {
            // Starting point of line.
            int x = x0, y = y0;

            // Direction of line.
            int dx = x1 - x0, dy = y1 - y0;

            // Increment or decrement depending on direction of line.
            int sx = (dx > 0 ? 1 : (dx < 0 ? -1 : 0));
            int sy = (dy > 0 ? 1 : (dy < 0 ? -1 : 0));

            // Decision parameters for pixel selection.
            if (dx < 0)
            {
                dx = -dx;
            }
            if (dy < 0)
            {
                dy = -dy;
            }
            int ax = 2 * dx, ay = 2 * dy;
            int decX, decY;

            // Determine largest direction component, single-step related
            // variable.
            int maxValue = dx, var = 0;
            if (dy > maxValue)
            {
                var = 1;
            }

            // Traverse Bresenham line.
            switch (var)
            {
            case 0:  // Single-step in x-direction.
                decY = ay - dx;
                for (/**/; /**/; x += sx, decY += ay)
                {
                    callback(x, y);

                    // Take Bresenham step.
                    if (x == x1)
                    {
                        break;
                    }
                    if (decY >= 0)
                    {
                        decY -= ax;
                        y += sy;
                    }
                }
                break;
            case 1:  // Single-step in y-direction.
                decX = ax - dy;
                for (/**/; /**/; y += sy, decX += ax)
                {
                    callback(x, y);

                    // Take Bresenham step.
                    if (y == y1)
                    {
                        break;
                    }
                    if (decX >= 0)
                    {
                        decX -= ay;
                        x += sx;
                    }
                }
                break;
            }
        }

        // Visit pixels using Bresenham's circle drawing algorithm.  Set
        // 'solid' to false for drawing only the circle.  Set 'solid' to true
        // to draw all pixels on and inside the circle.
        static void DrawCircle(int xCenter, int yCenter, int radius, bool solid,
            std::function<void(int, int)> const& callback)
        {
            int x, y, dec;

            if (solid)
            {
                int xValue, yMin, yMax, i;
                for (x = 0, y = radius, dec = 3 - 2 * radius; x <= y; ++x)
                {
                    xValue = xCenter + x;
                    yMin = yCenter - y;
                    yMax = yCenter + y;
                    for (i = yMin; i <= yMax; ++i)
                    {
                        callback(xValue, i);
                    }

                    xValue = xCenter - x;
                    for (i = yMin; i <= yMax; ++i)
                    {
                        callback(xValue, i);
                    }

                    xValue = xCenter + y;
                    yMin = yCenter - x;
                    yMax = yCenter + x;
                    for (i = yMin; i <= yMax; ++i)
                    {
                        callback(xValue, i);
                    }

                    xValue = xCenter - y;
                    for (i = yMin; i <= yMax; ++i)
                    {
                        callback(xValue, i);
                    }

                    if (dec >= 0)
                    {
                        dec += -4 * (y--) + 4;
                    }
                    dec += 4 * x + 6;
                }
            }
            else
            {
                for (x = 0, y = radius, dec = 3 - 2 * radius; x <= y; ++x)
                {
                    callback(xCenter + x, yCenter + y);
                    callback(xCenter + x, yCenter - y);
                    callback(xCenter - x, yCenter + y);
                    callback(xCenter - x, yCenter - y);
                    callback(xCenter + y, yCenter + x);
                    callback(xCenter + y, yCenter - x);
                    callback(xCenter - y, yCenter + x);
                    callback(xCenter - y, yCenter - x);

                    if (dec >= 0)
                    {
                        dec += -4 * (y--) + 4;
                    }
                    dec += 4 * x + 6;
                }
            }
        }

        // Visit pixels in a rectangle of the specified dimensions.  Set
        // 'solid' to false for drawing only the rectangle.  Set 'solid' to
        // true to draw all pixels on and inside the rectangle.
        static void DrawRectangle(int xMin, int yMin, int xMax, int yMax,
            bool solid, std::function<void(int, int)> const& callback)
        {
            int x, y;

            if (solid)
            {
                for (y = yMin; y <= yMax; ++y)
                {
                    for (x = xMin; x <= xMax; ++x)
                    {
                        callback(x, y);
                    }
                }
            }
            else
            {
                for (x = xMin; x <= xMax; ++x)
                {
                    callback(x, yMin);
                    callback(x, yMax);
                }
                for (y = yMin + 1; y <= yMax - 1; ++y)
                {
                    callback(xMin, y);
                    callback(xMax, y);
                }
            }
        }

        // Visit the pixels using Bresenham's algorithm for the axis-aligned
        // ellipse ((x-xc)/a)^2 + ((y-yc)/b)^2 = 1, where xCenter is xc,
        // yCenter is yc, xExtent is a, and yExtent is b.
        static void DrawEllipse(int xCenter, int yCenter, int xExtent, int yExtent,
            std::function<void(int, int)> const& callback)
        {
            int xExtSqr = xExtent * xExtent, yExtSqr = yExtent * yExtent;
            int x, y, dec;

            x = 0;
            y = yExtent;
            dec = 2 * yExtSqr + xExtSqr * (1 - 2 * yExtent);
            for (/**/; yExtSqr * x <= xExtSqr * y; ++x)
            {
                callback(xCenter + x, yCenter + y);
                callback(xCenter - x, yCenter + y);
                callback(xCenter + x, yCenter - y);
                callback(xCenter - x, yCenter - y);

                if (dec >= 0)
                {
                    dec += 4 * xExtSqr * (1 - y);
                    --y;
                }
                dec += yExtSqr * (4 * x + 6);
            }
            if (y == 0 && x < xExtent)
            {
                // The discretization caused us to reach the y-axis before the
                // x-values reached the ellipse vertices.  Draw a solid line
                // along the x-axis to those vertices.
                for (/**/; x <= xExtent; ++x)
                {
                    callback(xCenter + x, yCenter);
                    callback(xCenter - x, yCenter);
                }
                return;
            }

            x = xExtent;
            y = 0;
            dec = 2 * xExtSqr + yExtSqr * (1 - 2 * xExtent);
            for (/**/; xExtSqr * y <= yExtSqr * x; ++y)
            {
                callback(xCenter + x, yCenter + y);
                callback(xCenter - x, yCenter + y);
                callback(xCenter + x, yCenter - y);
                callback(xCenter - x, yCenter - y);

                if (dec >= 0)
                {
                    dec += 4 * yExtSqr * (1 - x);
                    --x;
                }
                dec += xExtSqr * (4 * y + 6);
            }
            if (x == 0 && y < yExtent)
            {
                // The discretization caused us to reach the x-axis before the
                // y-values reached the ellipse vertices.  Draw a solid line
                // along the y-axis to those vertices.
                for (/**/; y <= yExtent; ++y)
                {
                    callback(xCenter, yCenter + y);
                    callback(xCenter, yCenter - y);
                }
            }
        }

        // Use a depth-first search for filling a 4-connected region.  This is
        // nonrecursive, simulated by using a heap-allocated "stack".  The
        // input (x,y) is the seed point that starts the fill.  The x-value is
        // in {0..xSize-1} and the y-value is in {0..ySize-1}.
        template <typename PixelType>
        static void DrawFloodFill4(int x, int y, int xSize, int ySize,
            PixelType foreColor, PixelType backColor,
            std::function<void(int, int, PixelType)> const& setCallback,
            std::function<PixelType(int, int)> const& getCallback)
        {
            // Test for a valid seed.
            if (x < 0 || x >= xSize || y < 0 || y >= ySize)
            {
                // The seed point is outside the image domain, so nothing to
                // fill.
                return;
            }

            // Allocate the maximum amount of space needed for the stack.  An
            // empty stack has top == -1.
            int const numPixels = xSize * ySize;
            std::vector<int> xStack(numPixels), yStack(numPixels);

            // Push seed point onto stack if it has the background color.  All
            // points pushed onto stack have background color backColor.
            int top = 0;
            xStack[top] = x;
            yStack[top] = y;

            while (top >= 0)  // stack is not empty
            {
                // Read top of stack.  Do not pop since we need to return to
                // this top value later to restart the fill in a different
                // direction.
                x = xStack[top];
                y = yStack[top];

                // Fill the pixel.
                setCallback(x, y, foreColor);

                int xp1 = x + 1;
                if (xp1 < xSize && getCallback(xp1, y) == backColor)
                {
                    // Push pixel with background color.
                    ++top;
                    xStack[top] = xp1;
                    yStack[top] = y;
                    continue;
                }

                int xm1 = x - 1;
                if (0 <= xm1 && getCallback(xm1, y) == backColor)
                {
                    // Push pixel with background color.
                    ++top;
                    xStack[top] = xm1;
                    yStack[top] = y;
                    continue;
                }

                int yp1 = y + 1;
                if (yp1 < ySize && getCallback(x, yp1) == backColor)
                {
                    // Push pixel with background color.
                    ++top;
                    xStack[top] = x;
                    yStack[top] = yp1;
                    continue;
                }

                int ym1 = y - 1;
                if (0 <= ym1 && getCallback(x, ym1) == backColor)
                {
                    // Push pixel with background color.
                    ++top;
                    xStack[top] = x;
                    yStack[top] = ym1;
                    continue;
                }

                // Done in all directions, pop and return to search other
                // directions of the predecessor.
                --top;
            }
        }

    private:
        // Connected component labeling using depth-first search.
        static void GetComponents(int numNeighbors, int const* delta,
            Image2<int>& image, std::vector<std::vector<size_t>>& components)
        {
            size_t const numPixels = image.GetNumPixels();
            std::vector<int> numElements(numPixels);
            std::vector<size_t> vstack(numPixels);
            size_t i, numComponents = 0;
            int label = 2;
            for (i = 0; i < numPixels; ++i)
            {
                if (image[i] == 1)
                {
                    int top = -1;
                    vstack[++top] = i;

                    int& count = numElements[numComponents + 1];
                    count = 0;
                    while (top >= 0)
                    {
                        size_t v = vstack[top];
                        image[v] = -1;
                        int j;
                        for (j = 0; j < numNeighbors; ++j)
                        {
                            size_t adj = v + delta[j];
                            if (image[adj] == 1)
                            {
                                vstack[++top] = adj;
                                break;
                            }
                        }
                        if (j == numNeighbors)
                        {
                            image[v] = label;
                            ++count;
                            --top;
                        }
                    }

                    ++numComponents;
                    ++label;
                }
            }

            if (numComponents > 0)
            {
                components.resize(numComponents + 1);
                for (i = 1; i <= numComponents; ++i)
                {
                    components[i].resize(numElements[i]);
                    numElements[i] = 0;
                }

                for (i = 0; i < numPixels; ++i)
                {
                    int value = image[i];
                    if (value != 0)
                    {
                        // Labels started at 2 to support the depth-first
                        // search, so they need to be decremented for the
                        // correct labels.
                        image[i] = --value;
                        components[value][numElements[value]] = i;
                        ++numElements[value];
                    }
                }
            }
        }

        // Support for GetL2Distance.
        static void L2Check(int x, int y, int dx, int dy, Image2<int>& xNear,
            Image2<int>& yNear, Image2<int>& dist)
        {
            int const dim0 = dist.GetDimension(0);
            int const dim1 = dist.GetDimension(1);
            int xp = x + dx, yp = y + dy;
            if (0 <= xp && xp < dim0 && 0 <= yp && yp < dim1)
            {
                if (dist(xp, yp) < dist(x, y))
                {
                    int dx0 = xNear(xp, yp) - x;
                    int dy0 = yNear(xp, yp) - y;
                    int newDist = dx0 * dx0 + dy0 * dy0;
                    if (newDist < dist(x, y))
                    {
                        xNear(x, y) = xNear(xp, yp);
                        yNear(x, y) = yNear(xp, yp);
                        dist(x, y) = newDist;
                    }
                }
            }
        }

        // Support for GetSkeleton.
        static bool Interior2(Image2<int>& image, int x, int y)
        {
            bool b1 = (image(x, y - 1) != 0);
            bool b3 = (image(x + 1, y) != 0);
            bool b5 = (image(x, y + 1) != 0);
            bool b7 = (image(x - 1, y) != 0);
            return (b1 && b3) || (b3 && b5) || (b5 && b7) || (b7 && b1);
        }

        static bool Interior3(Image2<int>& image, int x, int y)
        {
            int numNeighbors = 0;
            if (image(x - 1, y) != 0)
            {
                ++numNeighbors;
            }
            if (image(x + 1, y) != 0)
            {
                ++numNeighbors;
            }
            if (image(x, y - 1) != 0)
            {
                ++numNeighbors;
            }
            if (image(x, y + 1) != 0)
            {
                ++numNeighbors;
            }
            return numNeighbors == 3;
        }

        static bool Interior4(Image2<int>& image, int x, int y)
        {
            return image(x - 1, y) != 0
                && image(x + 1, y) != 0
                && image(x, y - 1) != 0
                && image(x, y + 1) != 0;
        }

        static bool MarkInterior(Image2<int>& image, int value,
            bool (*function)(Image2<int>&, int, int))
        {
            int const dim0 = image.GetDimension(0);
            int const dim1 = image.GetDimension(1);
            bool noInterior = true;
            for (int y = 0; y < dim1; ++y)
            {
                for (int x = 0; x < dim0; ++x)
                {
                    if (image(x, y) > 0)
                    {
                        if (function(image, x, y))
                        {
                            image(x, y) = value;
                            noInterior = false;
                        }
                        else
                        {
                            image(x, y) = 1;
                        }
                    }
                }
            }
            return noInterior;
        }

        static bool IsArticulation(Image2<int>& image, int x, int y)
        {
            static std::array<int, 256> const articulation =
            {
                0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,
                0,1,1,1,1,1,1,1,0,1,0,0,0,1,0,0,
                0,1,1,1,1,1,1,1,0,1,0,0,0,1,0,0,
                0,1,1,1,1,1,1,1,0,1,0,0,0,1,0,0,
                0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                0,1,1,1,1,1,1,1,0,1,0,0,0,1,0,0,
                0,1,1,1,1,1,1,1,0,1,0,0,0,1,0,0,
                0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,
                1,1,1,1,1,1,1,1,1,1,0,0,1,1,0,0,
                0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,
                1,1,1,1,1,1,1,1,1,1,0,0,1,1,0,0,
                0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,
                0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0
            };

            // Converts 8 neighbors of pixel (x,y) to an 8-bit value,
            // bit = 1 iff pixel is set.
            int byteMask = 0;
            if (image(x - 1, y - 1) != 0)
            {
                byteMask |= 0x01;
            }
            if (image(x, y - 1) != 0)
            {
                byteMask |= 0x02;
            }
            if (image(x + 1, y - 1) != 0)
            {
                byteMask |= 0x04;
            }
            if (image(x + 1, y) != 0)
            {
                byteMask |= 0x08;
            }
            if (image(x + 1, y + 1) != 0)
            {
                byteMask |= 0x10;
            }
            if (image(x, y + 1) != 0)
            {
                byteMask |= 0x20;
            }
            if (image(x - 1, y + 1) != 0)
            {
                byteMask |= 0x40;
            }
            if (image(x - 1, y) != 0)
            {
                byteMask |= 0x80;
            }

            return articulation[byteMask] == 1;
        }

        static bool ClearInteriorAdjacent(Image2<int>& image, int value)
        {
            int const dim0 = image.GetDimension(0);
            int const dim1 = image.GetDimension(1);
            bool noRemoval = true;
            for (int y = 0; y < dim1; ++y)
            {
                for (int x = 0; x < dim0; ++x)
                {
                    if (image(x, y) == 1)
                    {
                        bool interiorAdjacent =
                            image(x - 1, y - 1) == value ||
                            image(x, y - 1) == value ||
                            image(x + 1, y - 1) == value ||
                            image(x + 1, y) == value ||
                            image(x + 1, y + 1) == value ||
                            image(x, y + 1) == value ||
                            image(x - 1, y + 1) == value ||
                            image(x - 1, y) == value;

                        if (interiorAdjacent && !IsArticulation(image, x, y))
                        {
                            image(x, y) = 0;
                            noRemoval = false;
                        }
                    }
                }
            }
            return noRemoval;
        }
    };
}
