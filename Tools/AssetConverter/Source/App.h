#pragma once

#include <wx/app.h>
#include <wx/config.h>

class Frame;

class ConverterApp : public wxApp
{
public:
	ConverterApp();
	virtual ~ConverterApp();

	virtual bool OnInit(void) override;
	virtual int OnExit(void) override;

	Frame* GetFrame() { return this->frame; }
	wxConfig* GetConfig() { return this->config; }

	wxString MakeAssetFileReference(const wxString& assetFile);
	wxString GetAssetsRootFolder();

private:
	Frame* frame;
	wxConfig* config;
};

wxDECLARE_APP(ConverterApp);