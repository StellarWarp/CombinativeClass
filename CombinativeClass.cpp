#include <type_traits>
#include <concepts>
#include <tuple>

template <typename... T> struct type_list {
	static constexpr size_t size = sizeof...(T);
	static constexpr bool is_empty = std::bool_constant<sizeof...(T) == 0>::value;

	using tuple_t = std::tuple<T...>;

private:
	template <size_t I, typename... Ts> struct get_helper;
	template <> struct get_helper<0> {
		using type = void;
	};
	template <typename U, typename... Us> struct get_helper<0, U, Us...> {
		using type = U;
	};
	template <size_t I, typename U, typename... Us>
	struct get_helper<I, U, Us...> {
		using type = typename get_helper<I - 1, Us...>::type;
	};

public:
	template <size_t I> using get = typename get_helper<I, T...>::type;

	using remove_ref = type_list<std::remove_reference_t<T>...>;
	using decay_type = typename type_list<std::decay_t<T>...>;

private:
	template <typename U>
	static constexpr bool contains_helper =
		std::bool_constant<(... || std::is_same_v<T, U>)>::value;

public:
	template <typename... U>
	static constexpr bool contains =
		std::bool_constant<(... && contains_helper<U>)>::value;

	template <typename Other>
	static constexpr bool contains_in = Other::template contains<T...>;

	template <typename Other>
	static constexpr bool contains_all = Other::template contains_in<type_list<T...>>;


private:
	template <typename... Us> struct is_unique_helper;
	template <> struct is_unique_helper<> {
		static constexpr bool value = true;
	};
	template <typename U, typename... Us> struct is_unique_helper<U, Us...> {
		static constexpr bool value = !type_list<Us...>::template contains<U> &&
			is_unique_helper<Us...>::value;
	};

	template <typename... Us> struct is_same_helper;
	template <> struct is_same_helper<> {
		static constexpr bool value = true;
	};
	template <typename U> struct is_same_helper<U> {
		static constexpr bool value = true;
	};
	template <typename U0, typename U1, typename... Us>
	struct is_same_helper<U0, U1, Us...> {
		static constexpr bool value = std::is_same_v<U0, U1>&&
			is_same_helper<U1, Us...>::value;
	};

public:
	static constexpr bool is_unique = is_unique_helper<T...>::value;
	static constexpr bool is_same = is_same_helper<T...>::value;

private:
	template <typename... Us> struct pop_front_helper;
	template <typename U, typename... Us> struct pop_front_helper<U, Us...> {
		using type = type_list<Us...>;
	};
	template <> struct pop_front_helper<> {
		using type = type_list<>;
	};

	template <typename L1, typename L2> struct pop_back_helper;
	template <typename... T1s>
	struct pop_back_helper<type_list<T1s...>, type_list<>> {
		using type = type_list<T1s...>;
	};
	template <typename... T1s, typename T2>
	struct pop_back_helper<type_list<T1s...>, type_list<T2>> {
		using type = type_list<T1s...>;
	};
	template <typename... T1s, typename T2, typename... T2s>
	struct pop_back_helper<type_list<T1s...>, type_list<T2, T2s...>> {
		using type = typename
			pop_back_helper<type_list<T1s..., T2>, type_list<T2s...>>::type;
	};

	template <typename... L> struct cat_helper;
	template <> struct cat_helper<> {
		using type = type_list<>;
	};
	template <typename... T1> struct cat_helper<type_list<T1...>> {
		using type = type_list<T1...>;
	};
	template <typename... T1, typename... T2>
	struct cat_helper<type_list<T1...>, type_list<T2...>> {
		using type = type_list<T1..., T2...>;
	};
	template <typename L, typename... Ls> struct cat_helper<L, Ls...> {
		using type = typename cat_helper<L, typename cat_helper<Ls...>::type>::type;
	};

	template <typename L1, typename L2, size_t Remove> struct remove_helper;
	/// invalid remove
	// template <typename... T1s, size_t Remove>
	// struct remove_helper<type_list<T1s...>, type_list<>, Remove> {
	//	using type = type_list<>;
	// };
	template <typename... T1s, typename T2, typename... T2s>
	struct remove_helper<type_list<T1s...>, type_list<T2, T2s...>, 0> {
		using type = type_list<T1s..., T2s...>;
	};
	template <typename... T1s, typename T2, typename... T2s, size_t Remove>
	struct remove_helper<type_list<T1s...>, type_list<T2, T2s...>, Remove> {
		using type = typename remove_helper<type_list<T1s..., T2>, type_list<T2s...>, Remove - 1>::type;
	};



	template <typename Unsolve, typename Solved, typename BlackList> struct t_remove_helper;
	template <typename BlackList, typename... Solved>
	struct t_remove_helper<type_list<>, type_list<Solved...>, BlackList>
	{
		using type = type_list<Solved...>;
	};
	template <typename... Remove, typename U, typename... Unsolve, typename... Solved>
	struct t_remove_helper<type_list<U, Unsolve...>, type_list<Solved...>, type_list<Remove...>> {
		static constexpr bool filtered = type_list<Remove...>::template contains<U>;
		using new_solve_list = std::conditional_t<filtered, type_list<Solved...>, type_list<Solved..., U>>;
		using type = t_remove_helper<type_list<Unsolve...>, new_solve_list, type_list<Remove...>>::type;
	};


public:
	template <typename... List>
	using cat = typename cat_helper<type_list<T...>, List...>::type;

	using pop_front = typename pop_front_helper<T...>::type;

	using pop_back = typename pop_back_helper<type_list<>, type_list<T...>>::type;

	template <typename... U> using push_back = type_list<T..., U...>;

	template <typename... U> using push_front = type_list<U..., T...>;

	template <size_t I>
	using remove = typename remove_helper<type_list<>, type_list<T...>, I>::type;
	template <typename... U>
	using erase = typename t_remove_helper<type_list<T...>, type_list<>, type_list<U...>>::type;


private:
	template <typename U> struct from_tuple_helper;
	template <typename... U> struct from_tuple_helper<std::tuple<U...>> {
		using type = type_list<U...>;
	};

public:
	template <typename... Tuple>
	using from_tuple = typename cat_helper<
		typename from_tuple_helper<std::remove_cvref_t<Tuple>>::type...
	>::type;

private:
	template <template <typename> typename Condition, bool TargetCondition,
		typename LL>
	struct filter_helper;
	template <template <typename> typename Condition, bool TargetCondition,
		typename... T1s>
	struct filter_helper<Condition, TargetCondition,
		type_list<type_list<T1s...>, type_list<>>> {
		using type = type_list<T1s...>;
	};

	template <bool C, typename L1, typename L2> struct filter_helper_branch;
	template <typename... T1s, typename T2, typename... T2s>
	struct filter_helper_branch<true, type_list<T1s...>, type_list<T2, T2s...>> {
		using type = type_list<type_list<T1s..., T2>, type_list<T2s...>>;
	};
	template <typename... T1s, typename T2, typename... T2s>
	struct filter_helper_branch<false, type_list<T1s...>, type_list<T2, T2s...>> {
		using type = type_list<type_list<T1s...>, type_list<T2s...>>;
	};

	template <template <typename> typename Condition, bool TargetCondition,
		typename... T1s, typename T2, typename... T2s>
	struct filter_helper<Condition, TargetCondition,
		type_list<type_list<T1s...>, type_list<T2, T2s...>>> {
		using filtered = type_list<T1s...>;
		using unfiltered = type_list<T2, T2s...>;
		using f = typename filter_helper_branch<Condition<T2>::value == TargetCondition,
			filtered, unfiltered>::type;
		using type = typename filter_helper<Condition, TargetCondition, f>::type;
	};

public:

	template <template <typename> typename Condition>
	using filter_with = typename
		filter_helper<Condition, true,
		type_list<type_list<>, type_list<T...>>>::type;
	template <template <typename> typename Condition>
	using filter_without = typename
		filter_helper<Condition, false,
		type_list<type_list<>, type_list<T...>>>::type;

	template <template <typename> typename Op>
	using modify_with = type_list<Op<T>...>;

private:
	template <typename... Us> struct unique_helper;
	template <> struct unique_helper<> {
		using type = type_list<>;
	};
	template <typename U, typename... Us> struct unique_helper<U, Us...> {
		using type = std::conditional_t<
			type_list<Us...>::template contains<U>,
			typename unique_helper<Us...>::type,
			typename cat_helper<type_list<U>,
			typename unique_helper<Us...>::type>::type>;
	};

public:
	using unique = typename unique_helper<T...>::type;

private:
	template <typename Target, int I, typename...> struct index_of_helper;
	template <typename Target, int I> struct index_of_helper<Target, I> {
		using type = std::integral_constant<int, -1>;
	};
	template <typename Target, int I, typename U, typename... Us>
	struct index_of_helper<Target, I, U, Us...> {
		using type = std::conditional_t<
			std::is_same_v<Target, U>,
			std::integral_constant<int, I>,
			typename index_of_helper<Target, I + 1, Us...>::type>;
	};

public:
	template <typename U>
	static constexpr int index_of = index_of_helper<U, 0, T...>::type::value;

public:

	template<template <typename> typename Caster>
	using cast = type_list<Caster<T>...>;


private:
	template <template <typename> typename Condition, int I, typename...> struct find_helper;
	template <template <typename> typename Condition, int I> struct find_helper<Condition, I> {
		using type = std::integral_constant<int, -1>;
	};
	template <template <typename> typename Condition, int I, typename U, typename... Us>
	struct find_helper<Condition, I, U, Us...> {
		using type = std::conditional_t<
			Condition<U>::value,
			std::integral_constant<int, I>,
			typename find_helper<Condition, I + 1, Us...>::type>;
	};

public:
	template<template <typename> typename Condition>
	static constexpr int find_first = find_helper<Condition, 0, T...>::type::value;


private:
	template <int I, typename V, typename Left, typename Right>
	struct replace_helper;
	template <typename V>
	struct replace_helper<0, V, type_list<>, type_list<>> {
		using type = type_list<>;
	};
	template <typename V, typename... Left, typename U, typename... Right>
	struct replace_helper<0, V, type_list<Left...>, type_list<U, Right...>> {
		using type = type_list< Left..., V, Right...>;
	};
	template <int I, typename V, typename... Left, typename U, typename... Right>
	struct replace_helper<I, V, type_list<Left...>, type_list<U, Right...>> {
		using type = replace_helper<I - 1, V, type_list<Left..., U>, type_list< Right...>>::type;
	};

public:
	template <int I, typename V>
	using replace_at = replace_helper<I, V, type_list<>, type_list<T...>>::type;

};
//template<typename T> struct is_double : std::is_same<T, double> {};
//auto x = type_list<int, float, double>::find_first<is_double>;
//type_list<int, float, double>::replace_at<1, int>;

#ifdef _MSC_VER
#define MSC_EBO __declspec(empty_bases)
#else
#define MSC_EBO
#endif // DEBUG



template<typename... T>
struct MSC_EBO MultiInherit : T... {};
template<>
struct MultiInherit<> {};
template<typename... T>
struct MSC_EBO MultiInherit<type_list<T...>> : T... {};

template<typename T>
struct pub;
template<typename T>
struct priv;

template<typename>
struct ProtectedMultiInherit;
template<typename... T>
struct MSC_EBO ProtectedMultiInherit<type_list<T...>> : protected T... {};

template<typename Pub, typename Prot, typename Priv>
struct UnWarpControlSymbolsImplRes;
template<typename... Pub, typename... Prot, typename... Priv>
struct UnWarpControlSymbolsImplRes<type_list<Pub...>, type_list<Prot...>, type_list<Priv...>>
{
	using pubs = type_list<Pub...>;
	using prots = type_list<Prot...>;
	using privs = type_list<Priv...>;
};
template<typename Unsolve, typename Pub, typename Prot, typename Priv>
struct UnWarpControlSymbolsImpl;
template<typename... Pub, typename... Prot, typename... Priv>
struct UnWarpControlSymbolsImpl<
	type_list<>,
	type_list<Pub...>,
	type_list<Prot...>,
	type_list<Priv...>>
{
	using type = UnWarpControlSymbolsImplRes<
		type_list<Pub...>,
		type_list<Prot...>,
		type_list<Priv...>>;
};
template<typename U, typename... Unsolve,
	typename... Pub, typename... Prot, typename... Priv>
struct UnWarpControlSymbolsImpl<
	type_list<U, Unsolve...>,
	type_list<Pub...>,
	type_list<Prot...>,
	type_list<Priv...>>
{
	using type = UnWarpControlSymbolsImpl<
		type_list<Unsolve...>,
		type_list<Pub...>,
		type_list<Prot..., U>,
		type_list<Priv...>>::type;
};
template<typename U, typename... Unsolve,
	typename... Pub, typename... Prot, typename... Priv>
struct UnWarpControlSymbolsImpl<
	type_list<pub<U>, Unsolve...>,
	type_list<Pub...>,
	type_list<Prot...>,
	type_list<Priv...>>
{
	using type = UnWarpControlSymbolsImpl<
		type_list<Unsolve...>,
		type_list<Pub..., U>,
		type_list<Prot...>,
		type_list<Priv...>>::type;
};
template<typename U, typename... Unsolve,
	typename... Pub, typename... Prot, typename... Priv>
struct UnWarpControlSymbolsImpl<
	type_list<priv<U>, Unsolve...>,
	type_list<Pub...>,
	type_list<Prot...>,
	type_list<Priv...>>
{
	using type = UnWarpControlSymbolsImpl<
		type_list<Unsolve...>,
		type_list<Pub...>,
		type_list<Prot...>,
		type_list<Priv..., U>>::type;
};
template<typename... T>
using UnWarpControlSymbols = UnWarpControlSymbolsImpl<
	type_list<T...>, type_list<>, type_list<>, type_list<>>::type;

template<typename, typename, typename>
struct ControlledMultiInheritImpl;
template<typename... Pub, typename... Prot, typename... Priv>
struct MSC_EBO ControlledMultiInheritImpl<
	type_list<Pub...>,
	type_list<Prot...>,
	type_list<Priv...>> : public Pub..., protected Prot..., private Priv... { };


template<typename... T>
struct MSC_EBO ControlledMultiInherit :
	ControlledMultiInheritImpl<
	typename UnWarpControlSymbols<T...>::pubs,
	typename UnWarpControlSymbols<T...>::prots,
	typename UnWarpControlSymbols<T...>::privs> { };
template<>
struct ControlledMultiInherit<> {};
template<typename... T>
struct MSC_EBO ControlledMultiInherit<type_list<T...>> :
	ControlledMultiInherit<T...> { };


template <typename... Method>
struct function_set_impl : MultiInherit<Method...>
{
	using method_list = type_list<Method...>;
};
template <typename... Method>
struct function_set_impl<type_list< Method...>> : function_set_impl< Method...> {};

template <typename T, typename ValidList, typename Unverified>
struct valid_method;

template <typename T, typename... Valid>
struct valid_method<T, type_list<Valid...>, type_list<>>
{
	using method_list = type_list<Valid...>;
};

template <typename T, typename... Valid, typename Method, typename... Methods>
struct valid_method<T, type_list<Valid...>, type_list<Method, Methods...>>
{
	constexpr static bool is_valid = Method::template __cond__<T>;
	using new_list = std::conditional_t<is_valid, type_list<Valid..., Method>, type_list<Valid...>>;
	using method_list = typename valid_method<T, new_list, type_list<Methods...>>::method_list;
};

template <typename Final, typename FunctionSet>
struct ValidInterface : MultiInherit<typename valid_method<Final, type_list<>, typename FunctionSet::method_list>::method_list>
{
	using method_list = typename FunctionSet::method_list;
};


template <typename FunctionSet, typename... T>
struct MSC_EBO TestInherit : FunctionSet, ControlledMultiInherit<T...>
{
	template <typename T>
	friend struct method;
};

template<typename FunctionSets, typename MethodList>
struct UnwarpMethodsImpl;
template<typename MethodList>
struct UnwarpMethodsImpl<type_list<>, MethodList>
{
	using method_list = MethodList;
};
template<typename... FunctionSet, typename... Methods>
struct UnwarpMethodsImpl<type_list<FunctionSet...>, type_list<Methods...>>
{
	using method_list = type_list<Methods...>::template cat<typename FunctionSet::method_list...>;
};

template<typename... FunctionSet>
struct UnwarpMethods
{
	using method_list = typename UnwarpMethodsImpl<type_list<FunctionSet...>, type_list<>>::method_list;
};

template<typename... FunctionSet>
struct FunctionSetCat : function_set_impl<typename UnwarpMethods<FunctionSet...>::method_list> {};

template <typename FunctionSets, typename Fragments>
struct InheritImpl;
template <typename... FunctionSet, typename... Fragment>
struct MSC_EBO InheritImpl<type_list<FunctionSet...>, type_list<Fragment...>> :
	ControlledMultiInherit<Fragment...>,
	ValidInterface<TestInherit<FunctionSetCat<FunctionSet...>, Fragment...>, FunctionSetCat<FunctionSet...>>
{
	using fragment_list = type_list<Fragment...>;
	using function_sets = type_list<FunctionSet...>;
	using methods = FunctionSetCat<FunctionSet...>;

	template <typename T>
	friend struct method;
};

template <typename T>
struct method;

template <typename... T>
struct function_set : function_set_impl<method<T>...> {};

template <typename T>
concept has_fragment_list = requires { typename T::fragment_list; };
template <typename T>
concept has_method_list = requires { typename T::method_list; };
template <typename T>
concept has_function_sets = requires { typename T::function_sets; };


struct fragment_info_base
{
	enum visibility { PRIV,PROT,PUB };
};
template<typename T>
struct fragment_info : fragment_info_base
{
	using ident = T;
	using type = T;
	static constexpr visibility vib = PROT;
};
template<typename T>
struct fragment_info<pub<T>> : fragment_info_base
{
	using ident = pub<T>;
	using type = T;
	static constexpr visibility vib = PUB;
};
template<typename T>
struct fragment_info<priv<T>> : fragment_info_base
{
	using ident = priv<T>;
	using type = T;
	static constexpr visibility vib = PRIV;
};

template<typename Unsolve, typename InfoList>
struct mega_frag_idents_impl;
template<typename... Infos>
struct mega_frag_idents_impl<type_list<>, type_list<Infos...>>
{
	template<typename Info>
	using cast_ident = Info::ident;
	using type = type_list<Infos...>::cast<cast_ident>;
};

template<typename List, int I, typename New>
struct fragment_replace
{
	static constexpr auto old_vib = List::template get<I>::vib;
	static constexpr auto new_vib = New::vib;
	static constexpr bool use_new = new_vib > old_vib;
	using type = std::conditional_t< use_new,
		typename List::template replace_at<I, New>,
		List>;
};
template<typename... T, typename New>
struct fragment_replace<type_list<T...>, -1, New> {
	using type = type_list<T..., New>;
};

template<typename U, typename...  Unsolve, typename... Infos>
struct mega_frag_idents_impl<type_list<U, Unsolve...>, type_list<Infos...>>
{
	template<typename Info>
	struct is_same_fragment : std::is_same<typename Info::type,typename U::type> {};
	static constexpr size_t index = type_list<Infos...>::template find_first<is_same_fragment>;
	using new_list = fragment_replace<type_list<Infos...>, index, U>::type;
	using type = mega_frag_idents_impl<type_list<Unsolve...>, new_list>::type;
};


template <typename... Fragment>
using mega_frag_idents = mega_frag_idents_impl<type_list<fragment_info<Fragment>...> , type_list<> >::type;


template <typename FuncSets, typename Fragments>
struct inherit_infos_impl_res;
template <typename... FuncSet, typename... Fragment>
struct inherit_infos_impl_res<type_list<FuncSet...>, type_list<Fragment...>>
{
	using function_sets = type_list<FuncSet...>;
	using fragments = mega_frag_idents<Fragment...>;
};

template < typename UnsolveTypes, typename FuncSets, typename Fragments>
struct inherit_infos_impl;
template <typename... FuncSet, typename... Fragment>
struct inherit_infos_impl<type_list<>, type_list<FuncSet...>, type_list<Fragment...>>
{
	using info = inherit_infos_impl_res<
		typename type_list<FuncSet...>::unique,
		type_list<Fragment...>
	>;
};
template <typename T, typename... Ts, typename... FuncSet, typename... Fragment> requires (!has_fragment_list<T>) && (!has_method_list<T>) //normal fragment
struct inherit_infos_impl<
	type_list<T, Ts...>,
	type_list<FuncSet...>,
	type_list<Fragment...>>
{
	using info = typename inherit_infos_impl<
		type_list<Ts...>,
		type_list<FuncSet...>,
		type_list<Fragment..., T>>::info;
};
template <typename T, typename... Ts, typename... FuncSet, typename... Fragment> requires (!has_fragment_list<T>) && (has_method_list<T>) //func set
struct inherit_infos_impl<
	type_list<T, Ts...>,
	type_list<FuncSet...>,
	type_list<Fragment...>>
{
	using info = typename inherit_infos_impl<
		type_list<Ts...>,
		type_list<FuncSet..., T>,
		type_list<Fragment...>>::info;
};
template <typename T, typename... Ts, typename... FuncSet, typename... Fragment> requires (has_fragment_list<T>) && (!has_function_sets<T>) //object
struct inherit_infos_impl<
	type_list<T, Ts...>,
	type_list<FuncSet...>,
	type_list<Fragment...>>
{
	using new_fragment_list = type_list<Fragment...>::template cat<typename T::fragment_list>;
	using info = typename inherit_infos_impl<
		type_list<Ts...>,
		type_list<FuncSet...>,
		new_fragment_list>::info;
};
template <typename T, typename... Ts, typename... FuncSet, typename... Fragment> requires (has_fragment_list<T>) && (has_function_sets<T>)
struct inherit_infos_impl<
	type_list<T, Ts...>,
	type_list<FuncSet...>,
	type_list<Fragment...>>
{
	using new_fragment_list = type_list<Fragment...>::template cat<typename T::fragment_list>;
	using new_function_sets = type_list <FuncSet...>::template cat<typename T::function_sets>;
	using info = typename inherit_infos_impl<
		type_list<Ts...>,
		new_function_sets,
		new_fragment_list>::info;
};
template <typename... T>
struct inherit_infos
{
	using info = inherit_infos_impl<type_list<T...>, type_list<>, type_list<>>::info;
	using function_sets = info::function_sets;
	using fragments = info::fragments;
};

template <typename... T>
struct combine : InheritImpl<
	typename inherit_infos<T...>::function_sets,
	typename inherit_infos<T...>::fragments> {

	template<typename... U>
	struct remove_helper
	{
		template<typename V>
		using cast_to_type = fragment_info<V>::type;
		using remove_list = type_list<U...>::template cast<cast_to_type>;
		template<typename V>
		struct in_remove_list
		{
			constexpr static bool value = remove_list::template contains<cast_to_type<V>>;
		};
		using filtered_fragments = inherit_infos<T...>::fragments::template filter_without<in_remove_list>;
	};

	template<typename... U>
	using remove = InheritImpl<
		typename inherit_infos<T...>::function_sets,
		typename remove_helper<U...>::filtered_fragments>;
};


template<typename... T>
struct exclude_impl;
template<typename... T>
struct any_impl;
template<typename... T>
class impl_for
{
	template<typename Self, typename T>
	static constexpr bool transform = std::is_base_of_v<T, Self>;
	template<typename Self, typename... T>
	static constexpr bool transform<Self, exclude_impl<T...>> = (!std::is_base_of_v<T,Self> && ...);
	template<typename Self, typename... T>
	static constexpr bool transform<Self, any_impl<T...>> = (std::is_base_of_v<T,Self> || ...);

	template <typename Self> static constexpr bool __cond__ = (transform<Self, T> && ...);
	template <typename T, typename ValidList, typename Unverified>
	friend struct valid_method;

public:
	template<typename... U>
	using exclude = impl_for<T..., exclude_impl<U...>>;
	template<typename... U>
	using any = impl_for<T..., any_impl<U...>>;
};

#define METHOD_GROUP(name)template <> struct method<struct name>


struct FragmentA { int32_t a{}; };
struct FragmentB { int32_t b{}; };
struct FragmentC { int32_t c{}; };
struct FragmentD { int32_t d{}; };

METHOD_GROUP(Methods1) : impl_for<FragmentA, FragmentB>::exclude<FragmentC> {
	auto func_ab(this auto && self) { return self.a + self.b; }
	auto func_ab_1(this auto && self) { return self.a - self.b; }
};
METHOD_GROUP(Methods2) : impl_for<FragmentA, FragmentC>{
	auto func_ac(this auto && self) { return self.a + self.c; }
};
METHOD_GROUP(Methods3) : impl_for<FragmentB, FragmentC>{
	auto func_bc(this auto && self) { return self.b + self.c; }
};
METHOD_GROUP(Methods4) : impl_for<FragmentC>::exclude<FragmentA, FragmentB>{
	auto func_c(this auto && self) { return self.c; }
};
METHOD_GROUP(Methods5) : impl_for<FragmentA, FragmentB, FragmentC>{
	auto initializer(this auto && self, int32_t a, int32_t b, int32_t c) {
		self.a = a;
		self.b = b;
		self.c = c;
	}
};
struct FuncSet1 : function_set<Methods1, Methods2, Methods3, Methods4, Methods5> {};

struct Object1 : combine<FuncSet1, pub<FragmentA>, FragmentB> {};
static_assert(sizeof(Object1) == 2 * sizeof(int32_t));

struct Object2 : combine<FuncSet1, FragmentA, FragmentC> {};
static_assert(sizeof(Object2) == 2 * sizeof(int32_t));

struct Object3 : combine<Object1, Object2> {};
static_assert(sizeof(Object3) == 3 * sizeof(int32_t));

struct Object4 : combine<Object3, priv<FragmentD>>::remove<FragmentA, FragmentB> {};
static_assert(sizeof(Object4) == 2 * sizeof(int32_t));

int main() {

	Object1 o1;
	o1.a = 1;
	o1.func_ab();
	o1.func_ab_1();

	Object2 o2;
	o2.func_ac();

	Object3 o3;

	o3.initializer(1, 2, 3);
	o3.func_ac();
	o3.func_bc();

	Object4 o4;
	o4.func_c();
}


