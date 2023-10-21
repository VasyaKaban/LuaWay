#pragma once

#include <utility>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <cstring>

#include <version>

#ifdef __cpp_lib_start_lifetime_as
#include <memory>
#endif

template<typename E>
constexpr auto convert_to_type_ptr(void *ptr) -> E *
{
#ifdef __cpp_lib_start_lifetime_as
	return std::start_lifetime_as<E>(ptr);
#else
	return reinterpret_cast<E *>(ptr);
#endif
}

namespace hrs
{
	struct union_size_align
	{
		size_t size;
		size_t alignment;
	};

	template<typename T, typename ...Args>
	consteval auto common_size_align() -> union_size_align
	{
		size_t max_size = std::max({sizeof(T), sizeof(Args)...});
		size_t max_align = std::max({alignof(T), alignof(Args)...});

		if(max_size > max_align)
		{
			if(max_size % max_align != 0)
				max_size = (max_size / max_align + 1) * max_align;
				//max_size += (max_size % max_align);
		}

		return {max_size, max_align};
	}

	template<typename E>
	struct unexpected
	{
		E error;
		auto operator=(const E &err) -> E &
		{
			error = err;
		}
	};

	template<typename T, typename E>
	class expected
	{
	public:

		constexpr inline static size_t DataSize = common_size_align<T, E>().size;
		constexpr inline static size_t DataAlign = common_size_align<T, E>().alignment;

		expected()
		{
			is_error = false;
			new(union_block) T{};
		}

		expected(const T &val)
		{
			is_error = false;
			new(union_block) T{val};
		}

		expected(T &&val) noexcept
		{
			is_error = false;
			new(union_block) T{std::move(val)};
		}

		expected(const E &err)
			requires (!std::is_same_v<T, E>)
		{
			is_error = true;
			new(union_block) E{err};
		}

		expected(E &&err) noexcept
			requires (!std::is_same_v<T, E>)
		{
			is_error = true;
			new(union_block) E{std::move(err)};
		}

		expected(const unexpected<E> &unex)
		{
			is_error = true;
			new(union_block) E{unex.error};
		}

		expected(unexpected<E> &&unex) noexcept
		{
			is_error = true;
			new(union_block) E{unex.error};
		}

		~expected()
		{

			if(is_error)
				convert_to_type_ptr<E>(union_block)->~E();
			else
				convert_to_type_ptr<T>(union_block)->~T();
		}

		expected(const expected &ex)
		{
			is_error = ex.is_error;

			copy(ex.union_block, &ex.union_block[common_size_align<T, E>().first], union_block);
		}

		expected(expected &&ex) noexcept
		{
			if(ex.is_error)
			{
				is_error = true;
				new(union_block) E{std::move(*reinterpret_cast<E *>(ex.union_block))};
			}
			else
			{
				is_error = false;
				new(union_block) T{std::move(*reinterpret_cast<T *>(ex.union_block))};
			}

			ex.is_error = false;
			new(ex.union_block) T{};
		}

		auto operator=(expected &&ex) noexcept -> expected &
		{
			~expected();
			is_error = ex.is_error;
			if(ex.is_error)
				new(union_block) E{std::move(*reinterpret_cast<E *>(ex.union_block))};
			else
				new(union_block) T{std::move(*reinterpret_cast<T *>(ex.union_block))};

			ex.is_error = false;
			new(ex.union_block) T{};

			return *this;
		}

		auto operator=(const expected &ex) -> expected &
		{
			~expected();
			is_error = ex.is_error;
			if(ex.is_error)
				new(union_block) E{*reinterpret_cast<E *>(ex.union_block)};
			else
				new(union_block) T{*reinterpret_cast<T *>(ex.union_block)};

			return *this;
		}

		operator bool()
		{
			return !is_error;
		}

		auto error() -> E &
		{
			return *reinterpret_cast<E *>(union_block);
		}

		auto value() -> T &
		{
			return *reinterpret_cast<T *>(union_block);
		}

		auto has_value() -> bool
		{
			if(is_error)
				return false;
			else
				return true;
		}

	private:
		alignas(common_size_align<T, E>().alignment) uint8_t union_block[common_size_align<T, E>().size];
		bool is_error;
	};
}
