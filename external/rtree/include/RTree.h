//
// RTree.h
//
#ifndef RTREE_H
#define RTREE_H

#include <cassert>
#include <algorithm>
#include <functional>
#include <type_traits>
#include <cmath>

#define RTreeAssert assert // RTree uses RTreeAssert( condition )


#define RTREE_DONT_USE_MEMPOOLS         // This version does not contain a fixed memory allocator, fill in lines with EXAMPLE to implement one.
#define RTREE_USE_SPHERICAL_VOLUME      // Better split classification, may be slower on some systems


// Because there is not stream support, this is a quick and dirty file I/O helper.
// Users will likely replace its usage with a Stream implementation from their favorite API.
class RTFileStream {
public:
    RTFileStream() = default;

    ~RTFileStream() {
        Close();
    }

    bool OpenRead(const char* a_fileName) {
        m_file = fopen(a_fileName, "rb");
        if (!m_file) {
            return false;
        }
        return true;
    }

    bool OpenWrite(const char* a_fileName) {
        m_file = fopen(a_fileName, "wb");
        if (!m_file) {
            return false;
        }
        return true;
    }

    void Close() {
        if (m_file) {
            fclose(m_file);
            m_file = nullptr;
        }
    }

    template< typename TYPE >
    size_t Write(const TYPE& a_value) {
        RTreeAssert(m_file);
        return fwrite((void*)&a_value, sizeof(a_value), 1, m_file);
    }

    template< typename TYPE >
    size_t WriteArray(const TYPE* a_array, int a_count) {
        RTreeAssert(m_file);
        return fwrite((void*)a_array, sizeof(TYPE) * a_count, 1, m_file);
    }

    template< typename TYPE >
    size_t Read(TYPE& a_value) {
        RTreeAssert(m_file);
        return fread((void*)&a_value, sizeof(a_value), 1, m_file);
    }

    template< typename TYPE >
    size_t ReadArray(TYPE* a_array, int a_count) {
        RTreeAssert(m_file);
        return fread((void*)a_array, sizeof(TYPE) * a_count, 1, m_file);
    }

private:
    FILE* m_file = nullptr;
};


/// \class RTree
/// Implementation of RTree, a multidimensional bounding rectangle tree.
/// Example usage: For a 3-dimensional tree use RTree<Object*, float, 3> myTree;
///
/// This modified, templated C++ version by Greg Douglas at Auran (http://www.auran.com)
///
/// _DataType Referenced data, should be int, void*, obj* etc. no larger than sizeof<void*> and simple type
/// _ElementType Type of element such as int or float
/// _NumDimensions Number of dimensions such as 2 or 3
/// _ElementTypeReal Type of element that allows fractional and large values such as float or double, for use in volume calcs
///
/// NOTES: Inserting and removing data requires the knowledge of its constant Minimal Bounding Rectangle.
///        This version uses new/delete for nodes, I recommend using a fixed size allocator for efficiency.
///        Instead of using a callback function for returned results, I recommend and efficient pre-sized, grow-only memory
///        array similar to MFC CArray or STL Vector for returning search query result.
///
template<class _DataType, class _ElementType, int _NumDimensions, 
    class _ElementTypeReal = _ElementType, int _MaxNodeCount = 8, int _MinNodeCount = _MaxNodeCount / 2>
class RTree
{
    static_assert(1 < _NumDimensions, "_NumDimensions must be larger than 1");
    static_assert(0 < _MinNodeCount, "_MinNodeCount must be larger than 0");
    static_assert(_MinNodeCount < _MaxNodeCount, "_MaxNodeCount must be larger than _MinNodeCount");
    static_assert(std::is_floating_point<_ElementTypeReal>::value, "_ElementTypeReal must be floating point type");
protected:

    struct Node;  // Fwd decl.  Used by other internal structs and iterator

public:
    using DataType = _DataType;
    using ElementType = _ElementType;
    using ElementTypeReal = _ElementTypeReal;
    using Element = ElementType[_NumDimensions];

    constexpr static const int kNumDimensions = _NumDimensions;
    constexpr static const int kMaxNodeCount = _MaxNodeCount;
    constexpr static const int kMinNodeCount = _MinNodeCount;

    template<typename ValueType>
    constexpr static ElementType CastElementType(const ValueType val) noexcept {
        return static_cast<ElementType>(val);
    }
    template<typename ValueType>
    constexpr static ElementTypeReal CastElementTypeReal(const ValueType val) noexcept {
        return static_cast<ElementTypeReal>(val);
    }
    constexpr static const ElementTypeReal kElementTypeRealOne = CastElementTypeReal(1.0);
    constexpr static const ElementTypeReal kElementTypeRealZero = CastElementTypeReal(0.0);

    RTree() {
        // Precomputed volumes of the unit spheres for the first few dimensions
        constexpr const ElementTypeReal kUnitSphereVolumes[] = {
          CastElementTypeReal(0.000000), CastElementTypeReal(2.000000), CastElementTypeReal(3.141593), // Dimension  0,1,2
          CastElementTypeReal(4.188790), CastElementTypeReal(4.934802), CastElementTypeReal(5.263789), // Dimension  3,4,5
          CastElementTypeReal(5.167713), CastElementTypeReal(4.724766), CastElementTypeReal(4.058712), // Dimension  6,7,8
          CastElementTypeReal(3.298509), CastElementTypeReal(2.550164), CastElementTypeReal(1.884104), // Dimension  9,10,11
          CastElementTypeReal(1.335263), CastElementTypeReal(0.910629), CastElementTypeReal(0.599265), // Dimension  12,13,14
          CastElementTypeReal(0.381443), CastElementTypeReal(0.235331), CastElementTypeReal(0.140981), // Dimension  15,16,17
          CastElementTypeReal(0.082146), CastElementTypeReal(0.046622), CastElementTypeReal(0.025807), // Dimension  18,19,20 
        };

        this->m_root = this->AllocNode();
        this->m_root->m_level = 0;
        this->m_unitSphereVolume = kUnitSphereVolumes[kNumDimensions];
    }

    RTree(const RTree& other) : RTree() {
        this->CopyRec(this->m_root, other.m_root);
    }

    ~RTree() {
        this->Reset();
    }

    /// Insert entry
    /// \param a_min Min of bounding rect
    /// \param a_max Max of bounding rect
    /// \param a_dataId Positive Id of data.  Maybe zero, but negative numbers not allowed.
    void Insert(const Element& a_min, const Element& a_max, const DataType& a_dataId) {
#ifdef _DEBUG
        for (int index = 0; index < kNumDimensions; ++index) {
            RTreeAssert(a_min[index] <= a_max[index]);
        }
#endif //_DEBUG

        Branch branch;
        branch.m_data = a_dataId;
        branch.m_child = nullptr;

        for (int axis = 0; axis < kNumDimensions; ++axis) {
            branch.m_rect.m_min[axis] = a_min[axis];
            branch.m_rect.m_max[axis] = a_max[axis];
        }

        InsertRect(branch, &m_root, 0);
    }

    /// Remove entry
    /// \param a_min Min of bounding rect
    /// \param a_max Max of bounding rect
    /// \param a_dataId Positive Id of data.  Maybe zero, but negative numbers not allowed.
    void Remove(const Element& a_min, const Element& a_max, const DataType& a_dataId) {
#ifdef _DEBUG
        for (int index = 0; index < kNumDimensions; ++index) {
            RTreeAssert(a_min[index] <= a_max[index]);
        }
#endif //_DEBUG

        Rect rect;

        for (int axis = 0; axis < kNumDimensions; ++axis) {
            rect.m_min[axis] = a_min[axis];
            rect.m_max[axis] = a_max[axis];
        }

        RemoveRect(&rect, a_dataId, &m_root);
    }

    /// Find all within search rectangle
    /// \param a_min Min of search bounding rect
    /// \param a_max Max of search bounding rect
    /// \param a_searchResult Search result array.  Caller should set grow size. Function will reset, not append to array.
    /// \param a_resultCallback Callback function to return result.  Callback should return 'true' to continue searching
    /// \param a_context User context to pass as parameter to a_resultCallback
    /// \return Returns the number of entries found
    int Search(const Element& a_min, const Element& a_max, std::function<bool(const DataType&)> callback) const {
#ifdef _DEBUG
        for (int index = 0; index < kNumDimensions; ++index) {
            RTreeAssert(a_min[index] <= a_max[index]);
        }
#endif //_DEBUG

        Rect rect;

        for (int axis = 0; axis < kNumDimensions; ++axis) {
            rect.m_min[axis] = a_min[axis];
            rect.m_max[axis] = a_max[axis];
        }

        // NOTE: May want to return search result another way, perhaps returning the number of found elements here.

        int foundCount = 0;
        Search(m_root, &rect, foundCount, callback);

        return foundCount;
    }

    /// Remove all entries from tree
    void RemoveAll() {

        // Delete all existing nodes
        Reset();

        m_root = AllocNode();
        m_root->m_level = 0;
    }

    /// Count the data elements in this container.  This is slow as no internal counter is maintained.
    int Count() const {

        int count = 0;
        CountRec(m_root, count);

        return count;
    }

    /// Load tree contents from file
    bool Load(const char* a_fileName) {
        RemoveAll(); // Clear existing tree

        RTFileStream stream;
        if (!stream.OpenRead(a_fileName)) {
            return false;
        }

        bool result = Load(stream);

        stream.Close();

        return result;
    }
    /// Load tree contents from stream
    bool Load(RTFileStream& a_stream) {
        // Write some kind of header
        int _dataFileId = ('R' << 0) | ('T' << 8) | ('R' << 16) | ('E' << 24);
        int _dataSize = sizeof(DataType);
        int _dataNumDims = kNumDimensions;
        int _dataElemSize = sizeof(ElementType);
        int _dataElemRealSize = sizeof(ElementTypeReal);
        int _dataMaxNodes = kMaxNodeCount;
        int _dataMinNodes = kMinNodeCount;

        int dataFileId = 0;
        int dataSize = 0;
        int dataNumDims = 0;
        int dataElemSize = 0;
        int dataElemRealSize = 0;
        int dataMaxNodes = 0;
        int dataMinNodes = 0;

        a_stream.Read(dataFileId);
        a_stream.Read(dataSize);
        a_stream.Read(dataNumDims);
        a_stream.Read(dataElemSize);
        a_stream.Read(dataElemRealSize);
        a_stream.Read(dataMaxNodes);
        a_stream.Read(dataMinNodes);

        bool result = false;

        // Test if header was valid and compatible
        if ((dataFileId == _dataFileId)
            && (dataSize == _dataSize)
            && (dataNumDims == _dataNumDims)
            && (dataElemSize == _dataElemSize)
            && (dataElemRealSize == _dataElemRealSize)
            && (dataMaxNodes == _dataMaxNodes)
            && (dataMinNodes == _dataMinNodes)
            ) {
            // Recursively load tree
            result = LoadRec(m_root, a_stream);
        }

        return result;
    }


    /// Save tree contents to file
    bool Save(const char* a_fileName) {

        RTFileStream stream;
        if (!stream.OpenWrite(a_fileName)) {
            return false;
        }

        bool result = Save(stream);

        stream.Close();

        return result;
    }
    /// Save tree contents to stream
    bool Save(RTFileStream& a_stream) {

        // Write some kind of header
        int dataFileId = ('R' << 0) | ('T' << 8) | ('R' << 16) | ('E' << 24);
        int dataSize = sizeof(DataType);
        int dataNumDims = kNumDimensions;
        int dataElemSize = sizeof(ElementType);
        int dataElemRealSize = sizeof(ElementTypeReal);
        int dataMaxNodes = kMaxNodeCount;
        int dataMinNodes = kMinNodeCount;

        a_stream.Write(dataFileId);
        a_stream.Write(dataSize);
        a_stream.Write(dataNumDims);
        a_stream.Write(dataElemSize);
        a_stream.Write(dataElemRealSize);
        a_stream.Write(dataMaxNodes);
        a_stream.Write(dataMinNodes);

        // Recursively save tree
        bool result = SaveRec(m_root, a_stream);

        return result;
    }

    /// Iterator is not remove safe.
    class Iterator
    {
    private:

        constexpr static const int kMaxStackSize = 32;  //  Max stack size. Allows almost n^32 where n is number of branches in node

        struct StackElement
        {
            Node* m_node = nullptr;
            int m_branchIndex = 0;
        };

    public:

        Iterator() { Init(); }

        ~Iterator() = default;

        /// Is iterator invalid
        bool IsNull() const noexcept { return (m_tos <= 0); }

        /// Is iterator pointing to valid data
        bool IsNotNull() const noexcept { return (m_tos > 0); }

        /// Access the current data element. Caller must be sure iterator is not NULL first.
        DataType& operator*()
        {
            RTreeAssert(IsNotNull());
            auto& curTos = m_stack[m_tos - 1];
            return curTos.m_node->m_branch[curTos.m_branchIndex].m_data;
        }

        /// Access the current data element. Caller must be sure iterator is not NULL first.
        const DataType& operator*() const
        {
            RTreeAssert(IsNotNull());
            auto& curTos = m_stack[m_tos - 1];
            return curTos.m_node->m_branch[curTos.m_branchIndex].m_data;
        }

        /// Find the next data element
        bool operator++() { return FindNextData(); }

        /// Get the bounds for this node
        void GetBounds(Element& a_min, Element& a_max)
        {
            RTreeAssert(IsNotNull());
            auto& curTos = m_stack[m_tos - 1];
            auto& curBranch = curTos.m_node->m_branch[curTos.m_branchIndex];

            for (int index = 0; index < kNumDimensions; ++index)
            {
                a_min[index] = curBranch.m_rect.m_min[index];
                a_max[index] = curBranch.m_rect.m_max[index];
            }
        }

    private:

        /// Reset iterator
        void Init() { m_tos = 0; }

        /// Find the next data element in the tree (For internal use only)
        bool FindNextData()
        {
            for (;;)
            {
                if (m_tos <= 0)
                {
                    return false;
                }
                auto curTos = Pop(); // Copy stack top cause it may change as we use it

                if (curTos.m_node->IsLeaf())
                {
                    // Keep walking through data while we can
                    if (curTos.m_branchIndex + 1 < curTos.m_node->m_count)
                    {
                        // There is more data, just point to the next one
                        Push(curTos.m_node, curTos.m_branchIndex + 1);
                        return true;
                    }
                    // No more data, so it will fall back to previous level
                }
                else
                {
                    if (curTos.m_branchIndex + 1 < curTos.m_node->m_count)
                    {
                        // Push sibling on for future tree walk
                        // This is the 'fall back' node when we finish with the current level
                        Push(curTos.m_node, curTos.m_branchIndex + 1);
                    }
                    // Since cur node is not a leaf, push first of next level to get deeper into the tree
                    auto nextLevelnode = curTos.m_node->m_branch[curTos.m_branchIndex].m_child;
                    Push(nextLevelnode, 0);

                    // If we pushed on a new leaf, exit as the data is ready at TOS
                    if (nextLevelnode->IsLeaf())
                    {
                        return true;
                    }
                }
            }
        }

        /// Push node and branch onto iteration stack (For internal use only)
        void Push(Node* a_node, int a_branchIndex)
        {
            m_stack[m_tos].m_node = a_node;
            m_stack[m_tos].m_branchIndex = a_branchIndex;
            ++m_tos;
            RTreeAssert(m_tos <= kMaxStackSize);
        }

        /// Pop element off iteration stack (For internal use only)
        StackElement& Pop()
        {
            RTreeAssert(m_tos > 0);
            --m_tos;
            return m_stack[m_tos];
        }

        StackElement m_stack[kMaxStackSize];          ///< Stack as we are doing iteration instead of recursion
        int m_tos = 0;                                ///< Top Of Stack index

        friend class RTree; // Allow hiding of non-public functions while allowing manipulation by logical owner
    };

    /// Get 'first' for iteration
    void GetFirst(Iterator& a_it) const
    {
        a_it.Init();
        auto first = m_root;
        while (first)
        {
            if (first->IsInternalNode() && first->m_count > 1)
            {
                a_it.Push(first, 1); // Descend sibling branch later
            }
            else if (first->IsLeaf())
            {
                if (first->m_count)
                {
                    a_it.Push(first, 0);
                }
                break;
            }
            first = first->m_branch[0].m_child;
        }
    }

    /// Get Next for iteration
    void GetNext(Iterator& a_it) const { ++a_it; }

    /// Is iterator NULL, or at end?
    bool IsNull(const Iterator& a_it) const { return a_it.IsNull(); }

    /// Get object at iterator position
    DataType& GetAt(Iterator& a_it) { return *a_it; }

protected:

    /// Minimal bounding rectangle (n-dimensional)
    struct Rect
    {
        ElementType m_min[kNumDimensions] = { 0, };                      ///< Min dimensions of bounding box 
        ElementType m_max[kNumDimensions] = { 0, };                      ///< Max dimensions of bounding box 
    };

    /// May be data or may be another subtree
    /// The parents level determines this.
    /// If the parents level is 0, then this is data
    struct Branch
    {
        Rect m_rect;                                  ///< Bounds
        Node* m_child = nullptr;                      ///< Child node
        DataType m_data;                             ///< Data Id
    };

    /// Node for each branch level
    struct Node
    {
        bool IsInternalNode() const noexcept { return (m_level > 0); } // Not a leaf, but a internal node
        bool IsLeaf() const noexcept { return (m_level == 0); }        // A leaf, contains data

        int m_count = 0;                              ///< Count
        int m_level = 0;                              ///< Leaf is zero, others positive
        Branch m_branch[_MaxNodeCount];               ///< Branch
    };

    /// A link list of nodes for reinsertion after a delete operation
    struct ListNode
    {
        ListNode* m_next = nullptr;                   ///< Next in list
        Node* m_node = nullptr;                       ///< Node
    };

    /// Variables for finding a split partition
    struct PartitionVars
    {
        constexpr static const int kNotTaken = -1; // indicates that position

        int m_partition[_MaxNodeCount + 1] = { 0, };
        int m_total = 0;
        int m_minFill = 0;
        int m_count[2] = { 0, };
        Rect m_cover[2];
        ElementTypeReal m_area[2] = { kElementTypeRealZero, };

        Branch m_branchBuf[_MaxNodeCount + 1];
        int m_branchCount = 0;
        Rect m_coverSplit;
        ElementTypeReal m_coverSplitArea = kElementTypeRealZero;
    };

    Node* AllocNode() {
        Node* newNode;
#ifdef RTREE_DONT_USE_MEMPOOLS
        newNode = new Node;
#else // RTREE_DONT_USE_MEMPOOLS
        // EXAMPLE
#endif // RTREE_DONT_USE_MEMPOOLS
        InitNode(newNode);
        return newNode;
    }

    void FreeNode(Node* a_node) {
        RTreeAssert(a_node);

#ifdef RTREE_DONT_USE_MEMPOOLS
        delete a_node;
#else // RTREE_DONT_USE_MEMPOOLS
        // EXAMPLE
#endif // RTREE_DONT_USE_MEMPOOLS
    }

    void InitNode(Node* a_node) {
        a_node->m_count = 0;
        a_node->m_level = -1;
    }

    void InitRect(Rect* a_rect) {
        for (int index = 0; index < kNumDimensions; ++index) {
            a_rect->m_min[index] = CastElementType(0);
            a_rect->m_max[index] = CastElementType(0);
        }
    }

    // Inserts a new data rectangle into the index structure.
    // Recursively descends tree, propagates splits back up.
    // Returns 0 if node was not split.  Old node updated.
    // If node was split, returns 1 and sets the pointer pointed to by
    // new_node to point to the new node.  Old node updated to become one of two.
    // The level argument specifies the number of steps up from the leaf
    // level to insert; e.g. a data rectangle goes in at level = 0.
    bool InsertRectRec(const Branch& a_branch, Node* a_node, Node** a_newNode, int a_level) {
        RTreeAssert(a_node && a_newNode);
        RTreeAssert(a_level >= 0 && a_level <= a_node->m_level);

        // recurse until we reach the correct level for the new record. data records
        // will always be called with a_level == 0 (leaf)
        if (a_node->m_level > a_level) {
            // Still above level for insertion, go down tree recursively
            Node* otherNode;

            // find the optimal branch for this record
            int index = PickBranch(&a_branch.m_rect, a_node);

            // recursively insert this record into the picked branch
            bool childWasSplit = InsertRectRec(a_branch, a_node->m_branch[index].m_child, &otherNode, a_level);

            if (!childWasSplit) {
                // Child was not split. Merge the bounding box of the new record with the
                // existing bounding box
                a_node->m_branch[index].m_rect = CombineRect(&a_branch.m_rect, &(a_node->m_branch[index].m_rect));
                return false;
            } else {
                // Child was split. The old branches are now re-partitioned to two nodes
                // so we have to re-calculate the bounding boxes of each node
                a_node->m_branch[index].m_rect = NodeCover(a_node->m_branch[index].m_child);
                Branch branch;
                branch.m_child = otherNode;
                branch.m_rect = NodeCover(otherNode);

                // The old node is already a child of a_node. Now add the newly-created
                // node to a_node as well. a_node might be split because of that.
                return AddBranch(&branch, a_node, a_newNode);
            }
        } else if (a_node->m_level == a_level) {
            // We have reached level for insertion. Add rect, split if necessary
            return AddBranch(&a_branch, a_node, a_newNode);
        } else {
            // Should never occur
            RTreeAssert(0);
            return false;
        }
    }

    // Insert a data rectangle into an index structure.
    // InsertRect provides for splitting the root;
    // returns 1 if root was split, 0 if it was not.
    // The level argument specifies the number of steps up from the leaf
    // level to insert; e.g. a data rectangle goes in at level = 0.
    // InsertRect2 does the recursion.
    bool InsertRect(const Branch& a_branch, Node** a_root, int a_level) {

        RTreeAssert(a_root);
        RTreeAssert(a_level >= 0 && a_level <= (*a_root)->m_level);
#ifdef _DEBUG
        for (int index = 0; index < kNumDimensions; ++index) {
            RTreeAssert(a_branch.m_rect.m_min[index] <= a_branch.m_rect.m_max[index]);
        }
#endif //_DEBUG  

        Node* newNode;

        if (InsertRectRec(a_branch, *a_root, &newNode, a_level))  // Root split
        {
            // Grow tree taller and new root
            Node* newRoot = AllocNode();
            newRoot->m_level = (*a_root)->m_level + 1;

            Branch branch;

            // add old root node as a child of the new root
            branch.m_rect = NodeCover(*a_root);
            branch.m_child = *a_root;
            AddBranch(&branch, newRoot, nullptr);

            // add the split node as a child of the new root
            branch.m_rect = NodeCover(newNode);
            branch.m_child = newNode;
            AddBranch(&branch, newRoot, nullptr);

            // set the new root as the root node
            *a_root = newRoot;

            return true;
        }

        return false;
    }

    // Find the smallest rectangle that includes all rectangles in branches of a node.
    Rect NodeCover(Node* a_node) {
        RTreeAssert(a_node);

        Rect rect = a_node->m_branch[0].m_rect;
        for (int index = 1; index < a_node->m_count; ++index) {
            rect = CombineRect(&rect, &(a_node->m_branch[index].m_rect));
        }

        return rect;
    }

    // Add a branch to a node.  Split the node if necessary.
    // Returns 0 if node not split.  Old node updated.
    // Returns 1 if node split, sets *new_node to address of new node.
    // Old node updated, becomes one of two.
    bool AddBranch(const Branch* a_branch, Node* a_node, Node** a_newNode) {

        RTreeAssert(a_branch);
        RTreeAssert(a_node);

        if (a_node->m_count < _MaxNodeCount)  // Split won't be necessary
        {
            a_node->m_branch[a_node->m_count] = *a_branch;
            ++a_node->m_count;

            return false;
        } else {
            RTreeAssert(a_newNode);

            SplitNode(a_node, a_branch, a_newNode);
            return true;
        }
    }

    // Disconnect a dependent node.
    // Caller must return (or stop using iteration index) after this as count has changed
    void DisconnectBranch(Node* a_node, int a_index) {
        RTreeAssert(a_node && (a_index >= 0) && (a_index < _MaxNodeCount));
        RTreeAssert(a_node->m_count > 0);

        // Remove element by swapping with the last element to prevent gaps in array
        a_node->m_branch[a_index] = a_node->m_branch[a_node->m_count - 1];

        --a_node->m_count;
    }

    // Pick a branch.  Pick the one that will need the smallest increase
    // in area to accomodate the new rectangle.  This will result in the
    // least total area for the covering rectangles in the current node.
    // In case of a tie, pick the one which was smaller before, to get
    // the best resolution when searching.
    int PickBranch(const Rect* a_rect, Node* a_node) {
        RTreeAssert(a_rect && a_node);

        bool firstTime = true;
        ElementTypeReal increase;
        ElementTypeReal bestIncr = CastElementTypeReal(-1);
        ElementTypeReal area;
        ElementTypeReal bestArea;
        int best = 0;
        Rect tempRect;

        for (int index = 0; index < a_node->m_count; ++index) {
            Rect* curRect = &a_node->m_branch[index].m_rect;
            area = CalcRectVolume(curRect);
            tempRect = CombineRect(a_rect, curRect);
            increase = CalcRectVolume(&tempRect) - area;
            if ((increase < bestIncr) || firstTime) {
                best = index;
                bestArea = area;
                bestIncr = increase;
                firstTime = false;
            } else if ((increase == bestIncr) && (area < bestArea)) {
                best = index;
                bestArea = area;
                bestIncr = increase;
            }
        }
        return best;
    }

    // Combine two rectangles into larger one containing both
    Rect CombineRect(const Rect* a_rectA, const Rect* a_rectB) {
        RTreeAssert(a_rectA && a_rectB);

        Rect newRect;

        for (int index = 0; index < kNumDimensions; ++index) {
            newRect.m_min[index] = (std::min)(a_rectA->m_min[index], a_rectB->m_min[index]);
            newRect.m_max[index] = (std::max)(a_rectA->m_max[index], a_rectB->m_max[index]);
        }

        return newRect;
    }

    // Split a node.
    // Divides the nodes branches and the extra one between two nodes.
    // Old node is one of the new ones, and one really new one is created.
    // Tries more than one method for choosing a partition, uses best result.
    void SplitNode(Node* a_node, const Branch* a_branch, Node** a_newNode) {
        RTreeAssert(a_node);
        RTreeAssert(a_branch);

        // Could just use local here, but member or external is faster since it is reused
        PartitionVars localVars;
        PartitionVars* parVars = &localVars;

        // Load all the branches into a buffer, initialize old node
        GetBranches(a_node, a_branch, parVars);

        // Find partition
        ChoosePartition(parVars, _MinNodeCount);

        // Create a new node to hold (about) half of the branches
        *a_newNode = AllocNode();
        (*a_newNode)->m_level = a_node->m_level;

        // Put branches from buffer into 2 nodes according to the chosen partition
        a_node->m_count = 0;
        LoadNodes(a_node, *a_newNode, parVars);

        RTreeAssert((a_node->m_count + (*a_newNode)->m_count) == parVars->m_total);
    }

    // The exact volume of the bounding sphere for the given Rect
    ElementTypeReal RectSphericalVolume(Rect* a_rect) {
        RTreeAssert(a_rect);

        ElementTypeReal sumOfSquares = kElementTypeRealZero;

        for (int index = 0; index < kNumDimensions; ++index) {
            const auto halfExtent = ((ElementTypeReal)a_rect->m_max[index] - (ElementTypeReal)a_rect->m_min[index]) * 0.5f;
            sumOfSquares += halfExtent * halfExtent;
        }

        const auto radius = CastElementTypeReal(sqrt(sumOfSquares));

        // Pow maybe slow, so test for common dims like 2,3 and just use x*x, x*x*x.
        if (kNumDimensions == 3) {
            return (radius * radius * radius * m_unitSphereVolume);
        } else if (kNumDimensions == 2) {
            return (radius * radius * m_unitSphereVolume);
        } else {
            return CastElementTypeReal(pow(radius, kNumDimensions) * m_unitSphereVolume);
        }
    }

    // Calculate the n-dimensional volume of a rectangle
    ElementTypeReal RectVolume(Rect* a_rect) {
        RTreeAssert(a_rect);

        auto volume = kElementTypeRealOne;

        for (int index = 0; index < kNumDimensions; ++index) {
            volume *= a_rect->m_max[index] - a_rect->m_min[index];
        }

        RTreeAssert(volume >= kElementTypeRealZero);

        return volume;
    }

    // Use one of the methods to calculate retangle volume
    ElementTypeReal CalcRectVolume(Rect* a_rect) {
#ifdef RTREE_USE_SPHERICAL_VOLUME
        return RectSphericalVolume(a_rect); // Slower but helps certain merge cases
#else // RTREE_USE_SPHERICAL_VOLUME
        return RectVolume(a_rect); // Faster but can cause poor merges
#endif // RTREE_USE_SPHERICAL_VOLUME  
    }

    // Load branch buffer with branches from full node plus the extra branch.
    void GetBranches(Node* a_node, const Branch* a_branch, PartitionVars* a_parVars) {
        RTreeAssert(a_node);
        RTreeAssert(a_branch);

        RTreeAssert(a_node->m_count == _MaxNodeCount);

        // Load the branch buffer
        for (int index = 0; index < _MaxNodeCount; ++index) {
            a_parVars->m_branchBuf[index] = a_node->m_branch[index];
        }
        a_parVars->m_branchBuf[_MaxNodeCount] = *a_branch;
        a_parVars->m_branchCount = _MaxNodeCount + 1;

        // Calculate rect containing all in the set
        a_parVars->m_coverSplit = a_parVars->m_branchBuf[0].m_rect;
        for (int index = 1; index < _MaxNodeCount + 1; ++index) {
            a_parVars->m_coverSplit = CombineRect(&a_parVars->m_coverSplit, &a_parVars->m_branchBuf[index].m_rect);
        }
        a_parVars->m_coverSplitArea = CalcRectVolume(&a_parVars->m_coverSplit);
    }

    // Method #0 for choosing a partition:
    // As the seeds for the two groups, pick the two rects that would waste the
    // most area if covered by a single rectangle, i.e. evidently the worst pair
    // to have in the same group.
    // Of the remaining, one at a time is chosen to be put in one of the two groups.
    // The one chosen is the one with the greatest difference in area expansion
    // depending on which group - the rect most strongly attracted to one group
    // and repelled from the other.
    // If one group gets too full (more would force other group to violate min
    // fill requirement) then other group gets the rest.
    // These last are the ones that can go in either group most easily.
    void ChoosePartition(PartitionVars* a_parVars, int a_minFill) {
        RTreeAssert(a_parVars);

        ElementTypeReal biggestDiff;
        int group, chosen = 0, betterGroup = 0;

        InitParVars(a_parVars, a_parVars->m_branchCount, a_minFill);
        PickSeeds(a_parVars);

        while (((a_parVars->m_count[0] + a_parVars->m_count[1]) < a_parVars->m_total)
            && (a_parVars->m_count[0] < (a_parVars->m_total - a_parVars->m_minFill))
            && (a_parVars->m_count[1] < (a_parVars->m_total - a_parVars->m_minFill))) {
            biggestDiff = CastElementTypeReal(-1);
            for (int index = 0; index < a_parVars->m_total; ++index) {
                if (PartitionVars::kNotTaken == a_parVars->m_partition[index]) {
                    Rect* curRect = &a_parVars->m_branchBuf[index].m_rect;
                    Rect rect0 = CombineRect(curRect, &a_parVars->m_cover[0]);
                    Rect rect1 = CombineRect(curRect, &a_parVars->m_cover[1]);
                    ElementTypeReal growth0 = CalcRectVolume(&rect0) - a_parVars->m_area[0];
                    ElementTypeReal growth1 = CalcRectVolume(&rect1) - a_parVars->m_area[1];
                    ElementTypeReal diff = growth1 - growth0;
                    if (diff >= 0) {
                        group = 0;
                    } else {
                        group = 1;
                        diff = -diff;
                    }

                    if (diff > biggestDiff) {
                        biggestDiff = diff;
                        chosen = index;
                        betterGroup = group;
                    } else if ((diff == biggestDiff) && (a_parVars->m_count[group] < a_parVars->m_count[betterGroup])) {
                        chosen = index;
                        betterGroup = group;
                    }
                }
            }
            Classify(chosen, betterGroup, a_parVars);
        }

        // If one group too full, put remaining rects in the other
        if ((a_parVars->m_count[0] + a_parVars->m_count[1]) < a_parVars->m_total) {
            if (a_parVars->m_count[0] >= a_parVars->m_total - a_parVars->m_minFill) {
                group = 1;
            } else {
                group = 0;
            }
            for (int index = 0; index < a_parVars->m_total; ++index) {
                if (PartitionVars::kNotTaken == a_parVars->m_partition[index]) {
                    Classify(index, group, a_parVars);
                }
            }
        }

        RTreeAssert((a_parVars->m_count[0] + a_parVars->m_count[1]) == a_parVars->m_total);
        RTreeAssert((a_parVars->m_count[0] >= a_parVars->m_minFill) &&
            (a_parVars->m_count[1] >= a_parVars->m_minFill));
    }

    // Copy branches from the buffer into two nodes according to the partition.
    void LoadNodes(Node* a_nodeA, Node* a_nodeB, PartitionVars* a_parVars) {
        RTreeAssert(a_nodeA);
        RTreeAssert(a_nodeB);
        RTreeAssert(a_parVars);

        for (int index = 0; index < a_parVars->m_total; ++index) {
            RTreeAssert(a_parVars->m_partition[index] == 0 || a_parVars->m_partition[index] == 1);

            int targetNodeIndex = a_parVars->m_partition[index];
            Node* targetNodes[] = { a_nodeA, a_nodeB };

            // It is assured that AddBranch here will not cause a node split. 
            bool nodeWasSplit = AddBranch(&a_parVars->m_branchBuf[index], targetNodes[targetNodeIndex], nullptr);
            (void)nodeWasSplit;
            RTreeAssert(!nodeWasSplit);
        }
    }

    // Initialize a PartitionVars structure.
    void InitParVars(PartitionVars* a_parVars, int a_maxRects, int a_minFill) {
        RTreeAssert(a_parVars);

        a_parVars->m_count[0] = a_parVars->m_count[1] = 0;
        a_parVars->m_area[0] = a_parVars->m_area[1] = kElementTypeRealZero;
        a_parVars->m_total = a_maxRects;
        a_parVars->m_minFill = a_minFill;
        for (int index = 0; index < a_maxRects; ++index) {
            a_parVars->m_partition[index] = PartitionVars::kNotTaken;
        }
    }

    void PickSeeds(PartitionVars* a_parVars) {
        int seed0 = 0, seed1 = 0;
        _ElementTypeReal worst{ 0.0 }, waste{ 0.0 };
        _ElementTypeReal area[_MaxNodeCount + 1] = { 0.0, };

        for (int index = 0; index < a_parVars->m_total; ++index) {
            area[index] = CalcRectVolume(&a_parVars->m_branchBuf[index].m_rect);
        }

        worst = -a_parVars->m_coverSplitArea - 1;
        for (int indexA = 0; indexA < a_parVars->m_total - 1; ++indexA) {
            for (int indexB = indexA + 1; indexB < a_parVars->m_total; ++indexB) {
                auto oneRect = CombineRect(&a_parVars->m_branchBuf[indexA].m_rect, &a_parVars->m_branchBuf[indexB].m_rect);
                waste = CalcRectVolume(&oneRect) - area[indexA] - area[indexB];
                if (waste > worst) {
                    worst = waste;
                    seed0 = indexA;
                    seed1 = indexB;
                }
            }
        }

        Classify(seed0, 0, a_parVars);
        Classify(seed1, 1, a_parVars);
    }

    // Put a branch in one of the groups.
    void Classify(int a_index, int a_group, PartitionVars* a_parVars) {
        RTreeAssert(a_parVars);
        RTreeAssert(PartitionVars::kNotTaken == a_parVars->m_partition[a_index]);

        a_parVars->m_partition[a_index] = a_group;

        // Calculate combined rect
        if (a_parVars->m_count[a_group] == 0) {
            a_parVars->m_cover[a_group] = a_parVars->m_branchBuf[a_index].m_rect;
        } else {
            a_parVars->m_cover[a_group] = CombineRect(&a_parVars->m_branchBuf[a_index].m_rect, &a_parVars->m_cover[a_group]);
        }

        // Calculate volume of combined rect
        a_parVars->m_area[a_group] = CalcRectVolume(&a_parVars->m_cover[a_group]);

        ++a_parVars->m_count[a_group];
    }

    // Delete a data rectangle from an index structure.
    // Pass in a pointer to a Rect, the tid of the record, ptr to ptr to root node.
    // Returns 1 if record not found, 0 if success.
    // RemoveRect provides for eliminating the root.
    bool RemoveRect(Rect* a_rect, const DataType& a_id, Node** a_root) {
        RTreeAssert(a_rect && a_root);
        RTreeAssert(*a_root);

        ListNode* reInsertList{ nullptr };

        if (!RemoveRectRec(a_rect, a_id, *a_root, &reInsertList)) {
            // Found and deleted a data item
            // Reinsert any branches from eliminated nodes
            while (reInsertList) {
                Node* tempNode = reInsertList->m_node;

                for (int index = 0; index < tempNode->m_count; ++index) {
                    // TODO go over this code. should I use (tempNode->m_level - 1)?
                    InsertRect(tempNode->m_branch[index],
                        a_root,
                        tempNode->m_level);
                }

                ListNode* remLNode = reInsertList;
                reInsertList = reInsertList->m_next;

                FreeNode(remLNode->m_node);
                FreeListNode(remLNode);
            }

            // Check for redundant root (not leaf, 1 child) and eliminate TODO replace
            // if with while? In case there is a whole branch of redundant roots...
            if ((*a_root)->m_count == 1 && (*a_root)->IsInternalNode()) {
                auto tempNode = (*a_root)->m_branch[0].m_child;

                RTreeAssert(tempNode);
                FreeNode(*a_root);
                *a_root = tempNode;
            }
            return false;
        } else {
            return true;
        }
    }

    // Delete a rectangle from non-root part of an index structure.
    // Called by RemoveRect.  Descends tree recursively,
    // merges branches on the way back up.
    // Returns 1 if record not found, 0 if success.
    bool RemoveRectRec(Rect* a_rect, const DataType& a_id, Node* a_node, ListNode** a_listNode) {
        RTreeAssert(a_rect && a_node && a_listNode);
        RTreeAssert(a_node->m_level >= 0);

        if (a_node->IsInternalNode())  // not a leaf node
        {
            for (int index = 0; index < a_node->m_count; ++index) {
                if (Overlap(a_rect, &(a_node->m_branch[index].m_rect))) {
                    if (!RemoveRectRec(a_rect, a_id, a_node->m_branch[index].m_child, a_listNode)) {
                        if (a_node->m_branch[index].m_child->m_count >= _MinNodeCount) {
                            // child removed, just resize parent rect
                            a_node->m_branch[index].m_rect = NodeCover(a_node->m_branch[index].m_child);
                        } else {
                            // child removed, not enough entries in node, eliminate node
                            ReInsert(a_node->m_branch[index].m_child, a_listNode);
                            DisconnectBranch(a_node, index); // Must return after this call as count has changed
                        }
                        return false;
                    }
                }
            }
            return true;
        } else // A leaf node
        {
            for (int index = 0; index < a_node->m_count; ++index) {
                if (a_node->m_branch[index].m_data == a_id) {
                    DisconnectBranch(a_node, index); // Must return after this call as count has changed
                    return false;
                }
            }
            return true;
        }
    }

    // Allocate space for a node in the list used in DeletRect to
    // store Nodes that are too empty.
    ListNode* AllocListNode() {
#ifdef RTREE_DONT_USE_MEMPOOLS
        return new ListNode;
#else // RTREE_DONT_USE_MEMPOOLS
        // EXAMPLE
#endif // RTREE_DONT_USE_MEMPOOLS
    }

    void FreeListNode(ListNode* a_listNode) {
#ifdef RTREE_DONT_USE_MEMPOOLS
        delete a_listNode;
#else // RTREE_DONT_USE_MEMPOOLS
        // EXAMPLE
#endif // RTREE_DONT_USE_MEMPOOLS
    }

    // Decide whether two rectangles overlap.
    bool Overlap(Rect* a_rectA, Rect* a_rectB) const {
        RTreeAssert(a_rectA && a_rectB);

        for (int index = 0; index < kNumDimensions; ++index) {
            if (a_rectA->m_min[index] > a_rectB->m_max[index] ||
                a_rectB->m_min[index] > a_rectA->m_max[index]) {
                return false;
            }
        }
        return true;
    }

    // Add a node to the reinsertion list.  All its branches will later
    // be reinserted into the index structure.
    void ReInsert(Node* a_node, ListNode** a_listNode) {
        ListNode* newListNode;

        newListNode = AllocListNode();
        newListNode->m_node = a_node;
        newListNode->m_next = *a_listNode;
        *a_listNode = newListNode;
    }

    // Search in an index tree or subtree for all data retangles that overlap the argument rectangle.
    bool Search(Node* a_node, Rect* a_rect, int& a_foundCount, std::function<bool(const DataType&)> callback) const {
        RTreeAssert(a_node);
        RTreeAssert(a_node->m_level >= 0);
        RTreeAssert(a_rect);

        if (a_node->IsInternalNode()) {
            // This is an internal node in the tree
            for (int index = 0; index < a_node->m_count; ++index) {
                if (Overlap(a_rect, &a_node->m_branch[index].m_rect)) {
                    if (!Search(a_node->m_branch[index].m_child, a_rect, a_foundCount, callback)) {
                        // The callback indicated to stop searching
                        return false;
                    }
                }
            }
        } else {
            // This is a leaf node
            for (int index = 0; index < a_node->m_count; ++index) {
                if (Overlap(a_rect, &a_node->m_branch[index].m_rect)) {
                    const auto& id = a_node->m_branch[index].m_data;
                    ++a_foundCount;

                    if (callback && !callback(id)) {
                        return false; // Don't continue searching
                    }
                }
            }
        }

        return true; // Continue searching
    }

    void RemoveAllRec(Node* a_node) {
        RTreeAssert(a_node);
        RTreeAssert(a_node->m_level >= 0);

        if (a_node->IsInternalNode()) // This is an internal node in the tree
        {
            for (int index = 0; index < a_node->m_count; ++index) {
                RemoveAllRec(a_node->m_branch[index].m_child);
            }
        }
        FreeNode(a_node);
    }

    void Reset() {
#ifdef RTREE_DONT_USE_MEMPOOLS
        // Delete all existing nodes
        RemoveAllRec(m_root);
#else // RTREE_DONT_USE_MEMPOOLS
        // Just reset memory pools.  We are not using complex types
        // EXAMPLE
#endif // RTREE_DONT_USE_MEMPOOLS
    }

    void CountRec(Node* a_node, int& a_count) const {
        if (a_node->IsInternalNode())  // not a leaf node
        {
            for (int index = 0; index < a_node->m_count; ++index) {
                CountRec(a_node->m_branch[index].m_child, a_count);
            }
        } else // A leaf node
        {
            a_count += a_node->m_count;
        }
    }

    bool SaveRec(Node* a_node, RTFileStream& a_stream) {
        a_stream.Write(a_node->m_level);
        a_stream.Write(a_node->m_count);

        if (a_node->IsInternalNode())  // not a leaf node
        {
            for (int index = 0; index < a_node->m_count; ++index) {
                const auto& curBranch = a_node->m_branch[index];

                a_stream.WriteArray(curBranch->m_rect.m_min, kNumDimensions);
                a_stream.WriteArray(curBranch->m_rect.m_max, kNumDimensions);

                SaveRec(curBranch->m_child, a_stream);
            }
        } else // A leaf node
        {
            for (int index = 0; index < a_node->m_count; ++index) {
                const auto& curBranch = &a_node->m_branch[index];

                a_stream.WriteArray(curBranch->m_rect.m_min, kNumDimensions);
                a_stream.WriteArray(curBranch->m_rect.m_max, kNumDimensions);

                a_stream.Write(curBranch->m_data);
            }
        }

        return true; // Should do more error checking on I/O operations
    }

    bool LoadRec(Node* a_node, RTFileStream& a_stream) {
        a_stream.Read(a_node->m_level);
        a_stream.Read(a_node->m_count);

        if (a_node->IsInternalNode())  // not a leaf node
        {
            for (int index = 0; index < a_node->m_count; ++index) {
                Branch* curBranch = &a_node->m_branch[index];

                a_stream.ReadArray(curBranch->m_rect.m_min, kNumDimensions);
                a_stream.ReadArray(curBranch->m_rect.m_max, kNumDimensions);

                curBranch->m_child = AllocNode();
                LoadRec(curBranch->m_child, a_stream);
            }
        } else // A leaf node
        {
            for (int index = 0; index < a_node->m_count; ++index) {
                Branch* curBranch = &a_node->m_branch[index];

                a_stream.ReadArray(curBranch->m_rect.m_min, kNumDimensions);
                a_stream.ReadArray(curBranch->m_rect.m_max, kNumDimensions);

                a_stream.Read(curBranch->m_data);
            }
        }

        return true; // Should do more error checking on I/O operations
    }

    void CopyRec(Node* current, Node* other) {
        current->m_level = other->m_level;
        current->m_count = other->m_count;

        if (current->IsInternalNode())  // not a leaf node
        {
            for (int index = 0; index < current->m_count; ++index) {
                auto& currentBranch = current->m_branch[index];
                const auto& otherBranch = other->m_branch[index];

                std::copy(otherBranch.m_rect.m_min,
                    otherBranch.m_rect.m_min + kNumDimensions,
                    currentBranch.m_rect.m_min);

                std::copy(otherBranch.m_rect.m_max,
                    otherBranch.m_rect.m_max + kNumDimensions,
                    currentBranch.m_rect.m_max);

                currentBranch.m_child = AllocNode();
                CopyRec(currentBranch.m_child, otherBranch.m_child);
            }
        } else // A leaf node
        {
            for (int index = 0; index < current->m_count; ++index) {
                auto& currentBranch = current->m_branch[index];
                const auto& otherBranch = other->m_branch[index];

                std::copy(otherBranch.m_rect.m_min,
                    otherBranch.m_rect.m_min + kNumDimensions,
                    currentBranch.m_rect.m_min);

                std::copy(otherBranch.m_rect.m_max,
                    otherBranch.m_rect.m_max + kNumDimensions,
                    currentBranch.m_rect.m_max);

                currentBranch.m_data = otherBranch.m_data;
            }
        }
    }

    Node* m_root = nullptr;                                         ///< Root of tree
    ElementTypeReal m_unitSphereVolume = kElementTypeRealZero;      ///< Unit sphere constant for required number of dimensions
};

#endif //RTREE_H
