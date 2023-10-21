#pragma once

#include "Ref.hpp"
#include "StringPath.hpp"
#include <vector>
#include <filesystem>
#include "expected.hpp"

namespace LuaWay
{
	class VM
	{
	public:

		VM();
		~VM();
		VM(const VM &) = delete;
		VM(VM &&vm) noexcept = default;

		auto operator=(const VM &) = delete;
		auto operator=(VM &&vm) noexcept -> VM & = default;

		auto Open(bool open_std_libs, int stack_size = LUA_MINSTACK) -> bool;
		auto Close() -> void;

		auto ExecuteString(const char *str, Ref fenv = {}) -> hrs::expected<FunctionResult, VMIOError>;
		auto ExecuteFile(const std::filesystem::path &path, Ref fenv = {}) -> hrs::expected<FunctionResult, VMIOError>;
		auto Get(const StringPath &str_path) -> Ref;
		auto CollectGarbage() -> void;

		template<Stack::HasPush T>
		auto CreateGlobal(DataType::String name, T &&value) -> void;

		auto CreateTable(int narr, int nrec, DataType::String name = "") -> Ref;

		template<Stack::HasPush T>
		auto CreateRef(T &&value) -> Ref;

		template<typename T>
		auto AllocateUserdata() -> Ref;

		auto AllocateUserdata(std::size_t size) -> Ref;

		template<typename T>
		auto CreateThread() -> Ref;

		constexpr auto GetState() const noexcept -> lua_State *;

	private:
		lua_State *state;
	};

	inline VM::VM()
	{
		state = nullptr;
	}

	inline VM::~VM()
	{
		Close();
	}

	inline auto VM::Open(bool open_std_libs, int stack_size) -> bool
	{
		assert(stack_size > 0);

		if(state)
			return true;

		state = luaL_newstate();
		if(!state)
			return false;

		if(open_std_libs)
			luaL_openlibs(state);

		if(stack_size != LUA_MINSTACK)
		{
			int res = lua_checkstack(state, stack_size);
			if(!res)
			{
				Close();
				return res;
			}
		}

#if LUA_VERSION_NUM < 502
		lua_pushthread(state);
		lua_rawseti(state, LUA_REGISTRYINDEX, __main_thread_ref);
#endif

		return true;
	}

	inline auto VM::Close() -> void
	{
		if(!state)
			return;

		lua_close(state);
		state = nullptr;
	}

	inline auto VM::ExecuteString(const char *str, Ref fenv) -> hrs::expected<FunctionResult, VMIOError>
	{
		assert(state);
		assert(!fenv || fenv.IsStateSame(state));
		int pre_top = lua_gettop(state);
		int result = luaL_loadstring(state, str);
		if(result)
			return VMIOError::ReceiveError(state, result);

		if(fenv.Holds(VMType::Table))
		{
			Stack::Push(state, fenv);
			lua_setfenv(state, -2);
		}

		result = lua_pcall(state, 0, LUA_MULTRET, 0);
		if(result)
			return VMIOError::ReceiveError(state, result);

		int return_value_count = lua_gettop(state) - pre_top;
		if(return_value_count == 0)
			return FunctionResult{};

		FunctionResult out_result;
		out_result.reserve(return_value_count);
		for(int i = 1; i <= return_value_count; i++)
			//out_result.push_back(Ref(state, luaL_ref(receive_state(state), LUA_REGISTRYINDEX)))
			out_result.push_back(Stack::Receive<Ref>(state, pre_top + i));

		Stack::Pop(state, return_value_count);
		return out_result;
	}

	inline auto VM::ExecuteFile(const std::filesystem::path &path, Ref fenv) -> hrs::expected<FunctionResult, VMIOError>
	{
		assert(state);
		assert(!fenv || fenv.IsStateSame(state));
		int result = luaL_loadfile(state, path.c_str());
		if(result)
			return VMIOError::ReceiveError(state, result);

		if(fenv.Holds(VMType::Table))
		{
			Stack::Push(state, fenv);
			lua_setfenv(state, -2);
		}

		int pre_top = lua_gettop(state);
		result = lua_pcall(state, 0, LUA_MULTRET, 0);
		if(result)
			return VMIOError::ReceiveError(state, result);

		int return_value_count = lua_gettop(state) - pre_top;
		if(return_value_count == 0)
			return FunctionResult{};

		FunctionResult out_result;
		out_result.reserve(return_value_count);
		for(int i = 1; i <= return_value_count; i++)
			out_result.push_back(Stack::Receive<Ref>(state, pre_top + i));

		Stack::Pop(state, return_value_count);
		return out_result;
	}

	inline auto VM::Get(const StringPath &str_path) -> Ref
	{
		assert(state);
		if(!str_path)
			return {};

		lua_getglobal(state, str_path.GetPath()[0].c_str());

		if(Stack::GetType(state, -1) == VMType::Nil)
		{
			Stack::Pop(state, 1);
			return {};
		}
		//obj
		for(auto start = str_path.begin() + 1; start != str_path.end(); start++)
		{
			Stack::PushCheck(state, *start);
			//obj, path
			lua_gettable(state, -2);
			//obj, field
			if(Stack::GetType(state, -1) == VMType::Nil)
			{
				Stack::Pop(state, 2);
				return {};
			}

			lua_replace(state, -2);
			//obj
		}

		Ref obj = Stack::Receive<Ref>(state, -1);
		Stack::Pop(state, 1);
		return obj;
	}

	inline auto VM::CollectGarbage() -> void
	{
		assert(state);
		lua_gc(state, LUA_GCCOLLECT, 0);
	}

	template<Stack::HasPush T>
	auto VM::CreateGlobal(DataType::String name, T &&value) -> void
	{
		assert(state);
		Stack::PushCheck(state, value);
		lua_setglobal(state, name.c_str());
	}

	inline auto VM::CreateTable(int narr, int nrec, DataType::String name) -> Ref
	{
		assert(state);
		lua_createtable(state, narr, nrec);
		if(!name.empty())
		{
			lua_pushvalue(state, -1);
			lua_setglobal(state, name.c_str());
		}

		return {state, luaL_ref(state, LUA_REGISTRYINDEX)};
	}

	template<Stack::HasPush T>
	auto VM::CreateRef(T &&value) -> Ref
	{
		assert(state);
		Stack::Push(state, std::forward<T>(value));
		return {state, luaL_ref(state, LUA_REGISTRYINDEX)};
	}

	template<typename T>
	auto VM::AllocateUserdata() -> Ref
	{
		assert(state);
		void *ptr = lua_newuserdata(state, sizeof(T));
		assert(ptr);
		return {state, luaL_ref(state, LUA_REGISTRYINDEX)};
	}

	inline auto VM::AllocateUserdata(std::size_t size) -> Ref
	{
		assert(state);
		void *ptr = lua_newuserdata(state, size);
		assert(ptr);
		return {state, luaL_ref(state, LUA_REGISTRYINDEX)};
	}

	template<typename T>
	auto VM::CreateThread() -> Ref
	{
		assert(state);
		void *ptr = lua_newthread(state);
		assert(ptr);
		return {state, luaL_ref(state, LUA_REGISTRYINDEX)};
	}

	constexpr auto VM::GetState() const noexcept -> lua_State *
	{
		return state;
	}
};
