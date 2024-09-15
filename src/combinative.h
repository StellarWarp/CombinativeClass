#pragma once
#include "type_list.h"



#ifdef _MSC_VER
#define MSC_EBO __declspec(empty_bases)
#else
#define MSC_EBO
#endif

namespace combinative
{
	template<typename... T>
	class caster : std::tuple<T&...>
	{
	public:
        //the as<T> is call to be lsp friendly
		template<typename U>
		caster(U& self) : std::tuple<T&...>(self.template as<T>()...) {}
		std::tuple<T&...>& ref() { return *this; }
		std::tuple<const T&...> cref() {
			return[this]<size_t... I>(std::index_sequence<I...>) {
				return std::tuple<const T&...>{ std::get<I>(ref())... };
			}(std::make_index_sequence<sizeof...(T)>());
		}
		std::tuple<T...> val() {
			return[this]<size_t... I>(std::index_sequence<I...>) {
				return std::make_tuple(std::get<I>(ref())...);
			}(std::make_index_sequence<sizeof...(T)>());
		}
        template<typename... U>
        std::tuple<U...> get()
        {
            return std::tuple<U...>{ [this]() -> U{
                using type = std::decay_t<U>;
                constexpr size_t index = detail::type_list<T...>::template index_of<type>;
                return std::get<index>(ref());
            }()... };
        }
        template<size_t I>
		decltype(auto) get() { return std::get<I>(ref()); }
	};
	template<typename T>
	class caster<T>
	{
		T& ref_;
	public:
		template<typename U>
		caster(U& self) : ref_{ self.template as<T>() } {}
		T& ref() { return ref_; }
		const T& cref() { return ref_; }
		T val() { return ref_; }
	};

}

//namespace std {
//	template <typename... T>
//	struct tuple_size<combinative::caster<T...>> : std::integral_constant<std::size_t, sizeof...(T)> {};
//
//	template <std::size_t Index, typename... T>
//	struct tuple_element<Index, combinative::caster<T...>> {
//		using type = typename std::tuple_element<Index, std::tuple<T&...>>::type;
//	};
//}

namespace combinative
{
	template<typename T>
	struct pub;
	template<typename T>
	struct prot;
	template<typename T>
	struct priv;

	namespace detail
	{
		template<typename... T>
		struct MSC_EBO MultiInherit : T... {};
		template<>
		struct MultiInherit<> {};
		template<typename... T>
		struct MSC_EBO MultiInherit<type_list<T...>> : T... {};


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
			using type = typename UnWarpControlSymbolsImpl<
				type_list<Unsolve...>,
				type_list<Pub...>,
				type_list<Prot..., U>,
				type_list<Priv...>>::type;
		};
		template<typename U, typename... Unsolve,
			typename... Pub, typename... Prot, typename... Priv>
		struct UnWarpControlSymbolsImpl<
			type_list<prot<U>, Unsolve...>,
			type_list<Pub...>,
			type_list<Prot...>,
			type_list<Priv...>>
		{
			using type = typename UnWarpControlSymbolsImpl<
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
			using type = typename UnWarpControlSymbolsImpl<
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
			using type = typename UnWarpControlSymbolsImpl<
				type_list<Unsolve...>,
				type_list<Pub...>,
				type_list<Prot...>,
				type_list<Priv..., U>>::type;
		};
		template<typename... T>
		using UnWarpControlSymbols = typename UnWarpControlSymbolsImpl<
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
		struct function_set_base : MultiInherit<Method...>
		{
			using method_list = type_list<Method...>;
		};
		template <typename... Method>
		struct function_set_base<type_list< Method...>> : function_set_base< Method...> {};

		template <typename T, typename ValidList, typename Unverified>
		struct valid_method;
		template <typename T, typename... Valid>
		struct valid_method<T, type_list<Valid...>, type_list<>>
		{
			using method_list = type_list<Valid...>;
		};
		//template <typename T, typename... Valid, typename Method, typename... Methods> requires (!requires {Method::template _custom_cond_<T>; })
		//	struct valid_method<T, type_list<Valid...>, type_list<Method, Methods...>>
		//{
		//	constexpr static bool is_valid = true;
		//	using new_list = std::conditional_t<is_valid, type_list<Valid..., Method>, type_list<Valid...>>;
		//	using method_list = typename valid_method<T, new_list, type_list<Methods...>>::method_list;
		//};
		template <typename T, typename... Valid, typename Method, typename... Methods>
		struct valid_method<T, type_list<Valid...>, type_list<Method, Methods...>>
		{
			constexpr static bool is_valid = Method::template _custom_cond_<T>::value;
			using new_list = std::conditional_t<is_valid, type_list<Valid..., Method>, type_list<Valid...>>;
			using method_list = typename valid_method<T, new_list, type_list<Methods...>>::method_list;
		};

		template<typename T>
		struct tags_from_methods;
		template<typename... Method>
		struct tags_from_methods<type_list<Method...>>
		{
			using list = typename type_list<>::template cat<typename Method::method_tags...>;
		};
		template<typename MethodList>
		using MethodInheritList = type_list<>::template cat<MethodList, typename tags_from_methods<MethodList>::list>;

		// filter out methods that are not satisfied with custom condition
		template <typename Final,
			typename MethodList = typename valid_method<Final, type_list<>, typename Final::method_list>::method_list
		>
		struct ValidInterface : MultiInherit<MethodInheritList<MethodList>>
		{
			using method_list = MethodList;
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
			constexpr static bool is_valid = Method::template _frag_cond_<T>;
			using new_list = std::conditional_t<is_valid, type_list<Valid..., Method>, type_list<Valid...>>;
			using method_list = typename valid_method_frag<T, new_list, type_list<Methods...>>::method_list;
		};
		// filter out methods that are not satisfied with fragment requirement
		template <typename Final, typename FunctionSet,
			typename MethodList = typename valid_method_frag<Final, type_list<>, typename FunctionSet::method_list>::method_list
		>
		struct ValidInterfaceFrag : MultiInherit<MethodInheritList<MethodList>>
		{
			using method_list = MethodList;
		};

		//wait for C++26 variadic friend
#define _COMBINATIVE_MAKE_FRIENDS_16(n)\
friend typename methods::template get<(n)*16+0>;\
friend typename methods::template get<(n)*16+1>;\
friend typename methods::template get<(n)*16+2>;\
friend typename methods::template get<(n)*16+3>;\
friend typename methods::template get<(n)*16+4>;\
friend typename methods::template get<(n)*16+5>;\
friend typename methods::template get<(n)*16+6>;\
friend typename methods::template get<(n)*16+7>;\
friend typename methods::template get<(n)*16+8>;\
friend typename methods::template get<(n)*16+9>;\
friend typename methods::template get<(n)*16+10>;\
friend typename methods::template get<(n)*16+11>;\
friend typename methods::template get<(n)*16+12>;\
friend typename methods::template get<(n)*16+13>;\
friend typename methods::template get<(n)*16+14>;\
friend typename methods::template get<(n)*16+15>;\

#define _COMBINATIVE_MAKE_FRIENDS_32(n) _COMBINATIVE_MAKE_FRIENDS_16((n)*2) _COMBINATIVE_MAKE_FRIENDS_16((n)*2+1)
#define _COMBINATIVE_MAKE_FRIENDS_64(n) _COMBINATIVE_MAKE_FRIENDS_32((n)*2) _COMBINATIVE_MAKE_FRIENDS_32((n)*2+1)
#define _COMBINATIVE_MAKE_FRIENDS_128(n) _COMBINATIVE_MAKE_FRIENDS_64((n)*2) _COMBINATIVE_MAKE_FRIENDS_64((n)*2+1)

		template <typename FunctionSet, typename... T>
		struct MSC_EBO TestInherit :
			ValidInterfaceFrag<ControlledMultiInherit<T...>, FunctionSet>,
			ControlledMultiInherit<T...>
		{
            template <typename U>
            U& as() { return *static_cast<U*>(this); }
		private:

			using methods = TestInherit::method_list;
			_COMBINATIVE_MAKE_FRIENDS_128(0);

			template <typename... V>
			friend class caster;

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
		struct FunctionSetCat : function_set_base<typename UnwarpMethods<FunctionSet...>::method_list> {};

		template <typename FunctionSets, typename Fragments>
		struct InheritImpl;
		template <typename... FunctionSet, typename... Fragment>
		struct MSC_EBO InheritImpl<type_list<FunctionSet...>, type_list<Fragment...>> :
			ControlledMultiInherit<Fragment...>,
			ValidInterface<TestInherit<FunctionSetCat<FunctionSet...>, Fragment...>>
		{
			using fragment_list = type_list<Fragment...>;
			using function_sets = type_list<FunctionSet...>;

            template <typename T>
            T& as() { return *static_cast<T*>(this); }

        private:

            using methods = InheritImpl::method_list;
			_COMBINATIVE_MAKE_FRIENDS_128(0);

			template <typename... V>
			friend class caster;
        };

#undef _COMBINATIVE_MAKE_FRIENDS_16
#undef _COMBINATIVE_MAKE_FRIENDS_32
#undef _COMBINATIVE_MAKE_FRIENDS_64
#undef _COMBINATIVE_MAKE_FRIENDS_128





		struct visibility_info_base
		{
			enum visibility { PRIV, PROT, PUB };
		};
		template<typename T>
		struct visibility_info : visibility_info_base
		{
			using ident = T;
			using type = T;
			static constexpr visibility vib = PROT;
		};
		template<typename T>
		struct visibility_info<prot<T>> : visibility_info_base
		{
			using ident = T;
			using type = T;
			static constexpr visibility vib = PROT;
		};
		template<typename T>
		struct visibility_info<pub<T>> : visibility_info_base
		{
			using ident = pub<T>;
			using type = T;
			static constexpr visibility vib = PUB;
		};
		template<typename T>
		struct visibility_info<priv<T>> : visibility_info_base
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
			using type = type_list<Infos...>::template cast<cast_ident>;
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
			static constexpr int index = type_list<Infos...>::template find_first<is_same_fragment>;
			using new_list = fragment_replace<type_list<Infos...>, index, U>::type;
			using type = mega_frag_idents_impl<type_list<Unsolve...>, new_list>::type;
		};


		template <typename... Fragment>
		using mega_frag_idents = mega_frag_idents_impl<type_list<visibility_info<Fragment>...>, type_list<> >::type;


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

		template <typename T>
		concept has_fragment_list = requires { typename T::fragment_list; };
		template <typename T>
		concept has_method_list = requires { typename T::method_list; };
		template <typename T>
		concept has_function_sets = requires { typename T::function_sets; };

		template <typename T>
		concept is_combinative_object = has_fragment_list<typename visibility_info<T>::type>;


		template <typename T, typename... Ts, typename... FuncSet, typename... Fragment>
			requires (!is_combinative_object<T>) && (!has_method_list<T>) //normal fragment
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
		template <typename T, typename... Ts, typename... FuncSet, typename... Fragment>
			requires (!is_combinative_object<T>) && (has_method_list<T>) //func set
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

		template<typename T>
		struct object_fragments_convert : object_fragments_convert<prot<T>> {}; //defualt to be protected combine
		template<typename T>
		struct object_fragments_convert<pub<T>>
		{
			using fragments = T::fragment_list;
		};
		template<typename T>
		struct object_fragments_convert<prot<T>>
		{
			template<typename U>
			using convert = std::conditional_t< (visibility_info<U>::vib > visibility_info_base::PROT),
				prot<typename visibility_info<U>::type>, U >;
			using fragments = T::fragment_list::template cast< convert >;
		};
		template<typename T>
		struct object_fragments_convert<priv<T>>
		{
			template<typename U>
			using convert = std::conditional_t< (visibility_info<U>::vib > visibility_info_base::PRIV),
				priv<typename visibility_info<U>::type>, U >;

			using fragments = T::fragment_list::template cast< convert >;
		};

		template <typename T, typename... Ts, typename... FuncSet, typename... Fragment>
			requires (is_combinative_object<T>) //object
		struct inherit_infos_impl<
			type_list<T, Ts...>,
			type_list<FuncSet...>,
			type_list<Fragment...>>
		{
			using object_type = visibility_info<T>::type;
			using object_fragments = object_fragments_convert<T>::fragments;
			using new_fragment_list = type_list<Fragment...>::template cat<object_fragments>;
			using new_function_sets = type_list <FuncSet...>::template cat<typename object_type::function_sets>;
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
				using cast_to_type = visibility_info<V>::type;
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
				using cast_to_type = visibility_info<V>::type;
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
		struct tag_impl;

		template<typename Conditons, typename Tags>
		class impl_for;

		struct impl_for_helper
		{
			template<typename Self, typename T>
			static constexpr bool transform = std::is_base_of_v<T, Self>;
			template<typename Self, typename... T>
			static constexpr bool transform<Self, any_impl<T...>> = (std::is_base_of_v<T, Self> || ...);
			template<typename Self, typename... T>
			static constexpr bool transform<Self, exclude_impl<T...>> = (!std::is_base_of_v<T, Self> && ...);
			template<typename Self, typename... T>
			static constexpr bool transform<Self, depends_on_any_impl<T...>> = (T::template _frag_cond_<Self> || ...);
			template<typename Self, typename... T>
			static constexpr bool transform<Self, depends_on_all_impl<T...>> = (T::template _frag_cond_<Self> && ...);
			template<typename Self, typename Lambda>
			static constexpr bool transform<Self, custom_cond_impl<Lambda>> = true;
			template<typename Self, typename Tag>
			static constexpr bool transform<Self, tag_impl<Tag>> = true;

			template<typename Self, typename U>
			static constexpr bool custom_cond_tmp = true;
			template<typename Self, typename Lambda>
			static constexpr bool custom_cond_tmp<Self, custom_cond_impl<Lambda>> = std::invocable<Lambda, Self>;

            template<typename,typename>
            struct access_list_helper;
            template<typename T>
            static constexpr bool is_access_fragment = true;
            template<typename... T>
            static constexpr bool is_access_fragment<any_impl<T...>> = false;
            template<typename... T>
            static constexpr bool is_access_fragment<exclude_impl<T...>> = false;
            template<typename... T>
            static constexpr bool is_access_fragment<depends_on_any_impl<T...>> = false;
            template<typename... T>
            static constexpr bool is_access_fragment<depends_on_all_impl<T...>> = false;
            template<typename Lambda>
            static constexpr bool is_access_fragment<custom_cond_impl<Lambda>> = false;
            template<typename Tag>
            static constexpr bool is_access_fragment<tag_impl<Tag>> = false;
            template<typename... Access>
            struct access_list_helper<type_list<>,type_list<Access...>>{
                using list = type_list<Access...>;
            };
            template<typename U,typename... Unsolve,typename... Access> requires is_access_fragment<U>
            struct access_list_helper<type_list<U,Unsolve...>,type_list<Access...>>{
                 using list = access_list_helper<type_list<Unsolve...>,type_list<Access...,U>>::list;
            };
            template<typename U,typename... Unsolve,typename... Access> requires (!is_access_fragment<U>)
            struct access_list_helper<type_list<U,Unsolve...>,type_list<Access...>>{
                using list = access_list_helper<type_list<Unsolve...>,type_list<Access...>>::list;
            };
            template<typename T>
            struct access_list_impl;
            template<typename... T>
            struct access_list_impl<type_list<T...>>{
                using type = caster<T...>;
            };
            template<typename... T>
            using access_list = access_list_impl<typename access_list_helper<type_list<T...>,type_list<>>::list>::type;
		};


		template<typename... T, typename... Tag>
		class MSC_EBO impl_for<type_list<T...>, type_list<Tag...>>
		{
			template <typename Self> static constexpr bool _frag_cond_ = (impl_for_helper::transform<Self, T> && ...);

			template <typename Self> using _custom_cond_ = std::bool_constant<(impl_for_helper::custom_cond_tmp<Self, T> && ...)>;

			template <typename , typename , typename >
			friend struct valid_method;
			template <typename , typename , typename >
			friend struct valid_method_frag;

			template<typename U>
            using ExtendCondition = impl_for<type_list<T..., U>, type_list<Tag...>>;

        protected:
            using access_list = impl_for_helper::access_list<T...>;
        public:
			using method_tags = type_list<Tag...>;

			template<typename... U>
			using exclude = ExtendCondition<exclude_impl<U...>>;
			template<typename... U>
			using any = ExtendCondition< any_impl<U...>>;
			template<typename... U>
			using depends_on_any = ExtendCondition<depends_on_any_impl<U...>>;
			template<typename... U>
			using depends_on = ExtendCondition<depends_on_all_impl<U...>>;
			template<auto Lambda>
			using custom_cond = ExtendCondition<custom_cond_impl<decltype(Lambda)>>;
			template<typename... U>
			using tag = impl_for<type_list<T...>, type_list<Tag..., U...>>;
		};


		template <typename Unsolve, typename Methods>
		struct function_set_unwrap;
		template <typename... Method>
		struct function_set_unwrap<type_list<>, type_list<Method...>>
		{
			using type = function_set_base<Method...>;
		};
		template <typename U, typename... Unsolve, typename... Method> requires has_method_list<U>
		struct function_set_unwrap<type_list<U, Unsolve...>, type_list<Method...>>
		{
			using new_list = type_list<Method...>::template cat<typename U::method_list>;
			using type = function_set_unwrap<type_list<Unsolve...>, new_list>::type;
		};
		template <typename U, typename... Unsolve, typename... Method> requires (!has_method_list<U>)
			struct function_set_unwrap<type_list<U, Unsolve...>, type_list<Method...>>
		{
			using new_list = type_list<Method..., U>;
			using type = function_set_unwrap<type_list<Unsolve...>, new_list>::type;
		};

        template <typename... T>
        using function_set = detail::function_set_unwrap<type_list<T...>, type_list<>>::type;
	}


    using detail::function_set;

    template<typename... T>
    using impl_for = detail::impl_for<detail::type_list<T...>, detail::type_list<>>;

	template <typename... T>
	using combine = detail::combine<T...>;

}

#undef MSC_EBO