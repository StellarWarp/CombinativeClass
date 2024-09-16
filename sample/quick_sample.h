#include "combinative.h"

using namespace combinative;

struct A{ int a{}; };
struct B{ int b{}; };
struct C{ int c{}; };

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