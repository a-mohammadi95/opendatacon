/*	opendatacon
 *
 *	Copyright (c) 2014:
 *
 *		DCrip3fJguWgVCLrZFfA7sIGgvx1Ou3fHfCxnrz4svAi
 *		yxeOtDhDCXf1Z4ApgXvX5ahqQmzRfJ2DoX8S05SqHA==
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 */

#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cstdint>
#include <chrono>

//fast rough random numbers
#define CONG(jcong) (jcong = 69069*jcong+1234567)
#define ZERO_TO_ONE(a) (CONG(a)*2.328306e-10)

namespace odc
{

typedef uint32_t rand_t;
typedef uint64_t msSinceEpoch_t;

inline msSinceEpoch_t msSinceEpoch()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>
		       (std::chrono::system_clock::now().time_since_epoch()).count();
}

//Bitwise OR for use with uint enum classes
template <typename T>
constexpr T operator |( const T one, const T another )
{
	switch(sizeof(T))
	{
		case 1: return T(uint8_t (one) | uint8_t (another));
		case 2: return T(uint16_t(one) | uint16_t(another));
		case 4: return T(uint32_t(one) | uint32_t(another));
		case 8: return T(uint64_t(one) | uint64_t(another));
		default: throw std::runtime_error("Weird size for bitwise...");
	}
}

bool getline_noncomment(std::istream& is, std::string& line);
bool extract_delimited_string(std::istream& ist, std::string& extracted);
bool extract_delimited_string(const std::string& delims, std::istream& ist, std::string& extracted);

bool GetBool(const std::string& value);

} //namspace odc

