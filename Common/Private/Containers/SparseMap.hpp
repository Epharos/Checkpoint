#pragma once

#include <concepts>
#include <limits>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <utility>
#include <vector>

#include "SparseSet.hpp"

namespace cp
{
    template<SparseKey Key, typename Value>
    class SparseMap
    {
    public:
        using KeyType = Key;
        using ValueType = Value;

        [[nodiscard]] bool Insert(const Key _key, const Value& _value)
        {
            return EmplaceInternal(_key, _value);
        }

        [[nodiscard]] bool Insert(const Key _key, Value&& _value)
        {
            return EmplaceInternal(_key, std::move(_value));
        }

        template<typename... Args>
        Value& EmplaceOrAssign(const Key _key, Args&&... _args)
        {
            const size_t sparseIndex = ToSparseIndex(_key);
            EnsureSparseCapacity(sparseIndex + 1);

            const size_t denseIndex = sparse[sparseIndex];
            if (denseIndex != DenseInvalidIndex)
            {
                values[denseIndex] = Value(std::forward<Args>(_args)...);
                return values[denseIndex];
            }

            sparse[sparseIndex] = keys.size();
            keys.push_back(_key);
            values.emplace_back(std::forward<Args>(_args)...);
            return values.back();
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

            const size_t lastDenseIndex = keys.size() - 1;
            const Key lastDenseKey = keys[lastDenseIndex];

            if (denseIndex != lastDenseIndex)
            {
                keys[denseIndex] = lastDenseKey;
                values[denseIndex] = std::move(values[lastDenseIndex]);
                sparse[ToSparseIndex(lastDenseKey)] = denseIndex;
            }

            keys.pop_back();
            values.pop_back();
            sparse[sparseIndex] = DenseInvalidIndex;
            return true;
        }

        [[nodiscard]] bool Contains(const Key _key) const
        {
            const size_t sparseIndex = ToSparseIndex(_key);
            return sparseIndex < sparse.size() && sparse[sparseIndex] != DenseInvalidIndex;
        }

        [[nodiscard]] Value* TryGet(const Key _key)
        {
            const size_t sparseIndex = ToSparseIndex(_key);
            if (sparseIndex >= sparse.size())
            {
                return nullptr;
            }

            const size_t denseIndex = sparse[sparseIndex];
            if (denseIndex == DenseInvalidIndex)
            {
                return nullptr;
            }

            return &values[denseIndex];
        }

        [[nodiscard]] const Value* TryGet(const Key _key) const
        {
            return const_cast<SparseMap*>(this)->TryGet(_key);
        }

        [[nodiscard]] Value& At(const Key _key)
        {
            Value* value = TryGet(_key);
            CP_ASSERT_MSG(value != nullptr, "Key does not exist.");
            return *value;
        }

        [[nodiscard]] const Value& At(const Key _key) const
        {
            return const_cast<SparseMap*>(this)->At(_key);
        }

        void Clear()
        {
            keys.clear();
            values.clear();
            sparse.clear();
        }

        void ReserveDense(const size_t _size)
        {
            keys.reserve(_size);
            values.reserve(_size);
        }

        void ReserveSparse(const size_t _size)
        {
            EnsureSparseCapacity(_size);
        }

        [[nodiscard]] size_t Size() const
        {
            return keys.size();
        }

        [[nodiscard]] bool IsEmpty() const
        {
            return keys.empty();
        }

        [[nodiscard]] const std::vector<Key>& Keys() const
        {
            return keys;
        }

        [[nodiscard]] const std::vector<Value>& Values() const
        {
            return values;
        }

    private:
        template<typename ValueInput>
        [[nodiscard]] bool EmplaceInternal(const Key _key, ValueInput&& _value)
        {
            const size_t sparseIndex = ToSparseIndex(_key);
            EnsureSparseCapacity(sparseIndex + 1);

            if (sparse[sparseIndex] != DenseInvalidIndex)
            {
                return false;
            }

            sparse[sparseIndex] = keys.size();
            keys.push_back(_key);
            values.emplace_back(std::forward<ValueInput>(_value));
            return true;
        }

        void EnsureSparseCapacity(const size_t _size)
        {
            if (_size > sparse.size())
            {
                sparse.resize(_size, DenseInvalidIndex);
            }
        }

    private:
        static constexpr size_t DenseInvalidIndex = std::numeric_limits<size_t>::max();
        std::vector<Key> keys;
        std::vector<Value> values;
        std::vector<size_t> sparse;
    };

    template<SparseKey Key, typename Value>
    class ThreadSafeSparseMap
    {
    public:
        using KeyType = Key;
        using ValueType = Value;

        [[nodiscard]] bool Insert(const Key _key, const Value& _value)
        {
            std::unique_lock lock(mutex);
            return sparseMap.Insert(_key, _value);
        }

        [[nodiscard]] bool Insert(const Key _key, Value&& _value)
        {
            std::unique_lock lock(mutex);
            return sparseMap.Insert(_key, std::move(_value));
        }

        template<typename... TArgs>
        void EmplaceOrAssign(const Key _key, TArgs&&... _args)
        {
            std::unique_lock lock(mutex);
            sparseMap.EmplaceOrAssign(_key, std::forward<TArgs>(_args)...);
        }

        [[nodiscard]] bool Erase(const Key _key)
        {
            std::unique_lock lock(mutex);
            return sparseMap.Erase(_key);
        }

        [[nodiscard]] bool Contains(const Key _key) const
        {
            std::shared_lock lock(mutex);
            return sparseMap.Contains(_key);
        }

        [[nodiscard]] std::optional<Value> TryGet(const Key _key) const
        {
            std::shared_lock lock(mutex);
            if (const Value* value = sparseMap.TryGet(_key))
            {
                return *value;
            }

            return std::nullopt;
        }

        void Clear()
        {
            std::unique_lock lock(mutex);
            sparseMap.Clear();
        }

        void ReserveDense(const size_t _size)
        {
            std::unique_lock lock(mutex);
            sparseMap.ReserveDense(_size);
        }

        void ReserveSparse(const size_t _size)
        {
            std::unique_lock lock(mutex);
            sparseMap.ReserveSparse(_size);
        }

        [[nodiscard]] size_t Size() const
        {
            std::shared_lock lock(mutex);
            return sparseMap.Size();
        }

        [[nodiscard]] bool IsEmpty() const
        {
            std::shared_lock lock(mutex);
            return sparseMap.IsEmpty();
        }

        [[nodiscard]] std::vector<Key> KeysSnapshot() const
        {
            std::shared_lock lock(mutex);
            return sparseMap.Keys();
        }

        [[nodiscard]] std::vector<Value> ValuesSnapshot() const
        {
            std::shared_lock lock(mutex);
            return sparseMap.Values();
        }

    private:
        SparseMap<Key, Value> sparseMap;
        mutable std::shared_mutex mutex;
    };
}
