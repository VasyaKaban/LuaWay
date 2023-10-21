#include "FunctionTraits.hpp"
#include "Ref.hpp"
#include <limits>

namespace LuaWay
{
	auto __emit_error(lua_State *state, bool condition, const std::string_view msg, int pop_count = 0) -> void
	{
		if(!condition)
		{
			if(pop_count > 0)
				lua_pop(state, pop_count);
			lua_pushlstring(state, msg.data(), msg.size());
			lua_error(state);
		}
	}

	template<Stack::HasReceive ...Args, int ...Ind>
	auto __receive_arguments(lua_State *state, const std::integer_sequence<int, Ind...> &) -> std::tuple<Args...>
	{
		using TupleType = std::tuple<Args...>;
		return TupleType{Stack::Receive<std::tuple_element_t<Ind, TupleType>>(state, Ind - static_cast<int>(sizeof...(Args)))...};
	}

	template<auto func, typename C, typename R, typename ...Args>
	auto __cfunction_wrapper(lua_State *state) -> int
	{
		//recieve args
		//call function
		//push return value
		//return count of results

		static_assert(sizeof...(Args) < std::numeric_limits<int>::max(), "Too many arguments!");
		constexpr int args_count = sizeof...(Args);
		int pre_top = lua_gettop(state);
		__emit_error(state, pre_top >= args_count, std::to_string(args_count) + " arguments were expected but " + std::to_string(pre_top) + " has been received!", pre_top);
		auto arguments = __receive_arguments(state, std::make_integer_sequence<int, args_count>{});
		if constexpr(std::same_as<C, void>)
		{
			//plain function
			if constexpr(std::same_as<R, void>)
			{
				std::apply([&]<typename ...Targs>(Targs &...targs)
				{
					func(targs...);
				}, arguments);
				Stack::Pop(state, args_count);
				return 0;
			}
			else
			{
				R return_value = std::apply([&]<typename ...Targs>(Targs &...targs)
				{
					return func(targs...);
				}, arguments);
				Stack::Pop(state, args_count);
				Stack::Push(state, return_value);
				return 1;
			}
		}
		else
		{
			//member function
			__emit_error(state, pre_top >= (args_count + 1), std::to_string(args_count + 1) + " arguments were expected but " + std::to_string(pre_top) + " has been received!", pre_top);
			VMType vm_type = Stack::GetType(state, pre_top);
			__emit_error(state, vm_type == VMType::Userdata, "Bad object type: " + std::string(ToString(vm_type)), args_count + 1);
			C *this_ptr = static_cast<C *>(Stack::Receive<DataType::Userdata>(state, pre_top).data);
			if constexpr(std::same_as<R, void>)
			{
				std::apply([&]<typename ...Targs>(Targs &...targs)
				{
					(this_ptr->*func)(targs...);
				}, arguments);
				Stack::Pop(state, args_count + 1);
				return 0;
			}
			else
			{
				R return_value = std::apply([&]<typename ...Targs>(Targs &...targs)
				{
					return (this_ptr->*func)(targs...);
				}, arguments);
				Stack::Pop(state, args_count + 1);
				Stack::Push(state, return_value);
				return 1;
			}
		}
	}

	template<auto func>
	requires
		std::is_function_v<std::remove_pointer_t<decltype(func)>> ||
		std::is_member_function_pointer_v<decltype(func)>
	constexpr auto CreateCFunctionWrapper() -> DataType::CFunction
	{
		if constexpr(std::is_function_v<std::remove_pointer_t<decltype(func)>>)
		{
			//plain foo
			using FunctionType = std::remove_pointer_t<decltype(func)>;
			constexpr auto func_argument_deductor = []<typename ...Args>(std::tuple<Args...>) -> DataType::CFunction
			{
				using ClassType = void;
				using ReturnType = function_return_type_t<FunctionType>;
				static_assert((Stack::HasPush<ReturnType>), "ReturnType doesn't have a Push fucntion overload!");
				static_assert((Stack::HasReceive<Args> && ...), "Not all Args have a Stack::Recieve!");
				return __cfunction_wrapper
						<
							func,
							ClassType,
							ReturnType,
							Args...
						>;
			};
			return func_argument_deductor(function_arguments_t<FunctionType>{});
		}
		else if constexpr(std::is_member_function_pointer_v<decltype(func)>)
		{
			//pointer to member function
			using FunctionType = decltype(func);
			constexpr auto func_argument_deductor = []<typename ...Args>(std::tuple<Args...>) -> DataType::CFunction
			{
				using ClassType = member_function_pointer_class_t<FunctionType>;
				using ReturnType = member_function_pointer_return_type_t<FunctionType>;
				static_assert((Stack::HasPush<ReturnType>), "ReturnType doesn't have a Push fucntion overload!");
				static_assert((Stack::HasReceive<Args> && ...), "Not all Args have a Stack::Receive!");
				return __cfunction_wrapper
						<
							func,
							ClassType,
							ReturnType,
							Args...
						>;
			};
			return func_argument_deductor(member_function_pointer_arguments_t<FunctionType>{});
		}
	}

	template<typename C>
		requires std::is_class_v<C>
	auto __destructor_wrapper(lua_State *state) -> int
	{
		int pre_top = lua_gettop(state);
		__emit_error(state, pre_top >= 1, "No object to destroy!");
		VMType vm_type = Stack::GetType(state, -1);
		__emit_error(state, vm_type == VMType::Userdata, "Userdata was expected but " + std::string(ToString(vm_type)) + " has been received!", 1);
		DataType::Userdata udata = Stack::Receive<DataType::Userdata>(state, -1);
		Stack::Pop(state, 1);
		if constexpr(std::is_destructible_v<C>)
		{
			C *ptr = static_cast<C *>(udata.data);
			ptr->~C();
		}

		return 0;
	}

	template<typename C>
		requires std::is_class_v<C>
	constexpr auto CreateDestructorWrapper() -> DataType::CFunction
	{
		return __destructor_wrapper<C>;
	}

	template<typename C, LuaWay::Stack::HasReceive ...Args>
		requires std::is_class_v<C>
	auto __constructor_wrapper(lua_State *state) -> int
	{
		static_assert(sizeof...(Args) < std::numeric_limits<int>::max(), "Too many arguments!");
		constexpr int args_count = sizeof...(Args);
		int pre_top = lua_gettop(state);
		__emit_error(state, pre_top >= (args_count + 1), std::to_string(args_count + 1) + " arguments were expected but " + std::to_string(pre_top) + " has been received!", pre_top);
		auto arguments = __receive_arguments<Args...>(state, std::make_integer_sequence<int, args_count>{});
		LuaWay::Ref mt = Stack::Receive<Ref>(state, -(args_count + 1));
		Stack::Pop(state, args_count + 1);
		VMType mt_type = mt.Type();
		__emit_error(state, mt_type == VMType::Table, "Bad metatable type: " + std::string(ToString(mt_type)));
		std::apply([&]<typename ...Targs>(Targs &...targs)
		{
			void *ptr = lua_newuserdata(state, sizeof(C));
			new(ptr) C(targs...);
			Stack::Push(state, mt);
			//udata, metatable
			lua_setmetatable(state, -2);
		}, arguments);
		return 1;
	}

	template<typename C, LuaWay::Stack::HasReceive ...Args>
		requires std::is_class_v<C>
	constexpr auto CreateConstructorWrapper() -> DataType::CFunction
	{
		return __constructor_wrapper<C, Args...>;
	}
};



/*template<auto func, typename R, typename ...Args>
	requires
	requires()
{
	{LuaCFunctionStack<R>{}};
	{(LuaCFunctionStack<Args>{}, ...)};
}
auto call(lua_State *state) -> int
{
	assert(state);

	std::tuple<Args...> lua_arguments{LuaCFunctionStack<Args>::Get(state)...};

	auto make_call = [&]<typename ...ApplyArgs>(ApplyArgs &&...args)
	{
		if constexpr(std::is_member_function_pointer_v<decltype(func)>)
		{
			using ThisClassType = member_function_pointer_class_t<decltype(func)>;

			auto get_this = [&]<typename T, typename ...OArgs>(T &&t, OArgs ...)
			{
#warning make here check for userdata or lightuserdata!
				return std::forward<T>(t);
			};

			ThisClassType *this_ptr = reinterpret_cast<ThisClassType *>(get_this(std::forward<ApplyArgs>(args)...));

			return (*this_ptr.*func)(std::forward<ApplyArgs>(args)...);
		}
		else
			return func(std::forward<ApplyArgs>(args)...);
	};

	if constexpr(std::is_void_v<R>)
	{
		std::apply(make_call, lua_arguments);
	}
	else
	{
		R result = std::apply(make_call, lua_arguments);
		LuaCFunctionStack<R>::Push(state, result);
	}

	return LuaCFunctionStack<R>::ReturnTypeCount();
}

template<auto f>
	requires
	std::is_function_v<std::remove_pointer_t<decltype(f)>> ||
	std::is_member_function_pointer_v<decltype(f)> ||
	is_object_has_call_operator<std::remove_reference_t<decltype(f)>>
	auto Call()
{
	using F = decltype(f);
	using PureF = std::remove_pointer_t<F>;
	if constexpr(std::is_function_v<PureF>)
	{
		auto retrieve = [&]<typename ...Args>(std::tuple<Args...>)
		{
			return call<f, function_return_type_t<PureF>, Args...>;
		};

		return retrieve(function_arguments_t<PureF>{});
	}
	else if constexpr(std::is_member_function_pointer_v<F>)
	{
		auto retrieve = [&]<typename ...Args>(std::tuple<Args...>)
		{
			return call<f, member_function_pointer_return_type_t<F>, Args...>;
		};

		return retrieve(member_function_pointer_arguments_t<F>{});
	}
	else
	{
		constexpr auto call_operator_ptr = &std::remove_reference_t<F>::operator();
		using CallOperatorT = decltype(&std::remove_reference_t<F>::operator());
		auto retrieve = [&]<typename ...Args>(std::tuple<Args...>)
		{
			return call<call_operator_ptr, member_function_pointer_return_type_t<CallOperatorT>, Args...>;
		};

		return retrieve(member_function_pointer_arguments_t<CallOperatorT>{});
	}
};*/

