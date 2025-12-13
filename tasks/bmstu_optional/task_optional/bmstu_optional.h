#pragma once
#include <cstdint>
#include <exception>
#include <new>
#include <type_traits>
#include <utility>

namespace bmstu
{
struct nullopt_t
{
	constexpr explicit nullopt_t(int) {}
};
inline constexpr nullopt_t nullopt{0};

class bad_optional_access : public std::exception
{
   public:
	using exception::exception;

	[[nodiscard]] const char* what() const noexcept override
	{
		return "Bad optional access";
	}
};

template <typename T>
class optional
{
   public:
	optional() = default;

	optional(const T& value) : is_initialized_(true) { new (data_) T(value); }

	optional(T&& value) : is_initialized_(true)
	{
		new (data_) T(std::move(value));
	}

	optional(const optional& other) : is_initialized_(other.is_initialized_)
	{
		if (is_initialized_)
		{
			new (data_) T(*other.get_ptr());
		}
	}

	optional(optional&& other) noexcept : is_initialized_(other.is_initialized_)
	{
		if (is_initialized_)
		{
			new (data_) T(std::move(*other.get_ptr()));
			other.is_initialized_ = false;
		}
	}

	optional& operator=(const T& value)
	{
		if (is_initialized_)
		{
			*get_ptr() = value;
		}
		else
		{
			new (data_) T(value);
			is_initialized_ = true;
		}
		return *this;
	}

	optional& operator=(T&& value)
	{
		if (is_initialized_)
		{
			*get_ptr() = std::move(value);
		}
		else
		{
			new (data_) T(std::move(value));
			is_initialized_ = true;
		}
		return *this;
	}

	optional& operator=(const optional& other)
	{
		if (this != &other)
		{
			if (other.is_initialized_)
			{
				if (is_initialized_)
				{
					*get_ptr() = *other.get_ptr();
				}
				else
				{
					new (data_) T(*other.get_ptr());
					is_initialized_ = true;
				}
			}
			else
			{
				if (is_initialized_)
				{
					get_ptr()->~T();
					is_initialized_ = false;
				}
			}
		}
		return *this;
	}

	optional& operator=(optional&& other) noexcept
	{
		if (this != &other)
		{
			if (other.is_initialized_)
			{
				if (is_initialized_)
				{
					*get_ptr() = std::move(*other.get_ptr());
				}
				else
				{
					new (data_) T(std::move(*other.get_ptr()));
					is_initialized_ = true;
				}
				other.is_initialized_ = false;
			}
			else
			{
				if (is_initialized_)
				{
					get_ptr()->~T();
					is_initialized_ = false;
				}
			}
		}
		return *this;
	}

	T& operator*() & { return *get_ptr(); }

	const T& operator*() const& { return *get_ptr(); }

	T* operator->() { return get_ptr(); }

	const T* operator->() const { return get_ptr(); }

	T&& operator*() && { return std::move(*get_ptr()); }

	T& value() &
	{
		if (!is_initialized_)
		{
			throw bad_optional_access();
		}
		return *get_ptr();
	}

	const T& value() const&
	{
		if (!is_initialized_)
		{
			throw bad_optional_access();
		}
		return *get_ptr();
	}

	T&& value() &&
	{
		if (!is_initialized_)
		{
			throw bad_optional_access();
		}
		return std::move(*get_ptr());
	}

	template <typename... Args>
	void emplace(Args&&... args)
	{
		if (is_initialized_)
		{
			get_ptr()->~T();
		}
		new (data_) T(std::forward<Args>(args)...);
		is_initialized_ = true;
	}

	void reset()
	{
		if (is_initialized_)
		{
			get_ptr()->~T();
			is_initialized_ = false;
		}
	}

	~optional()
	{
		if (is_initialized_)
		{
			get_ptr()->~T();
		}
	}

	[[nodiscard]] bool has_value() const { return is_initialized_; }

   private:
	T* get_ptr() { return reinterpret_cast<T*>(data_); }

	const T* get_ptr() const { return reinterpret_cast<const T*>(data_); }

	alignas(T) uint8_t data_[sizeof(T)];
	bool is_initialized_ = false;
};
}  // namespace bmstu