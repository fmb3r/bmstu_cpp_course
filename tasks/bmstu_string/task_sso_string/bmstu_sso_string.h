#pragma once

#include <exception>
#include <iostream>

namespace bmstu
{
template <typename T>
class basic_string;

using string = basic_string<char>;
using wstring = basic_string<wchar_t>;
using u16string = basic_string<char16_t>;
using u32string = basic_string<char32_t>;

template <typename T>
class basic_string
{
   private:
	static constexpr size_t SSO_CAPACITY =
		(sizeof(T*) + sizeof(size_t) + sizeof(size_t)) / sizeof(T) - 1;

	struct LongString
	{
		T* ptr;
		size_t size;
		size_t capacity;
	};

	struct ShortString
	{
		T buffer[SSO_CAPACITY + 1];
		unsigned char size;
	};

	union Data
	{
		LongString long_str;
		ShortString short_str;
	};

	Data data_;
	bool is_long_;

	bool is_long() const { return is_long_; }

	T* get_ptr()
	{
		if (is_long())
		{
			return data_.long_str.ptr;
		}
		else
		{
			return data_.short_str.buffer;
		}
	}

	const T* get_ptr() const
	{
		if (is_long())
		{
			return data_.long_str.ptr;
		}
		else
		{
			return data_.short_str.buffer;
		}
	}

	size_t get_size() const
	{
		if (is_long())
		{
			return data_.long_str.size;
		}
		else
		{
			return data_.short_str.size;
		}
	}

	size_t get_capacity() const
	{
		if (is_long())
		{
			return data_.long_str.capacity;
		}
		else
		{
			return SSO_CAPACITY;
		}
	}

   public:
	basic_string()
	{
		is_long_ = false;
		data_.short_str.size = 0;
		data_.short_str.buffer[0] = T(0);
	}

	basic_string(size_t size)
	{
		if (size <= SSO_CAPACITY)
		{
			is_long_ = false;
			data_.short_str.size = size;

			for (size_t i = 0; i < size; ++i)
			{
				data_.short_str.buffer[i] = T(' ');
			}
			data_.short_str.buffer[size] = T(0);
		}
		else
		{
			is_long_ = true;
			data_.long_str.size = size;
			data_.long_str.capacity = size;

			data_.long_str.ptr = new T[size + 1];

			for (size_t i = 0; i < size; i++)
			{
				data_.long_str.ptr[i] = T(' ');
			}
			data_.long_str.ptr[size] = T(0);
		}
	}

	basic_string(std::initializer_list<T> il)
	{
		size_t len = il.size();

		if (len <= SSO_CAPACITY)
		{
			is_long_ = false;
			data_.short_str.size = len;

			size_t i = 0;
			for (auto elem = il.begin(); elem != il.end(); ++elem)
			{
				data_.short_str.buffer[i] = *elem;
				++i;
			}
			data_.short_str.buffer[len] = T(0);
		}
		else
		{
			is_long_ = true;
			data_.long_str.size = len;
			data_.long_str.capacity = len;

			data_.long_str.ptr = new T[len + 1];

			size_t i = 0;
			for (auto elem = il.begin(); elem != il.end(); ++elem)
			{
				data_.long_str.ptr[i] = *elem;
				++i;
			}
			data_.long_str.ptr[len] = T(0);
		}
	}

	basic_string(const T* c_str)
	{
		if (!c_str)
		{
			is_long_ = false;
			data_.short_str.size = 0;
			data_.short_str.buffer[0] = T(0);
			return;
		}

		size_t len = strlen_(c_str);

		if (len <= SSO_CAPACITY)
		{
			is_long_ = false;
			data_.short_str.size = len;

			for (size_t i = 0; i < len; ++i)
			{
				data_.short_str.buffer[i] = c_str[i];
			}
			data_.short_str.buffer[len] = T(0);
		}
		else
		{
			is_long_ = true;
			data_.long_str.size = len;
			data_.long_str.capacity = len;

			data_.long_str.ptr = new T[len + 1];

			for (size_t i = 0; i < len; ++i)
			{
				data_.long_str.ptr[i] = c_str[i];
			}
			data_.long_str.ptr[len] = T(0);
		}
	}

	// конструктор копирования
	basic_string(const basic_string& other)
	{
		if (other.is_long_)
		{
			is_long_ = true;
			data_.long_str.size = other.data_.long_str.size;
			data_.long_str.capacity = other.data_.long_str.capacity;

			data_.long_str.ptr = new T[data_.long_str.capacity + 1];

			for (size_t i = 0; i < data_.long_str.size; ++i)
			{
				data_.long_str.ptr[i] = other.data_.long_str.ptr[i];
			}
			data_.long_str.ptr[data_.long_str.size] = T(0);
		}
		else
		{
			is_long_ = false;
			data_.short_str.size = other.data_.short_str.size;

			for (size_t i = 0; i < data_.short_str.size; ++i)
			{
				data_.short_str.buffer[i] = other.data_.short_str.buffer[i];
			}
			data_.short_str.buffer[data_.short_str.size] = T(0);
		}
	}

	// конструктор перемещения
	basic_string(basic_string&& dying) noexcept
	{
		if (dying.is_long_)
		{
			is_long_ = true;
			data_.long_str.ptr = dying.data_.long_str.ptr;
			data_.long_str.size = dying.data_.long_str.size;
			data_.long_str.capacity = dying.data_.long_str.capacity;

			dying.is_long_ = false;
			dying.data_.short_str.size = 0;
			dying.data_.short_str.buffer[0] = T();
		}
		else
		{
			is_long_ = false;
			data_.short_str.size = dying.data_.short_str.size;

			for (size_t i = 0; i < data_.short_str.size; ++i)
			{
				data_.short_str.buffer[i] = dying.data_.short_str.buffer[i];
			}
			data_.short_str.buffer[data_.short_str.size] = T(0);
		}
	}

	// деструктор
	~basic_string()
	{
		if (is_long_)
		{
			delete[] data_.long_str.ptr;
		}
	}

	const T* c_str() const { return get_ptr(); }

	size_t size() const { return get_size(); }

	bool is_using_sso() const { return !is_long(); }

	size_t capacity() const { return get_capacity(); }

	// оператор перемещающего присваивания
	basic_string& operator=(basic_string&& other) noexcept
	{
		if (this != &other)
		{
			if (is_long_)
			{
				delete[] data_.long_str.ptr;
			}
			if (other.is_long_)
			{
				is_long_ = true;
				data_.long_str.size = other.data_.long_str.size;
				data_.long_str.capacity = other.data_.long_str.capacity;
				data_.long_str.ptr = other.data_.long_str.ptr;

				other.is_long_ = false;
				other.data_.short_str.size = 0;
				other.data_.short_str.buffer[0] = T(0);
			}
			else
			{
				is_long_ = false;
				data_.short_str.size = other.data_.short_str.size;

				for (size_t i = 0; i < data_.short_str.size; ++i)
				{
					data_.short_str.buffer[i] = other.data_.short_str.buffer[i];
				}
				data_.short_str.buffer[data_.short_str.size] = T(0);
			}
			other.is_long_ = false;
			other.data_.short_str.size = 0;
			other.data_.short_str.buffer[0] = T(0);
		}
		return *this;
	}

	basic_string& operator=(const T* c_str)
	{
		if (data() != c_str)
		{
			if (is_long_)
			{
				delete[] data_.long_str.ptr;
			}

			if (!c_str)
			{
				is_long_ = false;
				data_.short_str.size = 0;
				data_.short_str.buffer[0] = T(0);
				return *this;
			}

			size_t len = strlen_(c_str);

			if (len <= SSO_CAPACITY)
			{
				is_long_ = false;
				data_.short_str.size = len;

				for (size_t i = 0; i < len; ++i)
				{
					data_.short_str.buffer[i] = c_str[i];
				}
				data_.short_str.buffer[len] = T(0);
			}
			else
			{
				is_long_ = true;
				data_.long_str.size = len;
				data_.long_str.capacity = len;

				data_.long_str.ptr = new T[len + 1];

				for (size_t i = 0; i < len; ++i)
				{
					data_.long_str.ptr[i] = c_str[i];
				}
				data_.long_str.ptr[len] = T(0);
			}
		}
		return *this;
	}

	// оператор копирующего присваивания
	basic_string& operator=(const basic_string& other)
	{
		if (this != &other)
		{
			if (is_long_)
			{
				delete[] data_.long_str.ptr;
			}
			if (other.is_long_)
			{
				is_long_ = true;
				data_.long_str.size = other.data_.long_str.size;
				data_.long_str.capacity = other.data_.long_str.capacity;

				data_.long_str.ptr = new T[data_.long_str.capacity + 1];

				for (size_t i = 0; i < data_.long_str.size; ++i)
				{
					data_.long_str.ptr[i] = other.data_.long_str.ptr[i];
				}
				data_.long_str.ptr[data_.long_str.size] = T(0);
			}
			else
			{
				is_long_ = false;
				data_.short_str.size = other.data_.short_str.size;

				for (size_t i = 0; i < data_.short_str.size; ++i)
				{
					data_.short_str.buffer[i] = other.data_.short_str.buffer[i];
				}
				data_.short_str.buffer[data_.short_str.size] = T(0);
			}
		}
		return *this;
	}

	// сложение строк
	friend basic_string<T> operator+(const basic_string<T>& left,
									 const basic_string<T>& right)
	{
		basic_string<T> result(left.size() + right.size());

		size_t pos = 0;

		for (size_t i = 0; i < left.size(); ++i, ++pos)
		{
			*(result.data() + i) = *(left.c_str() + i);
		}

		for (size_t i = 0; i < right.size(); ++i)
		{
			*(result.data() + pos + i) = *(right.c_str() + i);
		}

		result.data()[left.size() + right.size()] = T(0);

		return result;
	}

	// вывод
	template <typename S>
	friend S& operator<<(S& os, const basic_string& obj)
	{
		os << obj.c_str();
		return os;
	}

	// ввод
	template <typename S>
	friend S& operator>>(S& is, basic_string& obj)
	{
		T* buffer = new T[4096];
		size_t z = 0;

		T ch;

		while (is.get(ch))
		{
			buffer[z++] = ch;
		}
		buffer[z] = T(0);
		obj = buffer;
		delete[] buffer;
		return is;
	}

	basic_string& operator+=(const basic_string& other)
	{
		size_t l_size = size();
		size_t r_size = other.size();
		size_t new_size = l_size + r_size;

		T* new_ptr = new T[new_size + 1];

		for (size_t i = 0; i < l_size; ++i)
		{
			new_ptr[i] = get_ptr()[i];
		}

		for (size_t i = 0; i < r_size; ++i)
		{
			new_ptr[l_size + i] = other.c_str()[i];
		}
		new_ptr[new_size] = T(0);

		if (is_long_)
		{
			delete[] data_.long_str.ptr;
		}

		is_long_ = true;
		data_.long_str.ptr = new_ptr;
		data_.long_str.size = new_size;
		data_.long_str.capacity = new_size;

		return *this;
	}

	basic_string& operator+=(T symbol)
	{
		size_t l_size = size();
		size_t new_size = l_size + 1;

		T* new_ptr = new T[new_size + 1];

		for (size_t i = 0; i < l_size; ++i)
		{
			new_ptr[i] = get_ptr()[i];
		}

		new_ptr[l_size] = symbol;
		new_ptr[new_size] = T(0);

		if (is_long_)
		{
			delete[] data_.long_str.ptr;
		}

		is_long_ = true;

		data_.long_str.ptr = new_ptr;
		data_.long_str.size = new_size;
		data_.long_str.capacity = new_size;

		return *this;
	}

	T& operator[](size_t index) noexcept { return get_ptr()[index]; }

	T& at(size_t index) { throw std::out_of_range("Wrong index"); }

	T* data() { return get_ptr(); }

   private:
	static size_t strlen_(const T* str)
	{
		size_t len = 0;
		while (str[len] != T(0))
		{
			++len;
		}
		return len;
	}

	void clean_() {}
};
}  // namespace bmstu
