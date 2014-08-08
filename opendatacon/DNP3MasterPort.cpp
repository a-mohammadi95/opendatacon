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
/*
 * DNP3ClientPort.cpp
 *
 *  Created on: 15/07/2014
 *      Author: Neil Stephens <dearknarl@gmail.com>
 */

#include <asiodnp3/DefaultMasterApplication.h>
#include <opendnp3/app/ClassField.h>
#include <opendnp3/app/MeasurementTypes.h>
#include "DNP3MasterPort.h"
#include "CommandCallbackPromise.h"

void DNP3MasterPort::Enable()
{
	if(enabled)
		return;
	pMaster->Enable();
	enabled = true;
}
void DNP3MasterPort::Disable()
{
	if(!enabled)
		return;
	pMaster->Disable();
	enabled = false;
}
void DNP3MasterPort::BuildOrRebuild(asiodnp3::DNP3Manager& DNP3Mgr, openpal::LogFilters& LOG_LEVEL)
{
	DNP3PortConf* pConf = static_cast<DNP3PortConf*>(this->pConf.get());
	auto IPPort = pConf->mAddrConf.IP + std::to_string(pConf->mAddrConf.Port);
	auto log_id = "tcp_client_"+IPPort;

	//create a new channel if one isn't already up
	if(!TCPChannels.count(IPPort))
	{
		TCPChannels[IPPort] = DNP3Mgr.AddTCPClient(log_id.c_str(), LOG_LEVEL.GetBitfield(),
											openpal::TimeDuration::Seconds(1),
											openpal::TimeDuration::Seconds(300),
											pConf->mAddrConf.IP,
											pConf->mAddrConf.Port);
	}
	//Add a callback to get notified when the channel changes state
	TCPChannels[IPPort]->AddStateListener([=](opendnp3::ChannelState state)
	{
		for(auto IOHandler_pair : Subscribers)
		{
			bool failed;
			if(state == opendnp3::ChannelState::CLOSED || state == opendnp3::ChannelState::SHUTDOWN)
			{
				for(auto index : pConf->pPointConf->AnalogIndicies)
					IOHandler_pair.second->Event(opendnp3::Analog(0.0,static_cast<uint8_t>(opendnp3::AnalogQuality::COMM_LOST)),index,this->Name);
				for(auto index : pConf->pPointConf->BinaryIndicies)
					IOHandler_pair.second->Event(opendnp3::Binary(false,static_cast<uint8_t>(opendnp3::BinaryQuality::COMM_LOST)),index,this->Name);
				failed = pConf->pPointConf->mCommsPoint.first.value;
			}
			else
				failed = !pConf->pPointConf->mCommsPoint.first.value;

			if(pConf->pPointConf->mCommsPoint.first.quality == static_cast<uint8_t>(opendnp3::BinaryQuality::ONLINE))
					IOHandler_pair.second->Event(opendnp3::Binary(failed),pConf->pPointConf->mCommsPoint.second,this->Name);
		}
	});

	opendnp3::MasterStackConfig StackConfig;
	StackConfig.link.LocalAddr = pConf->mAddrConf.MasterAddr;
	StackConfig.link.RemoteAddr = pConf->mAddrConf.OutstationAddr;

	//TODO: add config items for these
	StackConfig.link.NumRetry = 0;
	StackConfig.link.Timeout = openpal::TimeDuration::Seconds(30);

	StackConfig.link.UseConfirms = pConf->pPointConf->UseConfirms;
	StackConfig.master.disableUnsolOnStartup = !pConf->pPointConf->DoUnsolOnStartup;
	StackConfig.master.unsolClassMask = pConf->pPointConf->GetUnsolClassMask();
	StackConfig.master.startupIntegrityClassMask = opendnp3::ClassField::ALL_CLASSES; //TODO: report/investigate bug - doesn't recognise response to integrity scan if not ALL_CLASSES

	pMaster = TCPChannels[IPPort]->AddMaster(Name.c_str(), *this, asiodnp3::DefaultMasterApplication::Instance(), StackConfig);

	// configure integrity scans
	if(pConf->pPointConf->IntegrityScanRateSec > 0)
		pMaster->AddClassScan(opendnp3::ClassField::ALL_CLASSES, openpal::TimeDuration::Seconds(pConf->pPointConf->IntegrityScanRateSec));

	// configure event scans
	if(pConf->pPointConf->EventClass1ScanRateSec > 0)
		pMaster->AddClassScan(opendnp3::ClassField::CLASS_1, openpal::TimeDuration::Seconds(pConf->pPointConf->EventClass1ScanRateSec));
	if(pConf->pPointConf->EventClass2ScanRateSec > 0)
		pMaster->AddClassScan(opendnp3::ClassField::CLASS_2, openpal::TimeDuration::Seconds(pConf->pPointConf->EventClass2ScanRateSec));
	if(pConf->pPointConf->EventClass3ScanRateSec > 0)
		pMaster->AddClassScan(opendnp3::ClassField::CLASS_3, openpal::TimeDuration::Seconds(pConf->pPointConf->EventClass3ScanRateSec));
}
//implement ISOEHandler
void DNP3MasterPort::OnReceiveHeader(const HeaderRecord& header, TimestampMode tsmode, const IterableBuffer<IndexedValue<Binary, uint16_t>>& meas){ LoadT(meas); };
void DNP3MasterPort::OnReceiveHeader(const HeaderRecord& header, TimestampMode tsmode, const IterableBuffer<IndexedValue<DoubleBitBinary, uint16_t>>& meas){ LoadT(meas); };
void DNP3MasterPort::OnReceiveHeader(const HeaderRecord& header, TimestampMode tsmode, const IterableBuffer<IndexedValue<Analog, uint16_t>>& meas){ LoadT(meas); };
void DNP3MasterPort::OnReceiveHeader(const HeaderRecord& header, TimestampMode tsmode, const IterableBuffer<IndexedValue<Counter, uint16_t>>& meas){ LoadT(meas); };
void DNP3MasterPort::OnReceiveHeader(const HeaderRecord& header, TimestampMode tsmode, const IterableBuffer<IndexedValue<FrozenCounter, uint16_t>>& meas){ LoadT(meas); };
void DNP3MasterPort::OnReceiveHeader(const HeaderRecord& header, TimestampMode tsmode, const IterableBuffer<IndexedValue<BinaryOutputStatus, uint16_t>>& meas){ LoadT(meas); };
void DNP3MasterPort::OnReceiveHeader(const HeaderRecord& header, TimestampMode tsmode, const IterableBuffer<IndexedValue<AnalogOutputStatus, uint16_t>>& meas){ LoadT(meas); };
void DNP3MasterPort::OnReceiveHeader(const HeaderRecord& header, TimestampMode tsmode, const IterableBuffer<IndexedValue<OctetString, uint16_t>>& meas){/*LoadT(meas);*/ };

template<typename T>
inline void DNP3MasterPort::LoadT(const IterableBuffer<IndexedValue<T, uint16_t>>& meas)
{
	meas.foreach([&](const IndexedValue<T, uint16_t>& pair)
	{
		for(auto IOHandler_pair : Subscribers)
		{
			IOHandler_pair.second->Event(pair.value,pair.index,this->Name);
		}
	});
}

//Implement some IOHandler - parent DNP3Port implements the rest to return NOT_SUPPORTED
std::future<opendnp3::CommandStatus> DNP3MasterPort::Event(const opendnp3::ControlRelayOutputBlock& arCommand, uint16_t index, const std::string& SenderName){ return EventT(arCommand, index, SenderName); };
std::future<opendnp3::CommandStatus> DNP3MasterPort::Event(const opendnp3::AnalogOutputInt16& arCommand, uint16_t index, const std::string& SenderName){ return EventT(arCommand, index, SenderName); };
std::future<opendnp3::CommandStatus> DNP3MasterPort::Event(const opendnp3::AnalogOutputInt32& arCommand, uint16_t index, const std::string& SenderName){ return EventT(arCommand, index, SenderName); };
std::future<opendnp3::CommandStatus> DNP3MasterPort::Event(const opendnp3::AnalogOutputFloat32& arCommand, uint16_t index, const std::string& SenderName){ return EventT(arCommand, index, SenderName); };
std::future<opendnp3::CommandStatus> DNP3MasterPort::Event(const opendnp3::AnalogOutputDouble64& arCommand, uint16_t index, const std::string& SenderName){ return EventT(arCommand, index, SenderName); };

template<typename T>
inline std::future<opendnp3::CommandStatus> DNP3MasterPort::EventT(T& arCommand, uint16_t index, const std::string& SenderName)
{
	auto cmd_promise = std::promise<opendnp3::CommandStatus>();
	auto cmd_future = cmd_promise.get_future();

	if(!enabled)
	{
		cmd_promise.set_value(opendnp3::CommandStatus::UNDEFINED);
		return cmd_future;
	}

	auto pConf = static_cast<DNP3PortConf*>(this->pConf.get());
	for(auto i : pConf->pPointConf->ControlIndicies)
	{
		if(i == index)
		{
			auto cmd_proc = this->pMaster->GetCommandProcessor();
			cmd_proc->DirectOperate(arCommand,index, *CommandCorrespondant::GetCallback(std::move(cmd_promise)));
			return cmd_future;
		}
	}
	cmd_promise.set_value(opendnp3::CommandStatus::UNDEFINED);
	return cmd_future;
}

