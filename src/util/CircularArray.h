#pragma once

/*  Constant-at-construction sized circular buffer.
*/

#include <memory>
#include <concepts>

namespace norm
{

template <typename T>
concept Number = std::is_integral_v<T> ||
                 std::is_floating_point_v<T>;

template<Number type = float>
class CircularArray
{
public:
    CircularArray(int size)
        : mSize(size > 1 ? (size_t)size : 1)
        , mData(std::make_unique<type[]>(mSize))
        , mIndex(0)
    {
    }
    CircularArray(CircularArray&& other) noexcept
        : mSize(other.mSize)
        , mData (std::move(other.mData))
        , mIndex(other.mIndex)
    {
    }
    CircularArray(const CircularArray& other)
        : mSize(other.mSize)
        , mData(std::make_unique<type[]>(mSize))
        , mIndex(other.mIndex)
    {
        for (size_t i = 0; i < mSize ; i++)
        {
            mData[i] = other.mData[i];
        }
    }   
    ~CircularArray() {}

    void push(type element)
    {
        mData[mIndex++] = element;
        mIndex %= mSize;
    }
    type pushAndPop(type element)
    {
        type pop = mData[mIndex];
        mData[mIndex++] = element;
        mIndex %= mSize;
        return pop;
    }
    type operator[](size_t index)
    {
        size_t virtualIndex = mIndex + index;
        virtualIndex %= mSize;
        return mData[virtualIndex];
    }
    const type operator[](size_t index) const
    {
        size_t virtualIndex = mIndex + index;
        virtualIndex %= mSize;
        return mData[virtualIndex];
    }
    type operator[](ptrdiff_t index)
    { 
        return (*this)[(size_t) index];
    }
    const type operator[](ptrdiff_t index) const
    {
        return (*this)[(size_t) index];
    }
    const type* accesUnordered() const
    {
        return mData.get();
    }
    std::shared_ptr<type[]> getArray() const
    {
        auto array = std::make_shared<type[]>(mSize);
        for (ptrdiff_t i = 0; i < (ptrdiff_t)mSize ; i++)
        {
            array[i] = (*this)[i];
        }
        return array;
    }
    type getSum() const
    {
        type sum = (type)0;
        for (size_t i = 0; i < mSize; i++)
        {
            sum += mData[i];
        }
        return sum;
    }
    void reset()
    {
        for (size_t i = 0; i < mSize; i++)
        {
            mData[i] = (type)(0);
        }
        mIndex = 0;
    }

private:
    const size_t mSize;
    std::unique_ptr<type[]> mData;
    size_t mIndex;
};

} // namespace norm