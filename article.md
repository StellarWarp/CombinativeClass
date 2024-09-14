# Combinative Class ：使用 CRTP 自由组装类型

代码仓库

https://github.com/StellarWarp/CombinativeClass

最近看到了cpp23的deduce this，突然想到可以用做些魔法让CRTP来重新定义类

想起来不少继承问题产生的原因就是因为继承不够灵活

把继承转换为组合

把搅在一起的数据与逻辑分离


在 `type_info` 示例中，有一个 `type_info` 结构包含以下字段：

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

`type_info` 不允许移动和复制。所以 `type_index` 作为 `type_info` 的包装器来引用 `type_info` 并提供一些信息访问方法：

```cpp
class type_index
{
    const type_info* m_info;
public:
    type_index(const type_info& info) : m_info(&info) {}
    /* 其他方法来访问信息 */
};
```

这会带来一个问题，如果需要访问某个类型的任何信息，则访问操作需要间接引用。

您可能希望在 `type_index` 中内联缓存一些热属性，在某些情况下，部分类型的信息被频繁访问：

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
```

或者只需要存储类型的一个或两个属性：

```cpp
class type_size_hash
{
    size_t m_size;
    size_t m_hash;
};
```

对于每个版本添加到项目中：

- 所有与更改属性相关的方法需要重写
- 需要添加每个版本之间的转换
- 需要添加每个版本之间的二进制操作符

这些问题使得维护变得困难。

解决方案见 `sample_type_info.h`。提供了 CRTP 实现和 Combinative Class 实现。


先来看看实现了什么效果

## 快速示例

```cpp
#include "combinative.h"

using namespace combinative;

struct A{ int a; };
struct B{ int b; };
struct C{ int c; };
struct D{ int d; };

struct MethodA : impl_for<A>
{
    int FuncA(this auto&& self)
    {
        return self.a;
    }
};

struct MethodC : impl_for<C>
{
    int FuncC(this access_list self)
    {
        return self.cref().c;//tip friendly
    }
};

struct MethodAB : impl_for<A,B>
{
    int FuncAB(this access_list self)
    {
        auto [fa,fb] = self.cref();
        return fa.a + fb.b;
    }
};

using MyFuncSet = function_set<MethodA, MethodC, MethodAB>;

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

## 特性

Combinative Class 基本上是对 CRTP 的扩展，具有以下特性：

- 组合：
  - 属性被分成片段
  - 组合类是片段及其匹配方法的组合
  - 组合类可以是组合类的组合
- 访问控制：
  - 片段可见性默认是 protected
  - 组合类的组合默认是 protected
  - 方法结构的继承是 public（将来可能支持函数集的访问控制）
  - 可见性可以通过模板 `pub`、`prot` 和 `priv` 显式控制
  - 隐含声明 CRTP 父类为 friend
- 方法可见性友好：
  - 不符合其要求的方法不会出现在最终类中
- 无宏定义：
  - 零暴露的宏
- 智能提示友好：
  - 在 msvc 工具链上测试：智能提示可以推断出组合类的成员和方法

### 方法约束

### Embeded Caster

### 访问控制



## 背后的魔法

### EBO

### 访问控制

### 自动友元

### Fragment Caster



