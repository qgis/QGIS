//
// MemoryTest.cpp
//
// This demonstrates a use of RTree
//

// RTree
#include <RTree.h>

#include <stdio.h>
#include <memory.h>
#ifdef WIN32
#include <crtdbg.h>
#endif //WIN32

#include <random>

// Use CRT Debug facility to dump memory leaks on app exit
#ifdef WIN32
  // These two are for MSVS 2005 security consciousness until safe std lib funcs are available
#pragma warning(disable : 4996) // Deprecated functions
#define _CRT_SECURE_NO_DEPRECATE // Allow old unsecure standard library functions, Disable some 'warning C4996 - function was deprecated'

// The following macros set and clear, respectively, given bits
// of the C runtime library debug flag, as specified by a bitmask.
#ifdef   _DEBUG
#define  SET_CRT_DEBUG_FIELD(a) \
              _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#define  CLEAR_CRT_DEBUG_FIELD(a) \
              _CrtSetDbgFlag(~(a) & _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#else
#define  SET_CRT_DEBUG_FIELD(a)   ((void) 0)
#define  CLEAR_CRT_DEBUG_FIELD(a) ((void) 0)
#endif
#endif //WIN32

//
// Get a random float b/n two values
// The returned value is >= min && < max (exclusive of max)
//
static std::random_device sRandomDevice;
static std::mt19937 sMT(sRandomDevice());
static float RandFloat(float a_min, float a_max)
{
    std::uniform_real_distribution<float> distribution(a_min, a_max);
    return distribution(sMT);
}


/// Simplify handling of 3 dimensional coordinate
struct Vec3
{
    /// Default constructor
    Vec3() noexcept = default;

    /// Construct from three elements
    constexpr Vec3(float a_x, float a_y, float a_z) noexcept
        : fVal{ a_x, a_y, a_z }
    {
    }

    /// Add two vectors and return result
    Vec3 operator+ (const Vec3& a_other) const
    {
        return Vec3(fVal[0] + a_other.fVal[0],
            fVal[1] + a_other.fVal[1],
            fVal[2] + a_other.fVal[2]);
    }

    float fVal[3] = { 0.0f, };     ///< 3 float components for axes or dimensions
};


static bool BoxesIntersect(const Vec3& a_boxMinA, const Vec3& a_boxMaxA,
    const Vec3& a_boxMinB, const Vec3& a_boxMaxB)
{
    if (a_boxMinA.fVal[0] > a_boxMaxB.fVal[0] || a_boxMaxA.fVal[0] < a_boxMinB.fVal[0])
        return false;
    if (a_boxMinA.fVal[1] > a_boxMaxB.fVal[1] || a_boxMaxA.fVal[1] < a_boxMinB.fVal[1])
        return false;
    if (a_boxMinA.fVal[2] > a_boxMaxB.fVal[2] || a_boxMaxA.fVal[2] < a_boxMinB.fVal[2])
        return false;
    return true;
}


/// A user type to test with, instead of a simple type such as an 'int'
struct SomeThing
{
    SomeThing()
    {
        ++s_outstandingAllocs;
    }
    ~SomeThing()
    {
        --s_outstandingAllocs;
    }

    int m_creationCounter;                          ///< Just a number for identifying within test program
    Vec3 m_min, m_max;                              ///< Minimal bounding rect, values must be known and constant in order to remove from RTree

    static int s_outstandingAllocs;                 ///< Count how many outstanding objects remain
};

/// Init static
int SomeThing::s_outstandingAllocs = 0;


/// A callback function to obtain query results in this implementation
bool QueryResultCallback(SomeThing* a_data)
{
    printf("search found %d\n", a_data->m_creationCounter);

    return true;
}


int main(int argc, char* argv[])
{
    constexpr const int NUM_OBJECTS = 40;       // Number of objects in test set
    constexpr const int FRAC_OBJECTS = 4;
    static_assert(NUM_OBJECTS > FRAC_OBJECTS, "NUM_OBJECTS must be bigger than FRAC_OBJECTS for this test");

    constexpr const float MAX_WORLDSIZE = 10.0f;
    constexpr const float FRAC_WORLDSIZE = MAX_WORLDSIZE / 2;

    // typedef the RTree useage just for conveniance with iteration
    using SomeThingTree = RTree<SomeThing*, float, 3>;

    SomeThing* thingArray[NUM_OBJECTS * 2] = { nullptr, }; // Store objects in another container to test with, sized larger than we need

    // Create intance of RTree

    SomeThingTree tree;


    // Add some nodes
    int counter = 0;
    for (int index = 0; index < NUM_OBJECTS; ++index)
    {
        auto newThing = new SomeThing{};

        newThing->m_creationCounter = counter++;
        newThing->m_min = Vec3(RandFloat(-MAX_WORLDSIZE, MAX_WORLDSIZE), RandFloat(-MAX_WORLDSIZE, MAX_WORLDSIZE), RandFloat(-MAX_WORLDSIZE, MAX_WORLDSIZE));
        Vec3 extent = Vec3(RandFloat(0, FRAC_WORLDSIZE), RandFloat(0, FRAC_WORLDSIZE), RandFloat(0, FRAC_WORLDSIZE));
        newThing->m_max = newThing->m_min + extent;

        thingArray[counter - 1] = newThing;

        tree.Insert(newThing->m_min.fVal, newThing->m_max.fVal, newThing);
        printf("inserting %d\n", newThing->m_creationCounter);
    }

    printf("tree count = %d\n", tree.Count());

    int numToDelete = NUM_OBJECTS / FRAC_OBJECTS;
    int numToStep = FRAC_OBJECTS;

    // Delete some nodes
    for (int index = 0; index < NUM_OBJECTS; index += numToStep)
    {
        auto curThing = thingArray[index];

        if (curThing)
        {
            tree.Remove(curThing->m_min.fVal, curThing->m_max.fVal, curThing);
            printf("removing %d\n", curThing->m_creationCounter);

            delete curThing;
            thingArray[index] = nullptr;
        }
    }

    printf("tree count = %d\n", tree.Count());

    // Add some more nodes
    for (int index = 0; index < numToDelete; ++index)
    {
        auto newThing = new SomeThing{};

        newThing->m_creationCounter = counter++;
        newThing->m_min = Vec3(RandFloat(-MAX_WORLDSIZE, MAX_WORLDSIZE), RandFloat(-MAX_WORLDSIZE, MAX_WORLDSIZE), RandFloat(-MAX_WORLDSIZE, MAX_WORLDSIZE));
        Vec3 extent = Vec3(RandFloat(0, FRAC_WORLDSIZE), RandFloat(0, FRAC_WORLDSIZE), RandFloat(0, FRAC_WORLDSIZE));
        newThing->m_max = newThing->m_min + extent;

        thingArray[counter - 1] = newThing;

        tree.Insert(newThing->m_min.fVal, newThing->m_max.fVal, newThing);
        printf("inserting %d\n", newThing->m_creationCounter);
    }

    printf("tree count = %d\n", tree.Count());

    Vec3 searchMin(0, 0, 0);
    Vec3 searchMax(FRAC_WORLDSIZE, FRAC_WORLDSIZE, FRAC_WORLDSIZE);
    tree.Search(searchMin.fVal, searchMax.fVal, &QueryResultCallback);

    // NOTE: Even better than just dumping text, it would be nice to render the 
    // tree contents and search result for visualization.


    // List values.  Iterator is NOT delete safe
    SomeThingTree::Iterator it;
    for (tree.GetFirst(it); !tree.IsNull(it); tree.GetNext(it))
    {
        SomeThing* curThing = tree.GetAt(it);

        if (BoxesIntersect(searchMin, searchMax, curThing->m_min, curThing->m_max))
        {
            printf("brute found %d\n", curThing->m_creationCounter);
        }
    }

    // Delete our nodes, NOTE, we are NOT deleting the tree nodes, just our data
    // of course the tree will now contain invalid pointers that must not be used any more.
    for (tree.GetFirst(it); !tree.IsNull(it); tree.GetNext(it))
    {
        SomeThing* removeElem = tree.GetAt(it);
        if (removeElem)
        {
            printf("deleting %d\n", removeElem->m_creationCounter);
            delete removeElem;
        }
    }

    // Remove all contents (This would have happened automatically during destructor)
    tree.RemoveAll();

    if (SomeThing::s_outstandingAllocs > 0)
    {
        printf("Memory leak!\n");
        printf("s_outstandingAllocs = %d\n", SomeThing::s_outstandingAllocs);
    }
    else
    {
        printf("No memory leaks detected by app\n");
    }

#ifdef WIN32
    // Use CRT Debug facility to dump memory leaks on app exit
    SET_CRT_DEBUG_FIELD(_CRTDBG_LEAK_CHECK_DF);
#endif //WIN32

    return 0;
}

