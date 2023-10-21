#pragma once

#include <type_traits>
#include <tuple>

//is_const
template<typename F>
requires std::is_function_v<F>
struct is_function_const_qual : std::false_type{};

template<typename F>
requires std::is_function_v<F>
constexpr inline bool is_function_const_qual_v = is_function_const_qual<F>::value;

template<typename R, typename ...Args>
struct is_function_const_qual<auto (Args...) const -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_const_qual<auto (Args...) const volatile -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_const_qual<auto (Args...) const noexcept -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_const_qual<auto (Args...) const & -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_const_qual<auto (Args...) const && -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_const_qual<auto (Args...) const volatile noexcept -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_const_qual<auto (Args...) const volatile & -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_const_qual<auto (Args...) const volatile && -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_const_qual<auto (Args...) const volatile & noexcept -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_const_qual<auto (Args...) const volatile && noexcept -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_const_qual<auto (Args...) const & noexcept -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_const_qual<auto (Args...) const && noexcept -> R> : std::true_type{};

template<typename F>
requires std::is_member_function_pointer_v<F>
struct is_member_function_pointer_const_qual : std::false_type{};

template<typename F>
requires std::is_member_function_pointer_v<F>
constexpr inline bool is_member_function_pointer_const_qual_v = is_member_function_pointer_const_qual<F>::value;

template<typename T, typename C>
struct is_member_function_pointer_const_qual<T C::*> : is_function_const_qual<T>{};
//is_const

//is_volatile
template<typename F>
requires std::is_function_v<F>
struct is_function_volatile_qual : std::false_type{};

template<typename F>
requires std::is_function_v<F>
constexpr inline bool is_function_volatile_qual_v = is_function_volatile_qual<F>::value;

template<typename R, typename ...Args>
struct is_function_volatile_qual<auto (Args...) volatile -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_volatile_qual<auto (Args...) const volatile -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_volatile_qual<auto (Args...) volatile noexcept -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_volatile_qual<auto (Args...) volatile & -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_volatile_qual<auto (Args...) volatile && -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_volatile_qual<auto (Args...) const volatile noexcept -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_volatile_qual<auto (Args...) const volatile & -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_volatile_qual<auto (Args...) const volatile && -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_volatile_qual<auto (Args...) const volatile & noexcept -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_volatile_qual<auto (Args...) const volatile && noexcept -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_volatile_qual<auto (Args...) volatile & noexcept -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_volatile_qual<auto (Args...) volatile && noexcept -> R> : std::true_type{};

template<typename F>
requires std::is_member_function_pointer_v<F>
struct is_member_function_pointer_volatile_qual : std::false_type{};

template<typename F>
requires std::is_member_function_pointer_v<F>
constexpr inline bool is_member_function_pointer_volatile_qual_v = is_member_function_pointer_volatile_qual<F>::value;

template<typename T, typename C>
struct is_member_function_pointer_volatile_qual<T C::*> : is_function_volatile_qual<T>{};
//is_volatile

//is_lvalue_qual
template<typename F>
requires std::is_function_v<F>
struct is_function_lvalue_ref_qual : std::false_type{};

template<typename F>
requires std::is_function_v<F>
constexpr inline bool is_function_lvalue_ref_qual_v = is_function_lvalue_ref_qual<F>::value;

template<typename R, typename ...Args>
struct is_function_lvalue_ref_qual<auto (Args...) & -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_lvalue_ref_qual<auto (Args...) const & -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_lvalue_ref_qual<auto (Args...) volatile & -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_lvalue_ref_qual<auto (Args...) & noexcept -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_lvalue_ref_qual<auto (Args...) const volatile & -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_lvalue_ref_qual<auto (Args...) const & noexcept -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_lvalue_ref_qual<auto (Args...) const volatile & noexcept-> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_lvalue_ref_qual<auto (Args...) volatile & noexcept-> R> : std::true_type{};

template<typename F>
requires std::is_member_function_pointer_v<F>
struct is_member_function_pointer_lvalue_ref_qual : std::false_type{};

template<typename F>
requires std::is_member_function_pointer_v<F>
constexpr inline bool is_member_function_pointer_lvalue_ref_qual_v = is_member_function_pointer_lvalue_ref_qual<F>::value;

template<typename T, typename C>
struct is_member_function_pointer_lvalue_ref_qual<T C::*> : is_function_lvalue_ref_qual<T>{};
//is_lvalue_qual

//is_rvalue_qual
template<typename F>
requires std::is_function_v<F>
struct is_function_rvalue_ref_qual : std::false_type{};

template<typename F>
requires std::is_function_v<F>
constexpr inline bool is_function_rvalue_ref_qual_v = is_function_rvalue_ref_qual<F>::value;

template<typename R, typename ...Args>
struct is_function_rvalue_ref_qual<auto (Args...) && -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_rvalue_ref_qual<auto (Args...) const && -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_rvalue_ref_qual<auto (Args...) volatile && -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_rvalue_ref_qual<auto (Args...) && noexcept -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_rvalue_ref_qual<auto (Args...) const volatile && -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_rvalue_ref_qual<auto (Args...) const && noexcept -> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_rvalue_ref_qual<auto (Args...) const volatile && noexcept-> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_rvalue_ref_qual<auto (Args...) volatile && noexcept-> R> : std::true_type{};

template<typename F>
requires std::is_member_function_pointer_v<F>
struct is_member_function_pointer_rvalue_ref_qual : std::false_type{};

template<typename F>
requires std::is_member_function_pointer_v<F>
constexpr inline bool is_member_function_pointer_rvalue_ref_qual_v = is_member_function_pointer_rvalue_ref_qual<F>::value;

template<typename T, typename C>
struct is_member_function_pointer_rvalue_ref_qual<T C::*> : is_function_rvalue_ref_qual<T>{};
//is_rvalue_qual

//is_function_noexcept
template<typename F>
requires std::is_function_v<F>
struct is_function_noexcept_qual : std::false_type{};

template<typename F>
requires std::is_function_v<F>
constexpr inline bool is_function_noexcept_qual_v = is_function_noexcept_qual<F>::value;

template<typename R, typename ...Args>
struct is_function_noexcept_qual<auto (Args...) noexcept-> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_noexcept_qual<auto (Args...) const noexcept-> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_noexcept_qual<auto (Args...) volatile noexcept-> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_noexcept_qual<auto (Args...) & noexcept-> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_noexcept_qual<auto (Args...) && noexcept-> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_noexcept_qual<auto (Args...) const volatile noexcept-> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_noexcept_qual<auto (Args...) const & noexcept-> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_noexcept_qual<auto (Args...) const && noexcept-> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_noexcept_qual<auto (Args...) const volatile & noexcept-> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_noexcept_qual<auto (Args...) const volatile && noexcept-> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_noexcept_qual<auto (Args...) volatile & noexcept-> R> : std::true_type{};

template<typename R, typename ...Args>
struct is_function_noexcept_qual<auto (Args...) volatile && noexcept-> R> : std::true_type{};

template<typename F>
requires std::is_member_function_pointer_v<F>
struct is_member_function_pointer_noexcept_qual : std::false_type{};

template<typename F>
requires std::is_member_function_pointer_v<F>
constexpr inline bool is_member_function_pointer_noexcept_qual_v = is_member_function_pointer_noexcept_qual<F>::value;

template<typename T, typename C>
struct is_member_function_pointer_noexcept_qual<T C::*> : is_member_function_pointer_noexcept_qual<T>{};
//is_rvalue_qual

//function_arguments
template<typename F>
requires std::is_function_v<F>
struct function_arguments;

template<typename F>
requires std::is_function_v<F>
using function_arguments_t = typename function_arguments<F>::Arguments;

template<typename R, typename ...Args>
struct function_arguments<auto (Args...) -> R> {using Arguments = std::tuple<Args...>;};

template<typename R, typename ...Args>
struct function_arguments<auto (Args...) const -> R> : function_arguments<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_arguments<auto (Args...) volatile -> R> : function_arguments<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_arguments<auto (Args...) noexcept -> R> : function_arguments<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_arguments<auto (Args...) & -> R> : function_arguments<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_arguments<auto (Args...) && -> R> : function_arguments<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_arguments<auto (Args...) const volatile -> R> : function_arguments<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_arguments<auto (Args...) const noexcept -> R> : function_arguments<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_arguments<auto (Args...) const & -> R> : function_arguments<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_arguments<auto (Args...) const && -> R> : function_arguments<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_arguments<auto (Args...) const volatile noexcept -> R> : function_arguments<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_arguments<auto (Args...) const volatile & -> R> : function_arguments<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_arguments<auto (Args...) const volatile && -> R> : function_arguments<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_arguments<auto (Args...) const & noexcept -> R> : function_arguments<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_arguments<auto (Args...) const && noexcept -> R> : function_arguments<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_arguments<auto (Args...) const volatile & noexcept -> R> : function_arguments<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_arguments<auto (Args...) const volatile && noexcept -> R> : function_arguments<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_arguments<auto (Args...) volatile noexcept -> R> : function_arguments<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_arguments<auto (Args...) volatile & -> R> : function_arguments<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_arguments<auto (Args...) volatile && -> R> : function_arguments<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_arguments<auto (Args...) volatile & noexcept -> R> : function_arguments<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_arguments<auto (Args...) volatile && noexcept -> R> : function_arguments<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_arguments<auto (Args...) & noexcept -> R> : function_arguments<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_arguments<auto (Args...) && noexcept -> R> : function_arguments<auto (Args...) -> R>{};

template<typename F>
requires std::is_member_function_pointer_v<F>
struct member_function_pointer_arguments;

template<typename F>
requires std::is_member_function_pointer_v<F>
using member_function_pointer_arguments_t = typename member_function_pointer_arguments<F>::Arguments;

template<typename T, typename C>
struct member_function_pointer_arguments<T C::*> : function_arguments<T>{};
//function_arguments

//function_return_type
template<typename F>
requires std::is_function_v<F>
struct function_return_type;

template<typename F>
requires std::is_function_v<F>
using function_return_type_t = typename function_return_type<F>::type;

template<typename R, typename ...Args>
struct function_return_type<auto (Args...) -> R> {using type = R;};

template<typename R, typename ...Args>
struct function_return_type<auto (Args...) const -> R> : function_return_type<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_return_type<auto (Args...) volatile -> R> : function_return_type<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_return_type<auto (Args...) noexcept -> R> : function_return_type<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_return_type<auto (Args...) & -> R> : function_return_type<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_return_type<auto (Args...) && -> R> : function_return_type<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_return_type<auto (Args...) const volatile -> R> : function_return_type<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_return_type<auto (Args...) const noexcept -> R> : function_return_type<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_return_type<auto (Args...) const & -> R> : function_return_type<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_return_type<auto (Args...) const && -> R> : function_return_type<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_return_type<auto (Args...) const volatile noexcept -> R> : function_return_type<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_return_type<auto (Args...) const volatile & -> R> : function_return_type<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_return_type<auto (Args...) const volatile && -> R> : function_return_type<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_return_type<auto (Args...) const & noexcept -> R> : function_return_type<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_return_type<auto (Args...) const && noexcept -> R> : function_return_type<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_return_type<auto (Args...) const volatile & noexcept -> R> : function_return_type<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_return_type<auto (Args...) const volatile && noexcept -> R> : function_return_type<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_return_type<auto (Args...) volatile noexcept -> R> : function_return_type<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_return_type<auto (Args...) volatile & -> R> : function_return_type<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_return_type<auto (Args...) volatile && -> R> : function_return_type<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_return_type<auto (Args...) volatile & noexcept -> R> : function_return_type<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_return_type<auto (Args...) volatile && noexcept -> R> : function_return_type<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_return_type<auto (Args...) & noexcept -> R> : function_return_type<auto (Args...) -> R>{};

template<typename R, typename ...Args>
struct function_return_type<auto (Args...) && noexcept -> R> : function_return_type<auto (Args...) -> R>{};

template<typename F>
requires std::is_member_function_pointer_v<F>
struct member_function_pointer_return_type;

template<typename F>
requires std::is_member_function_pointer_v<F>
using member_function_pointer_return_type_t = typename member_function_pointer_return_type<F>::type;

template<typename T, typename C>
struct member_function_pointer_return_type<T C::*> : function_return_type<T>{};
//function_return_type

//member_function_pointer_class
template<typename F>
struct member_function_pointer_class;

template<typename F>
using member_function_pointer_class_t = typename member_function_pointer_class<F>::type;

template<typename T, typename C>
struct member_function_pointer_class<T C::*> {using type = C;};
//member_function_pointer_class


