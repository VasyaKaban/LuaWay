#pragma once

#include <type_traits>
#include <limits>

namespace hrs
{
	template<typename E>
	requires std::is_enum_v<E>
	struct Flags
	{
		using UnderlyingType = std::underlying_type_t<E>;

		UnderlyingType mask;

		constexpr static UnderlyingType full_mask = std::numeric_limits<UnderlyingType>::max();

		constexpr Flags() noexcept : mask{}{}
		constexpr Flags(const UnderlyingType other_mask) noexcept : mask(other_mask){}
		constexpr Flags(const E eval) noexcept : mask(static_cast<UnderlyingType>(eval)){}

		constexpr Flags(const Flags &flag) noexcept = default;
		constexpr Flags(Flags &&flag) noexcept = default;

		constexpr auto operator=(const Flags &flag) noexcept -> Flags<E> & = default;
		constexpr auto operator=(Flags &&flag) noexcept -> Flags<E> & = default;

		constexpr auto operator |(const Flags flag) const noexcept -> Flags
		{
			return (flag.mask | mask);
		}

		constexpr auto operator |(const E e) const noexcept -> Flags
		{
			return (mask | static_cast<UnderlyingType>(e));
		}

		constexpr auto operator |=(const Flags flag) noexcept -> Flags &
		{
			mask |= flag.mask;
			return *this;
		}

		constexpr auto operator |=(const E e) noexcept -> Flags &
		{
			mask |= static_cast<UnderlyingType>(e);
			return *this;
		}

		constexpr auto operator &(const Flags flag) const noexcept -> Flags
		{
			return (flag.mask & mask);
		}

		constexpr auto operator &(const E e) const noexcept -> Flags
		{
			return (mask & static_cast<UnderlyingType>(e));
		}

		constexpr auto operator &=(const Flags flag) noexcept -> Flags &
		{
			mask &= flag.mask;
			return *this;
		}

		constexpr auto operator &=(const E e) noexcept -> Flags &
		{
			mask &= static_cast<UnderlyingType>(e);
			return *this;
		}

		constexpr auto operator==(const Flags flag) const noexcept -> bool
		{
			return is_set(flag);
		}

		constexpr auto is_set(const Flags flag) const noexcept -> bool
		{
			return ((mask & flag.mask) == flag.mask);
		}

		constexpr auto clear() noexcept -> void
		{
			mask = {};
		}

		constexpr operator UnderlyingType() const noexcept
		{
			return mask;
		}

		constexpr operator bool() const noexcept
		{
			return static_cast<bool>(mask);
		}
	};

	template<typename E>
	requires std::is_enum_v<E>
	constexpr auto operator |(const E &eval0, const E &eval1) -> Flags<E>
	{
		using UnderlyingType = std::underlying_type_t<E>;
		return static_cast<UnderlyingType>(eval0) | static_cast<UnderlyingType>(eval1);
	}
};
