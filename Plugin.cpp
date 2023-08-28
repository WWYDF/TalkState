// Forked from the testPlugin found here on GitHub: https://github.com/mumble-voip/mumble/blob/master/plugins/testPlugin/testPlugin.cpp
// Created for using in junction with the Positional Audio Mod for GTFO. https://github.com/WWYDF/OpenPA/
// Downloaded from GitHub here: https://github.com/WWYDF/TalkState/

#include "include/MumbleAPI_v_1_0_x.h"
#include "include/MumblePlugin_v_1_0_x.h"

#include <cstring>
#include <String>
#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")
using namespace std;

// These are just some utility functions facilitating writing logs and the like
// The actual implementation of the plugin is further down
std::ostream& pLog() {
	std::cout << "xnull: ";
	return std::cout;
}

template< typename T > void pluginLog(T log) {
	pLog() << log << std::endl;
}

std::ostream& operator<<(std::ostream& stream, const mumble_version_t version) {
	stream << "v" << version.major << "." << version.minor << "." << version.patch;
	return stream;
}


//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////// PLUGIN IMPLEMENTATION ///////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

mumble_api_t mumAPI;
mumble_connection_t activeConnection;
mumble_plugin_id_t ownID;

//////////////////////////////////////////////////////////////
//////////////////// OBLIGATORY FUNCTIONS ////////////////////
//////////////////////////////////////////////////////////////
// All of the following function must be implemented in order for Mumble to load the plugin

const char* fstate = "unset\0";
HANDLE hMapFile = nullptr;
char* sharedText = nullptr;

void CreateMemoryMappedFile() {
	HANDLE hFile = CreateFile(
		"%temp%\gtfo_posaudio_mumlink",
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		nullptr,
		PAGE_READWRITE,
		0,
		1024, // Size of the shared memory region for the string
		"posaudio_mumlink"
	);

	sharedText = static_cast<char*>(MapViewOfFile(
		hMapFile,
		FILE_MAP_WRITE,
		0,
		0,
		1024
	));
}

void UpdateMemoryMappedFileContents(const std::string& data) {
	if (sharedText) {
		strncpy(sharedText, data.c_str(), 1024);
	}
}

mumble_error_t mumble_init(uint32_t id) {

	std::thread serverThread(CreateMemoryMappedFile);
	serverThread.detach(); // Allow the thread to run independently
	//CreateMemoryMappedFile();

	pluginLog("Initialized plugin");
	UpdateMemoryMappedFileContents("unset");

	ownID = id;

	// Print the connection ID at initialization. If not connected to a server it should be -1.
	pLog() << "Plugin ID is " << id << std::endl;

	mumAPI.log(ownID, "Intitialized");

	// Little showcase for how to retrieve a setting from Mumble
	int64_t voiceHold;
	mumble_error_t error = mumAPI.getMumbleSetting_int(ownID, MUMBLE_SK_AUDIO_INPUT_VOICE_HOLD, &voiceHold);
	if (error == MUMBLE_STATUS_OK) {
		pLog() << "The voice hold is set to " << voiceHold << std::endl;
	}
	else {
		pluginLog("Failed to retrieve voice hold");
		pLog() << mumble_errorMessage(error) << std::endl;
	}

	// MUMBLE_STATUS_OK is a macro set to the appropriate status flag (ErrorCode)
	// If you need to return any other status have a look at the ErrorCode enum
	// inside PluginComponents.h and use one of its values
	return MUMBLE_STATUS_OK;
}

void mumble_shutdown() {
	WSACleanup();
	if (sharedText) {
		UnmapViewOfFile(sharedText);
	}
	if (hMapFile) {
		CloseHandle(hMapFile);
	}
	pluginLog("Shutdown plugin");

	mumAPI.log(ownID, "Shutdown");
}

MumbleStringWrapper mumble_getName() {
	static const char* name = "TalkState";

	MumbleStringWrapper wrapper;
	wrapper.data = name;
	wrapper.size = strlen(name);
	// It's a static String and therefore doesn't need releasing
	wrapper.needsReleasing = false;

	return wrapper;
}

mumble_version_t mumble_getAPIVersion() {
	// MUMBLE_PLUGIN_API_VERSION will always contain the API version of the used header file (the one used to build
	// this plugin against). Thus you should always return that here in order to no have to worry about it.
	return MUMBLE_PLUGIN_API_VERSION;
}

void mumble_registerAPIFunctions(void* api) {
	// In this function the plugin is presented with a struct of function pointers that can be used
	// to interact with Mumble. Thus you should store it somewhere safe for later usage.

	// The pointer has to be cast to the respective API struct. You always have to cast to the same API version
	// as this plugin itself is using. Thus if this plugin is compiled using the API version 1.0.x (where x is an
	// arbitrary version) the pointer has to be cast to MumbleAPI_v_1_0_x (where x is a literal "x"). Furthermore the
	// struct HAS TO BE COPIED!!! Storing the pointer is not an option as it will become invalid quickly!

	// **If** you are using the same API version that is specified in the included header file (as you should), you
	// can simply use the MUMBLE_API_CAST to cast the pointer to the correct type and automatically dereferencing it.
	mumAPI = MUMBLE_API_CAST(api);

	pluginLog("Registered Mumble's API functions");
}

void mumble_releaseResource(const void* pointer) {
	std::cerr << "[ERROR]: Unexpected call to mumble_releaseResources" << std::endl;
	std::terminate();
	// This plugin doesn't use resources that are explicitly allocated (only static Strings are used). Therefore
	// we don't have to implement this function.
	//
	// If you allocated resources using malloc(), you're implementation for releasing that would be
	// free(const_cast<void *>(pointer));
	//
	// If however you allocated a resource using the new operator (C++ only), you have figure out the pointer's
	// original type and then invoke
	// delete static_cast<ActualType *>(pointer);

	// Mark as unused
	(void)pointer;
}


//////////////////////////////////////////////////////////////
///////////////////// OPTIONAL FUNCTIONS /////////////////////
//////////////////////////////////////////////////////////////
// The implementation of below functions is optional. If you don't need them, don't include them in your
// plugin

void mumble_setMumbleInfo(mumble_version_t mumbleVersion, mumble_version_t mumbleAPIVersion,
	mumble_version_t minimumExpectedAPIVersion) {
	// this function will always be the first one to be called. Even before init()
	// In here you can get info about the Mumble version this plugin is about to run in.
	pLog() << "Mumble version: " << mumbleVersion << "; Mumble API-Version: " << mumbleAPIVersion
		<< "; Minimal expected API-Version: " << minimumExpectedAPIVersion << std::endl;
}

mumble_version_t mumble_getVersion() {
	// Mumble uses semantic versioning (see https://semver.org/)
	// { major, minor, patch }
	return { 1, 0, 0 };
}

MumbleStringWrapper mumble_getAuthor() {
	static const char* author = "Traveller";

	MumbleStringWrapper wrapper;
	wrapper.data = author;
	wrapper.size = strlen(author);
	// It's a static String and therefore doesn't need releasing
	wrapper.needsReleasing = false;

	return wrapper;
}

MumbleStringWrapper mumble_getDescription() {
	static const char* description = "Sends active talking state to Memory. Supposed to be used in junction with the Positional Audio Mod for GTFO.";

	MumbleStringWrapper wrapper;
	wrapper.data = description;
	wrapper.size = strlen(description);
	// It's a static String and therefore doesn't need releasing
	wrapper.needsReleasing = false;

	return wrapper;
}

void mumble_onServerDisconnected(mumble_connection_t connection) {
	activeConnection = -1;

	const char* serverHash;
	if (mumAPI.getServerHash(ownID, connection, &serverHash) == MUMBLE_STATUS_OK) {
		pLog() << "Disconnected from server-connection with ID " << connection << "(hash: " << serverHash << ")"
			<< std::endl;

		mumAPI.freeMemory(ownID, serverHash);
	}
	else {
		pluginLog("[ERROR]: mumble_onServerDisconnected - Unable to fetch server-hash");
	}
}

void mumble_onUserTalkingStateChanged(mumble_connection_t connection, mumble_userid_t userID, mumble_talking_state_t talkingState)
{

	// The possible values are contained in the TalkingState enum inside PluginComponent.h
	switch (talkingState) {
	case MUMBLE_TS_INVALID:
		fstate = "Invalid\0";
		break;
	case MUMBLE_TS_PASSIVE:
		fstate = "Passive\0";
		break;
	case MUMBLE_TS_TALKING:
		fstate = "Talking\0";
		break;
	case MUMBLE_TS_WHISPERING:
		fstate = "Whispering\0";
		break;
	case MUMBLE_TS_SHOUTING:
		fstate = "Shouting\0";
		break;
	default:
		fstate = "Unknown\0";
	}

	UpdateMemoryMappedFileContents(fstate);
}

