#pragma once
#include <array>
#include "rect_structs.h"

namespace rectpack2D {
	struct created_splits {
		int count = 0;
		std::array<space_rect, 2> spaces;

		static auto failed() {
			created_splits result;
			result.count = -1;
			return result;
		}

		static auto none() {
			return created_splits();
		}

		template <class... Args>
		created_splits(Args&&... args) : spaces({ std::forward<Args>(args)... }) {
			count = sizeof...(Args);
		}

		bool better_than(const created_splits& b) const {
			return count < b.count;
		}

		explicit operator bool() const {
			return count != -1;
		}
	};

	inline created_splits insert_and_split(
		const rect_wh& im, /* Image rectangle */
		const space_rect& sp /* Space rectangle */
	) {
		const auto free_w = sp.w - im.w;
		const auto free_h = sp.h - im.h;

		if (free_w < 0 || free_h < 0) {
			/*
				Image is bigger than the candidate empty space.
				We'll need to look further.
			*/

			return created_splits::failed();
		}

		if (free_w == 0 && free_h == 0) {
			/*
				If the image dimensions equal the dimensions of the candidate empty space (image fits exactly),
				we will just delete the space and create no splits.  
			*/

			return created_splits::none();
		}

		/*
			If the image fits into the candidate empty space, 
			but exactly one of the image dimensions equals the respective dimension of the candidate empty space 
			(e.g. image = 20x40, candidate space = 30x40)
			we delete the space and create a single split. In this case a 10x40 space.
		*/

		if (free_w > 0 && free_h == 0) {
			auto r = sp;
			r.x += im.w;
			r.w -= im.w;
			return created_splits(r);
		}

		if (free_w == 0 && free_h > 0) {
			auto r = sp;
			r.y += im.h;
			r.h -= im.h;
			return created_splits(r);
		}

		/* 
			Every other option has been exhausted,
			so at this point the image must be *strictly* smaller than the empty space,
			that is, it is smaller in both width and height.

			Thus, free_w and free_h must be positive.
		*/

		/*
			Decide which way to split.

			Instead of having two normally-sized spaces,
			it is better - though I have no proof of that - to have a one tiny space and a one huge space.
			This creates better opportunity for insertion of future rectangles.

			This is why, if we had more of width remaining than we had of height,
			we split along the vertical axis,
			and if we had more of height remaining than we had of width,
			we split along the horizontal axis.
		*/

		if (free_w > free_h) {
			const auto bigger_split = space_rect(
				sp.x + im.w,
				sp.y,
			   	free_w,
			   	sp.h
			);

			const auto lesser_split = space_rect(
				sp.x,
				sp.y + im.h,
				im.w,
				free_h
			);

			return created_splits(bigger_split, lesser_split);
		}

		const auto bigger_split = space_rect(
			sp.x,
			sp.y + im.h,
			sp.w,
			free_h
		);

		const auto lesser_split = space_rect(
			sp.x + im.w,
			sp.y,
			free_w,
			im.h
		);

		return created_splits(bigger_split, lesser_split);
	}
}
