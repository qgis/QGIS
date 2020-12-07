//
// Test.cpp
//
// This is a direct port of the C version of the RTree test program.
//

// RTree
#include <RTree.h>

#include <iostream>

using namespace std;

typedef int ValueType;

struct Rect
{
    Rect() noexcept = default;

    constexpr Rect(int a_minX, int a_minY, int a_maxX, int a_maxY) noexcept
        : fMin{ a_minX, a_minY }
        , fMax{ a_maxX, a_maxY }
    {
    }


    int fMin[2] = { 0, };
    int fMax[2] = { 0, };
};

struct Rect rects[] =
{
  Rect(0, 0, 2, 2), // xmin, ymin, xmax, ymax (for 2 dimensional RTree)
  Rect(5, 5, 7, 7),
  Rect(8, 5, 9, 6),
  Rect(7, 1, 9, 2),
};

constexpr const auto nrects = sizeof(rects) / sizeof(rects[0]);

Rect search_rect(6, 4, 10, 6); // search will find above rects that this one overlaps


bool MySearchCallback(ValueType id)
{
    cout << "Hit data rect " << id << "\n";
    return true; // keep going
}


int main()
{
    using MyTree = RTree<ValueType, int, 2, float>;
    static_assert(std::is_same<MyTree::ElementType, int>::value, "ElementType must match");
    static_assert(std::is_same<MyTree::Element, int[2]>::value, "Element must match");
    static_assert(std::is_same<MyTree::ElementTypeReal, float>::value, "ElementTypeReal must match");
    static_assert(MyTree::kNumDimensions == 2, "Dimension must match");

    MyTree tree;

    cout << "nrects = " << nrects << "\n";

    for (size_t i = 0; i < nrects; i++)
    {
        tree.Insert(rects[i].fMin, rects[i].fMax, static_cast<int>(i)); // Note, all values including zero are fine in this version
    }

    const auto nhits = tree.Search(search_rect.fMin, search_rect.fMax, MySearchCallback);

    cout << "Search resulted in " << nhits << " hits\n";

    // Iterator test
    int itIndex = 0;
    MyTree::Iterator it;
    for (tree.GetFirst(it);
        !tree.IsNull(it);
        tree.GetNext(it))
    {
        int value = tree.GetAt(it);

        int boundsMin[2] = { 0,0 };
        int boundsMax[2] = { 0,0 };
        it.GetBounds(boundsMin, boundsMax);
        cout << "it[" << itIndex++ << "] " << value << " = (" << boundsMin[0] << "," << boundsMin[1] << "," << boundsMax[0] << "," << boundsMax[1] << ")\n";
    }

    // Iterator test, alternate syntax
    itIndex = 0;
    tree.GetFirst(it);
    while (!it.IsNull())
    {
        int value = *it;
        ++it;
        cout << "it[" << itIndex++ << "] " << value << "\n";
    }

    // test copy constructor
    MyTree copy = tree;

    // Iterator test
    itIndex = 0;
    for (copy.GetFirst(it);
        !copy.IsNull(it);
        copy.GetNext(it))
    {
        int value = copy.GetAt(it);

        int boundsMin[2] = { 0,0 };
        int boundsMax[2] = { 0,0 };
        it.GetBounds(boundsMin, boundsMax);
        cout << "it[" << itIndex++ << "] " << value << " = (" << boundsMin[0] << "," << boundsMin[1] << "," << boundsMax[0] << "," << boundsMax[1] << ")\n";
    }

    // Iterator test, alternate syntax
    itIndex = 0;
    copy.GetFirst(it);
    while (!it.IsNull())
    {
        int value = *it;
        ++it;
        cout << "it[" << itIndex++ << "] " << value << "\n";
    }

    return 0;

    // Output:
    //
    // nrects = 4
    // Hit data rect 1
    // Hit data rect 2
    // Search resulted in 2 hits
    // it[0] 0 = (0,0,2,2)
    // it[1] 1 = (5,5,7,7)
    // it[2] 2 = (8,5,9,6)
    // it[3] 3 = (7,1,9,2)
    // it[0] 0
    // it[1] 1
    // it[2] 2
    // it[3] 3
    // it[0] 0 = (0,0,2,2)
    // it[1] 1 = (5,5,7,7)
    // it[2] 2 = (8,5,9,6)
    // it[3] 3 = (7,1,9,2)
    // it[0] 0
    // it[1] 1
    // it[2] 2
    // it[3] 3
}

