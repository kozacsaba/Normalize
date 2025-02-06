/*  Constant-at-construction sized circular buffer.
*/

#include <memory>
#include <concepts>

namespace norm
{

template <typename T>
concept Number = std::is_integral_v<T> ||
                 std::is_floating_point_v<T>;

template<Number type>
class CircularArray
{
public:
    CircularArray(int size)
        : mSize(size > 1 ? size : 1)
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
        for (int i = 0; i < mSize ; i++)
        {
            mData[i] = other.mData[i];
        }
    }   
    ~CircularArray() {}

    void push(type element)
    {
        mData[mIndex] = element;
        mIndex %= mSize;
    }
    type pushAndPop(type element)
    {
        type pop = mData[mIndex];
        mData[mIndex] = element;
        mIndex %= mSize;
        return pop;
    }
    type operator[](int index)
    {
        int virtualIndex = mIndex + index;
        virtualIndex %= mSize;
        return mData[virtualIndex];
    }
    const type operator[](int index) const
    {
        type element = (*this)[index];
        return element;
    }
    const type* accesUnordered() const
    {
        return mData.get();
    }
    std::shared_ptr<type> getArray() const
    {
        auto array = std::make_shared<type[]>(mSize);
        for (int i = 0; i < mSize ; i++)
        {
            array[i] = (*this)[i];
        }
        return array;
    }
    type getSum() const
    {
        type sum = (type)0;
        for (int i = 0; i < mSize; i++)
        {
            sum += mData[i];
        }
        return sum;
    }
    void reset()
    {
        for (int i = 0; i < mSize; i++)
        {
            mData[i] = (type)(0);
        }
        mIndex = 0;
    }

private:
    const int mSize;
    std::unique_ptr<type[]> mData;
    int mIndex;
};

} // namespace norm