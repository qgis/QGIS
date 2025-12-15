#pragma once
#include <array>
#include <vector>
#include <type_traits>

#include "rect_structs.h"

namespace rectpack2D {
	class default_empty_spaces {
		std::vector<space_rect> empty_spaces;

	public:
		void remove(const int i) {
			empty_spaces[i] = empty_spaces.back();
			empty_spaces.pop_back();
		}

		bool add(const space_rect r) {
			empty_spaces.emplace_back(r);
			return true;
		}

		auto get_count() const {
			return empty_spaces.size();
		}

		void reset() {
			empty_spaces.clear();
		}

		const auto& get(const int i) {
			return empty_spaces[i];
		}
	};

	template <int MAX_SPACES>
	class static_empty_spaces {
		int count_spaces = 0;
		std::array<space_rect, MAX_SPACES> empty_spaces;

	public:
		void remove(const int i) {
			empty_spaces[i] = empty_spaces[count_spaces - 1];
			--count_spaces;
		}

		bool add(const space_rect r) {
			if (count_spaces < static_cast<int>(empty_spaces.size())) {
				empty_spaces[count_spaces] = r;
				++count_spaces;

				return true;
			}

			return false;
		}
		
		auto get_count() const {
			return count_spaces;
		}

		void reset() {
			count_spaces = 0;
		}

		const auto& get(const int i) {
			return empty_spaces[i];
		}
	};
}
