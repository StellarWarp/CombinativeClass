
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
		auto func_c(this auto&& self) { return static_cast<FragmentC&>(self).c; }
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
			auto [fa, fb, fc] = caster< FragmentA, FragmentB, FragmentC >(self).cref();
			return fa.a + fb.b + fc.c;
		}
		auto func_abc_copy(this auto&& self) {
			auto [fa, fb, fc] = caster< FragmentA, FragmentB, FragmentC >(self).val();//copy
			return std::forward_as_tuple(fa.a, fb.b, fc.c);
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
			auto& x = caster<FragmentC>(self).cref().c;
			auto& y = caster<FragmentD>(self).cref().c;
			return x + y;
		}
		auto func_cd_2(this auto&& self) {
			auto [fc, fd] = caster<FragmentC, FragmentD>(self).cref();
			return fc.c + fd.c;
		}
		//not recommended to use this is unfriendly to IDE
		auto func_cd_3(this auto&& self) {
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
	o3.func_abc_copy();

	Object4 o4;
	o4.func_c();
	o4.func_cd();

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
