
#include <ranges>
#include "combinative.h"

using namespace combinative;


namespace generic
{

	using default_constructor_ptr = void* (*)(void*);
	using copy_constructor_ptr_t = void* (*)(void*, const void*);
	using move_constructor_ptr_t = void* (*)(void*, void*);
	using destructor_ptr_t = void(*)(void*);

	template <typename T>
	constexpr void* default_constructor(void* addr)
	{
		return (T*)new(addr) T();
	}

	template <typename T>
	constexpr void* move_constructor(void* dest, void* src)
	{
		return (T*)new(dest) T(std::move(*(T*)src));
	}

	template <typename T>
	constexpr void* copy_constructor(void* dest, const void* src)
	{
		return (T*)new(dest) T(*(T*)src);
	}

	template <typename T>
	constexpr void destructor(void* addr)
	{
		((T*)addr)->~T();
	}

	template <typename T>
	constexpr auto nullable_move_constructor()
	{
		if constexpr (std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>)
			return move_constructor<T>;
		else
			return nullptr;
	}

	template <typename T>
	constexpr auto nullable_copy_constructor()
	{
		if constexpr (std::is_copy_constructible_v<T>)
			return copy_constructor<T>;
		else
			return nullptr;
	}

	template <typename T>
	constexpr auto nullable_destructor()
	{
		if constexpr (!std::is_trivially_destructible_v<T>)
			return destructor<T>;
		else
			return nullptr;
	}


	struct type_info
	{
		size_t size;
		copy_constructor_ptr_t copy_constructor;
		move_constructor_ptr_t move_constructor;
		destructor_ptr_t destructor;
		uint64_t hash;
		const char* name;

		type_info(size_t size,
			copy_constructor_ptr_t copy_constructor,
			move_constructor_ptr_t move_constructor,
			destructor_ptr_t destructor,
			size_t hash,
			const char* name)
			: size(size),
			copy_constructor(copy_constructor),
			move_constructor(move_constructor),
			destructor(destructor), hash(hash), name(name)
		{
		}

		type_info() = delete;
		type_info(const type_info&) = delete;
		type_info(type_info&&) = delete;
		type_info& operator=(const type_info&) = delete;

		template <typename T>
		static const type_info& of()
		{
			static_assert(std::is_same_v<T, std::decay_t<T>>, "T must be a decayed type");
			const static type_info info{
				std::is_empty_v<T> ? 0 : sizeof(T),
				nullable_copy_constructor<T>(),
				nullable_move_constructor<T>(),
				nullable_destructor<T>(),
				{},
				{}
			};
			return info;
		}
	};


}
#define USE_COMBINATIVE

#ifdef USE_COMBINATIVE

namespace generic
{
	namespace info_frag
	{
		struct index { const type_info* info_; };
		struct size_frag { size_t size_; };
		struct hash_frag { uint64_t hash_; };
		struct name_frag { const char* name_; };
		struct copy_constructor { copy_constructor_ptr_t copy_constructor_; };
		struct move_constructor { move_constructor_ptr_t move_constructor_; };
		struct destructor_frag { destructor_ptr_t destructor_; };
	}
}


#define MS COMBINATIVE_METHOD_GROUP
#define F generic::info_frag

MS(type_index_size) : impl_for<F::index>::exclude<F::size_frag>{
	size_t size(this auto && self) { return self.info_->size; }
};
MS(type_cache_size) : impl_for<F::size_frag>{
	size_t size(this auto && self) { return self.size_; }
};

MS(type_index_hash) : impl_for<F::index>::exclude<F::hash_frag>{
	uint64_t hash(this auto && self) { return self.info_->hash; }
};
MS(type_cache_hash) : impl_for<F::hash_frag>{
	uint64_t hash(this auto && self) { return self.hash_; }
};
MS(type_index_name) : impl_for<F::index>::exclude<F::name_frag>{
	const char* name(this auto && self) { return self.info_->name; }
};
MS(type_cache_name) : impl_for<F::name_frag>{
	const char* name(this auto && self) { return self.name_; }
};

MS(type_index_copy_constructor_ptr) : impl_for<F::index>::exclude<F::copy_constructor>{
protected:
	auto copy_constructor_ptr(this auto && self) { return self.info_->copy_constructor; }
};
MS(type_index_move_constructor_ptr) : impl_for<F::index>::exclude<F::move_constructor>{
protected:
	auto move_constructor_ptr(this auto && self) { return self.info_->move_constructor; }
};
MS(type_index_destructor_ptr) : impl_for<F::index>::exclude<F::destructor_frag>{
protected:
	auto destructor_ptr(this auto && self) { return self.info_->destructor; }
};
MS(type_cache_copy_constructor_ptr) : impl_for<F::copy_constructor>{
protected:
	auto copy_constructor_ptr(this auto && self) { return self.copy_constructor_; }
};
MS(type_cache_move_constructor_ptr) : impl_for<F::move_constructor>{
protected:
	auto move_constructor_ptr(this auto && self) { return self.move_constructor_; }
};
MS(type_cache_destructor_ptr) : impl_for<F::destructor_frag>{
protected:
	auto destructor_ptr(this auto && self) { return self.destructor_; }
};

MS(type_empty) : impl_for<>::any<F::index, F::size_frag>{
	bool is_empty(this auto && self) { return self.size() == 0; }
};

MS(type_copy_constructor) : impl_for<>/*::custom_cond <decltype([](auto t) requires requires { t.copy_constructor_ptr(); } {}) >*/
{
	template <typename T>
	using __cond__ = std::bool_constant < requires(T t) { t.copy_constructor_ptr(); } >;
	void* copy_constructor(this auto && self, void* dest, const void* src) { return self.copy_constructor_ptr()(dest, src); }
};
MS(type_move_constructor) : impl_for<>/*::custom_cond <decltype([](auto t) requires requires { t.move_constructor_ptr(); } {}) >*/
{
	template <typename T>
	using __cond__ = std::bool_constant < requires(T t) { t.move_constructor_ptr(); } > ;
	void* move_constructor(this auto && self, void* dest, void* src) { return self.move_constructor_ptr()(dest, src); }
	bool is_reallocable(this auto && self)
	{
		return self.move_constructor_ptr() == nullptr;
	}
};

MS(type_destructor) : impl_for<>
{
	template <typename T> 
	using __cond__ = std::bool_constant < requires(T t) { t.destructor_ptr(); t.size(); } > ;
	void destructor(this auto && self, void* dest) { self.destructor_ptr()(dest); }
	void destructor(this auto && self, void* addr, size_t count) {
		if (self.destructor_ptr() == nullptr)
			return;
		for (size_t i = 0; i < count; i++)
		{
			self.destructor_ptr()(addr);
			addr = (uint8_t*)addr + self.size();
		}
	}
	void destructor(this auto && self, void* begin, void* end)
	{
		if (self.destructor_ptr() == nullptr)
			return;
		for (; begin != end; (uint8_t*)begin + self.size())
		{
			self.destructor_ptr()(begin);
		}
	}
	bool is_trivially_destructible(this auto && self)
	{
		return self.destructor_ptr() == nullptr;
	}
};


#undef F
#undef MS

namespace generic
{
	using type_info_funcset = function_set<
		type_index_size, type_cache_size,
		type_index_hash, type_cache_hash,
		type_index_name, type_cache_name,
		type_index_copy_constructor_ptr, type_cache_copy_constructor_ptr,
		type_index_move_constructor_ptr, type_cache_move_constructor_ptr,
		type_index_destructor_ptr, type_cache_destructor_ptr,
		type_empty,
		type_copy_constructor,
		type_move_constructor,
		type_destructor
	>;

	auto operator<=> (auto&& a, auto&& b) requires (requires { a.hash(); b.hash(); })
	{
		return a.hash() <=> b.hash();
	}

	struct type_index : combine<info_frag::index, type_info_funcset> {
		type_index(const type_info& info) { info_ = &info; }
	};

	struct type_index_hash_only : combine<info_frag::hash_frag, type_info_funcset> {
		type_index_hash_only(const type_info& info) {
			hash_ = info.hash;
		}
	};

	struct type_index_hash_size_only : combine<info_frag::hash_frag, info_frag::size_frag, type_info_funcset> {
		type_index_hash_size_only(const type_info& info) {
			hash_ = info.hash;

		}
	};

	struct type_index_container : combine<type_index,
		info_frag::size_frag,
		info_frag::copy_constructor,
		info_frag::move_constructor,
		info_frag::destructor_frag>
	{
		type_index_container(const type_info& info) {
			info_ = &info;
			size_ = info.size;
			copy_constructor_ = info.copy_constructor;
			move_constructor_ = info.move_constructor;
			destructor_ = info.destructor;
		}
	};


	void sample_use()
	{
		generic::type_index idx1 = generic::type_info::of<int>();
		generic::type_index_container idx2 = generic::type_info::of<float>();
		idx1.size();
		idx1.copy_constructor(nullptr, nullptr);
		idx1 < idx2;


		type_index_hash_size_only idx3 = type_info::of<int>();
		type_index_container idx4 = type_info::of<float>();
		idx3.hash();
		idx3 > idx4;


		combine<info_frag::size_frag, info_frag::destructor_frag, type_info_funcset> idx5;
		idx5.destructor(nullptr);
	}

}

#else

namespace generic
{

	struct type_info_caching_interface
	{
		size_t size(this auto&& self) requires (requires { self.m_size; })
		{
			return self.m_size;
		}
		size_t size(this auto&& self) requires (!requires { self.m_size; }) && (requires { self.m_info; })
		{
			return self.m_info->size;
		}
		bool is_empty(this auto&& self) requires (requires { self.size(); }) { return self.size() == 0; }
		uint64_t hash(this auto&& self) requires (requires { self.m_hash; })
		{
			return self.m_hash;
		}
		uint64_t hash(this auto&& self) requires (!requires { self.m_hash; }) && (requires { self.m_info; })
		{
			return self.m_info->hash;
		}
		const char* name(this auto&& self) requires (requires { self.m_name; })
		{
			return self.m_name;
		}
		const char* name(this auto&& self) requires (!requires { self.m_name; }) && (requires { self.m_info; })
		{
			return self.m_info->name;
		}

	private:

		auto copy_constructor_ptr(this auto&& self) requires (requires { self.m_copy_constructor; })
		{
			return self.m_copy_constructor;
		}
		auto copy_constructor_ptr(this auto&& self) requires (!requires { self.m_copy_constructor; }) && (requires { self.m_info; })
		{
			return self.m_info->copy_constructor;
		}
		auto move_constructor_ptr(this auto&& self) requires (requires { self.m_move_constructor; })
		{
			return self.m_move_constructor;
		}
		auto move_constructor_ptr(this auto&& self) requires (!requires { self.m_move_constructor; }) && (requires { self.m_info; })
		{
			return self.m_info->move_constructor;
		}
		auto destructor_ptr(this auto&& self) requires (requires { self.m_destructor; })
		{
			return self.m_destructor;
		}
		auto destructor_ptr(this auto&& self) requires (!requires { self.m_destructor; }) && (requires { self.m_info; })
		{
			return self.m_info->destructor;
		}

	public:

		void* copy_constructor(this auto&& self, void* dest, const void* src) requires (requires { self.copy_constructor_ptr(); })
		{
			return self.copy_constructor_ptr()(dest, src);
		}
		void* move_constructor(this auto&& self, void* dest, void* src) requires (requires { self.move_constructor_ptr(); })
		{
			return self.move_constructor_ptr()(dest, src);
		}
		void destructor(this auto&& self, void* addr) requires (requires { self.destructor_ptr(); })
		{
			return self.destructor_ptr()(addr);
		}
		void destructor(this auto&& self, void* addr, size_t count) requires (requires { self.destructor_ptr(); self.size(); })
		{
			if (self.destructor_ptr() == nullptr)
				return;
			for (size_t i = 0; i < count; i++)
			{
				self.destructor_ptr()(addr);
				addr = (uint8_t*)addr + self.size();
			}
		}
		void destructor(this auto&& self, void* begin, void* end) requires (requires { self.destructor_ptr(); self.size(); })
		{
			if (self.destructor_ptr() == nullptr)
				return;
			for (; begin != end; (uint8_t*)begin + self.size())
			{
				self.destructor_ptr()(begin);
			}
		}

		bool is_trivially_destructible(this auto&& self) requires (requires { self.destructor_ptr(); })
		{
			return self.destructor_ptr() == nullptr;
		}
		bool is_reallocable(this auto&& self) requires (requires { self.move_constructor_ptr(); })
		{
			return self.move_constructor_ptr() == nullptr;
		}

	};

	std::strong_ordering operator <=>(
		std::derived_from<type_info_caching_interface> auto& a,
		std::derived_from<type_info_caching_interface> auto& b) requires (requires { a.hash(); b.hash(); })
	{
		return a.hash() <=> b.hash();
	}

	class type_index : public type_info_caching_interface
	{
		friend type_info_caching_interface;
		const type_info* m_info;

	public:
		type_index(const type_info& info) : m_info(&info) {}
	};

	class type_index_hash_only : public type_info_caching_interface
	{
		friend type_info_caching_interface;
		size_t m_hash;
	public:
		type_index_hash_only(const type_info& info) : m_hash(info.hash) {}
	};

	class type_index_container : public type_index
	{
		friend type_info_caching_interface;
		size_t m_size;
		copy_constructor_ptr_t m_copy_constructor;
		move_constructor_ptr_t m_move_constructor;
		destructor_ptr_t m_destructor;
	public:
		type_index_container(const type_info& info) :
			type_index(info),
			m_size(info.size),
			m_copy_constructor(info.copy_constructor),
			m_move_constructor(info.move_constructor),
			m_destructor(info.destructor)
		{}
	};


	void sample_use()
	{
		generic::type_index idx1 = generic::type_info::of<int>();
		generic::type_index idx2 = generic::type_info::of<float>();
		idx1.hash();
		idx1 < idx2;

		generic::type_index_hash_only idx3 = generic::type_info::of<int>();
		generic::type_index_hash_only idx4 = generic::type_info::of<float>();
		idx3.hash();
		idx3 < idx4;

		generic::type_index_container idx5 = generic::type_info::of<int>();
		generic::type_index_container idx6 = generic::type_info::of<float>();
		idx5.hash();
		idx5 < idx6;
	}
}

#endif // USE_COMBINATIVE
