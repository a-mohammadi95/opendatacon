#	opendatacon
 #
 #	Copyright (c) 2014:
 #
 #		DCrip3fJguWgVCLrZFfA7sIGgvx1Ou3fHfCxnrz4svAi
 #		yxeOtDhDCXf1Z4ApgXvX5ahqQmzRfJ2DoX8S05SqHA==
 #	
 #	Licensed under the Apache License, Version 2.0 (the "License");
 #	you may not use this file except in compliance with the License.
 #	You may obtain a copy of the License at
 #	
 #		http://www.apache.org/licenses/LICENSE-2.0
 #
 #	Unless required by applicable law or agreed to in writing, software
 #	distributed under the License is distributed on an "AS IS" BASIS,
 #	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 #	See the License for the specific language governing permissions and
 #	limitations under the License.
 # 
project(DNP3Port)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. DNP3Port_SRC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DASIO_STANDALONE")

include_directories(
	"../include"
	"../JSON"
	"${DNP3_HOME}/dnp3${MY_CONFIG_POSTFIX}/cpp/libs/asiodnp3/src/"
	"${DNP3_HOME}/dnp3${MY_CONFIG_POSTFIX}/cpp/libs/asiopal/src/"
	"${DNP3_HOME}/dnp3${MY_CONFIG_POSTFIX}/cpp/libs/opendnp3/src/"
	"${DNP3_HOME}/dnp3${MY_CONFIG_POSTFIX}/cpp/libs/openpal/src/"
	"${ASIO_HOME}/asio/include/"
)

link_directories("${PROJECT_SOURCE_DIR}/../ODC/${MY_BUILD_CONFIG}"
"${DNP3_HOME}/dnp3${MY_CONFIG_POSTFIX}/.libs")
link_libraries(ODC asiopal openpal asiodnp3 opendnp3)
add_library(${PROJECT_NAME} SHARED ${DNP3Port_SRC})