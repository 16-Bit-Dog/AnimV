#pragma once

#include "Main_Window.h"

struct MASTER_CodeError : MASTER_Function_Inherit {
	inline static MASTER_CodeError* obj;

	virtual void WindowSettingsMaker() {
		SettingWindowFlag = MASTER_IM_GUI::obj->WindowDrawFlagBuilder(
			false, false,
			false, false, false, false,
			false, false, false, false,
			false
		);
	}

	MASTER_CodeError() {
		WindowSettingsMaker();
	}

	virtual void BasicViewDraw(GroupData* GD) {

		if (DrawBasicWindow(this, GD, "Code Error:")) {

			
		
			ImGui::End();
		}

	}

};
