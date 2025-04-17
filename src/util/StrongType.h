#pragma once

template<typename Tag>
struct StrongFloat {
    float value;

    constexpr StrongFloat(float v = 0.0f) noexcept : value(v) {}
    constexpr float get() const noexcept { return value; }

    template<typename Tag> constexpr StrongFloat<Tag> 
    operator+(StrongFloat<Tag> a) noexcept { return a; }

    template<typename Tag> constexpr StrongFloat<Tag> 
    operator-(StrongFloat<Tag> a) noexcept 
    { return StrongFloat<Tag>{-a.value}; }

    template<typename Tag> constexpr StrongFloat<Tag>& 
    operator++(StrongFloat<Tag>& a) noexcept { ++a.value; return a; }

    template<typename Tag> constexpr StrongFloat<Tag> 
    operator++(StrongFloat<Tag>& a, int) noexcept 
    { StrongFloat<Tag> temp = a; ++a.value; return temp; }

    template<typename Tag> constexpr StrongFloat<Tag>& 
    operator--(StrongFloat<Tag>& a) noexcept { --a.value; return a; }

    template<typename Tag> constexpr StrongFloat<Tag> 
    operator--(StrongFloat<Tag>& a, int) noexcept 
    { StrongFloat<Tag> temp = a; --a.value; return temp; }
};

template<typename Tag> constexpr StrongFloat<Tag> 
operator+(StrongFloat<Tag> a, StrongFloat<Tag> b) noexcept 
{ return StrongFloat<Tag>{a.get() + b.get()}; }

template<typename Tag> constexpr StrongFloat<Tag> 
operator-(StrongFloat<Tag> a, StrongFloat<Tag> b) noexcept 
{ return StrongFloat<Tag>{a.get() - b.get()}; }

template<typename Tag> constexpr StrongFloat<Tag> 
operator*(StrongFloat<Tag> a, StrongFloat<Tag> b) noexcept 
{ return StrongFloat<Tag>{a.get() * b.get()}; }

template<typename Tag> constexpr StrongFloat<Tag> 
operator/(StrongFloat<Tag> a, StrongFloat<Tag> b) noexcept 
{ return StrongFloat<Tag>{a.get() / b.get()}; }

template<typename Tag> constexpr bool 
operator<=>(StrongFloat<Tag> a, StrongFloat<Tag> b) noexcept 
{ return a.get() <=> b.get(); }

template<typename Tag> constexpr bool 
operator<=>(StrongFloat<Tag> a, float b) noexcept 
{ return a.get() <=> b; }

template<typename Tag> constexpr bool 
operator<=>(float a, StrongFloat<Tag> b) noexcept 
{ return a <=> b.get(); }

template<typename Tag> constexpr StrongFloat<Tag>& 
operator+=(StrongFloat<Tag>& a, StrongFloat<Tag> b) noexcept 
{ a.get() += b.get(); return a; }

template<typename Tag> constexpr StrongFloat<Tag>& 
operator-=(StrongFloat<Tag>& a, StrongFloat<Tag> b) noexcept 
{ a.get() -= b.get(); return a; }

template<typename Tag> constexpr StrongFloat<Tag>& 
operator*=(StrongFloat<Tag>& a, StrongFloat<Tag> b) noexcept 
{ a.get() *= b.get(); return a; }

template<typename Tag> constexpr StrongFloat<Tag>& 
operator/=(StrongFloat<Tag>& a, StrongFloat<Tag> b) noexcept 
{ a.get() /= b.get(); return a; }

template<typename Tag> constexpr StrongFloat<Tag>& 
operator+=(StrongFloat<Tag>& a, float b) noexcept 
{ a.get() += b; return a; }

template<typename Tag> constexpr StrongFloat<Tag>& 
operator-=(StrongFloat<Tag>& a, float b) noexcept 
{ a.get() -= b; return a; }

template<typename Tag> constexpr StrongFloat<Tag>& 
operator*=(StrongFloat<Tag>& a, float b) noexcept 
{ a.get() *= b; return a; }

template<typename Tag> constexpr StrongFloat<Tag>& 
operator/=(StrongFloat<Tag>& a, float b) noexcept 
{ a.get() /= b; return a; }
