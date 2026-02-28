#pragma once

#include <array>
#include <cstdlib>

#include "../Utilities/Concepts.hpp"

namespace cp
{
	/**
	* @brief A template struct representing an extent in N-dimensional space, where N is determined by the template parameter. It provides basic arithmetic operations and comparison operators for the extent values.
	* 
	* @tparam T The numeric type of the extent values (e.g., int, float).
	* @tparam N The number of dimensions for the extent (e.g., 2 for 2D, 3 for 3D).
	*/
	template<Numeric T, size_t N>
	struct Extent
	{
		T values[N];

		Extent(const std::array<T, N>& _defaultValue = {}) noexcept
		{
			for (size_t i = 0; i < N; i++)
			{
				values[i] = _defaultValue[i];
			}
		}

		Extent(std::array<T, N>&& _defaultValue) noexcept
		{
			for (size_t i = 0; i < N; i++)
			{
				values[i] = std::move(_defaultValue[i]);
			}
		}

		constexpr T& operator[](size_t index) noexcept
		{
			return values[index];
		}

		constexpr const T& operator[](size_t index) const noexcept
		{
			return values[index];
		}

		constexpr bool operator==(const Extent& other) const noexcept
		{
			for (size_t i = 0; i < N; i++)
			{
				if (values[i] != other.values[i])
				{
					return false;
				}
			}
			return true;
		}

		constexpr bool operator!=(const Extent& other) const noexcept
		{
			return !(*this == other);
		}

		constexpr Extent operator+(const Extent& other) const noexcept
		{
			Extent result{};
			for (size_t i = 0; i < N; i++)
			{
				result.values[i] = values[i] + other.values[i];
			}
			return result;
		}

		constexpr Extent operator-(const Extent& other) const noexcept
		{
			Extent result{};
			for (size_t i = 0; i < N; i++)
			{
				result.values[i] = values[i] - other.values[i];
			}
			return result;
		}

		constexpr Extent operator*(const Extent& other) const noexcept
		{
			Extent result{};
			for (size_t i = 0; i < N; i++)
			{
				result.values[i] = values[i] * other.values[i];
			}
			return result;
		}

		constexpr Extent operator/(const Extent& other) const noexcept
		{
			Extent result{};
			for (size_t i = 0; i < N; i++)
			{
				result.values[i] = values[i] / other.values[i];
			}
			return result;
		}

		constexpr Extent operator%(const Extent& other) const noexcept
		{
			Extent result{};
			for (size_t i = 0; i < N; i++)
			{
				result.values[i] = values[i] % other.values[i];
			}
			return result;
		}

		constexpr Extent operator-() const noexcept
		{
			Extent result{};
			for (size_t i = 0; i < N; i++)
			{
				result.values[i] = -values[i];
			}
			return result;
		}

		constexpr Extent& operator+=(const Extent& other) noexcept
		{
			for (size_t i = 0; i < N; i++)
			{
				values[i] += other.values[i];
			}
			return *this;
		}

		constexpr Extent& operator-=(const Extent& other) noexcept
		{
			for (size_t i = 0; i < N; i++)
			{
				values[i] -= other.values[i];
			}
			return *this;
		}

		constexpr Extent& operator*=(const Extent& other) noexcept
		{
			for (size_t i = 0; i < N; i++)
			{
				values[i] *= other.values[i];
			}
			return *this;
		}

		constexpr Extent& operator/=(const Extent& other) noexcept
		{
			for (size_t i = 0; i < N; i++)
			{
				values[i] /= other.values[i];
			}
			return *this;
		}

		constexpr Extent& operator%=(const Extent& other) noexcept
		{
			for (size_t i = 0; i < N; i++)
			{
				values[i] %= other.values[i];
			}
			return *this;
		}
	};

	/**
	* @brief A template struct representing a 2D extent, derived from the Extent template struct with N set to 2. It provides convenient accessors for the x and y components of the extent.
	* 
	* @tparam T The numeric type of the extent values (e.g., int, float).
	*/
	template<Numeric T>
	struct Extent2D : public Extent<T, 2>
	{
		Extent2D(const std::array<T, 2>& _defaultValue = {}) noexcept : Extent<T, 2>(_defaultValue) {}
		Extent2D(std::array<T, 2>&& _defaultValue) noexcept : Extent<T, 2>(std::move(_defaultValue)) {}
		Extent2D(T x, T y) noexcept : Extent<T, 2>({ x, y }) {}

		constexpr T& x() noexcept
		{
			return this->values[0];
		}

		constexpr const T& x() const noexcept
		{
			return this->values[0];
		}

		constexpr T& y() noexcept
		{
			return this->values[1];
		}

		constexpr const T& y() const noexcept
		{
			return this->values[1];
		}
	};

	/**
	* @brief A template struct representing a 3D extent, derived from the Extent template struct with N set to 3. It provides convenient accessors for the x, y, and z components of the extent.
	* 
	* @tparam T The numeric type of the extent values (e.g., int, float).
	*/
	template<Numeric T>
	struct Extent3D : public Extent<T, 3>
	{
		Extent3D(const std::array <T, 3>& _defaultValue = {}) noexcept : Extent<T, 3>(_defaultValue) {}
		Extent3D(std::array<T, 3>&& _defaultValue) noexcept : Extent<T, 3>(std::move(_defaultValue)) {}
		Extent3D(T x, T y, T z) noexcept : Extent<T, 3>({ x, y, z }) {}

		constexpr T& x() noexcept
		{
			return this->values[0];
		}

		constexpr const T& x() const noexcept
		{
			return this->values[0];
		}

		constexpr T& y() noexcept
		{
			return this->values[1];
		}

		constexpr const T& y() const noexcept
		{
			return this->values[1];
		}

		constexpr T& z() noexcept
		{
			return this->values[2];
		}

		constexpr const T& z() const noexcept
		{
			return this->values[2];
		}
	};
}