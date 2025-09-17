<div align="center">
	
	
# rectpack2D

[![Linux & Windows Build](https://github.com/TeamHypersomnia/rectpack2D/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/TeamHypersomnia/rectpack2D/actions/workflows/cmake-multi-platform.yml)
	
**Used in [Assassin's Creed: Valhalla](https://www.youtube.com/watch?v=2KnjDL4DnwM&t=2382s)!**

**Used by [Skydio](https://pages.skydio.com/rs/784-TUF-591/images/Open%20Source%20Software%20Notice%20v0.2.html), one of the top drone manufacturers!**
	
**[2 scientific references](https://scholar.google.com/scholar?hl=en&as_sdt=0%2C5&q=teamhypersomnia&btnG=)!**
	
	
</div>

A header-only 2D rectangle packing library written in modern C++.  
This is a refactored and **highly optimized** version of the [original library](https://github.com/TeamHypersomnia/rectpack2D/tree/legacy).

It was originally developed for the needs of [Hypersomnia](https://github.com/TeamHypersomnia/Hypersomnia), a free and open-source multiplayer shooter.

![7](https://user-images.githubusercontent.com/3588717/42707552-d8b1c65e-86da-11e8-9412-54c580bd2696.jpg)

## Table of contents

- [Benchmarks](#benchmarks)
- [Usage](#usage)
- [Building the example](#building-the-example)
  * [Windows](#windows)
  * [Linux](#linux)
- [Algorithm](#algorithm)
  * [Insertion algorithm](#insertion-algorithm)
  * [Additional heuristics](#additional-heuristics)

## Benchmarks

Tests were conducted on a ``Intel(R) Core(TM) i7-4770K CPU @ 3.50GHz``.  
The binary was built with ``clang 6.0.0``, using an -03 switch.

### Arbitrary game sprites: 582 subjects.  

**Runtime: 0.8 ms**  
**Wasted pixels: 10982 (0.24% - equivalent of a 105 x 105 square)**  

Output (1896 x 2382):

![1](images/atlas_small.png)

In color:  
(black is wasted space)

![2](images/atlas_small_color.png)

### Arbitrary game sprites + Japanese glyphs: 3264 subjects.  

**Runtime: 4 ms**  
**Wasted pixels: 15538 (0.31% - equivalent of a 125 x 125 square)**  

Output (2116 x 2382):

![3](images/atlas_big.png)

In color:  
(black is wasted space)

![4](images/atlas_big_color.png)


### Japanese glyphs + some GUI sprites: 3122 subjects.  

**Runtime: 3.5 - 7 ms**  
**Wasted pixels: 9288 (1.23% - equivalent of a 96 x 96 square)**  

Output (866 x 871):

![5](images/atlas_tiny.png)

In color:  
(black is wasted space)

![6](images/atlas_tiny_color.png)

## Usage

This is a header-only library.
Just include the ``src/finders_interface.h`` and you should be good to go.

For an example use, see ``example/main.cpp``.

## Building the example

### Windows

From the repository's folder, run:

```bash
mkdir build
cd build
cmake -G "Visual Studio 15 2017 Win64" ..
````

Then just build the generated ``.sln`` file using the newest Visual Studio Preview.

### Linux

From the repository's folder, run:

```bash
cmake/build_example.sh Release gcc g++
cd build/current
ninja run
````

Or, if you want to use clang, run:

```bash
cmake/build_example.sh Release clang clang++
cd build/current
ninja run
````

## Algorithm

### Insertion algorithm

The library started as an implementation of this algorithm:

http://blackpawn.com/texts/lightmaps/default.html

The current version somewhat derives from the concept described there -  
however, it uses just a **vector of empty spaces, instead of a tree** - this turned out to be a performance breakthrough.  

Given

```cpp
struct rect_xywh {
	int x;
	int y;
	int w;
	int h;
};
````

Let us create a vector and call it empty_spaces.

```cpp
std::vector<rect_xywh> empty_spaces;
````

Given a user-specified initial bin, which is a square of some size S, we initialize the first empty space.

```cpp
empty_spaces.push_back(rect_xywh(0, 0, S, S));
````

Now, we'd like to insert the first image rectangle.  

To do this, we iterate the vector of empty spaces **backwards** and look for an empty space into which the image can fit.  
For now, we only have the S x S square: let's save the index of this candidate empty space,  
which is ``candidate_space_index = 0;``  

If our image is strictly smaller than the candidate space, we have something like this:

![diag01](images/diag01.png)

The blue is our image rectangle.  
We now calculate the gray rectangles labeled as "bigger split" and "smaller split",  
and save them like this:  

```cpp
// Erase the space that we've just inserted to, by swapping and popping.
empty_spaces[candidate_space_index] = empty_spaces.back();
empty_spaces.pop_back();

// Save the resultant splits
empty_spaces.push_back(bigger_split);
empty_spaces.push_back(smaller_split);
````

Notice that we push the smaller split *after* the bigger one.  
This is fairly important, because later the candidate images will encounter the smaller splits first,  
which will make better use of empty spaces overall.  

#### Corner cases:

- If the image dimensions equal the dimensions of the candidate empty space (image fits exactly),
	- we just delete the space and create no splits.  
- If the image fits into the candidate empty space, but exactly one of the image dimensions equals the respective dimension of the candidate empty space (e.g. image = 20x40, candidate space = 30x40)
	- we delete the space and create a single split. In this case a 10x40 space.

To see the complete, modular procedure for calculating the splits (along with the corner cases),
[see this source](src/rectpack2D/insert_and_split.h).

If the insertion fails, we also try the same procedure for a flipped image.

### Additional heuristics

Now we know how to insert individual images into a bin of a given initial size S.

1. However, what S should be passed to the algorithm so that the rectangles end up wasting the least amount of space?
	- We perform a binary search.
		- We start with the size specified by the library user. Typically, it would be the maximum texture size allowed on a particular GPU.
		- If the packing was successful on the given bin size, decrease the size and try to pack again.
		- If the packing has failed on the given bin size - because some rectangles could not be further inserted - increase the size and try to pack again.
	- The search is aborted if we've successfully inserted into a bin and the dimensions of the next candidate would differ from the previous by less than ``discard_step``.
		- This variable exists so that we may easily trade accuracy for a speedup. ``discard_step = 1`` yields the highest accuracy. ``discard_step = 128`` will yield worse packings, but will be a lot faster, etc.
	- The search is performed first by decreasing the bin size by both width and height, keeping it in square shape.
		- Then we do the same, but only decreasing width.
		- Then we do the same, but only decreasing height.
		- The last two were a breakthrough in packing tightness. It turns out important to consider non-square bins.
2. In what order should the rectangles be inserted so that they pack the tightest?
	- By default, the library tries 5 decreasing orders:
		- By area.
		- By perimeter.
		- By the bigger side.
		- By width.
		- By height.
