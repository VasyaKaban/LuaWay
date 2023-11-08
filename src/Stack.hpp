#pragma once

#include "Common.hpp"

namespace LuaWay
{
	template<typename T>
	struct Stack
	{
		using Type = T;
		static auto Push(lua_State *state, const T &value) -> void = delete;
		static auto Receive(lua_State *state, int pos) = delete;

		template<VMType type>
		constexpr static bool ConvertibleFromVM = false;
	};

	namespace StackUtil
	{
		template<typename T>
		concept HasPush = requires(T &&t)
		{
			{Stack<std::remove_cvref_t<T>>::Push(static_cast<lua_State *>(nullptr), std::forward<T>(t))}
				  -> std::same_as<void>;
		};

		template<typename T>
		concept HasReceive = requires(T &&t)
		{
			{Stack<std::remove_cvref_t<T>>::Receive(static_cast<lua_State *>(nullptr), -1)}
				  -> std::same_as<std::remove_cvref_t<T>>;
		};

		inline auto assert_pos_reacheable(lua_State *state, int pos) -> void
		{
			assert(pos != 0);
			assert_state_not_expired(state);
			assert((pos > 0 ? lua_gettop(state) >= pos : (-lua_gettop(state)) <= pos));
		}

		inline auto GetType(lua_State *state, int pos) -> VMType
		{
			assert_pos_reacheable(state, pos);
			int vm_type = lua_type(state, pos);
			VMType type = VMType::Nil;
			switch(vm_type)
			{
				case LUA_TNIL:
					type = VMType::Nil;
					break;
				case LUA_TBOOLEAN:
					type = VMType::Bool;
					break;
				case LUA_TLIGHTUSERDATA:
					type = VMType::LightUserdata;
					break;
				case LUA_TNUMBER:
					type = VMType::Number;
					break;
				case LUA_TSTRING:
					type = VMType::String;
					break;
				case LUA_TUSERDATA:
					type = VMType::Userdata;
					break;
				case LUA_TTHREAD:
					type = VMType::Thread;
					break;
				case LUA_TFUNCTION:
					if(lua_iscfunction(state, pos))
						type = VMType::CFunction;
					else
						type = VMType::Function;
					break;
				case LUA_TTABLE:
					type = VMType::Table;
					break;
				default:
					assert((false) && "LUA_TNONE type is not allowed!");
					break;
			}

			return type;
		}

		inline auto Pop(lua_State *state, std::size_t count) -> void
		{
			assert_state_not_expired(state);
			lua_pop(state, count);
		}

		template<HasPush T>
		auto PushCheck(lua_State *state, T &&value) -> void
		{
			assert_state_not_expired(state);
		#ifndef NDEBUG
			int target_pos = lua_gettop(state);
		#endif
			Stack<std::remove_cvref_t<T>>::Push(state, std::forward<T>(value));
		#ifndef NDEBUG
			assert((lua_gettop(state) - target_pos) == 1);
		#endif
		}

		template<HasReceive T>
		auto ReceiveCheck(lua_State *state, int pos) -> T
		{
			assert_pos_reacheable(state, pos);
		#ifndef NDEBUG
			int target_pos = lua_gettop(state);
		#endif
			T obj = Stack<T>::Receive(state, pos);
		#ifndef NDEBUG
			assert((lua_gettop(state) - target_pos) == 0);
		#endif
			return obj;
		}

		template<HasReceive T>
		constexpr auto check_type_is_convertible_from_vm(VMType type) -> bool
		{
			bool is_convertible = false;
			switch(type)
			{
				case VMType::Nil:
					is_convertible = Stack<T>::template ConvertibleFromVM<VMType::Nil>;
					break;
				case VMType::CFunction:
					is_convertible = Stack<T>::template ConvertibleFromVM<VMType::CFunction>;
					break;
				case VMType::Function:
					is_convertible = Stack<T>::template ConvertibleFromVM<VMType::Function>;
					break;
				case VMType::Bool:
					is_convertible = Stack<T>::template ConvertibleFromVM<VMType::Bool>;
					break;
				case VMType::Int:
					is_convertible = Stack<T>::template ConvertibleFromVM<VMType::Int>;
					break;
				case VMType::LightUserdata:
					is_convertible = Stack<T>::template ConvertibleFromVM<VMType::LightUserdata>;
					break;
				case VMType::Userdata:
					is_convertible = Stack<T>::template ConvertibleFromVM<VMType::Userdata>;
					break;
				case VMType::None:
					is_convertible = Stack<T>::template ConvertibleFromVM<VMType::None>;
					break;
				case VMType::Number:
					is_convertible = Stack<T>::template ConvertibleFromVM<VMType::Number>;
					break;
					case VMType::String:
					is_convertible = Stack<T>::template ConvertibleFromVM<VMType::String>;
					break;
				case VMType::Table:
					is_convertible = Stack<T>::template ConvertibleFromVM<VMType::Table>;
					break;
				case VMType::Thread:
					is_convertible = Stack<T>::template ConvertibleFromVM<VMType::Thread>;
					break;
			}

			return is_convertible;
		}
	};
};

//specialiation for DataType
namespace LuaWay
{
	template<>
	struct Stack<DataType::Nil>
	{
		using Type = DataType::Nil;
		static auto Push(lua_State *state, const Type &value) -> void
		{
			lua_pushnil(state);
		}

		static auto Receive(lua_State *state, int pos) -> Type
		{
			return Type();
		}

		template<VMType type>
		constexpr static bool ConvertibleFromVM = (type == VMType::Nil);
	};

	template<>
	struct Stack<DataType::Bool>
	{
		using Type = DataType::Bool;
		static auto Push(lua_State *state, const Type &value) -> void
		{
			lua_pushboolean(state, value);
		}

		static auto Receive(lua_State *state, int pos) -> Type
		{
			return lua_toboolean(state, pos);
		}

		template<VMType type>
		constexpr static bool ConvertibleFromVM = (type == VMType::Bool);
	};

	template<>
	struct Stack<DataType::Number>
	{
		using Type = DataType::Number;
		static auto Push(lua_State *state, const Type &value) -> void
		{
			lua_pushnumber(state, value);
		}

		static auto Receive(lua_State *state, int pos) -> Type
		{
			return lua_tonumber(state, pos);
		}

		template<VMType type>
		constexpr static bool ConvertibleFromVM = (type == VMType::Number);
	};

	template<>
	struct Stack<DataType::String>
	{
		using Type = DataType::String;
		static auto Push(lua_State *state, const Type &value) -> void
		{
			lua_pushlstring(state, value.c_str(), value.size());
		}

		static auto Receive(lua_State *state, int pos) -> Type
		{
			std::size_t len = 0;
			const char *str = lua_tolstring(state, pos, &len);
			return Type{str, str + len};
		}

		template<VMType type>
		constexpr static bool ConvertibleFromVM = (type == VMType::String);
	};

	template<>
	struct Stack<DataType::Int>
	{
		using Type = DataType::Int;
		static auto Push(lua_State *state, const Type &value) -> void
		{
			lua_pushinteger(state, value);
		}

		static auto Receive(lua_State *state, int pos) -> Type
		{
			return lua_tointeger(state, pos);
		}

		template<VMType type>
		constexpr static bool ConvertibleFromVM = (type == VMType::Int || type == VMType::Number);
	};

	template<>
	struct Stack<DataType::CFunction>
	{
		using Type = DataType::CFunction;
		static auto Push(lua_State *state, const Type &value) -> void
		{
			lua_pushcfunction(state, value);
		}

		static auto Receive(lua_State *state, int pos) -> Type
		{
			return lua_tocfunction(state, pos);
		}

		template<VMType type>
		constexpr static bool ConvertibleFromVM = (type == VMType::CFunction);
	};

	template<>
	struct Stack<DataType::LightUserdata>
	{
		using Type = DataType::LightUserdata;
		static auto Push(lua_State *state, const Type &value) -> void
		{
			lua_pushlightuserdata(state, value.data);
		}

		static auto Receive(lua_State *state, int pos) -> Type
		{
			return lua_touserdata(state, pos);
		}

		template<VMType type>
		constexpr static bool ConvertibleFromVM = (type == VMType::LightUserdata || type == VMType::Userdata);
	};

	template<>
	struct Stack<DataType::Thread>
	{
		using Type = DataType::Thread;
		static auto Push(lua_State *state, const Type &value) -> void
		{
			lua_pushthread(value);
		}

		static auto Receive(lua_State *state, int pos) -> Type
		{
			return lua_tothread(state, pos);
		}

		template<VMType type>
		constexpr static bool ConvertibleFromVM = (type == VMType::Thread);
	};

	template<>
	struct Stack<DataType::Userdata>
	{
		using Type = DataType::Userdata;
		static auto Push(lua_State *state, const Type &value) -> void = delete;

		static auto Receive(lua_State *state, int pos) -> Type
		{
			return lua_touserdata(state, pos);
		}

		template<VMType type>
		constexpr static bool ConvertibleFromVM = (type == VMType::Userdata || type == VMType::LightUserdata);
	};
};

