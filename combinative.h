#pragma once
#include "type_list.h"



#ifdef _MSC_VER
#define MSC_EBO __declspec(empty_bases)
#else
#define MSC_EBO
#endif

namespace combinative
{
	template<typename T>
	struct pub;
	template<typename T>
	struct priv;

	template <typename T>
	struct method;

	namespace detail
	{

		template<typename... T>
		struct MSC_EBO MultiInherit : T... {};
		template<>
		struct MultiInherit<> {};
		template<typename... T>
		struct MSC_EBO MultiInherit<type_list<T...>> : T... {};



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
		//template <typename T, typename... Valid, typename Method, typename... Methods> requires (!requires {Method::template __cond__<T>; })
		//	struct valid_method<T, type_list<Valid...>, type_list<Method, Methods...>>
		//{
		//	constexpr static bool is_valid = true;
		//	using new_list = std::conditional_t<is_valid, type_list<Valid..., Method>, type_list<Valid...>>;
		//	using method_list = typename valid_method<T, new_list, type_list<Methods...>>::method_list;
		//};
		template <typename T, typename... Valid, typename Method, typename... Methods>
		struct valid_method<T, type_list<Valid...>, type_list<Method, Methods...>>  
		{
			constexpr static bool is_valid = Method::template __cond__<T>::value;
			using new_list = std::conditional_t<is_valid, type_list<Valid..., Method>, type_list<Valid...>>;
			using method_list = typename valid_method<T, new_list, type_list<Methods...>>::method_list;
		};

		template <typename Final>
		struct ValidInterface : MultiInherit<typename valid_method<Final, type_list<>, typename Final::method_list>::method_list>
		{
			using method_list = valid_method<Final, type_list<>, typename Final::method_list>::method_list;
		};

		template <typename T, typename ValidList, typename Unverified>
		struct valid_method_frag;
		template <typename T, typename... Valid>
		struct valid_method_frag<T, type_list<Valid...>, type_list<>>
		{
			using method_list = type_list<Valid...>;
		};
		template <typename T, typename... Valid, typename Method, typename... Methods>
		struct valid_method_frag<T, type_list<Valid...>, type_list<Method, Methods...>>
		{
			constexpr static bool is_valid = Method::template __frag_cond__<T>;
			using new_list = std::conditional_t<is_valid, type_list<Valid..., Method>, type_list<Valid...>>;
			using method_list = typename valid_method_frag<T, new_list, type_list<Methods...>>::method_list;
		};
		template <typename Final, typename FunctionSet>
		struct ValidInterfaceFrag : MultiInherit<typename valid_method_frag<Final, type_list<>, typename FunctionSet::method_list>::method_list>
		{
			using method_list = valid_method_frag<Final, type_list<>, typename FunctionSet::method_list>::method_list;
		};


		template <typename FunctionSet, typename... T>
		struct MSC_EBO TestInherit : ValidInterfaceFrag<ControlledMultiInherit<T...>, FunctionSet>, ControlledMultiInherit<T...>
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
			ValidInterface<TestInherit<FunctionSetCat<FunctionSet...>, Fragment...>>
		{
			using fragment_list = type_list<Fragment...>;
			using function_sets = type_list<FunctionSet...>;
			using methods = FunctionSetCat<FunctionSet...>;

			template <typename T>
			friend struct method;
		};
	}



	template <typename... T>
	struct function_set : detail::function_set_impl<method<T>...> {};

	namespace detail {

		template <typename T>
		concept has_fragment_list = requires { typename T::fragment_list; };
		template <typename T>
		concept has_method_list = requires { typename T::method_list; };
		template <typename T>
		concept has_function_sets = requires { typename T::function_sets; };


		struct fragment_info_base
		{
			enum visibility { PRIV, PROT, PUB };
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
			struct is_same_fragment : std::is_same<typename Info::type, typename U::type> {};
			static constexpr size_t index = type_list<Infos...>::template find_first<is_same_fragment>;
			using new_list = fragment_replace<type_list<Infos...>, index, U>::type;
			using type = mega_frag_idents_impl<type_list<Unsolve...>, new_list>::type;
		};


		template <typename... Fragment>
		using mega_frag_idents = mega_frag_idents_impl<type_list<fragment_info<Fragment>...>, type_list<> >::type;


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
		template <typename T, typename... Ts, typename... FuncSet, typename... Fragment> requires (has_fragment_list<T>) && (has_function_sets<T>) //object
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
		private:
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
				using fragments = inherit_infos<T...>::fragments::template filter_without<in_remove_list>;
			};
			template<typename... U>
			struct visibility_override_helper
			{
				template<typename V>
				using cast_to_type = fragment_info<V>::type;
				using current_fragments = inherit_infos<T...>::fragments::template cast<cast_to_type>;
				static_assert((current_fragments::template contains<cast_to_type<U>> || ...),
					"visibility override can only be used with inheritance");
				using fragments = remove_helper<U...>::fragments::template cat<type_list<U...>>;
			};
		public:
			template<typename... U>
			using remove = InheritImpl<
				typename inherit_infos<T...>::function_sets,
				typename remove_helper<U...>::fragments>;
			template<typename... U>
			using visibility_override = InheritImpl<
				typename inherit_infos<T...>::function_sets,
				typename visibility_override_helper<U...>::fragments>;
		};


		template<typename... T>
		struct exclude_impl;
		template<typename... T>
		struct any_impl;
		template<typename... T>
		struct depends_on_any_impl;
		template<typename... T>
		struct depends_on_all_impl;
		template<typename Lambda>
		struct custom_cond_impl;

		template<typename... T>
		class impl_for
		{
			template<typename Self, typename T>
			static constexpr bool transform = std::is_base_of_v<T, Self>;
			template<typename Self, typename... T>
			static constexpr bool transform<Self, any_impl<T...>> = (std::is_base_of_v<T, Self> || ...);
			template<typename Self, typename... T>
			static constexpr bool transform<Self, exclude_impl<T...>> = (!std::is_base_of_v<T, Self> && ...);
			template<typename Self, typename... T>
			static constexpr bool transform<Self, depends_on_any_impl<T...>> = (method<T>::template __frag_cond__<Self> || ...);
			template<typename Self, typename... T>
			static constexpr bool transform<Self, depends_on_all_impl<T...>> = (method<T>::template __frag_cond__<Self> && ...);
			template<typename Self, typename Lambda>
			static constexpr bool transform<Self, custom_cond_impl<Lambda>> = true;

			template <typename Self> static constexpr bool __frag_cond__ = (transform<Self, T> && ...);

			template<typename Self, typename U>
			static constexpr bool custom_cond_tmp = true;
			template<typename Self, typename Lambda>
			static constexpr bool custom_cond_tmp<Self, custom_cond_impl<Lambda>> = std::invocable<Lambda, Self>;

			template <typename Self> using __cond__ = std::bool_constant<(custom_cond_tmp<Self, T> && ...)>;

			template <typename T, typename ValidList, typename Unverified>
			friend struct valid_method;
			template <typename T, typename ValidList, typename Unverified>
			friend struct valid_method_frag;

		public:
			template<typename... U>
			using exclude = impl_for<T..., exclude_impl<U...>>;
			template<typename... U>
			using any = impl_for<T..., any_impl<U...>>;
			template<typename... U>
			using depends_on_any = impl_for<T..., depends_on_any_impl<U...>>;
			template<typename... U>
			using depends_on = impl_for<T..., depends_on_all_impl<U...>>;
			template<typename Lambda>
			using custom_cond = impl_for<T..., custom_cond_impl<Lambda>>;
		};
	}


	template<typename... T>
	using impl_for = detail::impl_for<T...>;
	template <typename... T>
	using combine = detail::combine<T...>;


#define COMBINATIVE_METHOD_GROUP(name)template <> struct combinative::method<struct name>

}

#undef MSC_EBO