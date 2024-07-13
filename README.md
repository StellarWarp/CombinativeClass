# Combinative Class

A toy utilize C++23 feature for fun

this repos provides a template tool to combine data and implement method base on the combination.

making multi-inheritance free from the virtual bases

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
  - test on msvc tool chain : intellisense can infer the member and method of the combinative class
  

## Sample

```cpp
﻿
#include "sample_type_info.h"
#include "combinative.h"

namespace sample
{

	using namespace combinative;

	struct FragmentA { int32_t a{}; };
	struct FragmentB { int32_t b{}; };
	struct FragmentC { int32_t c{}; };
	struct FragmentD { int32_t c{}; };

	struct Methods1 : impl_for<FragmentA, FragmentB>::exclude<FragmentC> {
		auto func_ab(this auto&& self) { return self.a + self.b; }
		auto func_ab_1(this auto&& self) { return self.a - self.b; }
	};
	struct Methods2 : impl_for<FragmentA, FragmentC> {
		auto func_ac(this auto&& self) { return self.a + self.c; }
	};
	struct Methods3 : impl_for<FragmentB, FragmentC> {
		auto func_bc(this auto&& self) { return self.b + self.c; }
	};
	struct Methods4 : impl_for<FragmentC>::exclude<FragmentA, FragmentB> {
		auto func_c(this auto&& self) { return self.as<FragmentC>().c; }
		auto func_c_1(this caster<FragmentC> self) { return self.cref().c; }
	};

	struct Methods5 : impl_for<FragmentA, FragmentB, FragmentC> {
		auto initializer(this auto&& self, int32_t a, int32_t b, int32_t c) {
			auto [fa, fb, fc] = caster< FragmentA, FragmentB, FragmentC >(self);
			fa.a = a;
			fb.b = b;
			fc.c = c;
		}
		auto initializer_1(this auto&& self, int32_t a, int32_t b, int32_t c) {
			auto [fa, fb, fc] = caster< FragmentA, FragmentB, FragmentC >(self).ref();//same but explicit
			fa.a = a;
			fb.b = b;
			fc.c = c;
		}
		auto func_abc(this auto&& self) {
			return self.a + self.b + self.c;
		}
		//note that this cause a method tipping problem in intellisense 
		//as it didn't consider caster as a friend class
		auto func_abc_1(this caster<FragmentA, FragmentB, FragmentC> self) { //embedded caster
			auto [fa, fb, fc] = self.cref();
			return fa.a + fb.b + fc.c;
		}
		auto func_abc_copy(this caster< FragmentA, FragmentB, FragmentC > self) {
			auto [fa, fb, fc] = self.val();//copy
			return std::make_tuple(fa.a, fb.b, fc.c);
		}
	};
	struct Methods6 : impl_for<FragmentC, FragmentD> {

#define self_as(fragment_name) static_cast<fragment_name&>(self)

		auto func_cd(this auto&& self) {
			auto& x = self_as(FragmentC).c;
			auto& y = self_as(FragmentD).c;
			return x + y;
		}

#undef self_as
		auto func_cd_1(this auto&& self) {
            auto [fc, fd] = caster<FragmentC, FragmentD>(self).cref();
            return fc.c + fd.c;
		}
		auto func_cd_2(this caster<FragmentC, FragmentD> self) {
			auto [fc, fd] = self.cref();
			return fc.c + fd.c;
		}
		auto func_cd_3(this auto&& self) {
			auto& x = caster<FragmentC>(self).cref().c;
			auto& y = caster<FragmentD>(self).cref().c;
			return x + y;
		}
		//this is unfriendly to intellisense
		auto func_cd_4(this auto&& self) {
			auto& x = self.as<FragmentC>().c;
			auto& y = self.as<FragmentD>().c;
			return x + y;
		}
	};

	using FuncSet1 = function_set<Methods1, Methods2, Methods3, Methods4, Methods5, Methods6>;

	struct CompareTag {};
	struct MethodsComp1 : impl_for<FragmentA>::exclude<FragmentB>::tag<CompareTag> {
		auto comparable(this auto&& self) { return self.a; }
	};
	struct MethodsComp2 : impl_for<FragmentB>::exclude<FragmentA>::tag<CompareTag> {
		auto comparable(this auto&& self) { return self.b; }
	};
	struct MethodsComp3 : impl_for<FragmentA, FragmentB>::tag<CompareTag> {
		auto comparable(this auto&& self) { return self.a * self.b; }
	};

	using FuncSet2 = function_set<MethodsComp1, MethodsComp2, MethodsComp3>;

	auto operator<=> (
		std::derived_from<CompareTag> auto& a,
		std::derived_from<CompareTag> auto& b)
	{
		return a.comparable() <=> b.comparable();
	}

	using FuncSetFinal = function_set<FuncSet1, FuncSet2>;


	struct Object1 : combine<FuncSetFinal, pub<FragmentA>, FragmentB> {};
	static_assert(sizeof(Object1) == 2 * sizeof(int32_t));

	struct Object2 : combine<FuncSetFinal, FragmentA, FragmentC> {};
	static_assert(sizeof(Object2) == 2 * sizeof(int32_t));

	struct Object3 : combine<pub<Object1>, Object2> {};
	static_assert(sizeof(Object3) == 3 * sizeof(int32_t));

	struct Object4 : combine<Object3, FragmentD>::remove<FragmentA, FragmentB> {};
	static_assert(sizeof(Object4) == 2 * sizeof(int32_t));

	struct Object5 : combine<Object3>::visibility_override<priv<FragmentA>> {};
	static_assert(sizeof(Object5) == sizeof(Object3));

}

using namespace sample;

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
	o3.func_abc();
	o3.func_abc_1();
	o3.func_abc_copy();

	Object4 o4;
	o4.func_c();
	o4.func_cd();
	o4.func_cd_1();
	o4.func_cd_2();
    o4.func_cd_3();
    o4.func_c_1();
    o4.func_c_1();

	Object5 o5;
	using is_a_accessible = decltype([](auto t) requires requires { t.a; } {});
	static_assert(std::invocable<is_a_accessible, Object3 >);
	static_assert(!std::invocable<is_a_accessible, Object5 >);

	o1 <=> o2;
	o2 <=> o3;
	o1 <=> o3;

#ifdef TYPE_INFO_SAMPLE
	generic::sample_use();
#endif // TYPE_INFO_SAMPLE
}

```

a more realistic sample is provide in `sample_type_info.h`