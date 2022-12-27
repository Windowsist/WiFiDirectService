//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "stdafx.h"
#include "res.h"
#include "SimpleApp.h"
#include "Service.h"
#include "WlanHostedNetworkWinRT.h"

SimpleApp::SimpleApp()
	: _apEvent(CreateEventEx(nullptr, nullptr, 0, WRITE_OWNER | EVENT_ALL_ACCESS))
{
	HRESULT hr = _apEvent.IsValid() ? S_OK : HRESULT_FROM_WIN32(GetLastError());
	if (FAILED(hr))
	{
		SvcReportEvent(L"Failed to create AP event: ",SVC_ERROR, EVENTLOG_ERROR_TYPE);
		throw WlanHostedNetworkException("Create event failed", hr);
	}

	_hostedNetwork.RegisterListener(this);
}

SimpleApp::~SimpleApp()
{
	_hostedNetwork.RegisterListener(nullptr);
}

void SimpleApp::OnDeviceConnected(std::wstring remoteHostName)
{
	remoteHostName.insert(0, L"Peer connected: ");
	SvcReportEvent(remoteHostName.c_str(), SVC_INFO, EVENTLOG_INFORMATION_TYPE);
}

void SimpleApp::OnDeviceDisconnected(std::wstring deviceId)
{
	deviceId.insert(0, L"Peer disconnected: ");
	SvcReportEvent(deviceId.c_str(), SVC_INFO, EVENTLOG_INFORMATION_TYPE);
}

void SimpleApp::OnAdvertisementStarted()
{
	SvcReportEvent(L"Soft AP started!", SVC_INFO, EVENTLOG_INFORMATION_TYPE);
	SetEvent(_apEvent.Get());
}

void SimpleApp::OnAdvertisementStopped(std::wstring message)
{
	message.append(L"Soft AP stopped.");
	SvcReportEvent(message.c_str(), SVC_INFO, EVENTLOG_INFORMATION_TYPE);
	SetEvent(_apEvent.Get());
}

void SimpleApp::OnAdvertisementAborted(std::wstring message)
{
	message.append(L"Soft AP aborted: ");
	SvcReportEvent(message.c_str(), SVC_ERROR, EVENTLOG_ERROR_TYPE);
	SetEvent(_apEvent.Get());
}

void SimpleApp::OnAsyncException(std::wstring message)
{
	message.insert(0, L"Caught exception in asynchronous method: ");
	SvcReportEvent(message.c_str(), SVC_ERROR, EVENTLOG_ERROR_TYPE);
}

void SimpleApp::LogMessage(std::wstring message)
{
	SvcReportEvent(message.c_str(), SVC_INFO, EVENTLOG_INFORMATION_TYPE);
}

bool SimpleApp::AcceptIncommingConnection()
{
	return true;
}

void SimpleApp::Start()
{
	SvcReportEvent(L"Starting soft AP...", SVC_INFO, EVENTLOG_INFORMATION_TYPE);
	_hostedNetwork.Start();
	WaitForSingleObjectEx(_apEvent.Get(), INFINITE, FALSE);
}

void SimpleApp::Stop()
{
	SvcReportEvent(L"Stopping soft AP...", SVC_INFO, EVENTLOG_INFORMATION_TYPE);
	_hostedNetwork.Stop();
	WaitForSingleObjectEx(_apEvent.Get(), INFINITE, FALSE);
}

