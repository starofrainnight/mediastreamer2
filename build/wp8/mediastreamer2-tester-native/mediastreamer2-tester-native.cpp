﻿#include <string>

#include "mediastreamer2-tester-native.h"
#include "ortp/logging.h"
#include "cunit/Util.h"

using namespace Windows::Phone::Media::Devices;
using namespace Windows::Phone::Networking::Voip;

using namespace mediastreamer2_tester_native;
using namespace Platform;

#define MAX_TRACE_SIZE		512
#define MAX_SUITE_NAME_SIZE	128

static OutputTraceListener^ sTraceListener;

static void nativeOutputTraceHandler(OutputTraceLevel lev, const char *fmt, va_list args)
{
	if (sTraceListener) {
		wchar_t wstr[MAX_TRACE_SIZE];
		std::string str;
		str.resize(MAX_TRACE_SIZE);
		vsnprintf((char *)str.c_str(), MAX_TRACE_SIZE, fmt, args);
		mbstowcs(wstr, str.c_str(), sizeof(wstr));
		String^ msg = ref new String(wstr);
		sTraceListener->outputTrace(lev, msg);
	}
}

static void CUnitNativeOutputTraceHandler(int lev, const char *fmt, va_list args)
{
	nativeOutputTraceHandler(Raw, fmt, args);
}

static void Mediastreamer2NativeOutputTraceHandler(OrtpLogLevel lev, const char *fmt, va_list args)
{
	OutputTraceLevel level = Message;
	char fmt2[MAX_TRACE_SIZE];
	snprintf(fmt2, MAX_TRACE_SIZE, "%s\n", fmt);
	if (lev == ORTP_DEBUG) level = Debug;
	else if (lev == ORTP_MESSAGE) level = Message;
	else if (lev == ORTP_TRACE) level = Message;
	else if (lev == ORTP_WARNING) level = Warning;
	else if (lev == ORTP_ERROR) level = Error;
	else if (lev == ORTP_FATAL) level = Error;
	nativeOutputTraceHandler(level, fmt2, args);
}


Mediastreamer2TesterNative::Mediastreamer2TesterNative()
{
	mediastreamer2_tester_init();
}

Mediastreamer2TesterNative::~Mediastreamer2TesterNative()
{
	mediastreamer2_tester_uninit();
}

void Mediastreamer2TesterNative::setOutputTraceListener(OutputTraceListener^ traceListener)
{
	sTraceListener = traceListener;
}

void Mediastreamer2TesterNative::run(Platform::String^ suiteName, Platform::String^ caseName, Platform::Boolean verbose)
{
	std::wstring all(L"ALL");
	std::wstring wssuitename = suiteName->Data();
	std::wstring wscasename = caseName->Data();
	char csuitename[MAX_SUITE_NAME_SIZE] = { 0 };
	char ccasename[MAX_SUITE_NAME_SIZE] = { 0 };
	wcstombs(csuitename, wssuitename.c_str(), sizeof(csuitename));
	wcstombs(ccasename, wscasename.c_str(), sizeof(ccasename));

	if (verbose) {
		ortp_set_log_level_mask(ORTP_MESSAGE|ORTP_WARNING|ORTP_ERROR|ORTP_FATAL);
	} else {
		ortp_set_log_level_mask(ORTP_ERROR|ORTP_FATAL);
	}
	ortp_set_log_handler(Mediastreamer2NativeOutputTraceHandler);
	CU_set_trace_handler(CUnitNativeOutputTraceHandler);

	// Need to create a dummy VoipPhoneCall to be able to capture audio!
	Platform::String^ str = "Mediastreamer2";
	VoipCallCoordinator^ callCoordinator = VoipCallCoordinator::GetDefault();
	VoipPhoneCall^ phoneCall = nullptr;
	callCoordinator->RequestNewOutgoingCall(str, str, str, VoipCallMedia::Audio, &phoneCall);
	phoneCall->NotifyCallActive();
	AudioRoutingManager::GetDefault()->SetAudioEndpoint(AudioRoutingEndpoint::Speakerphone);
	mediastreamer2_tester_run_tests(wssuitename == all ? 0 : csuitename, wscasename == all ? 0 : ccasename);
	AudioRoutingManager::GetDefault()->SetAudioEndpoint(AudioRoutingEndpoint::Default);
	phoneCall->NotifyCallEnded();
}

unsigned int Mediastreamer2TesterNative::nbTestSuites()
{
	return mediastreamer2_tester_nb_test_suites();
}

unsigned int Mediastreamer2TesterNative::nbTests(Platform::String^ suiteName)
{
	std::wstring suitename = suiteName->Data();
	char cname[MAX_SUITE_NAME_SIZE] = { 0 };
	wcstombs(cname, suitename.c_str(), sizeof(cname));
	return mediastreamer2_tester_nb_tests(cname);
}

Platform::String^ Mediastreamer2TesterNative::testSuiteName(int index)
{
	const char *cname = mediastreamer2_tester_test_suite_name(index);
	wchar_t wcname[MAX_SUITE_NAME_SIZE];
	mbstowcs(wcname, cname, sizeof(wcname));
	return ref new String(wcname);
}

Platform::String^ Mediastreamer2TesterNative::testName(Platform::String^ suiteName, int testIndex)
{
	std::wstring suitename = suiteName->Data();
	char csuitename[MAX_SUITE_NAME_SIZE] = { 0 };
	wcstombs(csuitename, suitename.c_str(), sizeof(csuitename));
	const char *cname = mediastreamer2_tester_test_name(csuitename, testIndex);
	wchar_t wcname[MAX_SUITE_NAME_SIZE];
	mbstowcs(wcname, cname, sizeof(wcname));
	return ref new String(wcname);
}
