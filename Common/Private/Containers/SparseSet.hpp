#pragma once

#include <concepts>
#include <limits>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <type_traits>
#include <vector>

#include "../Core/Assert.hpp"

namespace cp
{
    template<typename Key>
    concept SparseKey = std::integral<Key> || std::is_enum_v<Key>;

    template<SparseKey Key>
    [[nodiscard]] constexpr size_t ToSparseIndex(const Key _key)
    {
        if constexpr (std::is_enum_v<Key>)
        {
            using UnderlyingType = std::underlying_type_t<Key>;
            if constexpr (std::signed_integral<UnderlyingType>)
            {
                CP_ASSERT_MSG(static_cast<UnderlyingType>(_key) >= 0, "Sparse container key must be non-negative.");
            }

            return static_cast<size_t>(static_cast<UnderlyingType>(_key));
        }
        else
        {
            if constexpr (std::signed_integral<Key>)
            {
                CP_ASSERT_MSG(_key >= 0, "Sparse container key must be non-negative.");
            }

            return static_cast<size_t>(_key);
        }
    }

    template<SparseKey Key>
    class SparseSet
    {
    public:
        using KeyType = Key;

        [[nodiscard]] bool Insert(const Key _key)
        {
            const size_t sparseIndex = ToSparseIndex(_key);
            EnsureSparseCapacity(sparseIndex + 1);

            if (sparse[sparseIndex] != DenseInvalidIndex)
            {
                return false;
            }

            sparse[sparseIndex] = dense.size();
            dense.push_back(_key);
            return true;
        }

        [[nodiscard]] bool Erase(const Key _key)
        {
            const size_t sparseIndex = ToSparseIndex(_key);
            if (sparseIndex >= sparse.size())
            {
                return false;
            }

            const size_t denseIndex = sparse[sparseIndex];
            if (denseIndex == DenseInvalidIndex)
            {
                return false;
            }

            const size_t lastDenseIndex = dense.size() - 1;
            const Key lastDenseKey = dense[lastDenseIndex];

            if (denseIndex != lastDenseIndex)
            {
                dense[denseIndex] = lastDenseKey;
                sparse[ToSparseIndex(lastDenseKey)] = denseIndex;
            }

            dense.pop_back();
            sparse[sparseIndex] = DenseInvalidIndex;
            return true;
        }

        [[nodiscard]] bool Contains(const Key _key) const
        {
            const size_t sparseIndex = ToSparseIndex(_key);
            return sparseIndex < sparse.size() && sparse[sparseIndex] != DenseInvalidIndex;
        }

        [[nodiscard]] std::optional<size_t> FindDenseIndex(const Key _key) const
        {
            const size_t sparseIndex = ToSparseIndex(_key);
            if (sparseIndex >= sparse.size())
            {
                return std::nullopt;
            }

            const size_t denseIndex = sparse[sparseIndex];
            if (denseIndex == DenseInvalidIndex)
            {
                return std::nullopt;
            }

            return denseIndex;
        }

        void Clear()
        {
            dense.clear();
            sparse.clear();
        }

        void ReserveDense(const size_t _size)
        {
            dense.reserve(_size);
        }

        void ReserveSparse(const size_t _size)
        {
            EnsureSparseCapacity(_size);
        }

        [[nodiscard]] size_t Size() const
        {
            return dense.size();
        }

        [[nodiscard]] bool IsEmpty() const
        {
            return dense.empty();
        }

        [[nodiscard]] const std::vector<Key>& Dense() const
        {
            return dense;
        }

    private:
        void EnsureSparseCapacity(const size_t _size)
        {
            if (_size > sparse.size())
            {
                sparse.resize(_size, DenseInvalidIndex);
            }
        }

    private:
        static constexpr size_t DenseInvalidIndex = std::numeric_limits<size_t>::max();
        std::vector<Key> dense;
        std::vector<size_t> sparse;
    };

    template<SparseKey K>
    class ThreadSafeSparseSet
    {
    public:
        [[nodiscard]] bool Insert(const K _key)
        {
            std::unique_lock lock(mutex);
            return sparseSet.Insert(_key);
        }

        [[nodiscard]] bool Erase(const K _key)
        {
            std::unique_lock lock(mutex);
            return sparseSet.Erase(_key);
        }

        [[nodiscard]] bool Contains(const K _key) const
        {
            std::shared_lock lock(mutex);
            return sparseSet.Contains(_key);
        }

        [[nodiscard]] std::optional<size_t> FindDenseIndex(const K _key) const
        {
            std::shared_lock lock(mutex);
            return sparseSet.FindDenseIndex(_key);
        }

        void Clear()
        {
            std::unique_lock lock(mutex);
            sparseSet.Clear();
        }

        void ReserveDense(const size_t _size)
        {
            std::unique_lock lock(mutex);
            sparseSet.ReserveDense(_size);
        }

        void ReserveSparse(const size_t _size)
        {
            std::unique_lock lock(mutex);
            sparseSet.ReserveSparse(_size);
        }

        [[nodiscard]] size_t Size() const
        {
            std::shared_lock lock(mutex);
            return sparseSet.Size();
        }

        [[nodiscard]] bool IsEmpty() const
        {
            std::shared_lock lock(mutex);
            return sparseSet.IsEmpty();
        }

        [[nodiscard]] std::vector<K> DenseSnapshot() const
        {
            std::shared_lock lock(mutex);
            return sparseSet.Dense();
        }

    private:
        SparseSet<K> sparseSet;
        mutable std::shared_mutex mutex;
    };
}
