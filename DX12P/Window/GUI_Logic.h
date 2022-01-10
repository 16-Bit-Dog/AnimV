#pragma once

#ifdef _WIN32
#define IM_NEWLINE  "\r\n"
#else
#define IM_NEWLINE  "\n"
#endif

#include <vector>
#include <string>
#include <functional>
#include <map>
#include <cstdint>
#include <../GLFW/glfw3.h>
#include <../GLFW/glfw3native.h>
#include <../imGUI/imgui.h>
#include <../imGUI/imgui_impl_glfw.h>
#include <../Renderer/DX11H.h>
#include <../Window/WindowType.h>
#include <../imGUI/imgui_stdlib.h>

//
#include <../imGUI/ImGUIFileDialog/ImGuiFileDialog.h>
#include <../Special_DX_Headers/ffmpegVideoWork.h>
//

struct GroupData;
struct MainDX11Objects;

namespace ImGui {
	static void HelpMarker(const char* desc)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	void InputTextMultilineQuick(std::string Tag, std::string* S, ImGuiInputTextFlags* flags) {
		ImGui::InputTextMultiline(Tag.c_str(), S, ImVec2(ImGui::GetWindowWidth() * 0.8f, 200), *flags, NULL, nullptr);
	}

}

//TODO: also allow switching what tab is what - option called - "Switch Tab" where it switches tab
struct GroupData {
	//the idea of this struct:
	/*
	when ever it moves - the pointer is passed
	if associated tab is closed, this is deleted :TODO:

	has map of uint to int which holds values of button unique id's from MASTER_*GUI_type* -- this value has the goal to set per window values to allow seperate views of things:
	ex: expand a menu in 1 MASTER_SETTING tab, but in another have that menu not expanded. This allows Unity style syncronous work to be faster when looking at shader to make
	//if ID does not exist it defaults it to 0 int value, and sets the UINT so it now exists
	*/

	std::map<int, bool> NewWindowCreationHandle;

	UINT ID = 0;

	int WindowType = 0;

	std::function<void()> ToDraw;

	std::map < UINT, int> BidI; //button id associated with for each i'm gui 

	bool DontKillWindowBool = true;

	int LinkBasedOnInt(int Input);

	GroupData* MakeNewMainWindowCheckAndDo(int WindowType);
};

struct MASTER_IM_GUI {
	ImGuiContext* GUIContext; // global for global use
	bool RendererMade = false;
	GLFWwindow* window;
	
	ImGuiStyle* style;
	
	void SetGUIWindow(GLFWwindow* w) {
		window = w;
	}

	ImGuiWindowFlags WindowDrawFlagBuilder(bool no_titlebar_t, bool no_scrollbar_t,
		bool no_menu_t, bool no_move_t, bool no_resize_t, bool no_collapse_t,
		bool no_close_t, bool no_nav_t, bool no_background_t, bool no_bring_to_front_t,
		bool unsaved_document_t) {

		bool no_titlebar = no_titlebar_t;
		bool no_scrollbar = no_scrollbar_t;
		bool no_menu = no_menu_t;
		bool no_move = no_move_t;
		bool no_resize = no_resize_t;
		bool no_collapse = no_collapse_t;
		bool no_close = no_close_t;
		bool no_nav = no_nav_t;
		bool no_background = no_background_t;
		bool no_bring_to_front = no_bring_to_front_t;
		bool unsaved_document = unsaved_document_t;

		ImGuiWindowFlags window_flags = 0;
		if (no_titlebar)        window_flags |= ImGuiWindowFlags_NoTitleBar;
		if (no_scrollbar)       window_flags |= ImGuiWindowFlags_NoScrollbar;
		if (!no_menu)           window_flags |= ImGuiWindowFlags_MenuBar;
		if (no_move)            window_flags |= ImGuiWindowFlags_NoMove;
		if (no_resize)          window_flags |= ImGuiWindowFlags_NoResize;
		if (no_collapse)        window_flags |= ImGuiWindowFlags_NoCollapse;
		if (no_nav)             window_flags |= ImGuiWindowFlags_NoNav;
		if (no_background)      window_flags |= ImGuiWindowFlags_NoBackground;
		if (no_bring_to_front)  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
		if (unsaved_document)   window_flags |= ImGuiWindowFlags_UnsavedDocument;

		return window_flags;
	}

	void SetAndCreateimGUIContext(GLFWwindow* w) {
		SetGUIWindow(w);
		if (RendererMade == false) {
			GUIContext = ImGui::CreateContext();
			ImGuiIO& GUIio = ImGui::GetIO();
			GUIio.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
			GUIio.ConfigFlags |= ImGuiConfigFlags_DockingEnable;


			style = &ImGui::GetStyle();
		
			//style->WindowRounding = 0.0f;
			//
			

			ImGui_ImplGlfw_InitForOther(window, true);

			DXM.ImGUIInit();


			RendererMade = true;

			//	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
			//	ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
			//	ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
		}

	}

	void EndRender() {
	//	ImGui::Render();
		DXM.NewImGUIDat = true;
		DXM.DrawLogic(true);
	}

	
	//Settings that adjust with settings tab TODO:
	
	//Global Settings:
	/*[All have Toggle option for USE_GLOBAL[Check box]*/
	//Text: 
	//TextSize
	//TextColor
	//TextFont - TTF file loader
	//
	//
	//Window BackGround Color
	//
	
	//Performance Settings:
	//
	//
	//
	//

}MASTER_IM_GUI_obj;


//TODO: make ID for object values to read from GroupData* GD, and add to map accordingly, default 0, else read from (And use consistant BUTTON ID) for new buttons

struct MASTER_Function_Inherit {
	ImGuiInputTextFlags TextType = ImGuiInputTextFlags_AllowTabInput;

	ImGuiWindowFlags SettingWindowFlag; //DO NOT USE VARS FROM THIS STRUCT, ONLY FROM ALIAS OF THIS STRUCT

	static void HelpMarker(const char* desc)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}
	void DrawTopMenuBar(MASTER_Function_Inherit* WF, GroupData* GD) {
		if (ImGui::BeginMenuBar()) {

			if (ImGui::BeginMenu("New Window"))
			{
				for (const auto &i : Win_Type_Name_Vector) {
					if (GD->NewWindowCreationHandle.count(i.first) == 0) { //new window bool set to false if not in map of new windows to be made bool map
						GD->NewWindowCreationHandle[i.first] = false;
					}
					ImGui::MenuItem(i.second.c_str(), NULL, &GD->NewWindowCreationHandle[i.first]);
				}
				
				ImGui::EndMenu();
			}
		
			
			ImGui::EndMenuBar();
		}
	}

	bool DrawBasicWindow(MASTER_Function_Inherit* WF, GroupData* GD, std::string WindowName) {
		if(GD->DontKillWindowBool){
			if (!ImGui::Begin((WindowName+std::to_string(GD->ID)).c_str() /*This adds garbage to the end, but it is unique garbage which I use as an invisible 'id' to seperate closing tabs and such*/
				, &GD->DontKillWindowBool, WF->SettingWindowFlag)) {
				ImGui::End();
				return false;
			}
			else {
				DrawTopMenuBar(WF, GD);

				//ImGui::Text("This is some useful text.");
				//ImGui::SameLine();
				//ImGui::Text("counter = empty");
				//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
				//ImGui::End();
				return true;
			}
		}
	}

};
/*
STEPS TO MAKE NEW IMGUI WINDOW HANDLE FOR PROGRAM:
1. make struct to inherit from MASTER_Function_Inherit with simmilar procedure to "MASTER_Setting" and associated
2. call before using your new window struct Add_New_Win_Type([](GroupData* GDV) { MASTER_NEW-WINDOW-NAME_m.BasicViewDraw(GDV); }, "NEW-WINDOW-NAME");
3. store the returned int as the window type ID
and everything now works :thumbsup: -- TODO: Still need to make this easier by allowing easy acsess to the StartUpFillVecs() and adding custom extension 
*/

struct MASTER_Window : MASTER_Function_Inherit {

	bool pathCache = false;

	bool FileExplorerOpen = false;

	bool HaveMP4 = false;

	bool Haveffmpeg = false;

	bool HaveDirOutput = false;

	bool LoadedImages = false;

	ImGuiFileDialog fd;
	
	ImGuiFileDialog fmd;

	ImGuiFileDialog dvod; //decode vid output dir


	void settingWindowSettingsMaker() {
		SettingWindowFlag = MASTER_IM_GUI_obj.WindowDrawFlagBuilder(
			false, false,
			false, false, false, false,
			false, false, false, false,
			false
		);
	}

	MASTER_Window() {
		settingWindowSettingsMaker();
	}

	void BasicViewDraw(GroupData* GD) {
		
		if (DrawBasicWindow(this, GD, "Settings:")) {

			ImGuiStyle style_T;
			
			if (pathCache == false) {
				HaveDirOutput = true;				
				HaveMP4 = true;
				Haveffmpeg = true;
				FFMPEG.filePathNameStore = FFMPEG.ReturnFile(&FFMPEG.DirPathCache);
				if (FFMPEG.filePathNameStore == "") {
					HaveDirOutput = false;
				}
				FFMPEG.filePathName = FFMPEG.ReturnFile(&FFMPEG.VidPathCache);
				if (FFMPEG.filePathName == "") {
					HaveMP4 = false;
				}
				FFMPEG.filePathNameffmpeg = FFMPEG.ReturnFile(&FFMPEG.FFMPEGPathCache);
				if (FFMPEG.filePathNameffmpeg == "") {
					Haveffmpeg = false;
				}
				pathCache = true;
			}
			

			if (ImGui::Button("Open MP4 File"))
				fd.OpenDialog("ChooseFileDlgKey", "Choose File", ".mp4", ".");
			ImGui::SameLine();
			if (HaveMP4 == false) {
				ImGui::Text("missing");
			}
			else {
				ImGui::Text(FFMPEG.filePathName.c_str());
			}
			// display
			if (fd.Display("ChooseFileDlgKey"))
			{
				FileExplorerOpen = true;
				// action if OK
				if (fd.IsOk() == true)
				{
					FFMPEG.filePathName = fd.GetFilePathName();
					HaveMP4 = true;
					// action
				}
				// close
				fd.Close();
			}


			
			ImGui::Separator();

			if (ImGui::Button("Open ffmpeg.exe"))
				fmd.OpenDialog("ChooseFileDlgKey", "Choose File", ".exe", ".");
			
			ImGui::SameLine();
			ImGui::HelpMarker("https://www.ffmpeg.org/download.html - you only need ffmpeg.exe");
			ImGui::SameLine();
			if (Haveffmpeg == false) {
				ImGui::Text("missing");
			}
			else {
				ImGui::Text(FFMPEG.filePathNameffmpeg.c_str());
			}
			// display
			if (fmd.Display("ChooseFileDlgKey"))
			{
				FileExplorerOpen = true;
				// action if OK
				if (fmd.IsOk() == true)
				{
					FFMPEG.filePathNameffmpeg = fmd.GetFilePathName();
					Haveffmpeg = true;
					// action
				}
				// close
				fmd.Close();
			}

			ImGui::Separator();

			if (Haveffmpeg && HaveMP4) {

				if (ImGui::Button("Directory For Video Frame Output"))
					dvod.OpenDialog("ChooseDirDlgKey", "Choose Dir", 0, ".");

				ImGui::SameLine();
				ImGui::HelpMarker("https://www.ffmpeg.org/download.html - you only need ffmpeg.exe");
				ImGui::SameLine();
				if (HaveDirOutput == false) {
					ImGui::Text("missing");
				}
				else {
					ImGui::Text(FFMPEG.filePathNameStore.c_str());
				}
				// display
				if (dvod.Display("ChooseDirDlgKey"))
				{
					FileExplorerOpen = true;
					// action if OK
					if (dvod.IsOk() == true)
					{
						FFMPEG.filePathNameStore = dvod.GetFilePathName()+"\\";
						HaveDirOutput = true;
						// action
					}
					// close
					dvod.Close();
				}
			}

			ImGui::Separator();

			if (HaveDirOutput) {
				
				if (ImGui::Button("Split Video Into Seperate Frames")) {
					FFMPEG.SplitVideo(&FFMPEG.filePathNameStore, &FFMPEG.filePathName, &FFMPEG.filePathNameffmpeg);
					LoadedImages = false;
				}
				ImGui::SameLine();
				ImGui::HelpMarker("if you previously have split the video you DO NOT need to split it again");
				
				ImGui::Separator();
				ImGui::Separator();
				if (ImGui::Button("Delete Video Cache")) {
					FFMPEG.DeleteVideoCache(&FFMPEG.filePathNameStore);
				}

				ImGui::Separator();
				ImGui::Separator();
				if (ImGui::Button("Set Paths To Cache")) {
					FFMPEG.SetPathCache(&FFMPEG.filePathNameStore, &FFMPEG.filePathName, &FFMPEG.filePathNameffmpeg);
				}
				ImGui::Separator();
				ImGui::Separator();

				ImGui::Checkbox("Save Video Images\n", &FFMPEG.SaveTex);
				
				ImGui::HelpMarker("Split images first, then compute pass, then compile video");


				ImGui::Checkbox("Cache Video Images\n", &FFMPEG.CacheVideoImages);
				ImGui::SameLine();
				ImGui::HelpMarker("caches split images\nExpensive on ram but should be faster");
				/*
				ImGui::Checkbox("Show Output During Work\n", &FFMPEG.ShowOutputPicture);
				ImGui::SameLine();
				ImGui::HelpMarker("ONLY shows furthest up check box\n(priority system)");
				*/

				ImGui::InputInt("Compute output to show", &FFMPEG.NumToShow);
				ImGui::SameLine();
				ImGui::HelpMarker("no compute output means that it just shows the video split into a video");

				ImGui::InputInt("Enter Sample Count", &DXM.SampleSize);
				ImGui::SameLine();
				ImGui::HelpMarker("+- frame count to check for data (should be 3-5)");


				ImGui::Checkbox("Compute Pixel Frequency Trend - num 0\n", &FFMPEG.ComputePixelChangeFrequency);
				ImGui::Checkbox("Compute Pixel Rate Of Change - num 1\n", &FFMPEG.ComputeRateOfChange);
				ImGui::Indent(); ImGui::Checkbox("Compute dist. accurate\n", &FFMPEG.ComputeDistAccurate);
				
				if (ImGui::Button("Run Compute Pass")) {
					DXM.StartComputePass();
				}

				ImGui::SameLine();
				ImGui::HelpMarker("Compute Data needed for compiling video");


				ImGui::Separator();
				ImGui::Separator();
				
				ImGui::InputInt("Enter Current Output FPS", &DXM.CurrentFrameRate);
				ImGui::SameLine();
				ImGui::HelpMarker("The Frame rate the program assumes the original video as\n\nFind the frame rate (round it) in\nRight_Click_File->Properties->Details->Frame_Rate");

				ImGui::InputInt("Enter Target Output FPS", &DXM.TargetFrameRate);
				
				if (ImGui::Button("Compile Video")) {
					if (DXM.VideoCreationPass() == false) {
						std::cout << "failed compiling";
					}
				}
				ImGui::Checkbox("Don't Build Video (Debug)", &FFMPEG.DontBuildVideo);

				ImGui::HelpMarker("Higher check box has more priority");

				ImGui::Checkbox("Lazy Video Frame Interpolation", &FFMPEG.CompileLazyIntrop);
				ImGui::Checkbox("Soft Body Video Frame Interpolation", &FFMPEG.CompileSoftBodyIntrop);

				ImGui::SameLine();
				
				

				//TODO sampleSize change input - must be less than 20
//TODO: check box for compute passes to-do
//TODO: add buttons to start compute pass
//TODO: compile video from frame data (uses specific folders + refrence material) to duplicate frames

			}
			
			ImGui::End();
		}
		//			ImGui::InputTextMultilineQuick("T0", &Globals, &TextType);
	}

}MASTER_Window_m;


int GroupData::LinkBasedOnInt(int Input) {
	if (Win_Type_ID_Vector.count(Input)) {
		WindowType = Input;
		ToDraw = [this,Input]() {
			Win_Type_Initialization_Vector_Of_Type[Input](this); 
		};
		return 1;
	}
	throw("Link Based On Int Fail");
	return 0; //won't return since this is a FATAL flaw if it does not work
}

GroupData* GroupData::MakeNewMainWindowCheckAndDo(int WindowType = 1) {
	//enter window type to check if needs to be made - this is individual
	if (Win_Type_ID_Vector.count(WindowType)) {

		if (NewWindowCreationHandle[WindowType] == true) {
				
			GroupData* TmpGUI_Win = new GroupData;
			TmpGUI_Win->ID = GLOBAL_WINDOW_ID_I();
			TmpGUI_Win->LinkBasedOnInt(WindowType);

			NewWindowCreationHandle[WindowType] = false;

			return TmpGUI_Win;
		}

		return nullptr;
	}

	return nullptr;
}

void StartUpFillVecs() {
	Add_New_Win_Type([](GroupData* GDV) { MASTER_Window_m.BasicViewDraw(GDV); }, "Window");

}
