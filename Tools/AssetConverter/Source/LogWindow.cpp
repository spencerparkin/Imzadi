#include "LogWindow.h"
#include "App.h"
#include "Frame.h"
#include <wx/sizer.h>

//---------------------------------- LogWindowRoute ----------------------------------

LogWindowRoute::LogWindowRoute()
{
	this->SetName("log_window");
	this->SetFilterFlags(IMZADI_LOG_ALL_FLAGS);
}

/*virtual*/ LogWindowRoute::~LogWindowRoute()
{
}

/*virtual*/ void LogWindowRoute::PrintLogMessage(uint32_t messageFlags, const std::string& logMessage)
{
	std::lock_guard guard(this->mutex);
	this->messageList.push_back(Message{ messageFlags, logMessage });
}

/*virtual*/ void LogWindowRoute::RouteRegistered()
{
	// Note that we do *not* cache a pointer to the log window, because it can become stale.
	Frame* frame = wxGetApp().GetFrame();
	wxPoint pos = frame->GetPosition();
	pos.x += frame->GetSize().GetWidth();
	auto logWindow = new LogWindow(frame, pos, wxSize(600, 400));
	logWindow->SetRouteHandle(this->GetHandle());
	logWindow->Show();
}

/*virtual*/ void LogWindowRoute::RouteUnregistered()
{
	auto* logWindow = dynamic_cast<LogWindow*>(wxWindow::FindWindowById(LogWindow::ID_LogWindow));
	if (logWindow)
		logWindow->CloseIfNotClosing();
}

bool LogWindowRoute::GrabLogMessage(LogWindowRoute::Message& message)
{
	if (this->messageList.size() == 0)
		return false;

	std::lock_guard guard(this->mutex);
	std::list<Message>::iterator iter = this->messageList.begin();
	message = *iter;
	this->messageList.erase(iter);
	return true;
}

//---------------------------------- LogWindow ----------------------------------

LogWindow::LogWindow(wxWindow* parent, const wxPoint& pos, const wxSize& size) : wxFrame(parent, ID_LogWindow, "Log", pos, size, wxCAPTION | wxCLIP_CHILDREN), timer(this, ID_Timer)
{
	this->windowClosing = false;
	this->logRouteHandle = 0;

	this->logTextCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH | wxHSCROLL);

	this->clearButton = new wxButton(this, wxID_ANY, "Clear", wxDefaultPosition, wxSize(100, -1));
	this->dismissButton = new wxButton(this, wxID_ANY, "Dismiss", wxDefaultPosition, wxSize(100, -1));

	wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	buttonSizer->AddStretchSpacer(1);
	buttonSizer->Add(this->clearButton, 0, wxLEFT | wxDOWN | wxRIGHT, 4);
	buttonSizer->Add(this->dismissButton, 0, wxLEFT | wxDOWN | wxRIGHT, 4);

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(this->logTextCtrl, 1, wxALL | wxEXPAND, 4);
	mainSizer->Add(buttonSizer);

	this->SetSizer(mainSizer);

	this->clearButton->Bind(wxEVT_BUTTON, &LogWindow::OnClearButtonPressed, this);
	this->dismissButton->Bind(wxEVT_BUTTON, &LogWindow::OnDismissButtonClicked, this);
	this->Bind(wxEVT_TIMER, &LogWindow::OnTimer, this, ID_Timer);
	this->Bind(wxEVT_CLOSE_WINDOW, &LogWindow::OnClose, this);

	this->timer.Start(500);
}

/*virtual*/ LogWindow::~LogWindow()
{
}

void LogWindow::SetRouteHandle(uint32_t logRouteHandle)
{
	this->logRouteHandle = logRouteHandle;
}

void LogWindow::OnClearButtonPressed(wxCommandEvent& event)
{
	this->logTextCtrl->Clear();
}

void LogWindow::OnDismissButtonClicked(wxCommandEvent& event)
{
	this->Close();
}

void LogWindow::CloseIfNotClosing()
{
	if (!this->windowClosing)
		this->Close();
}

void LogWindow::OnClose(wxCloseEvent& event)
{
	this->windowClosing = true;
	Imzadi::LoggingSystem::Get()->RemoveRoute("log_window");
	wxFrame::OnCloseWindow(event);
}

void LogWindow::OnTimer(wxTimerEvent& event)
{
	Imzadi::Reference<Imzadi::ReferenceCounted> routeRef;
	if (Imzadi::HandleManager::Get()->GetObjectFromHandle(this->logRouteHandle, routeRef))
	{
		auto logWindowRoute = dynamic_cast<LogWindowRoute*>(routeRef.Get());
		if (logWindowRoute)
		{
			LogWindowRoute::Message message;
			while (logWindowRoute->GrabLogMessage(message))
			{
				if ((message.flags & (IMZADI_LOG_ERROR_FLAG | IMZADI_LOG_FATAL_ERROR_FLAG)) != 0)
					this->logTextCtrl->SetDefaultStyle(wxTextAttr(*wxRED));
				else if ((message.flags & IMZADI_LOG_WARNING_FLAG) != 0)
					this->logTextCtrl->SetDefaultStyle(wxTextAttr(wxColour(128, 128, 0)));
				else
					this->logTextCtrl->SetDefaultStyle(wxTextAttr(*wxBLACK));

				this->logTextCtrl->AppendText(message.text.c_str());
			}
		}
	}
}