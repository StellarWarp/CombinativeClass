
#include "sample_type_info.h"
#include "combinative.h"

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
        auto Auto_AB_noC(this auto &&self) { return self.a + self.b; }
    };

    struct Methods2 : impl_for<FragmentA, FragmentC>
    {
        auto Auto_AC(this auto &&self) { return self.a + self.c; }
    };

    struct Methods3 : impl_for<FragmentB, FragmentC>
    {
        auto Auto_BC(this auto &&self) { return self.b + self.c; }
    };

    struct SingleFragmentAccess : impl_for<FragmentC>::exclude<FragmentA, FragmentB>
    {
        auto TemplateCast_C_noAB(this auto &&self) { return self.as<FragmentC>().c; }
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

        auto Auto_ABC(this auto &&self)
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
    };

    struct PropertiesConflict : impl_for<FragmentC, FragmentD>
    {
        auto EmbeddedTupleCaster_CD(this access_list self)
        {
            auto [fc, fd] = self.cref();
            return fc.c + fd.c;
        }
        auto StaticCast_CD(this auto &&self)
        {
            auto &x = static_cast<FragmentC &>(self).c; // u may define a marco to simplify this cast
            auto &y = static_cast<FragmentD &>(self).c;
            return x + y;
        }
        auto Caster_CD(this auto &&self)
        {
            auto &x = caster<FragmentC>(self).cref().c;
            auto &y = caster<FragmentD>(self).cref().c;
            return x + y;
        }
        auto TemplateCast_CD(this auto &&self)
        {
            auto &x = self.as<FragmentC>().c; // this is unfriendly to properties tipping
            auto &y = self.as<FragmentD>().c;
            return x + y;
        }
    };

    using FuncSet1 = function_set<Methods1, Methods2, Methods3,
                                  SingleFragmentAccess,
                                  MultiFragmentAccess,
                                  PropertiesConflict>;

    struct CompareTag {};

    struct MethodsComp1 : impl_for<FragmentA>::exclude<FragmentB>::tag<CompareTag>
    {
        auto Comparable(this auto &&self) { return self.a; }
    };

    struct MethodsComp2 : impl_for<FragmentB>::exclude<FragmentA>::tag<CompareTag>
    {
        auto Comparable(this auto &&self) { return self.b; }
    };

    struct MethodsComp3 : impl_for<FragmentA, FragmentB>::tag<CompareTag>
    {
        auto Comparable(this auto &&self) { return self.a * self.b; }
    };

    using FuncSet2 = function_set<MethodsComp1, MethodsComp2, MethodsComp3>;

    auto operator<=>(
        std::derived_from<CompareTag> auto &a,
        std::derived_from<CompareTag> auto &b)
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
    o3.Auto_AC();
    o3.Auto_BC();
    o3.Auto_ABC();
    o3.EmbeddedTupleCaster_ABC(); // intellisense as some problem in tipping this
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