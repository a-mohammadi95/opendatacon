/*	opendatacon
 *
 *	Copyright (c) 2017:
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
/*
 * PyPort.h
 *
 *  Created on: 26/03/2017
 *      Author: Alan Murray <alan@atmurray.net>
 */

#ifndef PYWRAPPER_H_
#define PYWRAPPER_H_

#include "Py.h"
#include <mutex>
#include <shared_mutex>
#include <unordered_set>
#include <opendatacon/util.h>
#include <opendatacon/DataPort.h>
#include "concurrentqueue.h"

using namespace odc;

typedef std::function<void (uint32_t, uint32_t)> SetTimerFnType;
typedef std::function<void ( const char*, uint32_t, const char*, const char*)> PublishEventCallFnType;

// Class to store the evnt as a stringified version, mainly so that when Python is retreving these records, it does minimal processing.
class EventQueueType
{
public:
	EventQueueType(const std::string& _EventType, const size_t _Index, odc::msSinceEpoch_t _TimeStamp,
		const std::string& _Quality, const std::string& _Payload, const std::string& _Sender):
		EventType(_EventType),
		Index(_Index),
		TimeStamp(_TimeStamp),
		Quality(_Quality),
		Payload(_Payload),
		Sender(_Sender)
	{};
	EventQueueType():
		EventType(""),
		Index(0),
		TimeStamp(0),
		Quality(""),
		Payload(""),
		Sender("")
	{};
	std::string EventType;
	size_t Index;
	odc::msSinceEpoch_t TimeStamp;
	std::string Quality;
	std::string Payload;
	std::string Sender;
};

class PythonInitWrapper
{
public:
	PythonInitWrapper();
	~PythonInitWrapper();
};

class PythonWrapper
{
private:
	friend class PythonInitWrapper;
	static PythonInitWrapper PythonInit;

	//TODO: Do we need a hard limit for the number of queued events, after which we start dumping elements. Better than running out of memory?
	// Would dothe limit using an atomic int - we dont need an "exact" maximum...
	const size_t MaximumQueueSize = 5000*1000; // 5 million

	moodycamel::ConcurrentQueue<EventQueueType> EventQueue = moodycamel::ConcurrentQueue<EventQueueType>(MaximumQueueSize);

public:
	PythonWrapper(const std::string& aName, SetTimerFnType SetTimerFn, PublishEventCallFnType PublishEventCallFn);
	~PythonWrapper();
	void Build(const std::string& modulename, const std::string& pyPathName, const std::string& pyLoadModuleName,
		const std::string& pyClassName, const std::string& PortName);
	void Config(const std::string& JSONMain, const std::string& JSONOverride);
	void PortOperational(); // Called when Build is complete.
	void Enable();
	void Disable();

	CommandStatus Event(std::shared_ptr<const EventInfo> odcevent, const std::string& SenderName);
	void QueueEvent(const std::string& EventType, const size_t Index, odc::msSinceEpoch_t TimeStamp, const std::string& Quality, const std::string& Payload, const std::string& Sender);

	bool DequeueEvent(EventQueueType& eq);

	void CallTimerHandler(uint32_t id);
	std::string RestHandler(const std::string& url, const std::string& content);

	SetTimerFnType GetPythonPortSetTimerFn() { return PythonPortSetTimerFn; };                         // Protect set access, only allow get.
	PublishEventCallFnType GetPythonPortPublishEventCallFn() { return PythonPortPublishEventCallFn; }; // Protect set access, only allow get.

	static void PyErrOutput();
	static void DumpStackTrace();

	std::string Name;

	// Do this to make sure it is always a valid pointer - the python code could pass anything back...
	static PythonWrapper* GetThisFromPythonSelf(uint64_t guid)
	{
		std::shared_lock<std::shared_timed_mutex> lck(PythonWrapper::WrapperHashMutex);
		if (PyWrappers.count(guid))
		{
			return (PythonWrapper*)guid;
		}
		return nullptr;
	}

private:
	void StoreWrapperMapping()
	{
		std::unique_lock<std::shared_timed_mutex> lck(PythonWrapper::WrapperHashMutex);
		PyWrappers.emplace((uint64_t)this);
		LOGDEBUG("Stored python wrapper guid into mapping table - {0:#x}", (uint64_t)this);
	}
	void RemoveWrapperMapping()
	{
		std::unique_lock<std::shared_timed_mutex> lck(PythonWrapper::WrapperHashMutex);
		PyWrappers.erase((uint64_t)this);
	}

	PyObject* GetFunction(PyObject* pyInstance, const std::string& sFunction);
	PyObject* PyCall(PyObject* pyFunction, PyObject* pyArgs);

	// Keep track of each PyWrapper so static methods can get access to the correct PyPort instance
	static std::unordered_set<uint64_t> PyWrappers;
	static std::shared_timed_mutex WrapperHashMutex;
	static PyThreadState* threadState;

	// Keep pointers to the methods in out Python code that we want to be able to call.
	PyObject* pyModule = nullptr;
	PyObject* pyInstance = nullptr;
	PyObject* pyFuncConfig = nullptr;
	PyObject* pyFuncOperational = nullptr;
	PyObject* pyFuncEnable = nullptr;
	PyObject* pyFuncDisable = nullptr;
	PyObject* pyFuncEvent = nullptr;
	PyObject* pyTimerHandler = nullptr;
	PyObject* pyRestHandler = nullptr;

	SetTimerFnType PythonPortSetTimerFn;
	PublishEventCallFnType PythonPortPublishEventCallFn;
};

#endif /* PYWRAPPER_H_ */
