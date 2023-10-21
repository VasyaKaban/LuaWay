#pragma once

#include <string>
#include <vector>

namespace LuaWay
{
	class StringPath
	{
	public:

		using ValueType = std::vector<std::string>;
		using IteratorType = typename ValueType::iterator;
		using ConstIteratorType = typename ValueType::const_iterator;

		StringPath() = default;
		constexpr StringPath(std::initializer_list<std::string> apath);
		~StringPath() = default;
		StringPath(const StringPath &apath) = default;
		StringPath(StringPath &&apath) = default;

		auto operator=(const StringPath &apath) -> StringPath & = default;
		auto operator=(StringPath &&apath) -> StringPath & = default;

		template<std::convertible_to<std::string> S>
		constexpr auto operator+=(S &&str) -> StringPath &;

		template<std::convertible_to<std::string> S>
		constexpr auto operator+(S &&str) const -> StringPath;

		constexpr auto operator+=(const StringPath &apath) -> StringPath &;
		constexpr auto operator+(const StringPath &apath) const -> StringPath;

		constexpr auto GetPath() -> ValueType &;
		constexpr auto GetPath() const -> const ValueType &;
		constexpr auto RemoveBack(size_t count) -> void;
		constexpr auto RemoveTop(size_t count) -> void;
		constexpr auto Clear() -> void;
		constexpr auto Erase(IteratorType start, IteratorType end) -> void;

		constexpr operator bool() const noexcept;

		constexpr auto Empty() const noexcept -> bool;

		constexpr auto Size() const noexcept -> std::size_t;

		constexpr auto begin() noexcept -> IteratorType;
		constexpr auto begin() const noexcept -> ConstIteratorType;
		constexpr auto end() noexcept -> IteratorType;
		constexpr auto end() const noexcept -> ConstIteratorType;

	private:
		std::vector<std::string> path;
	};

	constexpr StringPath::StringPath(std::initializer_list<std::string> apath)
	{
		path = apath;
	}

	template<std::convertible_to<std::string> S>
	constexpr auto StringPath::operator+=(S &&str) -> StringPath &
	{
		path.push_back(static_cast<std::string>(std::forward<S>(str)));
		return *this;
	}

	template<std::convertible_to<std::string> S>
	constexpr auto StringPath::operator+(S &&str) const -> StringPath
	{
		StringPath out_path = *this;
		out_path.path.push_back(static_cast<std::string>(std::forward<S>(str)));
		return out_path;
	}

	constexpr auto StringPath::operator+=(const StringPath &apath) -> StringPath &
	{
		path.insert(path.end(), apath.begin(), apath.end());
		return *this;
	}

	constexpr auto StringPath::operator+(const StringPath &apath) const -> StringPath
	{
		StringPath out_path = *this;
		out_path.path.insert(out_path.end(), apath.begin(), apath.end());

		return out_path;
	}

	constexpr auto StringPath::GetPath() -> ValueType &
	{
		return path;
	}

	constexpr auto StringPath::GetPath() const -> const ValueType &
	{
		return path;
	}

	constexpr auto StringPath::RemoveBack(size_t count) -> void
	{
		if(count == 0)
			return;

		if(path.size() <= count)
			path.clear();
		else
		{
			auto st_it = path.rend();
			std::advance(st_it, count);
			path.erase(st_it.base(), path.end());
		}
	}

	constexpr auto StringPath::RemoveTop(size_t count) -> void
	{
		if(count == 0)
			return;

		if(path.size() <= count)
			path.clear();
		else
		{
			auto st_it = path.begin();
			std::advance(st_it, count);
			path.erase(path.begin(), st_it);
		}
	}

	constexpr auto StringPath::Clear() -> void
	{
		path.clear();
	}

	constexpr auto StringPath::Erase(IteratorType start, IteratorType end) -> void
	{
		path.erase(start, end);
	}

	constexpr StringPath::operator bool() const noexcept
	{
		return !path.empty();
	}

	constexpr auto StringPath::Empty() const noexcept -> bool
	{
		return path.empty();
	}

	constexpr auto StringPath::Size() const noexcept -> std::size_t
	{
		return path.size();
	}

	constexpr auto StringPath::begin() noexcept -> IteratorType
	{
		return path.begin();
	}

	constexpr auto StringPath::begin() const noexcept -> ConstIteratorType
	{
		return path.begin();
	}

	constexpr auto StringPath::end() noexcept -> IteratorType
	{
		return path.end();
	}

	constexpr auto StringPath::end() const noexcept -> ConstIteratorType
	{
		return path.end();
	}
};
