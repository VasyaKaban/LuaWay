#pragma once

#include "Common.hpp"

namespace LuaWay
{
	namespace Stack
	{
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

		template<typename T>
		auto Push(lua_State *state, const T &value) -> void;

		template<typename T>
		concept HasPush = requires(T &val)
		{
			{Push<T>(static_cast<lua_State *>(nullptr), val)};
		};

		template<HasPush T>
		auto PushCheck(lua_State *state, const T &value) -> void
		{
			assert_state_not_expired(state);
		#ifndef NDEBUG
			int target_pos = lua_gettop(state);
		#endif
			Push<T>(state, value);
			assert((lua_gettop(state) - target_pos) == 1);
		}

		template<typename T>
		auto Receive(lua_State *state, int pos) -> T;

		template<typename T>
		concept HasReceive = requires(T &val)
		{
			{Receive<T>(static_cast<lua_State *>(nullptr), {})} -> std::same_as<T>;
		};

		template<HasReceive T>
		auto ReceiveCheck(lua_State *state, int pos) -> T
		{
			assert_pos_reacheable(state, pos);
			#ifndef NDEBUG
				int target_pos = lua_gettop(state);
			#endif
				T obj = Receive<T>(state, pos);
				assert((lua_gettop(state) - target_pos) == 0);
		}

		//for DataType:
		template<>
		inline auto Push(lua_State *state, const DataType::Nil &value) -> void
		{
			lua_pushnil(state);
		}

		template<>
		inline auto Push(lua_State *state, const DataType::Bool &value) -> void
		{
			lua_pushboolean(state, value);
		}

		template<>
		inline auto Push(lua_State *state, const DataType::LightUserdata &value) -> void
		{
			lua_pushlightuserdata(state, value.data);
		}

		template<>
		inline auto Push(lua_State *state, const DataType::Number &value) -> void
		{
			lua_pushnumber(state, value);
		}

		template<>
		inline auto Push(lua_State *state, const DataType::CFunction &value) -> void
		{
			lua_pushcfunction(state, value);
		}

		template<>
		inline auto Push(lua_State *state, const DataType::Int &value) -> void
		{
			lua_pushinteger(state, value);
		}

		template<>
		inline auto Push(lua_State *state, const DataType::String &value) -> void
		{
			lua_pushlstring(state, value.c_str(), value.size());
		}

		template<>
		inline auto Push(lua_State *state, const DataType::Thread &value) -> void
		{
			assert(state == value);
			lua_pushthread(value);
		}

		template<>
		inline auto Receive(lua_State *state, int pos) -> DataType::Nil
		{
			assert(Stack::GetType(state, pos) == VMType::Nil);
			return {};
		}

		template<>
		inline auto Receive(lua_State *state, int pos) -> DataType::Bool
		{
			assert(Stack::GetType(state, pos) == VMType::Bool);
			return lua_toboolean(state, pos);
		}

		template<>
		inline auto Receive(lua_State *state, int pos) -> DataType::LightUserdata
		{
			assert(Stack::GetType(state, pos) == VMType::LightUserdata);
			return lua_touserdata(state, pos);
		}

		template<>
		inline auto Receive(lua_State *state, int pos) -> DataType::Number
		{
			assert(Stack::GetType(state, pos) == VMType::Number);
			return lua_tonumber(state, pos);
		}

		template<>
		inline auto Receive(lua_State *state, int pos) -> DataType::CFunction
		{
			assert(Stack::GetType(state, pos) == VMType::CFunction);
			return lua_tocfunction(state, pos);
		}

		template<>
		inline auto Receive(lua_State *state, int pos) -> DataType::Int
		{
			assert(Stack::GetType(state, pos) == VMType::Number);
			return lua_tonumber(state, pos);
		}

		template<>
		inline auto Receive(lua_State *state, int pos) -> DataType::String
		{
			assert(Stack::GetType(state, pos) == VMType::String);
			std::size_t len = 0;
			auto str_ptr = lua_tolstring(state, pos, &len);
			return {str_ptr, str_ptr + len};
		}

		template<>
		inline auto Receive(lua_State *state, int pos) -> DataType::Userdata
		{
			assert(Stack::GetType(state, pos) == VMType::Userdata);
			return lua_touserdata(state, pos);
		}

		template<>
		inline auto Receive(lua_State *state, int pos) -> DataType::Thread
		{
			assert(Stack::GetType(state, pos) == VMType::Thread);
			return lua_tothread(state, pos);
		}
	};
};

