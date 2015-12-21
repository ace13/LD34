#pragma once

#include <SFML/System/InputStream.hpp>

#include <type_traits>
#include <utility>
#include <cstdint>

class InputStream : public sf::InputStream
{
public:
	InputStream() : mErrorBit(false) { }
	virtual ~InputStream() { }

	template<typename T>
	InputStream& operator>>(T&);

	operator bool() const { return !mErrorBit; }

protected:
	void setError(bool err = true) { mErrorBit = err; }

private:
	bool mErrorBit;

	template<typename T, typename Enable = void>
	struct RHelper
	{
		static bool readT(InputStream* obj, T& out)
		{
			return obj->read(&out, sizeof(T)) == sizeof(T);
		}
	};

	template<class T>
	struct RHelper<T, typename std::enable_if<std::is_member_function_pointer<decltype(&T::data)>::value>::type>
	{
		static bool readT(InputStream* obj, T& out)
		{
			uint16_t size;
			if (!obj->read(&size, sizeof(uint16_t)))
				return false;

			T temp;
			temp.resize(size);
			if (obj->read(&temp[0], size) != size)
				return false;

			out = std::move(temp);
			return true;
		}
	};
};


template<typename T>
InputStream& InputStream::operator>>(T& out)
{
	if (!RHelper<T>::readT(this, out))
		mErrorBit = true;
	return *this;
}
