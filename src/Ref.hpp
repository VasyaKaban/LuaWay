#pragma once

#include "Stack.hpp"
#include "Flags.hpp"
#include <variant>
#include "expected.hpp"
#include <sstream>

namespace LuaWay
{
	class RefIterator;

	class Ref
	{
	private:
		friend auto Stack::Receive<Ref>(lua_State *state, int pos) -> Ref;
		friend auto Stack::Push<Ref>(lua_State *state, const Ref &value) -> void;
		friend class RefIterator;
		friend class VM;

		Ref(lua_State *_state, int _ref);
	public:
		Ref();
		~Ref();
		Ref(const Ref &_ref) noexcept;
		Ref(Ref &&_ref) noexcept;

		static auto CreateNilRef(lua_State *state) noexcept -> Ref;

		auto operator=(const Ref &_ref) noexcept -> Ref &;
		auto operator=(Ref &&_ref) noexcept -> Ref &;

		//template<Stack::HasPush T>
		//auto operator=(T &&value) noexcept -> Ref &;

		auto Destroy() -> void;

		auto operator==(VMType type) const noexcept -> bool;

		auto operator==(const Ref &_ref) const noexcept -> bool;

		explicit operator bool() const noexcept;

		auto IsNil() const noexcept -> bool;

		auto IsStateSame(lua_State *_state) const noexcept -> bool;

		template<Stack::HasPush T>
		auto operator[](T &&key) const noexcept -> std::optional<Object>;

		template<Stack::HasPush T>
		auto GetRaw(T &&key, hrs::Flags<VMType> ref_flags) const noexcept -> std::optional<Object>;

		template<Stack::HasPush T>
		auto Get(T &&key, hrs::Flags<VMType> ref_flags) const noexcept -> std::optional<Object>;

		template<Stack::HasPush T>
		auto GetRawRef(T &&key) const noexcept -> std::optional<Ref>;

		template<Stack::HasPush T>
		auto GetRef(T &&key) const noexcept -> std::optional<Ref>;

		template<Stack::HasPush T>
		auto HasField(T &&key, hrs::Flags<VMType> acceptable_types = hrs::Flags<VMType>::full_mask) const noexcept -> bool;

		template<Stack::HasPush T>
		auto HasFieldRaw(T &&key, hrs::Flags<VMType> acceptable_types = hrs::Flags<VMType>::full_mask) const noexcept -> bool;

		template<Stack::HasPush K, Stack::HasPush V>
		auto Set(K &&key, V &&value) -> void;

		template<Stack::HasPush K, Stack::HasPush V>
		auto SetRaw(K &&key, V &&value) -> void;

		template<VMType vm_type>
		auto As() const noexcept -> std::optional<vm_type_to_data_type_t<vm_type>>;

		auto Type() const noexcept -> VMType;

		auto Holds(VMType type) const noexcept -> bool;

		auto GetLength() const noexcept -> std::size_t;

		auto GetState() const noexcept -> lua_State *;

		auto begin() noexcept -> RefIterator;
		auto end() noexcept -> RefIterator;
		auto begin() const noexcept -> const RefIterator;
		auto end() const noexcept -> const RefIterator;

		template<Stack::HasPush...Args>
		auto operator()(Args &&...args) const noexcept -> hrs::expected<FunctionResult, VMIOError>;

		auto SetMetatable(const Ref &ref) const noexcept -> void;
		auto GetMetatable(const Ref &ref) const noexcept -> Ref;

		template<typename F>
		requires
			std::invocable<F, std::pair<Ref, Ref>> ||
			std::invocable<F, std::pair<Ref, Ref> &>
		auto Traverse(F &f, bool traverse_keys, bool traverse_values, std::size_t max_level = std::numeric_limits<std::size_t>::max()) const noexcept -> void;

	private:

		auto push_value(lua_State *_state) const noexcept -> void;
		auto get_value() noexcept -> int;
		auto type(bool pop) const noexcept -> VMType;

		template<typename F>
		auto traverse(F &f, bool traverse_keys, bool traverse_values, std::vector<int> &refs, std::size_t level, std::size_t max_level) const noexcept -> void;

		lua_State *state;
		int ref;
	};

	class RefIterator
	{
	public:
		RefIterator();
		RefIterator(int _iterable, Ref &&_ref) noexcept;
		~RefIterator() = default;
		RefIterator(const RefIterator &it) noexcept;
		RefIterator(RefIterator &&it) noexcept;

		auto operator=(const RefIterator &it) noexcept -> RefIterator &;
		auto operator=(RefIterator &&it) noexcept -> RefIterator &;

		auto operator++() noexcept -> RefIterator &;
		auto operator*() noexcept -> std::pair<Ref, Ref>;
		auto operator!=(const RefIterator &it) noexcept -> bool;
	private:
		int iterable;
		Ref key_ref;
	};

	template<>
	inline auto Stack::Receive<Ref>(lua_State *state, int pos) -> Ref
	{
		lua_pushvalue(state, pos);
		return {receive_parent_state(state), luaL_ref(state, LUA_REGISTRYINDEX)};
	}

	template<>
	inline auto Stack::Receive<Object>(lua_State *state, int pos) -> Object
	{
		auto type = Stack::GetType(state, pos);
		switch(type)
		{
			case VMType::Nil:
				return Stack::Receive<DataType::Nil>(state, pos);
				break;
			case VMType::Bool:
				return Stack::Receive<DataType::Bool>(state, pos);
				break;
			case VMType::LightUserdata:
				return Stack::Receive<DataType::LightUserdata>(state, pos);
				break;
			case VMType::Number:
				return Stack::Receive<DataType::Number>(state, pos);
				break;
			case VMType::String:
				return Stack::Receive<DataType::String>(state, pos);
				break;
			case VMType::Userdata:
				return Stack::Receive<DataType::Userdata>(state, pos);
				break;
			case VMType::Thread:
				return Stack::Receive<DataType::Thread>(state, pos);
				break;
			case VMType::CFunction:
				return Stack::Receive<DataType::CFunction>(state, pos);
				break;
			case VMType::Int:
				return Stack::Receive<DataType::Int>(state, pos);
				break;
			default:
				return Stack::Receive<Ref>(state, pos);
				break;
		}
	}

	template<>
	inline auto Stack::Push(lua_State *state, const Ref &value) -> void
	{
		assert(value.state == receive_parent_state(state));
		value.push_value(state);
	}

	template<>
	inline auto Stack::Push(lua_State *state, const Object &value) -> void
	{
		std::visit([&]<typename O>(const O &val)
		{
			if constexpr(std::same_as<O, DataType::Userdata>)
				assert((false) && "Userdata cannot be pushed into lua stack!");
			else
			{
				if constexpr(std::same_as<O, DataType::Thread>)
					assert(state == val);
				Stack::Push<O>(state, val);
			}
		}, value);
	}

	inline Ref::Ref(lua_State *_state, int _ref)
	{
		//assume that _thread - main thread of vm!
		state = _state;
		ref = _ref;
	}

	inline Ref::Ref()
	{
		state = nullptr;
		ref = LUA_NOREF;
	}

	inline Ref::~Ref()
	{
		Destroy();
	}

	inline Ref::Ref(const Ref &_ref) noexcept
	{
		if(_ref)
		{
			state = _ref.state;
			_ref.push_value(state);
			ref = get_value();
		}
		else
		{
			state = {};
			ref = LUA_NOREF;
		}
	}

	inline Ref::Ref(Ref &&_ref) noexcept
	{
		state = _ref.state;
		ref = _ref.ref;
		_ref.state = nullptr;
		_ref.ref = LUA_NOREF;
	}

	inline auto Ref::CreateNilRef(lua_State *state) noexcept -> Ref
	{
		return Ref(state, LUA_REFNIL);
	}

	inline auto Ref::operator=(const Ref &_ref) noexcept -> Ref &
	{
		if(_ref)
		{
			Destroy();
			state = _ref.state;
			_ref.push_value(state);
			ref = get_value();
		}
		else
		{
			state = {};
			ref = LUA_NOREF;
		}
		return *this;
	}

	inline auto Ref::operator=(Ref &&_ref) noexcept -> Ref &
	{
		Destroy();
		state = _ref.state;
		ref = _ref.ref;
		_ref.state = nullptr;
		_ref.ref = LUA_NOREF;
		return *this;
	}

	/*template<Stack::HasPush T>
	auto Ref::operator=(T &&value) noexcept -> Ref &
	{
		if(state)
		{
			Stack::PushCheck(state, std::forward<T>(value));
			if(ref == LUA_NOREF || ref == LUA_REFNIL)
				ref = luaL_ref(state, LUA_REGISTRYINDEX);
			else
				lua_rawseti(state, LUA_REGISTRYINDEX, ref);
		}
		return *this;
	}*/

	inline auto Ref::Destroy() -> void
	{
		if(state)
		{
			luaL_unref(state, LUA_REGISTRYINDEX, ref);
			ref = LUA_NOREF;
			//state = nullptr;
		}
	}

	inline auto Ref::operator==(VMType type) const noexcept -> bool
	{
		return Type() == type;
	}

	inline auto Ref::operator==(const Ref &_ref) const noexcept -> bool
	{
		if(state != _ref.state)
			return false;

		if(ref == _ref.ref)
			return true;

		if(ref == LUA_NOREF || _ref.ref == LUA_NOREF)
			return false;

		push_value(state);
		_ref.push_value(state);
		int res = lua_rawequal(state, -1, -2);
		Stack::Pop(state, 2);
		return res;
	}

	inline Ref::operator bool() const noexcept
	{
		if(!state || ref == LUA_NOREF)
			return false;

		return true;
	}

	inline auto Ref::IsNil() const noexcept -> bool
	{
		if(!state || ref == LUA_NOREF)
			return false;

		if(ref == LUA_REFNIL)
			return true;

		//push_value();
		return Type() == VMType::Nil;
	}

	inline auto Ref::IsStateSame(lua_State *_state) const noexcept -> bool
	{
		return state == _state;
	}

	template<Stack::HasPush T>
	auto Ref::operator[](T &&key) const noexcept -> std::optional<Object>
	{
		return GetRaw(std::forward<T>(key, 0));
	}

	template<Stack::HasPush T>
	auto Ref::GetRaw(T &&key, hrs::Flags<VMType> ref_flags) const noexcept -> std::optional<Object>
	{
		if(!*this)
			return {};

		if(type(false) != VMType::Table)
		{
			Stack::Pop(state, 1);
			return {};
		}

		//table
		Stack::PushCheck(state, std::forward<T>(key));
		//table, key
		lua_rawget(state, -2);
		//table, obj
		VMType obj_type = Stack::GetType(state, -1);
		Object obj;
		if(ref_flags & obj_type)
			obj = Stack::Receive<Ref>(state, -1);
		else
			obj = Stack::Receive<Object>(state, -1);

		Stack::Pop(state, 2);

		return obj;
	}

	template<Stack::HasPush T>
	auto Ref::Get(T &&key, hrs::Flags<VMType> ref_flags) const noexcept -> std::optional<Object>
	{
		if(!*this)
			return {};

		//obj
		Stack::PushCheck(state, std::forward<T>(key));
		//obj, key
		lua_gettable(state, -2);
		//obj, value
		VMType obj_type = Stack::GetType(state, -1);
		Object obj;
		if(ref_flags & obj_type)
			obj = Stack::Receive<Ref>(state, -1);
		else
			obj = Stack::Receive<Object>(state, -1);

		Stack::Pop(state, 2);

		return obj;
	}

	template<Stack::HasPush T>
	auto Ref::GetRawRef(T &&key) const noexcept -> std::optional<Ref>
	{
		if(!*this)
			return {};

		if(type(false) != VMType::Table)
		{
			Stack::Pop(state, 1);
			return {};
		}

		//table
		Stack::PushCheck(state, std::forward<T>(key));
		//table, key
		lua_rawget(state, -2);
		//table, obj
		Ref ref = Stack::Receive<Ref>(state, -1);;
		Stack::Pop(state, 2);

		return ref;
	}

	template<Stack::HasPush T>
	auto Ref::GetRef(T &&key) const noexcept -> std::optional<Ref>
	{
		if(!*this)
			return {};

		//obj
		Stack::PushCheck(state, std::forward<T>(key));
		//obj, key
		lua_gettable(state, -2);
		//obj, value
		Ref ref = Stack::Receive<Ref>(state, -1);
		Stack::Pop(state, 2);

		return ref;
	}

	template<Stack::HasPush T>
	auto Ref::HasField(T &&key, hrs::Flags<VMType> acceptable_types) const noexcept -> bool
	{
		auto obj = Get(std::forward<T>(key), hrs::Flags<VMType>::full_mask);
		if(!obj)
			return false;

		Ref obj_ref = std::get<Ref>(obj.value());
		if(acceptable_types & obj_ref.Type())
			return true;

		return false;
	}

	template<Stack::HasPush T>
	auto Ref::HasFieldRaw(T &&key, hrs::Flags<VMType> acceptable_types) const noexcept -> bool
	{
		auto obj = GetRaw(std::forward<T>(key), hrs::Flags<VMType>::full_mask);
		if(!obj)
			return false;

		Ref obj_ref = std::get<Ref>(obj.value());
		if(acceptable_types & obj_ref.Type())
			return true;

		return false;
	}

	template<Stack::HasPush K, Stack::HasPush V>
	auto Ref::Set(K &&key, V &&value) -> void
	{
		if(!*this)
			return;

		//object
		Stack::PushCheck(state, std::forward<K>(key));
		Stack::PushCheck(state, std::forward<V>(value));
		//object, key, value
		lua_settable(state, -3);
		Stack::Pop(state, 1);
	}

	template<Stack::HasPush K, Stack::HasPush V>
	auto Ref::SetRaw(K &&key, V &&value) -> void
	{
		if(!*this)
			return;

		if(type(false) != VMType::Table)
		{
			Stack::Pop(state, 1);
			return;
		}

		//table
		Stack::PushCheck(state, std::forward<K>(key));
		Stack::PushCheck(state, std::forward<V>(value));
		//table, key, value
		lua_rawset(state, -3);
		Stack::Pop(state, 1);
	}

	template<VMType vm_type>
	auto Ref::As() const noexcept -> std::optional<vm_type_to_data_type_t<vm_type>>
	{
		if(!*this)
			return {};

		if(type(false) != vm_type)
		{
			Stack::Pop(state, 1);
			return {};
		}

		using OutType = vm_type_to_data_type_t<vm_type>;
		auto out_value = Stack::Receive<OutType>(state, -1);
		Stack::Pop(state, 1);
		return out_value;
	}

	inline auto Ref::Type() const noexcept -> VMType
	{
		return type(true);
	}

	inline auto Ref::Holds(VMType type) const noexcept -> bool
	{
		return Type() == type;
	}

	inline auto Ref::GetLength() const noexcept -> std::size_t
	{
		if(!*this)
			return 0;

		Stack::Push(state, *this);
		std::size_t len = lua_objlen(state, -1);
		Stack::Pop(state, 1);
		return len;
	}

	inline auto Ref::GetState() const noexcept -> lua_State *
	{
		return state;
	}

	inline auto Ref::begin() noexcept -> RefIterator
	{
		if(!*this)
			return {};

		if(type(false) != VMType::Table)
		{
			Stack::Pop(state, 1);
			return {};
		}

		//table
		Stack::PushCheck(state, DataType::Nil{});
		//table, nil
		RefIterator ref_iter;
		if(lua_next(state, -2) != 0)
		{
			//table, key, value
			Stack::Pop(state, 1);
			ref_iter = RefIterator(this->ref, Ref(this->state, luaL_ref(state, LUA_REGISTRYINDEX)));
		}
		//table
		Stack::Pop(state, 1);
		return ref_iter;
	}

	inline auto Ref::end() noexcept -> RefIterator
	{
		return {};
	}

	inline auto Ref::begin() const noexcept -> const RefIterator
	{
		if(!*this)
			return {};

		if(type(false) != VMType::Table)
		{
			Stack::Pop(state, 1);
			return {};
		}

		//table
		Stack::PushCheck(state, DataType::Nil{});
		//table, nil
		RefIterator ref_iter;
		if(lua_next(state, -2) != 0)
		{
			//table, key, value
			Stack::Pop(state, 1);
			ref_iter = RefIterator(this->ref, Ref(this->state, luaL_ref(state, LUA_REGISTRYINDEX)));
		}
		//table
		Stack::Pop(state, 1);
		return ref_iter;
	}

	inline auto Ref::end() const noexcept -> const RefIterator
	{
		return {};
	}

	template<Stack::HasPush ...Args>
	auto Ref::operator()(Args &&...args) const noexcept -> hrs::expected<FunctionResult, VMIOError>
	{
		if(!*this)
			return FunctionResult{};

		int pre_func_push = lua_gettop(state);
		Stack::PushCheck<Ref>(state, *this);
		int pre_top = lua_gettop(state);
		(Stack::PushCheck(state, std::forward<Args>(args)), ...);
		int res = lua_pcall(state, lua_gettop(state) - pre_top, LUA_MULTRET, 0);
		if(res != 0)
			return VMIOError::ReceiveError(state, res);

		int return_value_count = lua_gettop(state) - pre_func_push;
		FunctionResult out_result;
		out_result.reserve(return_value_count);
		for(int i = 1; i <= return_value_count; i++)
			out_result.push_back(Stack::Receive<Ref>(state, pre_func_push + i));

		Stack::Pop(state, return_value_count);
		return out_result;
	}

	inline auto Ref::SetMetatable(const Ref &ref) const noexcept -> void
	{
		if(!*this)
			return;

		if(!ref.IsStateSame(state))
			return;

		VMType vm_type = type(false);
		if(vm_type == VMType::Table || vm_type == VMType::Userdata)
		{
			push_value(state);
			ref.push_value(state);
			//type, table, metatable
			lua_setmetatable(state, -2);
			Stack::Pop(state, 2);
		}
		else
			Stack::Pop(state, 1);
	}

	inline auto Ref::GetMetatable(const Ref &ref) const noexcept -> Ref
	{
		if(*this)
			return {};

		push_value(state);
		int res = lua_getmetatable(state, -1);
		if(!res)
		{
			Stack::Pop(state, 1);
			return {};
		}

		Ref mt = Stack::Receive<Ref>(state, -1);
		Stack::Pop(state, 2);
		return mt;
	}

	template<typename F>
	requires
		std::invocable<F, std::pair<Ref, Ref>> ||
		std::invocable<F, std::pair<Ref, Ref> &>
	auto Ref::Traverse(F &f, bool traverse_keys, bool traverse_values, std::size_t max_level) const noexcept -> void
	{
		if(!*this)
			return;

		if(max_level == 0)
			return;

		if(Type() != VMType::Table)
			return;

		//table
		std::vector<int> refs;
		refs.reserve(16);
		traverse(f, traverse_keys, traverse_values, refs, 0, max_level);

		for(auto i : refs)
			luaL_unref(state, LUA_REGISTRYINDEX, i);
	}

	inline auto Ref::push_value(lua_State *_state) const noexcept -> void
	{
		assert(_state);
		assert(ref != LUA_NOREF);
		if(ref == LUA_REFNIL)
			lua_pushnil(_state);
		else
			lua_rawgeti(_state, LUA_REGISTRYINDEX, ref);
	}

	inline auto Ref::get_value() noexcept -> int
	{
		assert(lua_gettop(state) > 0);
		return luaL_ref(state, LUA_REGISTRYINDEX);
	}

	inline auto Ref::type(bool pop) const noexcept -> VMType
	{
		if(!*this)
			return VMType::None;

		push_value(state);
		VMType vm_type = Stack::GetType(state, -1);
		if(pop)
			Stack::Pop(state, 1);

		return vm_type;
	}

	template<typename F>
	auto Ref::traverse(F &f, bool traverse_keys, bool traverse_values, std::vector<int> &refs, std::size_t level, std::size_t max_level) const noexcept -> void
	{
		auto check_need_traverse = [&](bool traverse_it, bool &need_traverse, int stack_index)
		{
			if(traverse_it && Stack::GetType(state, stack_index) == VMType::Table)
			{
				for(int tmp_ref : refs)
				{
					lua_rawgeti(state, LUA_REGISTRYINDEX, tmp_ref);
					//table, key, value, ref
					if(lua_rawequal(state, -1, stack_index - 1))
					{
						need_traverse = false;
						Stack::Pop(state, 1);
						break;
					}
					Stack::Pop(state, 1);
				}
			}
			else
				need_traverse = false;
		};

		auto start_traverse = [&](bool need_traverse, bool pop)
		{
			if(need_traverse)
			{
				if(level + 1 <= max_level)
				{
					if(!pop)
						lua_pushvalue(state, -1);

					Ref r = {state, luaL_ref(state, LUA_REGISTRYINDEX)};
					r.traverse(f, traverse_keys, traverse_values, refs, level + 1, max_level);

				}
			}
			else if(pop)
				Stack::Pop(state, 1);
		};

		push_value(state);
		refs.push_back(luaL_ref(state, LUA_REGISTRYINDEX));
		push_value(state);
		Stack::Push(state, DataType::Nil{});
		//table, nil
		while(lua_next(state, -2) != 0)
		{
			//table, key, value
			bool need_traverse_value = true;
			bool need_traverse_key = true;
			std::pair<Ref, Ref> key_value{Stack::Receive<Ref>(state, -2), Stack::Receive<Ref>(state, -1)};
			check_need_traverse(traverse_keys, need_traverse_key, -2);
			check_need_traverse(traverse_values, need_traverse_value, -1);
			//table, key, value
			if(std::invocable<F, std::pair<Ref, Ref>>)
				f(std::move(key_value));
			else
				f(key_value);

			start_traverse(need_traverse_value, true);
			start_traverse(need_traverse_key, false);
		}

		Stack::Pop(state, 1);
	}

	inline RefIterator::RefIterator()
	{
		iterable = LUA_NOREF;
	}

	inline RefIterator::RefIterator(int _iterable, Ref &&_ref) noexcept
	{
		iterable = _iterable;
		key_ref = std::move(_ref);
	}

	inline RefIterator::RefIterator(const RefIterator &it) noexcept
	{
		iterable = it.iterable;
		key_ref = it.key_ref;
	}

	inline RefIterator::RefIterator(RefIterator &&it) noexcept
	{
		iterable = it.iterable;
		key_ref = std::move(it.key_ref);
	}

	inline auto RefIterator::operator=(const RefIterator &it) noexcept -> RefIterator &
	{
		iterable = it.iterable;
		key_ref = it.key_ref;
		return *this;
	}

	inline auto RefIterator::operator=(RefIterator &&it) noexcept -> RefIterator &
	{
		iterable = it.iterable;
		key_ref = std::move(it.key_ref);
		return *this;
	}

	inline auto RefIterator::operator++() noexcept -> RefIterator &
	{
		if(iterable == LUA_NOREF)
			return *this;

		if(!key_ref)
			return *this;

		lua_State *state = key_ref.GetState();
		lua_rawgeti(state, LUA_REGISTRYINDEX, iterable);
		key_ref.push_value(state);
		//table, key
		if(lua_next(state, -2) != 0)
		{
			//table, key, value
			Stack::Pop(state, 1);
			lua_rawseti(state, LUA_REGISTRYINDEX, key_ref.ref);
			//key_ref = Ref(key_ref.state.lock(), luaL_ref(plain_state, LUA_REGISTRYINDEX));
		}
		else
		{
			iterable = LUA_NOREF;
			key_ref = Ref{};
		}

		Stack::Pop(state, 1);
		return *this;
	}

	inline auto RefIterator::operator*() noexcept -> std::pair<Ref, Ref>
	{
		if(iterable == LUA_NOREF)
			return {};

		if(!key_ref)
			return {};

		lua_State *state = key_ref.GetState();
		lua_rawgeti(state, LUA_REGISTRYINDEX, iterable);
		key_ref.push_value(state);
		//table, key
		lua_rawget(state, -2);
		std::pair<Ref, Ref> out_key_value;
		out_key_value.first = key_ref;
		out_key_value.second = Ref(key_ref.state, luaL_ref(state, LUA_REGISTRYINDEX));
		Stack::Pop(state, 1);
		return out_key_value;
	}

	inline auto RefIterator::operator!=(const RefIterator &it) noexcept -> bool
	{
		if(iterable != it.iterable)
			return true;

		return key_ref != it.key_ref;
	}
};
