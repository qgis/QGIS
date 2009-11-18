/*
 * R-Tree index
 *
 * from http://www.superliminal.com/
 *
 * LICENSE : Entirely free for all uses. Enjoy!
 *
 * AUTORS
 *    - 1983 Original algorithm and test code by Antonin Guttman and Michael Stonebraker, UC Berkely
 *    - 1994 ANCI C ported from original test code by Melinda Green - melinda@superliminal.com
 *    - 1995 Sphere volume fix for degeneracy problem submitted by Paul Brook
 *    - 2004 Templated C++ port by Greg Douglas
 *    - 2008 Portability issues fixed by Maxence Laurent
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _CRT_SECURE_NO_DEPRECATE


#ifndef RTREE_H
#define RTREE_H

#include <cstdio>
#include <cmath>
#include <cassert>
#include <cstdlib>

#define ASSERT assert // RTree uses ASSERT( condition )
#ifndef Min
#define Min(a,b) (a<b?a:b)
#endif //Min
#ifndef Max
#define Max(a,b) (a>b?a:b)
#endif //Max

//
// RTree.h
//

#define RTREE_TEMPLATE template<class DATATYPE, class ELEMTYPE, int NUMDIMS, class ELEMTYPEREAL, int TMAXNODES, int TMINNODES>
#define RTREE_QUAL RTree<DATATYPE, ELEMTYPE, NUMDIMS, ELEMTYPEREAL, TMAXNODES, TMINNODES>

#define RTREE_DONT_USE_MEMPOOLS // This version does not contain a fixed memory allocator, fill in lines with EXAMPLE to implement one.
#define RTREE_USE_SPHERICAL_VOLUME // Better split classification, may be slower on some systems

namespace pal
{

// Fwd decl
  class RTFileStream;  // File I/O helper class, look below for implementation and notes.


/// \class RTree
/// Implementation of RTree, a multidimensional bounding rectangle tree.
/// Example usage: For a 3-dimensional tree use RTree<Object*, float, 3> myTree;
///
/// This modified, templated C++ version by Greg Douglas at Auran (http://www.auran.com)
///
/// DATATYPE Referenced data, should be int, void*, obj* etc. no larger than sizeof<void*> and simple type
/// ELEMTYPE Type of element such as int or float
/// NUMDIMS Number of dimensions such as 2 or 3
/// ELEMTYPEREAL Type of element that allows fractional and large values such as float or double, for use in volume calcs
///
/// NOTES: Inserting and removing data requires the knowledge of its constant Minimal Bounding Rectangle.
///        This version uses new/delete for nodes, I recommend using a fixed size allocator for efficiency.
///        Instead of using a callback function for returned results, I recommend and efficient pre-sized, grow-only memory
///        array similar to MFC CArray or STL Vector for returning search query result.
///
  template < class DATATYPE, class ELEMTYPE, int NUMDIMS,
  class ELEMTYPEREAL = ELEMTYPE, int TMAXNODES = 8, int TMINNODES = TMAXNODES / 2 >
  class RTree
  {
    protected:

      struct Node;  // Fwd decl.  Used by other internal structs and iterator

    public:

      // These constant must be declared after Branch and before Node struct
      // Stuck up here for MSVC 6 compiler.  NSVC .NET 2003 is much happier.
      enum
      {
        MAXNODES = TMAXNODES,                         ///< Max elements in node
        MINNODES = TMINNODES                         ///< Min elements in node
      };


    public:

      RTree();
      virtual ~RTree();

      /// Insert entry
      /// \param a_min Min of bounding rect
      /// \param a_max Max of bounding rect
      /// \param a_dataId Positive Id of data.  Maybe zero, but negative numbers not allowed.
      void Insert( const ELEMTYPE a_min[NUMDIMS], const ELEMTYPE a_max[NUMDIMS], const DATATYPE& a_dataId );

      /// Remove entry
      /// \param a_min Min of bounding rect
      /// \param a_max Max of bounding rect
      /// \param a_dataId Positive Id of data.  Maybe zero, but negative numbers not allowed.
      void Remove( const ELEMTYPE a_min[NUMDIMS], const ELEMTYPE a_max[NUMDIMS], const DATATYPE& a_dataId );

      /// Find all within search rectangle
      /// \param a_min Min of search bounding rect
      /// \param a_max Max of search bounding rect
      /// \param a_searchResult Search result array.  Caller should set grow size. Function will reset, not append to array.
      /// \param a_resultCallback Callback function to return result.  Callback should return 'true' to continue searching
      /// \param a_context User context to pass as parameter to a_resultCallback
      /// \return Returns the number of entries found
      int Search( const ELEMTYPE a_min[NUMDIMS], const ELEMTYPE a_max[NUMDIMS], bool a_resultCallback( DATATYPE a_data, void* a_context ), void* a_context );

      /// Remove all entries from tree
      void RemoveAll();

      /// Count the data elements in this container.  This is slow as no internal counter is maintained.
      int Count();

      /// Load tree contents from file
      bool Load( const char* a_fileName );
      /// Load tree contents from stream
      bool Load( RTFileStream& a_stream );


      /// Save tree contents to file
      bool Save( const char* a_fileName );
      /// Save tree contents to stream
      bool Save( RTFileStream& a_stream );

      /// Iterator is not remove safe.
      class Iterator
      {
        private:

          enum { MAX_STACK = 32 }; //  Max stack size. Allows almost n^32 where n is number of branches in node

          struct StackElement
          {
            Node* m_node;
            int m_branchIndex;
          };

        public:

          Iterator()                                    { Init(); }

          ~Iterator()                                   { }

          /// Is iterator invalid
          bool IsNull()                                 { return ( m_tos <= 0 ); }

          /// Is iterator pointing to valid data
          bool IsNotNull()                              { return ( m_tos > 0 ); }

          /// Access the current data element. Caller must be sure iterator is not NULL first.
          DATATYPE& operator*()
          {
            ASSERT( IsNotNull() );
            StackElement& curTos = m_stack[m_tos - 1];
            return curTos.m_node->m_branch[curTos.m_branchIndex].m_data;
          }

          /// Access the current data element. Caller must be sure iterator is not NULL first.
          const DATATYPE& operator*() const
          {
            ASSERT( IsNotNull() );
            StackElement& curTos = m_stack[m_tos - 1];
            return curTos.m_node->m_branch[curTos.m_branchIndex].m_data;
          }

          /// Find the next data element
          bool operator++()                             { return FindNextData(); }

        private:

          /// Reset iterator
          void Init()                                   { m_tos = 0; }

          /// Find the next data element in the tree (For internal use only)
          bool FindNextData()
          {
            for ( ;; )
            {
              if ( m_tos <= 0 )
              {
                return false;
              }
              StackElement curTos = Pop(); // Copy stack top cause it may change as we use it

              if ( curTos.m_node->IsLeaf() )
              {
                // Keep walking through data while we can
                if ( curTos.m_branchIndex + 1 < curTos.m_node->m_count )
                {
                  // There is more data, just point to the next one
                  Push( curTos.m_node, curTos.m_branchIndex + 1 );
                  return true;
                }
                // No more data, so it will fall back to previous level
              }
              else
              {
                if ( curTos.m_branchIndex + 1 < curTos.m_node->m_count )
                {
                  // Push sibling on for future tree walk
                  // This is the 'fall back' node when we finish with the current level
                  Push( curTos.m_node, curTos.m_branchIndex + 1 );
                }
                // Since cur node is not a leaf, push first of next level to get deeper into the tree
                Node* nextLevelnode = curTos.m_node->m_branch[curTos.m_branchIndex].m_child;
                Push( nextLevelnode, 0 );

                // If we pushed on a new leaf, exit as the data is ready at TOS
                if ( nextLevelnode->IsLeaf() )
                {
                  return true;
                }
              }
            }
          }

          /// Push node and branch onto iteration stack (For internal use only)
          void Push( Node* a_node, int a_branchIndex )
          {
            m_stack[m_tos].m_node = a_node;
            m_stack[m_tos].m_branchIndex = a_branchIndex;
            ++m_tos;
            ASSERT( m_tos <= MAX_STACK );
          }

          /// Pop element off iteration stack (For internal use only)
          StackElement& Pop()
          {
            ASSERT( m_tos > 0 );
            --m_tos;
            return m_stack[m_tos];
          }

          StackElement m_stack[MAX_STACK];              ///< Stack as we are doing iteration instead of recursion
          int m_tos;                                    ///< Top Of Stack index

          friend class RTree; // Allow hiding of non-public functions while allowing manipulation by logical owner
      };

      /// Get 'first' for iteration
      void GetFirst( Iterator& a_it )
      {
        a_it.Init();
        if ( m_root && ( m_root->m_count > 0 ) )
        {
          a_it.Push( m_root, 0 );
          a_it.FindNextData();
        }
      }

      /// Get Next for iteration
      void GetNext( Iterator& a_it )                    { ++a_it; }

      /// Is iterator NULL, or at end?
      bool IsNull( Iterator& a_it )                     { return a_it.IsNull(); }

      /// Get object at iterator position
      DATATYPE& GetAt( Iterator& a_it )                 { return *a_it; }

    protected:

      /// Minimal bounding rectangle (n-dimensional)
      struct Rect
      {
        ELEMTYPE m_min[NUMDIMS];                      ///< Min dimensions of bounding box
        ELEMTYPE m_max[NUMDIMS];                      ///< Max dimensions of bounding box
      };

      /// May be data or may be another subtree
      /// The parents level determines this.
      /// If the parents level is 0, then this is data
      struct Branch
      {
        Rect m_rect;                                  ///< Bounds
        union
        {
          Node* m_child;                              ///< Child node
          DATATYPE m_data;                            ///< Data Id or Ptr
        };
      };

      /// Node for each branch level
      struct Node
      {
        bool IsInternalNode()                         { return ( m_level > 0 ); } // Not a leaf, but a internal node
        bool IsLeaf()                                 { return ( m_level == 0 ); } // A leaf, contains data

        int m_count;                                  ///< Count
        int m_level;                                  ///< Leaf is zero, others positive
        Branch m_branch[MAXNODES];                    ///< Branch
      };

      /// A link list of nodes for reinsertion after a delete operation
      struct ListNode
      {
        ListNode* m_next;                             ///< Next in list
        Node* m_node;                                 ///< Node
      };

      /// Variables for finding a split partition
      struct PartitionVars
      {
        int m_partition[MAXNODES+1];
        int m_total;
        int m_minFill;
        int m_taken[MAXNODES+1];
        int m_count[2];
        Rect m_cover[2];
        ELEMTYPEREAL m_area[2];

        Branch m_branchBuf[MAXNODES+1];
        int m_branchCount;
        Rect m_coverSplit;
        ELEMTYPEREAL m_coverSplitArea;
      };

      Node* AllocNode();
      void FreeNode( Node* a_node );
      void InitNode( Node* a_node );
      void InitRect( Rect* a_rect );
      bool InsertRectRec( Rect* a_rect, const DATATYPE& a_id, Node* a_node, Node** a_newNode, int a_level );
      bool InsertRect( Rect* a_rect, const DATATYPE& a_id, Node** a_root, int a_level );
      Rect NodeCover( Node* a_node );
      bool AddBranch( Branch* a_branch, Node* a_node, Node** a_newNode );
      void DisconnectBranch( Node* a_node, int a_index );
      int PickBranch( Rect* a_rect, Node* a_node );
      Rect CombineRect( Rect* a_rectA, Rect* a_rectB );
      void SplitNode( Node* a_node, Branch* a_branch, Node** a_newNode );
      ELEMTYPEREAL RectSphericalVolume( Rect* a_rect );
      ELEMTYPEREAL RectVolume( Rect* a_rect );
      ELEMTYPEREAL CalcRectVolume( Rect* a_rect );
      void GetBranches( Node* a_node, Branch* a_branch, PartitionVars* a_parVars );
      void ChoosePartition( PartitionVars* a_parVars, int a_minFill );
      void LoadNodes( Node* a_nodeA, Node* a_nodeB, PartitionVars* a_parVars );
      void InitParVars( PartitionVars* a_parVars, int a_maxRects, int a_minFill );
      void PickSeeds( PartitionVars* a_parVars );
      void Classify( int a_index, int a_group, PartitionVars* a_parVars );
      bool RemoveRect( Rect* a_rect, const DATATYPE& a_id, Node** a_root );
      bool RemoveRectRec( Rect* a_rect, const DATATYPE& a_id, Node* a_node, ListNode** a_listNode );
      ListNode* AllocListNode();
      void FreeListNode( ListNode* a_listNode );
      bool Overlap( Rect* a_rectA, Rect* a_rectB );
      void ReInsert( Node* a_node, ListNode** a_listNode );
      bool Search( Node* a_node, Rect* a_rect, int& a_foundCount, bool a_resultCallback( DATATYPE a_data, void* a_context ), void* a_context );
      void RemoveAllRec( Node* a_node );
      void Reset();
      void CountRec( Node* a_node, int& a_count );

      bool SaveRec( Node* a_node, RTFileStream& a_stream );
      bool LoadRec( Node* a_node, RTFileStream& a_stream );

      Node* m_root;                                    ///< Root of tree
      ELEMTYPEREAL m_unitSphereVolume;                 ///< Unit sphere constant for required number of dimensions
  };


// Because there is not stream support, this is a quick and dirty file I/O helper.
// Users will likely replace its usage with a Stream implementation from their favorite API.
  class RTFileStream
  {
      FILE* m_file;

    public:


      RTFileStream()
      {
        m_file = NULL;
      }

      ~RTFileStream()
      {
        Close();
      }

      bool OpenRead( const char* a_fileName )
      {
        m_file = fopen( a_fileName, "rb" );
        if ( !m_file )
        {
          return false;
        }
        return true;
      }

      bool OpenWrite( const char* a_fileName )
      {
        m_file = fopen( a_fileName, "wb" );
        if ( !m_file )
        {
          return false;
        }
        return true;
      }

      void Close()
      {
        if ( m_file )
        {
          fclose( m_file );
          m_file = NULL;
        }
      }

      template< typename TYPE >
      size_t Write( const TYPE& a_value )
      {
        ASSERT( m_file );
        return fwrite(( void* ) &a_value, sizeof( a_value ), 1, m_file );
      }

      template< typename TYPE >
      size_t WriteArray( const TYPE* a_array, int a_count )
      {
        ASSERT( m_file );
        return fwrite(( void* ) a_array, sizeof( TYPE ) * a_count, 1, m_file );
      }

      template< typename TYPE >
      size_t Read( TYPE& a_value )
      {
        ASSERT( m_file );
        return fread(( void* ) &a_value, sizeof( a_value ), 1, m_file );
      }

      template< typename TYPE >
      size_t ReadArray( TYPE* a_array, int a_count )
      {
        ASSERT( m_file );
        return fread(( void* ) a_array, sizeof( TYPE ) * a_count, 1, m_file );
      }
  };


  RTREE_TEMPLATE
  RTREE_QUAL::RTree()
  {
    ASSERT( MAXNODES > MINNODES );
    ASSERT( MINNODES > 0 );


    // We only support machine word size simple data type eg. integer index or object pointer.
    // Since we are storing as union with non data branch
    ASSERT( sizeof( DATATYPE ) == sizeof( void* ) || sizeof( DATATYPE ) == sizeof( int ) );

    // Precomputed volumes of the unit spheres for the first few dimensions
    const float UNIT_SPHERE_VOLUMES[] =
    {
      0.000000f, 2.000000f, 3.141593f, // Dimension  0,1,2
      4.188790f, 4.934802f, 5.263789f, // Dimension  3,4,5
      5.167713f, 4.724766f, 4.058712f, // Dimension  6,7,8
      3.298509f, 2.550164f, 1.884104f, // Dimension  9,10,11
      1.335263f, 0.910629f, 0.599265f, // Dimension  12,13,14
      0.381443f, 0.235331f, 0.140981f, // Dimension  15,16,17
      0.082146f, 0.046622f, 0.025807f, // Dimension  18,19,20
    };

    m_root = AllocNode();
    m_root->m_level = 0;
    m_unitSphereVolume = ( ELEMTYPEREAL ) UNIT_SPHERE_VOLUMES[NUMDIMS];
  }


  RTREE_TEMPLATE
  RTREE_QUAL::~RTree()
  {
    Reset(); // Free, or reset node memory
  }


  RTREE_TEMPLATE
  void RTREE_QUAL::Insert( const ELEMTYPE a_min[NUMDIMS], const ELEMTYPE a_max[NUMDIMS], const DATATYPE& a_dataId )
  {
#ifdef _DEBUG
    for ( int index = 0; index < NUMDIMS; ++index )
    {
      ASSERT( a_min[index] <= a_max[index] );
    }
#endif //_DEBUG

    Rect rect;

    for ( int axis = 0; axis < NUMDIMS; ++axis )
    {
      rect.m_min[axis] = a_min[axis];
      rect.m_max[axis] = a_max[axis];
    }

    InsertRect( &rect, a_dataId, &m_root, 0 );
  }


  RTREE_TEMPLATE
  void RTREE_QUAL::Remove( const ELEMTYPE a_min[NUMDIMS], const ELEMTYPE a_max[NUMDIMS], const DATATYPE& a_dataId )
  {
#ifdef _DEBUG
    for ( int index = 0; index < NUMDIMS; ++index )
    {
      ASSERT( a_min[index] <= a_max[index] );
    }
#endif //_DEBUG

    Rect rect;

    for ( int axis = 0; axis < NUMDIMS; ++axis )
    {
      rect.m_min[axis] = a_min[axis];
      rect.m_max[axis] = a_max[axis];
    }

    RemoveRect( &rect, a_dataId, &m_root );
  }


  RTREE_TEMPLATE
  int RTREE_QUAL::Search( const ELEMTYPE a_min[NUMDIMS], const ELEMTYPE a_max[NUMDIMS], bool a_resultCallback( DATATYPE a_data, void* a_context ), void* a_context )
  {
#ifdef _DEBUG
    for ( int index = 0; index < NUMDIMS; ++index )
    {
      ASSERT( a_min[index] <= a_max[index] );
    }
#endif //_DEBUG

    Rect rect;

    for ( int axis = 0; axis < NUMDIMS; ++axis )
    {
      rect.m_min[axis] = a_min[axis];
      rect.m_max[axis] = a_max[axis];
    }

    // NOTE: May want to return search result another way, perhaps returning the number of found elements here.

    int foundCount = 0;
    Search( m_root, &rect, foundCount, a_resultCallback, a_context );

    return foundCount;
  }


  RTREE_TEMPLATE
  int RTREE_QUAL::Count()
  {
    int count = 0;
    CountRec( m_root, count );

    return count;
  }



  RTREE_TEMPLATE
  void RTREE_QUAL::CountRec( Node* a_node, int& a_count )
  {
    if ( a_node->IsInternalNode() ) // not a leaf node
    {
      for ( int index = 0; index < a_node->m_count; ++index )
      {
        CountRec( a_node->m_branch[index].m_child, a_count );
      }
    }
    else   // A leaf node
    {
      a_count += a_node->m_count;
    }
  }


  RTREE_TEMPLATE
  bool RTREE_QUAL::Load( const char* a_fileName )
  {
    RemoveAll(); // Clear existing tree

    RTFileStream stream;
    if ( !stream.OpenRead( a_fileName ) )
    {
      return false;
    }

    bool result = Load( stream );

    stream.Close();

    return result;
  }



  RTREE_TEMPLATE
  bool RTREE_QUAL::Load( RTFileStream& a_stream )
  {
    // Write some kind of header
    int _dataFileId = ( 'R' << 0 ) | ( 'T' << 8 ) | ( 'R' << 16 ) | ( 'E' << 24 );
    int _dataSize = sizeof( DATATYPE );
    int _dataNumDims = NUMDIMS;
    int _dataElemSize = sizeof( ELEMTYPE );
    int _dataElemRealSize = sizeof( ELEMTYPEREAL );
    int _dataMaxNodes = TMAXNODES;
    int _dataMinNodes = TMINNODES;

    int dataFileId = 0;
    int dataSize = 0;
    int dataNumDims = 0;
    int dataElemSize = 0;
    int dataElemRealSize = 0;
    int dataMaxNodes = 0;
    int dataMinNodes = 0;

    a_stream.Read( dataFileId );
    a_stream.Read( dataSize );
    a_stream.Read( dataNumDims );
    a_stream.Read( dataElemSize );
    a_stream.Read( dataElemRealSize );
    a_stream.Read( dataMaxNodes );
    a_stream.Read( dataMinNodes );

    bool result = false;

    // Test if header was valid and compatible
    if (( dataFileId == _dataFileId )
        && ( dataSize == _dataSize )
        && ( dataNumDims == _dataNumDims )
        && ( dataElemSize == _dataElemSize )
        && ( dataElemRealSize == _dataElemRealSize )
        && ( dataMaxNodes == _dataMaxNodes )
        && ( dataMinNodes == _dataMinNodes )
       )
    {
      // Recursively load tree
      result = LoadRec( m_root, a_stream );
    }

    return result;
  }


  RTREE_TEMPLATE
  bool RTREE_QUAL::LoadRec( Node* a_node, RTFileStream& a_stream )
  {
    a_stream.Read( a_node->m_level );
    a_stream.Read( a_node->m_count );

    if ( a_node->IsInternalNode() ) // not a leaf node
    {
      for ( int index = 0; index < a_node->m_count; ++index )
      {
        Branch* curBranch = &a_node->m_branch[index];

        a_stream.ReadArray( curBranch->m_rect.m_min, NUMDIMS );
        a_stream.ReadArray( curBranch->m_rect.m_max, NUMDIMS );

        curBranch->m_child = AllocNode();
        LoadRec( curBranch->m_child, a_stream );
      }
    }
    else   // A leaf node
    {
      for ( int index = 0; index < a_node->m_count; ++index )
      {
        Branch* curBranch = &a_node->m_branch[index];

        a_stream.ReadArray( curBranch->m_rect.m_min, NUMDIMS );
        a_stream.ReadArray( curBranch->m_rect.m_max, NUMDIMS );

        a_stream.Read( curBranch->m_data );
      }
    }

    return true; // Should do more error checking on I/O operations
  }


  RTREE_TEMPLATE
  bool RTREE_QUAL::Save( const char* a_fileName )
  {
    RTFileStream stream;
    if ( !stream.OpenWrite( a_fileName ) )
    {
      return false;
    }

    bool result = Save( stream );

    stream.Close();

    return result;
  }


  RTREE_TEMPLATE
  bool RTREE_QUAL::Save( RTFileStream& a_stream )
  {
    // Write some kind of header
    int dataFileId = ( 'R' << 0 ) | ( 'T' << 8 ) | ( 'R' << 16 ) | ( 'E' << 24 );
    int dataSize = sizeof( DATATYPE );
    int dataNumDims = NUMDIMS;
    int dataElemSize = sizeof( ELEMTYPE );
    int dataElemRealSize = sizeof( ELEMTYPEREAL );
    int dataMaxNodes = TMAXNODES;
    int dataMinNodes = TMINNODES;

    a_stream.Write( dataFileId );
    a_stream.Write( dataSize );
    a_stream.Write( dataNumDims );
    a_stream.Write( dataElemSize );
    a_stream.Write( dataElemRealSize );
    a_stream.Write( dataMaxNodes );
    a_stream.Write( dataMinNodes );

    // Recursively save tree
    bool result = SaveRec( m_root, a_stream );

    return result;
  }


  RTREE_TEMPLATE
  bool RTREE_QUAL::SaveRec( Node* a_node, RTFileStream& a_stream )
  {
    a_stream.Write( a_node->m_level );
    a_stream.Write( a_node->m_count );

    if ( a_node->IsInternalNode() ) // not a leaf node
    {
      for ( int index = 0; index < a_node->m_count; ++index )
      {
        Branch* curBranch = &a_node->m_branch[index];

        a_stream.WriteArray( curBranch->m_rect.m_min, NUMDIMS );
        a_stream.WriteArray( curBranch->m_rect.m_max, NUMDIMS );

        SaveRec( curBranch->m_child, a_stream );
      }
    }
    else   // A leaf node
    {
      for ( int index = 0; index < a_node->m_count; ++index )
      {
        Branch* curBranch = &a_node->m_branch[index];

        a_stream.WriteArray( curBranch->m_rect.m_min, NUMDIMS );
        a_stream.WriteArray( curBranch->m_rect.m_max, NUMDIMS );

        a_stream.Write( curBranch->m_data );
      }
    }

    return true; // Should do more error checking on I/O operations
  }


  RTREE_TEMPLATE
  void RTREE_QUAL::RemoveAll()
  {
    // Delete all existing nodes
    Reset();

    m_root = AllocNode();
    m_root->m_level = 0;
  }


  RTREE_TEMPLATE
  void RTREE_QUAL::Reset()
  {
#ifdef RTREE_DONT_USE_MEMPOOLS
    // Delete all existing nodes
    RemoveAllRec( m_root );
#else // RTREE_DONT_USE_MEMPOOLS
    // Just reset memory pools.  We are not using complex types
    // EXAMPLE
#endif // RTREE_DONT_USE_MEMPOOLS
  }


  RTREE_TEMPLATE
  void RTREE_QUAL::RemoveAllRec( Node* a_node )
  {
    ASSERT( a_node );
    ASSERT( a_node->m_level >= 0 );

    if ( a_node->IsInternalNode() ) // This is an internal node in the tree
    {
      for ( int index = 0; index < a_node->m_count; ++index )
      {
        RemoveAllRec( a_node->m_branch[index].m_child );
      }
    }
    FreeNode( a_node );
  }


  RTREE_TEMPLATE
  typename RTREE_QUAL::Node* RTREE_QUAL::AllocNode()
  {
    Node* newNode;
#ifdef RTREE_DONT_USE_MEMPOOLS
    newNode = new Node;
#else // RTREE_DONT_USE_MEMPOOLS
    // EXAMPLE
#endif // RTREE_DONT_USE_MEMPOOLS
    InitNode( newNode );
    return newNode;
  }


  RTREE_TEMPLATE
  void RTREE_QUAL::FreeNode( Node* a_node )
  {
    ASSERT( a_node );

#ifdef RTREE_DONT_USE_MEMPOOLS
    delete a_node;
#else // RTREE_DONT_USE_MEMPOOLS
    // EXAMPLE
#endif // RTREE_DONT_USE_MEMPOOLS
  }


// Allocate space for a node in the list used in DeletRect to
// store Nodes that are too empty.
  RTREE_TEMPLATE
  typename RTREE_QUAL::ListNode* RTREE_QUAL::AllocListNode()
  {
#ifdef RTREE_DONT_USE_MEMPOOLS
    return new ListNode;
#else // RTREE_DONT_USE_MEMPOOLS
    // EXAMPLE
#endif // RTREE_DONT_USE_MEMPOOLS
  }


  RTREE_TEMPLATE
  void RTREE_QUAL::FreeListNode( ListNode* a_listNode )
  {
#ifdef RTREE_DONT_USE_MEMPOOLS
    delete a_listNode;
#else // RTREE_DONT_USE_MEMPOOLS
    // EXAMPLE
#endif // RTREE_DONT_USE_MEMPOOLS
  }


  RTREE_TEMPLATE
  void RTREE_QUAL::InitNode( Node* a_node )
  {
    a_node->m_count = 0;
    a_node->m_level = -1;
  }


  RTREE_TEMPLATE
  void RTREE_QUAL::InitRect( Rect* a_rect )
  {
    for ( int index = 0; index < NUMDIMS; ++index )
    {
      a_rect->m_min[index] = ( ELEMTYPE ) 0;
      a_rect->m_max[index] = ( ELEMTYPE ) 0;
    }
  }


// Inserts a new data rectangle into the index structure.
// Recursively descends tree, propagates splits back up.
// Returns 0 if node was not split.  Old node updated.
// If node was split, returns 1 and sets the pointer pointed to by
// new_node to point to the new node.  Old node updated to become one of two.
// The level argument specifies the number of steps up from the leaf
// level to insert; e.g. a data rectangle goes in at level = 0.
  RTREE_TEMPLATE
  bool RTREE_QUAL::InsertRectRec( Rect* a_rect, const DATATYPE& a_id, Node* a_node, Node** a_newNode, int a_level )
  {
    ASSERT( a_rect && a_node && a_newNode );
    ASSERT( a_level >= 0 && a_level <= a_node->m_level );

    int index;
    Branch branch;
    Node* otherNode;

    // Still above level for insertion, go down tree recursively
    if ( a_node->m_level > a_level )
    {
      index = PickBranch( a_rect, a_node );
      if ( !InsertRectRec( a_rect, a_id, a_node->m_branch[index].m_child, &otherNode, a_level ) )
      {
        // Child was not split
        a_node->m_branch[index].m_rect = CombineRect( a_rect, & ( a_node->m_branch[index].m_rect ) );
        return false;
      }
      else   // Child was split
      {
        a_node->m_branch[index].m_rect = NodeCover( a_node->m_branch[index].m_child );
        branch.m_child = otherNode;
        branch.m_rect = NodeCover( otherNode );
        return AddBranch( &branch, a_node, a_newNode );
      }
    }
    else if ( a_node->m_level == a_level ) // Have reached level for insertion. Add rect, split if necessary
    {
      branch.m_rect = *a_rect;
      branch.m_child = ( Node* ) a_id;
      // Child field of leaves contains id of data record
      return AddBranch( &branch, a_node, a_newNode );
    }
    else
    {
      // Should never occur
      ASSERT( 0 );
      return false;
    }
  }


// Insert a data rectangle into an index structure.
// InsertRect provides for splitting the root;
// returns 1 if root was split, 0 if it was not.
// The level argument specifies the number of steps up from the leaf
// level to insert; e.g. a data rectangle goes in at level = 0.
// InsertRect2 does the recursion.
//
  RTREE_TEMPLATE
  bool RTREE_QUAL::InsertRect( Rect* a_rect, const DATATYPE& a_id, Node** a_root, int a_level )
  {
    ASSERT( a_rect && a_root );
    ASSERT( a_level >= 0 && a_level <= ( *a_root )->m_level );
#ifdef _DEBUG
    for ( int index = 0; index < NUMDIMS; ++index )
    {
      ASSERT( a_rect->m_min[index] <= a_rect->m_max[index] );
    }
#endif //_DEBUG  

    Node* newRoot;
    Node* newNode;
    Branch branch;

    if ( InsertRectRec( a_rect, a_id, *a_root, &newNode, a_level ) )   // Root split
    {
      newRoot = AllocNode();  // Grow tree taller and new root
      newRoot->m_level = ( *a_root )->m_level + 1;
      branch.m_rect = NodeCover( *a_root );
      branch.m_child = *a_root;
      AddBranch( &branch, newRoot, NULL );
      branch.m_rect = NodeCover( newNode );
      branch.m_child = newNode;
      AddBranch( &branch, newRoot, NULL );
      *a_root = newRoot;
      return true;
    }

    return false;
  }


// Find the smallest rectangle that includes all rectangles in branches of a node.
  RTREE_TEMPLATE
  typename RTREE_QUAL::Rect RTREE_QUAL::NodeCover( Node* a_node )
  {
    ASSERT( a_node );

    int firstTime = true;
    Rect rect;
    InitRect( &rect );

    for ( int index = 0; index < a_node->m_count; ++index )
    {
      if ( firstTime )
      {
        rect = a_node->m_branch[index].m_rect;
        firstTime = false;
      }
      else
      {
        rect = CombineRect( &rect, & ( a_node->m_branch[index].m_rect ) );
      }
    }

    return rect;
  }


// Add a branch to a node.  Split the node if necessary.
// Returns 0 if node not split.  Old node updated.
// Returns 1 if node split, sets *new_node to address of new node.
// Old node updated, becomes one of two.
  RTREE_TEMPLATE
  bool RTREE_QUAL::AddBranch( Branch* a_branch, Node* a_node, Node** a_newNode )
  {
    ASSERT( a_branch );
    ASSERT( a_node );

    if ( a_node->m_count < MAXNODES ) // Split won't be necessary
    {
      a_node->m_branch[a_node->m_count] = *a_branch;
      ++a_node->m_count;

      return false;
    }
    else
    {
      ASSERT( a_newNode );

      SplitNode( a_node, a_branch, a_newNode );
      return true;
    }
  }


// Disconnect a dependent node.
// Caller must return (or stop using iteration index) after this as count has changed
  RTREE_TEMPLATE
  void RTREE_QUAL::DisconnectBranch( Node* a_node, int a_index )
  {
    ASSERT( a_node && ( a_index >= 0 ) && ( a_index < MAXNODES ) );
    ASSERT( a_node->m_count > 0 );

    // Remove element by swapping with the last element to prevent gaps in array
    a_node->m_branch[a_index] = a_node->m_branch[a_node->m_count - 1];

    --a_node->m_count;
  }


// Pick a branch.  Pick the one that will need the smallest increase
// in area to accomodate the new rectangle.  This will result in the
// least total area for the covering rectangles in the current node.
// In case of a tie, pick the one which was smaller before, to get
// the best resolution when searching.
  RTREE_TEMPLATE
  int RTREE_QUAL::PickBranch( Rect* a_rect, Node* a_node )
  {
    ASSERT( a_rect && a_node );

    bool firstTime = true;
    ELEMTYPEREAL increase;
    ELEMTYPEREAL bestIncr = ( ELEMTYPEREAL ) - 1;
    ELEMTYPEREAL area;
    ELEMTYPEREAL bestArea =  0;
    int best = 0;
    Rect tempRect;

    for ( int index = 0; index < a_node->m_count; ++index )
    {
      Rect* curRect = &a_node->m_branch[index].m_rect;
      area = CalcRectVolume( curRect );
      tempRect = CombineRect( a_rect, curRect );
      increase = CalcRectVolume( &tempRect ) - area;
      if (( increase < bestIncr ) || firstTime )
      {
        best = index;
        bestArea = area;
        bestIncr = increase;
        firstTime = false;
      }
      else if (( increase == bestIncr ) && ( area < bestArea ) )
      {
        best = index;
        bestArea = area;
        bestIncr = increase;
      }
    }
    return best;
  }


// Combine two rectangles into larger one containing both
  RTREE_TEMPLATE
  typename RTREE_QUAL::Rect RTREE_QUAL::CombineRect( Rect* a_rectA, Rect* a_rectB )
  {
    ASSERT( a_rectA && a_rectB );

    Rect newRect;

    for ( int index = 0; index < NUMDIMS; ++index )
    {
      newRect.m_min[index] = Min( a_rectA->m_min[index], a_rectB->m_min[index] );
      newRect.m_max[index] = Max( a_rectA->m_max[index], a_rectB->m_max[index] );
    }

    return newRect;
  }



// Split a node.
// Divides the nodes branches and the extra one between two nodes.
// Old node is one of the new ones, and one really new one is created.
// Tries more than one method for choosing a partition, uses best result.
  RTREE_TEMPLATE
  void RTREE_QUAL::SplitNode( Node* a_node, Branch* a_branch, Node** a_newNode )
  {
    ASSERT( a_node );
    ASSERT( a_branch );

    // Could just use local here, but member or external is faster since it is reused
    PartitionVars localVars;
    PartitionVars* parVars = &localVars;
    int level;

    // Load all the branches into a buffer, initialize old node
    level = a_node->m_level;
    GetBranches( a_node, a_branch, parVars );

    // Find partition
    ChoosePartition( parVars, MINNODES );

    // Put branches from buffer into 2 nodes according to chosen partition
    *a_newNode = AllocNode();
    ( *a_newNode )->m_level = a_node->m_level = level;
    LoadNodes( a_node, *a_newNode, parVars );

    ASSERT(( a_node->m_count + ( *a_newNode )->m_count ) == parVars->m_total );
  }


// Calculate the n-dimensional volume of a rectangle
  RTREE_TEMPLATE
  ELEMTYPEREAL RTREE_QUAL::RectVolume( Rect* a_rect )
  {
    ASSERT( a_rect );

    ELEMTYPEREAL volume = ( ELEMTYPEREAL ) 1;

    for ( int index = 0; index < NUMDIMS; ++index )
    {
      volume *= a_rect->m_max[index] - a_rect->m_min[index];
    }

    ASSERT( volume >= ( ELEMTYPEREAL ) 0 );

    return volume;
  }


// The exact volume of the bounding sphere for the given Rect
  RTREE_TEMPLATE
  ELEMTYPEREAL RTREE_QUAL::RectSphericalVolume( Rect* a_rect )
  {
    ASSERT( a_rect );

    ELEMTYPEREAL sumOfSquares = ( ELEMTYPEREAL ) 0;
    ELEMTYPEREAL radius;

    for ( int index = 0; index < NUMDIMS; ++index )
    {
      ELEMTYPEREAL halfExtent = (( ELEMTYPEREAL ) a_rect->m_max[index] - ( ELEMTYPEREAL ) a_rect->m_min[index] ) * 0.5f;
      sumOfSquares += halfExtent * halfExtent;
    }

    radius = ( ELEMTYPEREAL ) sqrt( sumOfSquares );

    // Pow maybe slow, so test for common dims like 2,3 and just use x*x, x*x*x.
    if ( NUMDIMS == 3 )
    {
      return ( radius * radius * radius * m_unitSphereVolume );
    }
    else if ( NUMDIMS == 2 )
    {
      return ( radius * radius * m_unitSphereVolume );
    }
    else
    {
      return ( ELEMTYPEREAL )( pow( radius, NUMDIMS ) * m_unitSphereVolume );
    }
  }


// Use one of the methods to calculate retangle volume
  RTREE_TEMPLATE
  ELEMTYPEREAL RTREE_QUAL::CalcRectVolume( Rect* a_rect )
  {
#ifdef RTREE_USE_SPHERICAL_VOLUME
    return RectSphericalVolume( a_rect ); // Slower but helps certain merge cases
#else // RTREE_USE_SPHERICAL_VOLUME
    return RectVolume( a_rect ); // Faster but can cause poor merges
#endif // RTREE_USE_SPHERICAL_VOLUME  
  }


// Load branch buffer with branches from full node plus the extra branch.
  RTREE_TEMPLATE
  void RTREE_QUAL::GetBranches( Node* a_node, Branch* a_branch, PartitionVars* a_parVars )
  {
    ASSERT( a_node );
    ASSERT( a_branch );

    ASSERT( a_node->m_count == MAXNODES );

    int index;
    // Load the branch buffer
    for ( index = 0; index < MAXNODES; ++index )
    {
      a_parVars->m_branchBuf[index] = a_node->m_branch[index];
    }
    a_parVars->m_branchBuf[MAXNODES] = *a_branch;
    a_parVars->m_branchCount = MAXNODES + 1;

    // Calculate rect containing all in the set
    a_parVars->m_coverSplit = a_parVars->m_branchBuf[0].m_rect;
    for ( index = 1; index < MAXNODES + 1; ++index )
    {
      a_parVars->m_coverSplit = CombineRect( &a_parVars->m_coverSplit, &a_parVars->m_branchBuf[index].m_rect );
    }
    a_parVars->m_coverSplitArea = CalcRectVolume( &a_parVars->m_coverSplit );

    InitNode( a_node );
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
  RTREE_TEMPLATE
  void RTREE_QUAL::ChoosePartition( PartitionVars* a_parVars, int a_minFill )
  {
    ASSERT( a_parVars );

    ELEMTYPEREAL biggestDiff;
    int group, chosen = 0, betterGroup = 0;

    InitParVars( a_parVars, a_parVars->m_branchCount, a_minFill );
    PickSeeds( a_parVars );

    while ((( a_parVars->m_count[0] + a_parVars->m_count[1] ) < a_parVars->m_total )
           && ( a_parVars->m_count[0] < ( a_parVars->m_total - a_parVars->m_minFill ) )
           && ( a_parVars->m_count[1] < ( a_parVars->m_total - a_parVars->m_minFill ) ) )
    {
      biggestDiff = ( ELEMTYPEREAL ) - 1;
      for ( int index = 0; index < a_parVars->m_total; ++index )
      {
        if ( !a_parVars->m_taken[index] )
        {
          Rect* curRect = &a_parVars->m_branchBuf[index].m_rect;
          Rect rect0 = CombineRect( curRect, &a_parVars->m_cover[0] );
          Rect rect1 = CombineRect( curRect, &a_parVars->m_cover[1] );
          ELEMTYPEREAL growth0 = CalcRectVolume( &rect0 ) - a_parVars->m_area[0];
          ELEMTYPEREAL growth1 = CalcRectVolume( &rect1 ) - a_parVars->m_area[1];
          ELEMTYPEREAL diff = growth1 - growth0;
          if ( diff >= 0 )
          {
            group = 0;
          }
          else
          {
            group = 1;
            diff = -diff;
          }

          if ( diff > biggestDiff )
          {
            biggestDiff = diff;
            chosen = index;
            betterGroup = group;
          }
          else if (( diff == biggestDiff ) && ( a_parVars->m_count[group] < a_parVars->m_count[betterGroup] ) )
          {
            chosen = index;
            betterGroup = group;
          }
        }
      }
      Classify( chosen, betterGroup, a_parVars );
    }

    // If one group too full, put remaining rects in the other
    if (( a_parVars->m_count[0] + a_parVars->m_count[1] ) < a_parVars->m_total )
    {
      if ( a_parVars->m_count[0] >= a_parVars->m_total - a_parVars->m_minFill )
      {
        group = 1;
      }
      else
      {
        group = 0;
      }
      for ( int index = 0; index < a_parVars->m_total; ++index )
      {
        if ( !a_parVars->m_taken[index] )
        {
          Classify( index, group, a_parVars );
        }
      }
    }

    ASSERT(( a_parVars->m_count[0] + a_parVars->m_count[1] ) == a_parVars->m_total );
    ASSERT(( a_parVars->m_count[0] >= a_parVars->m_minFill ) &&
           ( a_parVars->m_count[1] >= a_parVars->m_minFill ) );
  }


// Copy branches from the buffer into two nodes according to the partition.
  RTREE_TEMPLATE
  void RTREE_QUAL::LoadNodes( Node* a_nodeA, Node* a_nodeB, PartitionVars* a_parVars )
  {
    ASSERT( a_nodeA );
    ASSERT( a_nodeB );
    ASSERT( a_parVars );

    for ( int index = 0; index < a_parVars->m_total; ++index )
    {
      ASSERT( a_parVars->m_partition[index] == 0 || a_parVars->m_partition[index] == 1 );

      if ( a_parVars->m_partition[index] == 0 )
      {
        AddBranch( &a_parVars->m_branchBuf[index], a_nodeA, NULL );
      }
      else if ( a_parVars->m_partition[index] == 1 )
      {
        AddBranch( &a_parVars->m_branchBuf[index], a_nodeB, NULL );
      }
    }
  }


// Initialize a PartitionVars structure.
  RTREE_TEMPLATE
  void RTREE_QUAL::InitParVars( PartitionVars* a_parVars, int a_maxRects, int a_minFill )
  {
    ASSERT( a_parVars );

    a_parVars->m_count[0] = a_parVars->m_count[1] = 0;
    a_parVars->m_area[0] = a_parVars->m_area[1] = ( ELEMTYPEREAL ) 0;
    a_parVars->m_total = a_maxRects;
    a_parVars->m_minFill = a_minFill;
    for ( int index = 0; index < a_maxRects; ++index )
    {
      a_parVars->m_taken[index] = false;
      a_parVars->m_partition[index] = -1;
    }
  }


  RTREE_TEMPLATE
  void RTREE_QUAL::PickSeeds( PartitionVars* a_parVars )
  {
    int seed0 = 0, seed1 = 0;
    ELEMTYPEREAL worst, waste;
    ELEMTYPEREAL area[MAXNODES+1];

    for ( int index = 0; index < a_parVars->m_total; ++index )
    {
      area[index] = CalcRectVolume( &a_parVars->m_branchBuf[index].m_rect );
    }

    worst = -a_parVars->m_coverSplitArea - 1;
    for ( int indexA = 0; indexA < a_parVars->m_total - 1; ++indexA )
    {
      for ( int indexB = indexA + 1; indexB < a_parVars->m_total; ++indexB )
      {
        Rect oneRect = CombineRect( &a_parVars->m_branchBuf[indexA].m_rect, &a_parVars->m_branchBuf[indexB].m_rect );
        waste = CalcRectVolume( &oneRect ) - area[indexA] - area[indexB];
        if ( waste > worst )
        {
          worst = waste;
          seed0 = indexA;
          seed1 = indexB;
        }
      }
    }
    Classify( seed0, 0, a_parVars );
    Classify( seed1, 1, a_parVars );
  }


// Put a branch in one of the groups.
  RTREE_TEMPLATE
  void RTREE_QUAL::Classify( int a_index, int a_group, PartitionVars* a_parVars )
  {
    ASSERT( a_parVars );
    ASSERT( !a_parVars->m_taken[a_index] );

    a_parVars->m_partition[a_index] = a_group;
    a_parVars->m_taken[a_index] = true;

    if ( a_parVars->m_count[a_group] == 0 )
    {
      a_parVars->m_cover[a_group] = a_parVars->m_branchBuf[a_index].m_rect;
    }
    else
    {
      a_parVars->m_cover[a_group] = CombineRect( &a_parVars->m_branchBuf[a_index].m_rect, &a_parVars->m_cover[a_group] );
    }
    a_parVars->m_area[a_group] = CalcRectVolume( &a_parVars->m_cover[a_group] );
    ++a_parVars->m_count[a_group];
  }


// Delete a data rectangle from an index structure.
// Pass in a pointer to a Rect, the tid of the record, ptr to ptr to root node.
// Returns 1 if record not found, 0 if success.
// RemoveRect provides for eliminating the root.
  RTREE_TEMPLATE
  bool RTREE_QUAL::RemoveRect( Rect* a_rect, const DATATYPE& a_id, Node** a_root )
  {
    ASSERT( a_rect && a_root );
    ASSERT( *a_root );

    Node* tempNode;
    ListNode* reInsertList = NULL;

    if ( !RemoveRectRec( a_rect, a_id, *a_root, &reInsertList ) )
    {
      // Found and deleted a data item
      // Reinsert any branches from eliminated nodes
      while ( reInsertList )
      {
        tempNode = reInsertList->m_node;

        for ( int index = 0; index < tempNode->m_count; ++index )
        {
          InsertRect( & ( tempNode->m_branch[index].m_rect ),
                      tempNode->m_branch[index].m_data,
                      a_root,
                      tempNode->m_level );
        }

        ListNode* remLNode = reInsertList;
        reInsertList = reInsertList->m_next;

        FreeNode( remLNode->m_node );
        FreeListNode( remLNode );
      }

      // Check for redundant root (not leaf, 1 child) and eliminate
      if (( *a_root )->m_count == 1 && ( *a_root )->IsInternalNode() )
      {
        tempNode = ( *a_root )->m_branch[0].m_child;

        ASSERT( tempNode );
        FreeNode( *a_root );
        *a_root = tempNode;
      }
      return false;
    }
    else
    {
      return true;
    }
  }


// Delete a rectangle from non-root part of an index structure.
// Called by RemoveRect.  Descends tree recursively,
// merges branches on the way back up.
// Returns 1 if record not found, 0 if success.
  RTREE_TEMPLATE
  bool RTREE_QUAL::RemoveRectRec( Rect* a_rect, const DATATYPE& a_id, Node* a_node, ListNode** a_listNode )
  {
    ASSERT( a_rect && a_node && a_listNode );
    ASSERT( a_node->m_level >= 0 );

    if ( a_node->IsInternalNode() ) // not a leaf node
    {
      for ( int index = 0; index < a_node->m_count; ++index )
      {
        if ( Overlap( a_rect, & ( a_node->m_branch[index].m_rect ) ) )
        {
          if ( !RemoveRectRec( a_rect, a_id, a_node->m_branch[index].m_child, a_listNode ) )
          {
            if ( a_node->m_branch[index].m_child->m_count >= MINNODES )
            {
              // child removed, just resize parent rect
              a_node->m_branch[index].m_rect = NodeCover( a_node->m_branch[index].m_child );
            }
            else
            {
              // child removed, not enough entries in node, eliminate node
              ReInsert( a_node->m_branch[index].m_child, a_listNode );
              DisconnectBranch( a_node, index ); // Must return after this call as count has changed
            }
            return false;
          }
        }
      }
      return true;
    }
    else   // A leaf node
    {
      for ( int index = 0; index < a_node->m_count; ++index )
      {
        if ( a_node->m_branch[index].m_child == ( Node* ) a_id )
        {
          DisconnectBranch( a_node, index ); // Must return after this call as count has changed
          return false;
        }
      }
      return true;
    }
  }


// Decide whether two rectangles overlap.
  RTREE_TEMPLATE
  bool RTREE_QUAL::Overlap( Rect* a_rectA, Rect* a_rectB )
  {
    ASSERT( a_rectA && a_rectB );

    for ( int index = 0; index < NUMDIMS; ++index )
    {
      if ( a_rectA->m_min[index] > a_rectB->m_max[index] ||
           a_rectB->m_min[index] > a_rectA->m_max[index] )
      {
        return false;
      }
    }
    return true;
  }


// Add a node to the reinsertion list.  All its branches will later
// be reinserted into the index structure.
  RTREE_TEMPLATE
  void RTREE_QUAL::ReInsert( Node* a_node, ListNode** a_listNode )
  {
    ListNode* newListNode;

    newListNode = AllocListNode();
    newListNode->m_node = a_node;
    newListNode->m_next = *a_listNode;
    *a_listNode = newListNode;
  }


// Search in an index tree or subtree for all data retangles that overlap the argument rectangle.
  RTREE_TEMPLATE
  bool RTREE_QUAL::Search( Node* a_node, Rect* a_rect, int& a_foundCount, bool ( *a_resultCallback )( DATATYPE a_data, void* a_context ), void* a_context )
  {
    ASSERT( a_node );
    ASSERT( a_node->m_level >= 0 );
    ASSERT( a_rect );

    if ( a_node->IsInternalNode() ) // This is an internal node in the tree
    {
      for ( int index = 0; index < a_node->m_count; ++index )
      {
        if ( Overlap( a_rect, &a_node->m_branch[index].m_rect ) )
        {
          if ( !Search( a_node->m_branch[index].m_child, a_rect, a_foundCount, a_resultCallback, a_context ) )
          {
            return false; // Don't continue searching
          }
        }
      }
    }
    else   // This is a leaf node
    {
      for ( int index = 0; index < a_node->m_count; ++index )
      {
        if ( Overlap( a_rect, &a_node->m_branch[index].m_rect ) )
        {
          DATATYPE& id = a_node->m_branch[index].m_data;

          // NOTE: There are different ways to return results.  Here's where to modify
          if ( a_resultCallback )
          {
            ++a_foundCount;
            if ( !a_resultCallback( id, a_context ) )
            {
              return false; // Don't continue searching
            }
          }
        }
      }
    }

    return true; // Continue searching
  }


#undef RTREE_TEMPLATE
#undef RTREE_QUAL

}

#endif //RTREE_H

