#pragma once

#include <memory>
#include <gtest/gtest.h>
#include <util/CircularArray.h>

namespace
{
    std::shared_ptr<float[]> randArray(int size)
    {
        auto ptr = std::make_shared<float[]>((size_t)size);

        for(int i = 0; i < size; i++)
        {
            ptr[i] = (float)std::rand() / (float)std::rand();
        }

        return ptr;
    }
}

TEST(CircularTest, FillBuffer1)
{
    const int size = 2;
    norm::CircularArray buffer(size);
    auto rnd = randArray(size);

    for(int i = 0; i < size; i++)
    {
        buffer.push(rnd[i]);
    }

    auto arr = buffer.accesUnordered();

    EXPECT_FLOAT_EQ(arr[0], rnd[0]);
    EXPECT_FLOAT_EQ(arr[1], rnd[1]);
}

TEST(CircularTest, FillBuffer2)
{
    const int size = 5;
    norm::CircularArray buffer(size);
    auto rnd = randArray(size);

    for(int i = 0; i < size; i++)
    {
        buffer.push(rnd[i]);
    }

    auto arr = buffer.accesUnordered();

    EXPECT_FLOAT_EQ(arr[0], rnd[0]);
    EXPECT_FLOAT_EQ(arr[1], rnd[1]);
    EXPECT_FLOAT_EQ(arr[2], rnd[2]);
    EXPECT_FLOAT_EQ(arr[3], rnd[3]);
    EXPECT_FLOAT_EQ(arr[4], rnd[4]);
}

TEST(CircularTest, RollBuffer)
{
    norm::CircularArray buffer(3);
    auto rnd = randArray(4);

    for(int i = 0; i < 4; i++)
    {
        buffer.push(rnd[i]);
    }

    auto arr = buffer.accesUnordered();

    EXPECT_FLOAT_EQ(arr[0], rnd[3]);
    EXPECT_FLOAT_EQ(arr[1], rnd[1]);
    EXPECT_FLOAT_EQ(arr[2], rnd[2]);
}

TEST(CircularTest, PushAndPop)
{
    const int size = 6;
    norm::CircularArray buffer(size);
    auto rnd = randArray(size+1);

    for(int i = 0; i < size; i++)
    {
        buffer.push(rnd[i]);
    }

    float popped = buffer.pushAndPop(rnd[size]);

    EXPECT_FLOAT_EQ(popped, rnd[0]);
}

TEST(CircularTest, PushAndPopFill)
{
    const int size = 5;
    norm::CircularArray buffer(size);
    auto rnd = randArray(size);

    for(int i = 0; i < size; i++)
    {
        buffer.pushAndPop(rnd[i]);
    }

    auto arr = buffer.accesUnordered();

    EXPECT_FLOAT_EQ(arr[0], rnd[0]);
    EXPECT_FLOAT_EQ(arr[1], rnd[1]);
    EXPECT_FLOAT_EQ(arr[2], rnd[2]);
    EXPECT_FLOAT_EQ(arr[3], rnd[3]);
    EXPECT_FLOAT_EQ(arr[4], rnd[4]);
}

TEST(CircularTest, ZeroSize)
{
    norm::CircularArray buffer(0);
    auto rnd = randArray(2);

    buffer.push(rnd[0]);
    float popped = buffer.pushAndPop(rnd[1]);

    auto data = buffer.accesUnordered();

    EXPECT_FLOAT_EQ(*data, rnd[1]);
    EXPECT_FLOAT_EQ(popped, rnd[0]);
}

TEST(CircularTest, CopyContruct)
{
    const int size = 2;
    norm::CircularArray buffer(size);
    auto rnd = randArray(size);

    for(int i = 0; i < size; i++)
    {
        buffer.push(rnd[i]);
    }

    norm::CircularArray copiedInstance(buffer);

    auto arr = copiedInstance.accesUnordered();

    EXPECT_FLOAT_EQ(arr[0], rnd[0]);
    EXPECT_FLOAT_EQ(arr[1], rnd[1]);
}

TEST(CircularTest, Indexing)
{
    const int size = 15;
    norm::CircularArray buffer(size);
    auto rnd = randArray(size);

    for(int i = 0; i < size; i++)
    {
        buffer.push(rnd[i]);
    }

    ptrdiff_t index1 = std::rand() % size;
    ptrdiff_t index2 = index1, index3 = index1;

    while(index2 == index1) 
    { 
        index2 = std::rand() % size; 
    }
    while(index3 == index1 || index3 == index2 ) 
    { 
        index3 = std::rand() % size; 
    }

    EXPECT_FLOAT_EQ(buffer[index1], rnd[index1]);
    EXPECT_FLOAT_EQ(buffer[index2], rnd[index2]);
    EXPECT_FLOAT_EQ(buffer[index3], rnd[index3]);
}

TEST(CircularTest, IndexingWithOffset)
{
    const int size = 15;
    norm::CircularArray buffer(size);
    auto rnd = randArray(size);

    for(int i = 0; i < 7; i++)
    {
        buffer.push(0.f);
    }

    for(int i = 0; i < size; i++)
    {
        buffer.push(rnd[i]);
    }

    ptrdiff_t index1 = std::rand() % size;
    ptrdiff_t index2 = index1, index3 = index1;

    while(index2 == index1) 
    { 
        index2 = std::rand() % size; 
    }
    while(index3 == index1 || index3 == index2 ) 
    { 
        index3 = std::rand() % size; 
    }

    EXPECT_FLOAT_EQ(buffer[index1], rnd[index1]);
    EXPECT_FLOAT_EQ(buffer[index2], rnd[index2]);
    EXPECT_FLOAT_EQ(buffer[index3], rnd[index3]);
}

TEST(CircularTest, OverIndexing)
{
    const int size = 15;
    norm::CircularArray buffer(size);
    auto rnd = randArray(size);

    for(int i = 0; i < size; i++)
    {
        buffer.push(rnd[i]);
    }

    ptrdiff_t index1 = std::rand() % size;
    ptrdiff_t index2 = index1, index3 = index1;

    while(index2 == index1) 
    { 
        index2 = std::rand() % size; 
    }
    while(index3 == index1 || index3 == index2 ) 
    { 
        index3 = std::rand() % size; 
    }

    const ptrdiff_t overIndex1 = index1 + size;
    const ptrdiff_t overIndex2 = index2 + 2 * size;
    const ptrdiff_t overIndex3 = index3 + 42 * size;

    EXPECT_FLOAT_EQ(buffer[index1], rnd[index1]);
    EXPECT_FLOAT_EQ(buffer[index2], rnd[index2]);
    EXPECT_FLOAT_EQ(buffer[index3], rnd[index3]);
}

TEST(CircularTest, GetArray)
{
    const int size = 7;
    norm::CircularArray buffer(size);
    auto rnd = randArray(size);
    const int offset = size / 2;

    for(int i = 0; i < offset; i++)
    {
        buffer.push(100.f);
    }

    for(int i = 0; i < size; i++)
    {
        buffer.push(rnd[i]);
    }

    auto data = buffer.getArray();

    for(int i = 0; i < size; i++)
    {
        EXPECT_FLOAT_EQ(data[i], rnd[i]);
    }
}

TEST(CircularTest, Sum)
{
    norm::CircularArray buffer(6);
    EXPECT_FLOAT_EQ(buffer.getSum(), 0.f);

    buffer.push(1.f);
    buffer.push(2.f);
    buffer.push(3.f);
    buffer.push(4.f);
    buffer.push(5.f);

    EXPECT_FLOAT_EQ(buffer.getSum(), 15.f);

    buffer.push(-3.f);
    buffer.push(-1.f);

    EXPECT_FLOAT_EQ(buffer.getSum(), 10.f);
}

TEST(CircularTest, Reset)
{
    const int size = 8;
    norm::CircularArray buffer(size);
    auto rnd = randArray(size);

    for(int i = 0; i < size; i++)
    {
        buffer.push(rnd[i]);
    }

    buffer.reset();
    EXPECT_FLOAT_EQ(buffer.getSum(), 0.f);
}
