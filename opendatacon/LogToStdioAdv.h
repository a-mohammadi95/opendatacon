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

#include <openpal/logging/ILogHandler.h>
#include <asiopal/UTCTimeSource.h>
#include <openpal/executor/UTCTimestamp.h>
#include <openpal/executor/TimeDuration.h>
#include <iostream>
#include <vector>
#include <regex>

class MessageCount
{
public:
	MessageCount(std::string aRegex_str, int aCount) :
		LogRegex(aRegex_str.c_str()),
		LogRegex_string(aRegex_str),
		Count(aCount),
		Decimate(0),
		IgnoreDuration(openpal::TimeDuration::Milliseconds(0)),
		PrintTime(0)
	{};
	MessageCount(std::string aRegex_str, int aCount, int decimate) :
		LogRegex(aRegex_str.c_str()),
		LogRegex_string(aRegex_str),
		Count(aCount),
		Decimate(decimate),
		IgnoreDuration(openpal::TimeDuration::Milliseconds(0)),
		PrintTime(0)
	{};
	MessageCount(std::string aRegex_str, int aCount, openpal::TimeDuration ignore_duration) :
		LogRegex(aRegex_str.c_str()),
		LogRegex_string(aRegex_str),
		Count(aCount),
		Decimate(0),
		IgnoreDuration(ignore_duration),
		PrintTime(asiopal::UTCTimeSource::Instance().Now())
	{};
	std::regex LogRegex;
	std::string LogRegex_string;
	int Count;
	int Decimate;
	openpal::TimeDuration IgnoreDuration;
	openpal::UTCTimestamp PrintTime;
};

class LogToStdioAdv : public openpal::ILogHandler
{
public:
	void Log(const openpal::LogEntry& arEntry);
	void AddIngoreMultiple(const std::string& str);
	void AddIngoreAlways(const std::string& str);
	void AddIngoreDecimate(const std::string& str, int decimate);
	void AddIngoreDuration(const std::string& str, openpal::TimeDuration ignore_duration);
	void RemoveIgnore(const std::string& str);
	void ShowIgnored();

private:
	static openpal::ILogHandler& console;
	std::vector<MessageCount> IgnoreRepeats;
};

