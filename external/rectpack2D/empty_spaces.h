#pragma once
#include "insert_and_split.h"

namespace rectpack2D {
	enum class flipping_option {
		DISABLED,
		ENABLED
	};

	class default_empty_spaces;

	template <bool allow_flip, class empty_spaces_provider = default_empty_spaces>
	class empty_spaces {
		rect_wh current_aabb;
		empty_spaces_provider spaces;

		/* MSVC fix for non-conformant if constexpr implementation */

		static auto make_output_rect(const int x, const int y, const int w, const int h) {
			return rect_xywh(x, y, w, h);
		}

		static auto make_output_rect(const int x, const int y, const int w, const int h, const bool flipped) {
			return rect_xywhf(x, y, w, h, flipped);
		}

	public:
		using output_rect_type = std::conditional_t<allow_flip, rect_xywhf, rect_xywh>;

		flipping_option flipping_mode = flipping_option::ENABLED;

		empty_spaces(const rect_wh& r) {
			reset(r);
		}

		void reset(const rect_wh& r) {
			current_aabb = {};

			spaces.reset();
			spaces.add(rect_xywh(0, 0, r.w, r.h));
		}

		template <class F>
		std::optional<output_rect_type> insert(const rect_wh image_rectangle, F report_candidate_empty_space) {
			for (int i = static_cast<int>(spaces.get_count()) - 1; i >= 0; --i) {
				const auto candidate_space = spaces.get(i);

				report_candidate_empty_space(candidate_space);

				auto accept_result = [this, i, image_rectangle, candidate_space](
					const created_splits& splits,
					const bool flipping_necessary
				) -> std::optional<output_rect_type> {
					spaces.remove(i);

					for (int s = 0; s < splits.count; ++s) {
						if (!spaces.add(splits.spaces[s])) {
							return std::nullopt;
						}
					}

					if constexpr(allow_flip) {
						const auto result = make_output_rect(
							candidate_space.x,
							candidate_space.y,
							image_rectangle.w,
							image_rectangle.h,
							flipping_necessary
						);

						current_aabb.expand_with(result);
						return result;
					}
					else if constexpr(!allow_flip) {
						(void)flipping_necessary;

						const auto result = make_output_rect(
							candidate_space.x,
							candidate_space.y,
							image_rectangle.w,
							image_rectangle.h
						);

						current_aabb.expand_with(result);
						return result;
					}
				};

				auto try_to_insert = [&](const rect_wh& img) {
					return rectpack2D::insert_and_split(img, candidate_space);
				};

				if constexpr(!allow_flip) {
					if (const auto normal = try_to_insert(image_rectangle)) {
						return accept_result(normal, false);
					}
				}
				else {
					if (flipping_mode == flipping_option::ENABLED) {
						const auto normal = try_to_insert(image_rectangle);
						const auto flipped = try_to_insert(rect_wh(image_rectangle).flip());

						/* 
							If both were successful, 
							prefer the one that generated less remainder spaces.
						*/

						if (normal && flipped) {
							if (flipped.better_than(normal)) {
								/* Accept the flipped result if it producues less or "better" spaces. */

								return accept_result(flipped, true);
							}

							return accept_result(normal, false);
						}

						if (normal) {
							return accept_result(normal, false);
						}

						if (flipped) {
							return accept_result(flipped, true);
						}
					}
					else {
						if (const auto normal = try_to_insert(image_rectangle)) {
							return accept_result(normal, false);
						}
					}
				}
			}

			return std::nullopt;
		}

		decltype(auto) insert(const rect_wh& image_rectangle) {
			return insert(image_rectangle, [](auto&){ });
		}

		auto get_rects_aabb() const {
			return current_aabb;
		}

		const auto& get_spaces() const {
			return spaces;
		}
	};
}
