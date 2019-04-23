#pragma once

#include "exception.hpp"
#include "platform.hpp"

#ifdef __linux__

namespace game::native
{
    struct timeproxy : timespec
    {
        timeproxy()
        {
        }

        timeproxy(unsigned long long microseconds)
        {
            constexpr unsigned long long second = 1'000'000;
            tv_sec = microseconds / second;
            tv_nsec = (microseconds % second) * 1000;
        }

        timeproxy& operator+=(const timeproxy& rhs)
        {
            tv_sec += rhs.tv_sec;
            tv_nsec += rhs.tv_nsec;
            return *this;
        }

        timeproxy operator+(const timeproxy& rhs) const
        {
            timeproxy ret;
            ret.tv_sec = tv_sec + rhs.tv_sec;
            ret.tv_nsec = tv_nsec + rhs.tv_nsec;
            return ret;
        }

        timeproxy& operator-=(const timeproxy& rhs)
        {
            tv_sec -= rhs.tv_sec;
            tv_nsec -= rhs.tv_nsec;
            return *this;
        }

        timeproxy operator-(const timeproxy& rhs) const
        {
            timeproxy ret;
            ret.tv_sec = tv_sec - rhs.tv_sec;
            ret.tv_nsec = tv_nsec - rhs.tv_nsec;
            return ret;
        }

        bool operator==(const timeproxy& rhs) const noexcept
        {
            return tv_nsec == rhs.tv_nsec && tv_sec == rhs.tv_sec;
        }

        bool operator!=(const timeproxy& rhs) const noexcept
        {
            return tv_nsec != rhs.tv_nsec || tv_sec != rhs.tv_sec;
        }

        bool operator>(const timeproxy& rhs) const noexcept
        {
            return tv_sec > rhs.tv_sec || (tv_sec == rhs.tv_sec && tv_nsec > rhs.tv_nsec);
        }

        bool operator<(const timeproxy& rhs) const noexcept
        {
            return tv_sec < rhs.tv_sec || (tv_sec == rhs.tv_sec && tv_nsec < rhs.tv_nsec);
        }

        bool operator>=(const timeproxy& rhs) const noexcept
        {
            return operator>(rhs) || operator==(rhs);
        }

        bool operator<=(const timeproxy& rhs) const noexcept
        {
            return operator<(rhs) || operator==(rhs);
        }

        void update()
        {
            auto ret = clock_gettime(CLOCK_MONOTONIC_RAW, this);
            if (ret == -1)
                throw exception(except_e::NATIVE_CLOCK, "clock_gettime");
        }

		double toseconds() const
		{
			return (static_cast<double>(tv_nsec)/1E9) + static_cast<double>(tv_sec);
		}

        static unsigned long resolution()
        {
            timespec res;
            int ret = clock_getres(CLOCK_MONOTONIC_RAW, &res);
            if (ret == -1)
                throw exception(except_e::NATIVE_CLOCK, "clock_getres");
            return  1'000'000'000 / res.tv_nsec;
        }
    };
}

#elif defined(_WIN32)

namespace game::native
{
	struct timeproxy
	{
		timeproxy()
		{
			if (_ticks.QuadPart == 0) {
				if (QueryPerformanceFrequency(&_ticks) == FALSE)
					throw exception(except_e::NATIVE_CLOCK, "QueryPerformanceFrequency");
			}
		}

		timeproxy(unsigned long long microseconds)
		{
			if (_ticks.QuadPart == 0) {
				if (QueryPerformanceFrequency(&_ticks) == FALSE)
					throw exception(except_e::NATIVE_CLOCK, "QueryPerformanceFrequency");
			}
			_us.QuadPart = microseconds*_ticks.QuadPart;
		}

		timeproxy& operator+=(const timeproxy& rhs)
		{
			_us.QuadPart += rhs._us.QuadPart;
			return *this;
		}

		timeproxy operator+(const timeproxy& rhs) const
		{
			timeproxy ret;
			ret._us.QuadPart = _us.QuadPart + rhs._us.QuadPart;
			return ret;
		}

		timeproxy& operator-=(const timeproxy& rhs)
		{
			_us.QuadPart -= rhs._us.QuadPart;
			return *this;
		}

		timeproxy operator-(const timeproxy& rhs) const
		{
			timeproxy ret;
			ret._us.QuadPart = _us.QuadPart - rhs._us.QuadPart;
			return ret;
		}

		bool operator==(const timeproxy& rhs) const noexcept
		{
			return _us.QuadPart == rhs._us.QuadPart;
		}

		bool operator!=(const timeproxy& rhs) const noexcept
		{
			return _us.QuadPart != rhs._us.QuadPart;
		}

		bool operator>(const timeproxy& rhs) const noexcept
		{
			return _us.QuadPart > rhs._us.QuadPart;
		}

		bool operator<(const timeproxy& rhs) const noexcept
		{
			return _us.QuadPart < rhs._us.QuadPart;
		}

		bool operator>=(const timeproxy& rhs) const noexcept
		{
			return _us.QuadPart >= rhs._us.QuadPart;
		}

		bool operator<=(const timeproxy& rhs) const noexcept
		{
			return _us.QuadPart <= rhs._us.QuadPart;
		}

		void update()
		{
			if (QueryPerformanceCounter(&_us) == FALSE)
				throw exception(except_e::NATIVE_CLOCK, "QueryPerformanceCounter");
		}

		double toseconds() const
		{
			return (static_cast<double>(_us.QuadPart)*(1.0/static_cast<double>(_ticks.QuadPart)));
		}

		static unsigned long resolution()
		{
			if (_ticks.QuadPart == 0) {
				if (QueryPerformanceFrequency(&_ticks) == FALSE)
					throw exception(except_e::NATIVE_CLOCK, "QueryPerformanceFrequency");
			}
			return _ticks.QuadPart;
		}

	private:

		LARGE_INTEGER _us;

		inline static LARGE_INTEGER _ticks ={};
	};
}

#endif