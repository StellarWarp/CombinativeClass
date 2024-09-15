<a href="https://github.com/StellarWarp/CombinativeClass/actions/workflows/cmake-multi-platform.yml" target="_blank">![Multi-Platform](https://github.com/StellarWarp/CombinativeClass/actions/workflows/cmake-multi-platform.yml/badge.svg)</a>


# Combinative Class

'combinative' is a header-only library that provides an improved Curiously Recurring Template Pattern (CRTP) for C++23.

This utility enables the combination of data and the implementation of methods based on these combinations, offering a clean and powerful approach to multiple inheritance without relying on virtual bases.

## Quick Sample

```cpp
#include "combinative.h"

using namespace combinative;

struct A{ int a{}; };
struct B{ int b{}; };
struct C{ int c{}; };
struct D{ int d{}; };

struct MethodA : impl_for<A>
{
    int FuncA(this auto&& self,...)
    {
        return self.a;
    }
};

struct MethodC : impl_for<C>
{
    int FuncC(this access_list self,...)
    {
        return self.cref().c;//tip friendly
    }
};

struct MethodAB : impl_for<A,B>
{
    int FuncAB(this access_list self,...)
    {
        auto [fa,fb] = self.cref();
        return fa.a + fb.b;
    }
};

using MyFuncSet = function_set<MethodA, MethodC, MethodAB>;

/****************************/
/*                          */
/*             A            */
/*            / \           */
/*           /   \          */
/*          AB   AC         */
/*           \   /          */
/*            \ /           */
/*            ABC           */
/*                          */
/****************************/
struct ObjectA : combine<MyFuncSet, A>
{
    ObjectA(int a_) { a = a_; }
};
struct ObjectAB : combine<ObjectA, B>
{
    ObjectAB(int a_,int b_) { a = a_; b= b_;}
};
struct ObjectAC : combine<ObjectA, C>
{
    ObjectAC(int a_,int c_) { a = a_; c= c_;}
};
struct ObjectABC : combine<ObjectAB, ObjectAC>
{
    ObjectABC(int a_,int b_,int c_) { a = a_; b= b_; c= c_; }
};

void quick_sample()
{
    ObjectA o1(1);
    o1.FuncA();
    ObjectAB o2(1,2);
    o2.FuncA();
    o2.FuncAB();
    ObjectAC o3(1,3);
    o3.FuncA();
    o3.FuncC();
    ObjectABC o4(1,2,3);
    o4.FuncA();
    o4.FuncC();
    o4.FuncAB();
};
```

## Purpose

As the name suggest, it's for combining data and method to form different classes.

in the `type_info` sample , there is a `type_info` struct which contains the following fields

```cpp
struct type_info
{
	size_t size;
	copy_constructor_ptr_t copy_constructor;
	move_constructor_ptr_t move_constructor;
	destructor_ptr_t destructor;
	uint64_t hash;
	const char* name;
};
```

but the `type_info` is not allow to move and copy.

thus a `type_index` is act as a warpper of `type_info` to referencing the `type_info` and provide some info access methods

```cpp
class type_index
{
	const type_info* m_info;
public:
	type_index(const type_info& info) : m_info(&info) {}
	/* other methods to access info*/
};
```

this casue a problem that if one want to access any info of a type, the access action requires an indirection

you may wish that the hot properties can be inlinely cache in the `type_index`, in some cases which partial of a type's info is frequently visited, 

```cpp
class type_index_cache_hash
{
	const type_info* m_info;
	size_t m_hash;
};

class type_index_cache_destructor
{
	const type_info* m_info;
	size_t m_size;
	destructor_ptr_t m_destructor;
};
//...and more
```
or it may only need to store one or two properties of the type
```cpp
class type_size_hash
{
	size_t m_size;
	size_t m_hash;
};
//...and more
```

for each version adding to the project
- all method related to the changing properties need to be rewrite
- convertion between each version need to be add
- binary operator between each version need to be add
  
those issues are making it hard to maintain

the solution see `sample_type_info.h`. both CRTP implementation and Combinative Class implementation are provided

## Features

The Combinative Class basically an extension of CRTP with following feature
- combination :
  - properties are divided into fragments
  - combinative class is a combination of fragments and it's matched method
  - combinative class can be the combination of combinative classes
- access control : 
  - fragment visibility defualt to be protected
  - combinative classes combination is default to be protected
  - method struct inheritance is public（may support function set access control in the future）
  - visibility can be explicitly control by warpping type in template `pub` `prot` and `priv`
  - friend declaration for CRTP parent is implied
- method visibility friendly :
  - methods that don't match it's requirement won't appear in the final class
- macro free :
  - zero exposed macro
- intellisense friendly :
  - VS intellisense and clangd can infer the member and method of the combinative class
  

## Feature Sample

```cpp
﻿
#include "sample_type_info.h"
#include "../src/combinative.h"
#include <iostream>

namespace sample
{

	using namespace combinative;

	struct FragmentA
	{
		int32_t a{};
	};
	struct FragmentB
	{
		int32_t b{};
	};
	struct FragmentC
	{
		int32_t c{};
	};
	struct FragmentD
	{
		int32_t c{};
	};

	struct Methods1 : impl_for<FragmentA, FragmentB>::exclude<FragmentC>
	{
		auto Auto_AB_noC(this auto&& self) { return self.a + self.b; }
	};

	struct Methods2 : impl_for<FragmentA, FragmentC>
	{
		auto Auto_AC(this auto&& self) { return self.a + self.c; }
	};

	struct Methods3 : impl_for<FragmentB, FragmentC>
	{
		auto Auto_BC(this auto&& self) { return self.b + self.c; }
	};

	struct SingleFragmentAccess : impl_for<FragmentC>::exclude<FragmentA, FragmentB>
	{
		auto TemplateCast_C_noAB(this auto&& self) { return self.template as<FragmentC>().c; }
		auto EmbeddedCaster_C_noAB(this access_list self) { return self.cref().c; }
	};

	struct MultiFragmentAccess : impl_for<FragmentA, FragmentB, FragmentC>
	{
		auto Setter_ABC(this access_list self, int32_t a, int32_t b, int32_t c)
		{
			auto [fa, fb, fc] = self.ref();
			fa.a = a;
			fb.b = b;
			fc.c = c;
		}

		auto Auto_ABC(this auto&& self)
		{
			return self.a + self.b + self.c;
		}

		auto EmbeddedTupleCaster_ABC(this access_list self)
		{
			auto [fa, fb, fc] = self.cref();
			return fa.a + fb.b + fc.c;
		}

		auto FragmentCopy_ABC(this access_list self)
		{
			auto [fa, fb, fc] = self.val();
			return std::make_tuple(fa.a, fb.b, fc.c);
		}

        auto EmbeddedTupleCasterCustomAccess_ABC(this access_list self)
        {
            auto [fc, fa, fb] = self.get<FragmentC, FragmentA&, const FragmentB&>();
            return fa.a + fb.b + fc.c;
        }
	};

	struct NamingConflict : impl_for<FragmentC, FragmentD>
	{
        // this is the advised way for access data from multiple fragments
		auto EmbeddedTupleCaster_CD(this access_list self)
		{
			auto [fc, fd] = self.cref();
			return fc.c + fd.c;
		}
		auto StaticCast_CD(this auto&& self)
		{
			auto& x = static_cast<FragmentC&>(self).c; // you may define a marco to simplify this cast
			auto& y = static_cast<FragmentD&>(self).c;
			return x + y;
		}
		auto Caster_CD(this auto&& self)
		{
			auto& x = caster<FragmentC>(self).cref().c;
			auto& y = caster<FragmentD>(self).cref().c;
			return x + y;
		}
		auto TemplateCast_CD(this auto&& self)
		{
			auto& x = self.as<FragmentC>().c; //note that this is unfriendly to lsp
			auto& y = self.as<FragmentD>().c;
			return x + y;
		}
	};

#ifndef __INTELLISENSE__ 
	// note that custom_cond can't depend on condition that needs a custom condition to form
    struct CustomCondition : impl_for<>::custom_cond<[](auto t)requires requires{ &decltype(t)::Setter_ABC; }{}>
    {
        auto ABC_SetDefault(this auto&& self) { return self.Setter_ABC(0,0,0); }
    };
#else
	// VS Intellisense has same problem in lambda with concept constraint
	// this is the alternative form for custom condition
	struct CustomCondition : impl_for<>
	{
		template <typename Self>
		using _custom_cond_ = std::bool_constant<requires{ &Self::Setter_ABC; }>;
		auto ABC_SetDefault(this auto&& self) { return self.Setter_ABC(0, 0, 0); }
	};
#endif

	using FuncSet1 = function_set<Methods1, Methods2, Methods3,
		SingleFragmentAccess,
		MultiFragmentAccess,
		NamingConflict,
        CustomCondition>;

	struct CompareTag {};

	struct MethodsComp1 : impl_for<FragmentA>::exclude<FragmentB>::tag<CompareTag>
	{
		auto Comparable(this auto&& self) { return self.a; }
	};

	struct MethodsComp2 : impl_for<FragmentB>::exclude<FragmentA>::tag<CompareTag>
	{
		auto Comparable(this auto&& self) { return self.b; }
	};

	struct MethodsComp3 : impl_for<FragmentA, FragmentB>::tag<CompareTag>
	{
		auto Comparable(this auto&& self) { return self.a * self.b; }
	};

	using FuncSet2 = function_set<MethodsComp1, MethodsComp2, MethodsComp3>;

    template<typename T1, typename T2>
    requires std::derived_from<std::decay_t<T1>, CompareTag> &&
             std::derived_from<std::decay_t<T2>, CompareTag>
	auto operator<=>(T1&& a, T2&& b)
	{
		return a.Comparable() <=> b.Comparable();
	}

	using FuncSetFinal = function_set<FuncSet1, FuncSet2>;

	struct ObjectAB : combine<FuncSetFinal, pub<FragmentA>, FragmentB>
	{
	};
	static_assert(sizeof(ObjectAB) == 2 * sizeof(int32_t));

	struct ObjectAC : combine<FuncSetFinal, FragmentA, FragmentC>
	{
	};
	static_assert(sizeof(ObjectAC) == 2 * sizeof(int32_t));

	struct ObjectABC : combine<pub<ObjectAB>, ObjectAC>
	{
	};
	static_assert(sizeof(ObjectABC) == 3 * sizeof(int32_t));

	struct ObjectABC_ : combine<ObjectABC>::visibility_override<priv<FragmentA>>
	{
	};
	static_assert(sizeof(ObjectABC_) == sizeof(ObjectABC));

	struct ObjectCD : combine<ObjectABC, FragmentD>::remove<FragmentA, FragmentB>
	{
	};
	static_assert(sizeof(ObjectCD) == 2 * sizeof(int32_t));

	using is_a_accessible = decltype([](auto t) requires requires { t.a; } {});
	static_assert(std::invocable<is_a_accessible, ObjectABC>);
	static_assert(!std::invocable<is_a_accessible, ObjectABC_>);
}

using namespace sample;


int main()
{

	ObjectAB o1;
	o1.a = 1;
	o1.Auto_AB_noC();

	ObjectAC o2;
	o2.Auto_AC();

	ObjectABC o3;
	o3.Setter_ABC(1, 2, 3);
    o3.ABC_SetDefault();
	o3.Auto_AC();
	o3.Auto_BC();
	o3.Auto_ABC();
	o3.EmbeddedTupleCaster_ABC(); // intellisense has some problem in tipping this, ok with clangd
    o3.EmbeddedTupleCasterCustomAccess_ABC();
	o3.FragmentCopy_ABC();

	ObjectCD o4;
	o4.TemplateCast_C_noAB();
	o4.EmbeddedCaster_C_noAB();
	o4.EmbeddedTupleCaster_CD();
	o4.StaticCast_CD();
	o4.Caster_CD();
	o4.TemplateCast_CD();

	o1 <=> o2;
	o2 <=> o3;
	o1 <=> o3;

#ifdef TYPE_INFO_SAMPLE
	generic::sample_use();
#endif // TYPE_INFO_SAMPLE
}
```

a more realistic sample is provide in `sample_type_info.h`

## Polymorphism

The parent class and subclass of the Combinative Class do not have any inheritance relationship, which means they cannot be converted to each other. eg. `ObjectABC` is not convertable to `ObjectAC`. 

The polymorphism of Combinative Class is obtained via  *[Duck Typing](https://en.wikipedia.org/wiki/Duck_typing)*.

For **compile-time** polymorphism. Passing Combinative Class as a templated argument.

```cpp
auto operator<=>(
	std::derived_from<CompareTag> auto& a,
	std::derived_from<CompareTag> auto& b)
{
	return a.Comparable() <=> b.Comparable();
}
```

For **runtime** polymorphism. Type-erasure is required.

following lib can be used to implement the runtime type-erasure.

* [boost-ext.TE](https://github.com/boost-ext/te)
* [Boost.TypeErasure](https://www.boost.org/doc/libs/1_66_0/doc/html/boost_typeerasure.html)
* [Folly.Poly](https://github.com/facebook/folly/blob/master/folly/docs/Poly.md)
* [Proxy](https://github.com/microsoft/proxy)
