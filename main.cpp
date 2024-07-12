
#include "combinative.h"
#include "sample_type_info.h"

using namespace combinative;

#define SELF_AS(fragment_name) static_cast<fragment_name&>(self)

struct FragmentA { int32_t a{}; };
struct FragmentB { int32_t b{}; };
struct FragmentC { int32_t c{}; };
struct FragmentD { int32_t c{}; };

struct Methods1 : impl_for<FragmentA, FragmentB>::exclude<FragmentC>{
	auto func_ab(this auto && self) { return self.a + self.b; }
	auto func_ab_1(this auto && self) { return self.a - self.b; }
};
struct Methods2 : impl_for<FragmentA, FragmentC>{
	auto func_ac(this auto && self) { return self.a + self.c; }
};
struct Methods3 : impl_for<FragmentB, FragmentC>{
	auto func_bc(this auto && self) { return self.b + self.c; }
};
struct Methods4 : impl_for<FragmentC>::exclude<FragmentA, FragmentB>{
	auto func_c(this auto && self) { return SELF_AS(FragmentC).c; }
};
struct Methods5 : impl_for<FragmentA, FragmentB, FragmentC>{
	auto initializer(this auto && self, int32_t a, int32_t b, int32_t c) {
		SELF_AS(FragmentA).a = a;
		SELF_AS(FragmentB).b = b;
		SELF_AS(FragmentC).c = c;
	}
};
struct Methods6 : impl_for<FragmentC, FragmentD> {
	auto func_cd(this auto&& self) { 
		auto& x = SELF_AS(FragmentC).c;
		auto& y = SELF_AS(FragmentD).c;
		return x + y;
	}
};

struct FuncSet1 : function_set<Methods1, Methods2, Methods3, Methods4, Methods5, Methods6> {};

struct Object1 : combine<FuncSet1, pub<FragmentA>, FragmentB> {};
static_assert(sizeof(Object1) == 2 * sizeof(int32_t));

struct Object2 : combine<FuncSet1, FragmentA, FragmentC> {};
static_assert(sizeof(Object2) == 2 * sizeof(int32_t));

struct Object3 : combine<Object1, Object2> {};
static_assert(sizeof(Object3) == 3 * sizeof(int32_t));

struct Object4 : combine<Object3, FragmentD>::remove<FragmentA, FragmentB> {};
static_assert(sizeof(Object4) == 2 * sizeof(int32_t));

struct Object5 : combine<Object3>::visibility_override<priv<FragmentA>> {};
static_assert(sizeof(Object5) == sizeof(Object3));


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
	o4.func_cd();

	Object5 o5;
	using is_a_accessible = decltype([](auto t) requires requires { t.a; } {});
	static_assert(std::invocable<is_a_accessible, Object3 > );
	static_assert(!std::invocable<is_a_accessible, Object5 > );
}


