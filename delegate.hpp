#ifndef _DELEGATE_HPP_
#define _DELEGATE_HPP_

#include <utility>
#include <type_traits>

namespace delegate_utility
{
	template<typename _TMethod, _TMethod _Method>
	class delegate;

	template<typename _TOut, typename... _TIn, _TOut(*_Method)(_TIn...)>
	class delegate<_TOut(*)(_TIn...), _Method>
	{
	public:
		constexpr delegate() = default;

		inline constexpr _TOut operator()(_TIn... args) const
		{
			return (*_Method)(std::forward<_TIn>(args)...);
		}
	};

	template<typename _TObject, typename _TOut, typename... _TIn, _TOut(_TObject::*_Method)(_TIn...)>
	class delegate<_TOut(_TObject::*)(_TIn...), _Method>
	{
	public:
		using object_ptr_t = _TObject * const;

		delegate() = delete;

		inline constexpr delegate(const object_ptr_t & _object_ptr) : object_ptr_{ _object_ptr }
		{ }

		inline constexpr _TOut operator()(_TIn... args) const
		{
			return (object_ptr_->*_Method)(std::forward<_TIn>(args)...);
		}

	private:
		object_ptr_t object_ptr_;
	};

	template<typename _TObject, typename _TOut, typename... _TIn, _TOut(_TObject::*_Method)(_TIn...) const>
	class delegate<_TOut(_TObject::*)(_TIn...) const, _Method>
	{
	public:
		using object_ptr_t = _TObject const * const;

		delegate() = delete;

		inline constexpr delegate(const object_ptr_t & _object_ptr) : object_ptr_{ _object_ptr }
		{ }

		inline constexpr _TOut operator()(_TIn... args) const
		{
			return (object_ptr_->*_Method)(std::forward<_TIn>(args)...);
		}

	private:
		object_ptr_t object_ptr_;
	};

	template<typename _TOut, typename... _TIn>
	struct param_pack_t
	{ };

	template<typename _TObject, typename _TParams, typename = void>
	struct _is_callable : std::false_type
	{ };

	template<typename _TObject, typename _TOut, typename... _TIn>
	struct _is_callable<_TObject, param_pack_t<_TOut, _TIn...>,
		decltype((_TOut)(std::declval<_TObject>()(std::declval<_TIn>()...)), void())> : std::true_type
	{ };

	template<typename _TObject, typename _TOut, typename... _TIn>
	struct is_callable : _is_callable<_TObject, param_pack_t<_TOut, _TIn...>>
	{ };

	template<typename _TOut, typename... _TIn>
	class delegate_any
	{
	public:
		using pointer_t = void * const;
		using callback_t = _TOut(*)(_TIn..., pointer_t);

		delegate_any() = delete;

		template<typename _TCallable, std::enable_if_t<is_callable<_TCallable, _TOut, _TIn...>::value, bool> = true>
		inline delegate_any(_TCallable & _callable) : pointer_{ (pointer_t)(&_callable) }
		{
			callback_ = [](_TIn... args, pointer_t ptr) { 
				return (*reinterpret_cast<_TCallable * const>(ptr))(std::forward<_TIn>(args)...); };
		}

		template<typename _TCallable, std::enable_if_t<is_callable<const _TCallable, _TOut, _TIn...>::value, bool> = true>
		inline delegate_any(const _TCallable & _callable) : pointer_{ (pointer_t)(&_callable) }
		{
			callback_ = [](_TIn... args, pointer_t ptr) { 
				return (*reinterpret_cast<_TCallable const * const>(ptr))(std::forward<_TIn>(args)...); };
		}

		inline constexpr delegate_any(const callback_t & _callback, const pointer_t & _pointer = nullptr) :
			callback_{ _callback }, pointer_{ _pointer }
		{ }

		inline constexpr _TOut operator()(_TIn... args) const
		{
			return (*callback_)(std::forward<_TIn>(args)..., pointer_);
		}

	private:
		callback_t callback_;
		pointer_t pointer_;
	};

	template<typename _TMethod, _TMethod _Method>
	class delegate_any_factory;

	template<typename _TOut, typename... _TIn, _TOut(*_Method)(_TIn...)>
	class delegate_any_factory<_TOut(*)(_TIn...), _Method>
	{
	public:
		using return_type_t = delegate_any<_TOut, _TIn...>;

		static inline constexpr return_type_t make()
		{
			return return_type_t([](_TIn... args, void * const) { return (*_Method)(std::forward<_TIn>(args)...); });
		}
	};

	template<typename _TObject, typename _TOut, typename... _TIn, _TOut(_TObject::*_Method)(_TIn...)>
	class delegate_any_factory<_TOut(_TObject::*)(_TIn...), _Method>
	{
	public:
		using core_ptr_t = _TObject * const;
		using return_type_t = delegate_any<_TOut, _TIn...>;

		static inline constexpr return_type_t make(core_ptr_t _object_ptr)
		{
			return return_type_t([](_TIn... args, void * const ptr) {
				return (reinterpret_cast<core_ptr_t>(ptr)->*_Method)(std::forward<_TIn>(args)...); },
				(void * const)_object_ptr);
		}
	};

	template<typename _TObject, typename _TOut, typename... _TIn, _TOut(_TObject::*_Method)(_TIn...) const>
	class delegate_any_factory<_TOut(_TObject::*)(_TIn...) const, _Method>
	{
	public:
		using core_ptr_t = _TObject const * const;
		using return_type_t = delegate_any<_TOut, _TIn...>;

		static inline constexpr return_type_t make(core_ptr_t _object_ptr)
		{
			return return_type_t([](_TIn... args, void * const ptr) {
				return (reinterpret_cast<core_ptr_t>(ptr)->*_Method)(std::forward<_TIn>(args)...); },
				(void * const)_object_ptr);
		}
	};
}

template<typename _TMethod, _TMethod _Method>
using delegate_t = delegate_utility::delegate<_TMethod, _Method>;

template<typename _TOut, typename... _TIn>
using delegate_any_t = delegate_utility::delegate_any<_TOut, _TIn...>;

template<typename _TMethod, _TMethod _Method>
inline constexpr auto make_delegate_any()
{
	return delegate_utility::delegate_any_factory<_TMethod, _Method>::make();
}

template<typename _TMethod, _TMethod _Method>
inline constexpr auto make_delegate_any(typename delegate_utility::delegate_any_factory<_TMethod, _Method>::core_ptr_t ptr)
{
	return delegate_utility::delegate_any_factory<_TMethod, _Method>::make(ptr);
}

#endif // !_DELEGATE_HPP_
