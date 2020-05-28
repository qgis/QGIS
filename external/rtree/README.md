# R-Trees: A Dynamic Index Structure for Spatial Searching

## Description

A C++ templated version of [this](http://www.superliminal.com/sources/sources.htm)
RTree algorithm.
The code it now generally compatible with the STL and Boost C++ libraries.

## Usage

```cpp
#include <RTree.h>

// ...

RTree<Foo*, double, 3> tree;
double min[3] = {0., 0., 0.};
double max[3] = {1., 1., 1.};
Foo* bar = new Foo();
tree.Insert(min, max, bar);
```

Provides search in and iteration over the tree. For examples see
[Test.cpp](https://github.com/nushoin/RTree/blob/master/Test.cpp)

## Testing

Run `make` to build and `make test` to run the tests. The RTree itself is
a single header file and can be included without compiling.

## Authors

- 1983 Original algorithm and test code by Antonin Guttman and Michael Stonebraker, UC Berkely
- 1994 ANCI C ported from original test code by Melinda Green - melinda@superliminal.com
- 1995 Sphere volume fix for degeneracy problem submitted by Paul Brook
- 2004 Templated C++ port by Greg Douglas
- 2011 Modified the container to support more data types, by Yariv Barkan
- 2017 Modified Search to take C++11 function to allow lambdas and added const qualifier, by Gero Mueller

## License

Original code was taken from http://www.superliminal.com/sources/sources.htm 
and is stored as git revision 0. This revision is entirely free for all
uses. Enjoy!

Due to restrictions on public domain in certain jurisdictions, code
contributed by Yariv Barkan is released in these jurisdictions under the
BSD, MIT or the GPL - you may choose one or more, whichever that suits you
best. 
    
In jurisdictions where public domain property is recognized, the user of
this software may choose to accept it either 1) as public domain, 2) under
the conditions of the BSD, MIT or GPL or 3) any combination of public
domain and one or more of these licenses.

Thanks [Baptiste Lepilleur](http://jsoncpp.sourceforge.net/LICENSE) for the
licensing idea.

## Recent Change Log

### 31 Jan 2018

- Added copy constructor
- Callback function is now `std::function`

### 05 Apr 2014

- Added tests

### 02 Sep 2011

- Modified the container to support more data types. The code it now generally
  compatible with the STL and Boost C++ libraries.

### 05 Jan 2010

- Fixed Iterator GetFirst() - Previous fix was not incomplete

### 03 Dec 2009

- Added Iteartor GetBounds()
- Added Iterator usage to simple test
- Fixed Iterator GetFirst() - Thanks Mathew Riek
- Minor updates for MSVC 2005/08 compilers
