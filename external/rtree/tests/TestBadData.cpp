//
// TestBadData.cpp
//

// RTree
#include <RTree.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

typedef int ValueType;
typedef long long CoordType;

struct Rect
{
    Rect() noexcept = default;

    constexpr Rect(CoordType a_minX, CoordType a_minY, CoordType a_maxX, CoordType a_maxY) noexcept
        : fMin{ a_minX, a_minY }
        , fMax{ a_maxX, a_maxY }
    {
    }


    CoordType fMin[2] = { 0, };
    CoordType fMax[2] = { 0, };
};


bool MySearchCallback(ValueType id)
{
    cout << "Hit data rect " << id << "\n";
    return true; // keep going
}


int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " inFile\n";
        return -1;
    }

    using RectVector = std::vector<Rect>;
    RectVector rectVector;

    // read the data
    {
        ifstream inFile(argv[1]);
        if (!inFile.is_open()) {
            std::cerr << "Can't open input file\n";
            return -1;
        }
        while (!inFile.eof()) {
            // security and robustness be damned
            CoordType xmin, ymin, xmax, ymax;
            string dummy;
            inFile >> xmin >> ymin >> xmax >> ymax;
            cout << xmin << " " << ymin << " " << xmax << " " << ymax << "\n";
            rectVector.emplace_back(xmin, ymin, xmin + xmax, ymin + ymax);
        }
    }

    using MyTree = RTree<ValueType, CoordType, 2, float>;
    MyTree tree;

    int i, nhits;
    cout << "number of rectangles is " << rectVector.size() << "\n";

    for (i = 0; i < rectVector.size(); i++)
    {
        tree.Insert(rectVector[i].fMin, rectVector[i].fMax, i); // Note, all values including zero are fine in this version
    }

    Rect search_rect(6, 4, 10, 6);
    nhits = tree.Search(search_rect.fMin, search_rect.fMax, MySearchCallback);

    cout << "Search resulted in " << nhits << " hits\n";

    // Iterator test
    int itIndex = 0;
    MyTree::Iterator it;
    for (tree.GetFirst(it);
        !tree.IsNull(it);
        tree.GetNext(it))
    {
        int value = tree.GetAt(it);

        CoordType boundsMin[2] = { 0,0 };
        CoordType boundsMax[2] = { 0,0 };
        it.GetBounds(boundsMin, boundsMax);
        cout << "it[" << itIndex++ << "] " << value << " = (" << boundsMin[0] << "," << boundsMin[1] << "," << boundsMax[0] << "," << boundsMax[1] << ")\n";
    }

    // Iterator test, alternate syntax
    itIndex = 0;
    tree.GetFirst(it);
    while (!it.IsNull())
    {
        CoordType value = *it;
        ++it;
        cout << "it[" << itIndex++ << "] " << value << "\n";
    }

    return 0;
}

