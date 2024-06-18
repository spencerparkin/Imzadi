#pragma once

#include "Error.h"
#include <map>
#include <string>
#include <fstream>
#include <mutex>
#include <wx/frame.h>
#include <wx/textctrl.h>
#include <wx/button.h>

#define LOG(format, ...)		Log::Get()->Print(format, __VA_ARGS__)

class LogRoute;

class Log : public Imzadi::ErrorCapture
{
public:
	Log();
	virtual ~Log();

	virtual void Error(const std::string& errorMessage) override;

	void Print(const char* format, ...);

	bool AddRoute(const std::string& logRouteKey, LogRoute* logRoute);
	bool RemoveRoute(const std::string& logRouteKey);
	void RemoveAllRoutes();
	bool RouteExists(const std::string& logRouteKey);

	static Log* Get();

private:
	std::mutex mutex;
	std::map<std::string, LogRoute*> logRouteMap;
};

class LogRoute
{
	friend class Log;

public:
	LogRoute();
	virtual ~LogRoute();

	virtual void Print(const char* logMessage) = 0;
	virtual bool Init() = 0;
	virtual bool Shutdown() = 0;

protected:
	std::string key;
};

class LogFileRoute : public LogRoute
{
public:
	LogFileRoute();
	virtual ~LogFileRoute();

	virtual void Print(const char* logMessage) override;
	virtual bool Init() override;
	virtual bool Shutdown() override;

private:
	std::fstream fileStream;
};

class LogWindow;

class LogWindowRoute : public LogRoute
{
public:
	LogWindowRoute();
	virtual ~LogWindowRoute();

	virtual void Print(const char* logMessage) override;
	virtual bool Init() override;
	virtual bool Shutdown() override;

	void OnWindowDestroyed();

private:
	LogWindow* logWindow;
};

class LogWindow : public wxFrame
{
public:
	LogWindow(wxWindow* parent, const wxPoint& pos, const wxSize& size);
	virtual ~LogWindow();

	void SetRoute(LogWindowRoute* logWindowRoute);
	void AddLogMessage(const char* logMessage);

	void OnClearButtonPressed(wxCommandEvent& event);
	void OnDismissButtonClicked(wxCommandEvent& event);

private:
	LogWindowRoute* logWindowRoute;
	wxTextCtrl* logTextCtrl;
	wxButton* clearButton;
	wxButton* dismissButton;
};