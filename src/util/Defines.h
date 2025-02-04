#pragma once

/*  Some shorthands for things that appear in the code more than enough times
    to make me want to use shorthands
*/

#ifndef FLOAT_TYPE
    #define FLOAT_TYPE float
#endif

#include <numbers>
#define pi std::numbers::pi_v<FLOAT_TYPE>


#define global inline static FLOAT_TYPE
#define globalc inline static const FLOAT_TYPE

