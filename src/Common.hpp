#pragma once

#include <lua5.1/lua.hpp>
#include <string>
#include <memory>
#include <cassert>
#include <optional>
#include <variant>

#ifndef NDEBUG
	#include <iostream>
	#include <vector>
#endif

namespace LuaWay
{
#if LUA_VERSION_NUM < 502
	constexpr static int __main_thread_ref = 1;
#else
	constexpr static int __main_thread_ref = LUA_RIDX_MAINTHREAD;
#endif

	struct _Nil
	{
		_Nil() noexcept = default;
		_Nil(const _Nil &) noexcept = default;
		_Nil(_Nil &&) noexcept = default;
		constexpr auto operator=(const _Nil &) noexcept -> _Nil & = default;
		constexpr auto operator=(_Nil &&) noexcept -> _Nil & = default;
	};

	struct _LightUserdata
	{
		constexpr _LightUserdata(void *_data) noexcept : data(_data){}
		constexpr _LightUserdata(const _LightUserdata &lu) noexcept : data(lu.data){}
		constexpr _LightUserdata(_LightUserdata &&lu) noexcept : data(lu.data)
		{
			lu.data = nullptr;
		}

		constexpr auto operator=(const _LightUserdata &lu) noexcept
		{
			data = lu.data;
			return *this;
		}

		constexpr auto operator=(_LightUserdata &&lu) noexcept
		{
			data = lu.data;
			lu.data = nullptr;
			return *this;
		}

		void *data;
	};

	struct _Userdata
	{
		constexpr _Userdata(void *_data) noexcept : data(_data){}
		constexpr _Userdata(const _Userdata &u) noexcept : data(u.data){}
		constexpr _Userdata(_Userdata &&u) noexcept : data(u.data)
		{
			u.data = nullptr;
		}

		constexpr auto operator=(const _Userdata &u) noexcept
		{
			data = u.data;
			return *this;
		}

		constexpr auto operator=(_Userdata &&u) noexcept
		{
			data = u.data;
			u.data = nullptr;
			return *this;
		}

		void *data;
	};

	template<typename T, typename ...Args>
	concept AnyOfType = (std::same_as<T, Args> || ...);

	inline auto assert_state_not_expired(lua_State *state) -> void
	{
		assert(state);
	}

	inline auto check_state_not_expired(lua_State *state) -> bool
	{
		return state;
	}

	inline auto receive_parent_state(lua_State *state) -> lua_State *
	{
		assert_state_not_expired(state);

		lua_rawgeti(state, LUA_REGISTRYINDEX, __main_thread_ref);
		assert(lua_isthread(state, -1));
		lua_State *parent = lua_tothread(state, -1);
		lua_pop(state, 1);
		return parent;
	}

#ifndef NDEBUG
	namespace Debug
	{
		inline auto PrintStack(lua_State *state) -> void
		{
			int top = lua_gettop(state);
			for(int i = top; i > 0; i--)
				std::clog<<i<<" -> "<<lua_typename(state, lua_type(state, i))<<std::endl;
		}

		inline auto GetTop(lua_State *state) -> int
		{
			return lua_gettop(state);
		}

		inline auto GetAllReferences(lua_State *state) -> std::vector<int>
		{
			lua_pushnil(state);
			std::vector<int> references;
			references.reserve(16);
			while(lua_next(state, LUA_REGISTRYINDEX) != 0)
			{
				//key, value
				if(lua_type(state, -2) == LUA_TNUMBER)
					references.push_back(lua_tointeger(state, -2));

				lua_pop(state, 1);
			}

			return references;
		}

		inline auto PrintAllReferences(lua_State *state) ->void
		{
			auto references = GetAllReferences(state);
			for(int ref : references)
			{
				lua_rawgeti(state, LUA_REGISTRYINDEX, ref);
				std::cout<<ref<<" -> "<<lua_typename(state, lua_type(state, -1))<<" : "<<((lua_type(state, -1) == LUA_TNUMBER) ? lua_tonumber(state,  -1) : INT_MAX)<<std::endl;
				lua_pop(state, 1);
			}
		}
	}
#endif

	enum class VMType
	{
		None = 1 << 0,
		Nil = 1 << 1,
		Bool = 1 << 2,
		LightUserdata = 1 << 3,
		Number = 1 << 4,
		String = 1 << 5,
		Userdata = 1 << 6,
		Thread = 1 << 7,
		CFunction = 1 << 8,
		Int = 1 << 9,
		Table = 1 << 10,
		Function = 1 << 11
	};

	class Ref;

	namespace DataType
	{
		using Nil = _Nil;
		using Bool = bool;
		using LightUserdata = _LightUserdata;
		using Number = lua_Number;
		using CFunction = lua_CFunction;
		using Int = lua_Integer;
		using String = std::string;
		using Userdata = _Userdata;
		using Thread = lua_State *;
	};

	using Object = std::variant
	<
		DataType::Nil,
		DataType::Bool,
		DataType::LightUserdata,
		DataType::Number,
		DataType::CFunction,
		DataType::Int,
		DataType::String,
		DataType::Userdata,
		DataType::Thread,
		Ref
	>;

	template<typename T>
	concept LuaType = AnyOfType<T,
								DataType::Nil,
								DataType::Bool,
								DataType::LightUserdata,
								DataType::Number,
								DataType::CFunction,
								DataType::Int,
								DataType::String,
								DataType::Userdata,
								DataType::Thread,
								Ref>;

	template<VMType type>
	struct vm_type_to_data_type;

	template<VMType type>
	using vm_type_to_data_type_t = typename vm_type_to_data_type<type>::type;

	template<> struct vm_type_to_data_type<VMType::Nil> {using type = DataType::Nil;};
	template<> struct vm_type_to_data_type<VMType::Bool> {using type = DataType::Bool;};
	template<> struct vm_type_to_data_type<VMType::LightUserdata> {using type = DataType::LightUserdata;};
	template<> struct vm_type_to_data_type<VMType::Number> {using type = DataType::Number;};
	template<> struct vm_type_to_data_type<VMType::String> {using type = DataType::String;};
	template<> struct vm_type_to_data_type<VMType::Userdata> {using type = DataType::Userdata;};
	template<> struct vm_type_to_data_type<VMType::Thread> {using type = DataType::Thread;};
	template<> struct vm_type_to_data_type<VMType::CFunction> {using type = DataType::CFunction;};
	template<> struct vm_type_to_data_type<VMType::Int> {using type = DataType::Int;};
	template<> struct vm_type_to_data_type<VMType::Table> {using type = Ref;};
	template<> struct vm_type_to_data_type<VMType::Function> {using type = Ref;};

	constexpr auto ToString(VMType type) -> std::string_view
	{
		std::string_view type_view;
		switch(type)
		{
			case VMType::None:
				type_view = "None";
				break;
			case VMType::Nil:
				type_view = "Nil";
				break;
			case VMType::Bool:
				type_view = "Bool";
				break;
			case VMType::LightUserdata:
				type_view = "LightUserdata";
				break;
			case VMType::Number:
				type_view = "Number";
				break;
			case VMType::String:
				type_view = "String";
				break;
			case VMType::Userdata:
				type_view = "Userdata";
				break;
			case VMType::Thread:
				type_view = "Thread";
				break;
			case VMType::CFunction:
				type_view = "CFunction";
				break;
			case VMType::Int:
				type_view = "Int";
				break;
			case VMType::Table:
				type_view = "LuaTable";
				break;
			case VMType::Function:
				type_view = "LuaFunction";
				break;
		}

		return type_view;
	}

	template<typename T, T arg1, T ...args>
	concept AnyOfValue = ((arg1 == args) || ...);

	struct VMIOError
	{
		enum class error_code
		{
			Success,
			OutOfMemory,
			SyntaxError,
			FileNotExist,
			InnerError,
			RuntimeError
		};

		DataType::String message;
		error_code code;

		VMIOError() = default;
		constexpr VMIOError(error_code err_code, const char *msg) : code(err_code), message(msg){}
		auto operator==(error_code c) -> bool
		{
			return c == code;
		}

		constexpr operator bool() const noexcept
		{
			return code != error_code::Success;
		}

		static auto ReceiveError(lua_State *state, int error)
		{
			assert(state);

			VMIOError vmio_error;

			switch(error)
			{
				case 0:
					vmio_error.code = error_code::Success;
					break;
				case LUA_ERRMEM:
					vmio_error.code = error_code::OutOfMemory;
					break;
				case LUA_ERRSYNTAX:
					vmio_error.code = error_code::SyntaxError;
					break;
				case LUA_ERRFILE:
					vmio_error.code = error_code::FileNotExist;
					break;
				case LUA_ERRRUN:
					vmio_error.code = error_code::RuntimeError;
					break;
				case LUA_ERRERR:
					vmio_error.code = error_code::InnerError;
					break;
			}

			if(vmio_error.code == error_code::Success)
				return vmio_error;

			std::size_t len = 0;
			const char *msg = lua_tolstring(state, -1, &len);
			vmio_error.message = std::string(msg, msg + len);
			lua_pop(state, 1);
			return vmio_error;
		}
	};

	using FunctionResult = std::vector<Ref>;
};
