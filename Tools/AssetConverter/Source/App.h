#pragma once

#include <wx/app.h>

class Frame;

class ConverterApp : public wxApp
{
public:
	ConverterApp();
	virtual ~ConverterApp();

	virtual bool OnInit(void) override;
	virtual int OnExit(void) override;

	Frame* GetFrame() { return this->frame; }

	wxString MakeAssetFileReference(const wxString& assetFile);
	wxString GetAssetsRootFolder();

private:
	Frame* frame;
};

wxDECLARE_APP(ConverterApp);