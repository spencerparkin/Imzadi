#pragma once

#include "Log.h"
#include <map>
#include <string>
#include <fstream>
#include <mutex>
#include <wx/frame.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/timer.h>

class LogWindowRoute : public Imzadi::LogRoute
{
public:
	LogWindowRoute();
	virtual ~LogWindowRoute();

	virtual void PrintLogMessage(uint32_t messageFlags, const std::string& logMessage) override;
	virtual void RouteRegistered() override;
	virtual void RouteUnregistered() override;

	struct Message
	{
		uint32_t flags;
		std::string text;
	};

	bool GrabLogMessage(LogWindowRoute::Message& message);

private:
	std::mutex mutex;
	std::list<Message> messageList;
};

class LogWindow : public wxFrame
{
public:
	LogWindow(wxWindow* parent, const wxPoint& pos, const wxSize& size);
	virtual ~LogWindow();

	void SetRouteHandle(uint32_t logRouteHandle);

	enum
	{
		ID_LogWindow = wxID_HIGHEST + 1000,
		ID_Timer
	};

	void OnClearButtonPressed(wxCommandEvent& event);
	void OnDismissButtonClicked(wxCommandEvent& event);
	void OnTimer(wxTimerEvent& event);
	void OnClose(wxCloseEvent& event);

	void CloseIfNotClosing();

private:
	uint32_t logRouteHandle;
	wxTextCtrl* logTextCtrl;
	wxButton* clearButton;
	wxButton* dismissButton;
	wxTimer timer;
	bool windowClosing;
};