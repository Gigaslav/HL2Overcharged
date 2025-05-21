#ifndef OVRPOSTPROCESSING_H
#define OVRPOSTPROCESSING_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/PropertyPage.h"
#include "vgui_controls/CheckButton.h"
#include "filesystem.h"

#include <vector>

class COVROptions;
class COVRPostProcessing : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(COVRPostProcessing, vgui::PropertyPage);

public:
	COVRPostProcessing(vgui::Panel *parent, int num);

	virtual void OnResetData();
	void OnApplyChanges(int num);

	MESSAGE_FUNC(OnRadioButtonChecked, "RadioButtonChecked");

private:

	std::vector<vgui::CheckButton*> ppButtons;
	std::vector<vgui::TextEntry*> pTexts;
	std::vector<vgui::Slider*> pPPSliders;
};


#endif 