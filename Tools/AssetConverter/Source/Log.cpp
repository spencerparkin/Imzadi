#include "Log.h"
#include "App.h"
#include "Frame.h"
#include <stdarg.h>
#include <filesystem>
#include <format>
#include <ctime>
#include <wx/sizer.h>

//---------------------------------- Log ----------------------------------

Log::Log()
{
}

/*virtual*/ Log::~Log()
{
	this->RemoveAllRoutes();
}

void Log::Print(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	char messageBuffer[1024];
	vsprintf_s(messageBuffer, sizeof(messageBuffer), format, args);

	std::time_t time = std::time(nullptr);
	char timeBuffer[128];
	std::strftime(timeBuffer, sizeof(timeBuffer), "%T", std::localtime(&time));

	char logLineBuffer[1024];
	sprintf_s(logLineBuffer, sizeof(logLineBuffer), "%s: %s\n", timeBuffer, messageBuffer);

	for (auto pair : this->logRouteMap)
	{
		LogRoute* logRoute = pair.second;
		logRoute->Print(logLineBuffer);
	}

	va_end(args);
}

bool Log::AddRoute(const std::string& logRouteKey, LogRoute* logRoute)
{
	if (this->logRouteMap.find(logRouteKey) != this->logRouteMap.end())
	{
		delete logRoute;
		return false;
	}

	if (!logRoute->Init())
	{
		delete logRoute;
		return false;
	}

	logRoute->key = logRouteKey;
	this->logRouteMap.insert(std::pair<std::string, LogRoute*>(logRouteKey, logRoute));
	return true;
}

bool Log::RemoveRoute(const std::string& logRouteKey)
{
	std::map<std::string, LogRoute*>::iterator iter = this->logRouteMap.find(logRouteKey);
	if (iter == this->logRouteMap.end())
		return false;

	LogRoute* logRoute = iter->second;
	logRoute->Shutdown();
	delete logRoute;
	this->logRouteMap.erase(iter);
	return true;
}

void Log::RemoveAllRoutes()
{
	while (this->logRouteMap.size() > 0)
	{
		std::map<std::string, LogRoute*>::iterator iter = this->logRouteMap.begin();
		LogRoute* logRoute = iter->second;
		logRoute->Shutdown();
		delete logRoute;
		this->logRouteMap.erase(iter);
	}
}

LogRoute* Log::FindRoute(const std::string& logRouteKey)
{
	std::map<std::string, LogRoute*>::iterator iter = this->logRouteMap.find(logRouteKey);
	if (iter == this->logRouteMap.end())
		return nullptr;

	return iter->second;
}

/*static*/ Log* Log::Get()
{
	static Log log;
	return &log;
}

//---------------------------------- LogRoute ----------------------------------

LogRoute::LogRoute()
{
}

/*virtual*/ LogRoute::~LogRoute()
{
}

//---------------------------------- LogFileRoute ----------------------------------

LogFileRoute::LogFileRoute()
{
}

/*virtual*/ LogFileRoute::~LogFileRoute()
{
}

/*virtual*/ void LogFileRoute::Print(const char* logMessage)
{
	this->fileStream.write(logMessage, ::strlen(logMessage));
	this->fileStream.flush();
}

/*virtual*/ bool LogFileRoute::Init()
{
	std::filesystem::path logFilePath = std::filesystem::current_path() / std::filesystem::path("Log.txt");

	if (std::filesystem::exists(logFilePath))
		std::filesystem::remove(logFilePath);

	this->fileStream.open(logFilePath, std::ios::out);
	return this->fileStream.is_open();
}

/*virtual*/ bool LogFileRoute::Shutdown()
{
	this->fileStream.close();
	return true;
}

//---------------------------------- LogWindowRoute ----------------------------------

LogWindowRoute::LogWindowRoute()
{
	this->logWindow = nullptr;
}

/*virtual*/ LogWindowRoute::~LogWindowRoute()
{
}

/*virtual*/ void LogWindowRoute::Print(const char* logMessage)
{
	if (this->logWindow)
		this->logWindow->AddLogMessage(logMessage);
}

/*virtual*/ bool LogWindowRoute::Init()
{
	Frame* frame = wxGetApp().GetFrame();
	wxPoint pos = frame->GetPosition();
	pos.x += frame->GetSize().GetWidth();
	this->logWindow = new LogWindow(frame, pos, wxSize(600, 400));
	this->logWindow->SetRoute(this);
	this->logWindow->Show();
	return true;
}

/*virtual*/ bool LogWindowRoute::Shutdown()
{
	if (this->logWindow)
	{
		this->logWindow->SetRoute(nullptr);
		this->logWindow->Close();
		this->logWindow = nullptr;	// Note that wxWidgets will delete the window pointer itself.
	}

	return true;
}

void LogWindowRoute::OnWindowDestroyed()
{
	this->logWindow = nullptr;
	Log::Get()->RemoveRoute(this->key);
}

//---------------------------------- LogWindow ----------------------------------

LogWindow::LogWindow(wxWindow* parent, const wxPoint& pos, const wxSize& size) : wxFrame(parent, wxID_ANY, "Log", pos, size, wxCAPTION | wxCLIP_CHILDREN)
{
	this->logWindowRoute = nullptr;

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
}

/*virtual*/ LogWindow::~LogWindow()
{
	if (this->logWindowRoute)
		this->logWindowRoute->OnWindowDestroyed();
}

void LogWindow::SetRoute(LogWindowRoute* logWindowRoute)
{
	this->logWindowRoute = logWindowRoute;
}

void LogWindow::OnClearButtonPressed(wxCommandEvent& event)
{
	this->logTextCtrl->Clear();
}

void LogWindow::OnDismissButtonClicked(wxCommandEvent& event)
{
	this->Close();
}

void LogWindow::AddLogMessage(const char* logMessage)
{
	wxString logMessageStr(logMessage);

	wxString logMessageLower = logMessageStr.Lower();
	if (logMessageLower.find("error") != std::string::npos || logMessageLower.find("failed") != std::string::npos)
		this->logTextCtrl->SetDefaultStyle(wxTextAttr(*wxRED));
	else
		this->logTextCtrl->SetDefaultStyle(wxTextAttr(*wxBLACK));

	this->logTextCtrl->AppendText(logMessageStr);
}