#pragma once

#include <type_traits>
#include <cstddef>
#include <cstdint>

class OutputStream
{
public:
	OutputStream() : mErrorBit(false) { }
	virtual ~OutputStream() { }

	virtual size_t getMaxSize() const = 0;
	virtual size_t tell() const = 0;
	virtual size_t write(const void* data, size_t len) = 0;
	virtual size_t seek(size_t pos) = 0;

	virtual size_t reserve(size_t len) = 0;

	template<typename T>
	OutputStream& operator<<(const T&);

	operator bool() const { return !mErrorBit; }

protected:
	void setError(bool err = true) { mErrorBit = err; }

private:
	bool mErrorBit;

	template<typename T, typename Enable = void>
	struct WHelper
	{
		static bool writeT(OutputStream* obj, const T& in)
		{
			return obj->write(&in, sizeof(T)) == sizeof(T);
		}
	};

	template<class T>
	struct WHelper<T, typename std::enable_if<std::is_object<typename T::iterator>::value>::type>
	{
		static bool writeT(OutputStream* obj, const T& in)
		{
			uint16_t size = uint16_t(in.size());
			if (!obj->operator<<(size))
				return false;

			for (auto& it : in)
				if (!obj->operator<<(it))
					return false;

			return true;
		}
	};
};

template<typename T>
OutputStream& OutputStream::operator<<(const T& in)
{
	if (!WHelper<T>::writeT(this, in))
		mErrorBit = true;
	return *this;
}
