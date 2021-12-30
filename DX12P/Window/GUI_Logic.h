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
#include <../imGUI/imgui_impl_dx12.h>
#include <../Renderer/DX12H.h>
#include <../Window/WindowType.h>
#include <../imGUI/imgui_stdlib.h>

struct GroupData;
struct MainDX12Objects;

namespace ImGui {

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

	void LinkToSettings();
	void LinkToScene();
	void LinkToEditor();
	void LinkToPipeline();
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
			


			ImGui_ImplDX12_Init(DXM.m_device.Get(), DXM.FrameCount, DXGI_FORMAT_R8G8B8A8_UNORM, DXM.ImGUIHeap.Get(), DXM.ImGUIHeap->GetCPUDescriptorHandleForHeapStart(), DXM.ImGUIHeap->GetGPUDescriptorHandleForHeapStart());

			ImGui_ImplGlfw_InitForOther(window, true);
			RendererMade = true;

			//	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
			//	ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
			//	ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
		}

	}

	void EndRender() {
		ImGui::Render();

		DXM.SetupAndSendimGUIData();
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

struct MASTER_Setting : MASTER_Function_Inherit {

	void settingWindowSettingsMaker() {
		SettingWindowFlag = MASTER_IM_GUI_obj.WindowDrawFlagBuilder(
			false, false,
			false, false, false, false,
			false, false, false, false,
			false
		);
	}

	MASTER_Setting() {
		settingWindowSettingsMaker();
	}

	void BasicViewDraw(GroupData* GD) {
		
		if (DrawBasicWindow(this, GD, "Settings:")) {

			ImGuiStyle style_T;

			
			if (ImGui::ShowStyleSelector("Colors##Selector"))
				MASTER_IM_GUI_obj.style = &style_T;
			
			if (ImGui::Button("Save Style Theme")) 
				
				//TODO Save Style settings as new theme

			ImGui::SameLine();
			
			if (ImGui::Button("Revert Style"))
				ImGui::StyleColorsDark();
				MASTER_IM_GUI_obj.style = &ImGui::GetStyle();
				MASTER_IM_GUI_obj.style->Colors[ImGuiCol_WindowBg].w = 1.0f;

			ImGui::Separator();


			if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None)) //Litterally borrowed directly from ImGui Demo, since it works *so very well*
			{
				if (ImGui::BeginTabItem("Color"))
				{
					static ImGuiTextFilter filter;
					filter.Draw("Filter color options", ImGui::GetFontSize() * 16);

					static ImGuiColorEditFlags alpha_flags = 0;
					if (ImGui::RadioButton("Opaque", alpha_flags == ImGuiColorEditFlags_None)) { alpha_flags = ImGuiColorEditFlags_None; } ImGui::SameLine();
					if (ImGui::RadioButton("Alpha", alpha_flags == ImGuiColorEditFlags_AlphaPreview)) { alpha_flags = ImGuiColorEditFlags_AlphaPreview; } ImGui::SameLine();
					if (ImGui::RadioButton("Both", alpha_flags == ImGuiColorEditFlags_AlphaPreviewHalf)) { alpha_flags = ImGuiColorEditFlags_AlphaPreviewHalf; } ImGui::SameLine();
					HelpMarker(
						"In the color list:\n"
						"Left-click on color square to open color picker,\n"
						"Right-click to open edit options menu.");

					ImGui::BeginChild("##colors", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened);
					ImGui::PushItemWidth(-160);

					for (int i = 0; i < ImGuiCol_COUNT; i++)
					{
						const char* name = ImGui::GetStyleColorName(i);
						if (!filter.PassFilter(name))
							continue;
						ImGui::PushID(i);
						ImGui::ColorEdit4("##color", (float*)&MASTER_IM_GUI_obj.style->Colors[i], ImGuiColorEditFlags_AlphaBar);

						ImGui::SameLine(0.0f, MASTER_IM_GUI_obj.style->ItemInnerSpacing.x);
						ImGui::TextUnformatted(name);
						ImGui::PopID();
					}
					ImGui::PopItemWidth();
					ImGui::EndChild();

					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}
			//MASTER_IM_GUI_obj.style.Colors

			ImGui::End();
		}
		
	}

}MASTER_Setting_m;
struct MASTER_Scene : MASTER_Function_Inherit {

	void settingWindowSettingsMaker() {
		SettingWindowFlag = MASTER_IM_GUI_obj.WindowDrawFlagBuilder(
			false, false,
			false, false, false, false,
			false, false, false, false,
			false
		);
	}

	MASTER_Scene() {
		settingWindowSettingsMaker();
	}

	void BasicViewDraw(GroupData* GD) {
		if (DrawBasicWindow(this, GD, "Scene:")) {




			ImGui::End();
		}
	}

}MASTER_Scene_m;
struct MASTER_Editor : MASTER_Function_Inherit {

	std::map<int, std::string> AutoAddGlobals;
	std::string Globals = "a";
	std::string VsString = "a";
	std::string PsString = "a";
	std::string HsString = "a";
	std::string DsString = "a";
	std::string GsString = "a";
	std::string CsString = "a";
	std::string MeshsString = "a";
	std::string AmpsString = "a";
	std::string RayGensString = "a";
	std::string MisssString = "a";
	std::string HitsString = "a";


	void settingWindowSettingsMaker() {
		SettingWindowFlag = MASTER_IM_GUI_obj.WindowDrawFlagBuilder(
			false, false,
			false, false, false, false,
			false, false, false, false,
			false
		);
	}

	MASTER_Editor() {
		settingWindowSettingsMaker();
	}
	

	void BasicViewDraw(GroupData* GD) {
		if (DrawBasicWindow(this, GD, "Editor:")) {
/*
			Register Inputs: // hint - auto adds to AutoAddGlobals and Globals compiled code 
			[Drop Down Menu]
			[[All existing Register inputs are added here]] - by default has a full screen quad here - every item has a drop down menu to change features about them
			{
			[+] //- Hint: click to add item, edit properties in existing input menu  //clicking one of these items adds them to Existing inputs - I put funky [] to show I need to add option to change in existing inputs
			-- [Vertex Buffer] Full screen quad [VariableName:___] -- hint: x = X pos;  y = Y pos; z =  Z pos;   
			-- [Vertex Buffer] Load .obj Model [VariableName:___]
			[SEPERATOR]
			-- [float] Time  //hint - time in seconds, is a float
			-- [float2] Viewport_Resolution 
			-- [float] TimeDelta
			-- [float2] MousePosition;
			-- [float] FrameCount;
			-- [float2] MousePosition;
			-- [float2] MouseButton; //x==1 -> LB down, y==2 -> RB down
			-- [texture2d] Depth_Texture  // links regiser to depth texture [starts black] - but you flag when it copies in pipeline setup
			-- [texture2d] Current_Output // links regiser to output texture [starts black] - but you flag when it copies in pipeline setup
			-- [texture2d<float4>] Load_Texture_From_Computer [VariableName:___]
			-- [RWtexture2d<float4>] Load_Texture_From_Computer [VariableName:___]
			-- [texture2d<float4>] Blank_Texture [VariableName:___:, SIZE: X ___ Y ___]
			

			}
*/

			ImGui::Text("//HELP");
			ImGui::SameLine();
			HelpMarker("HELP:\n"
				"Hold SHIFT or use mouse to select text.\n"
				"CTRL+Left/Right to word jump.\n"
				"CTRL+A or double-click to select all.\n"
				"CTRL+X,CTRL+C,CTRL+V clipboard.\n"
				"CTRL+Z,CTRL+Y undo/redo.\n"
				"ESCAPE to revert.\n\n");
			
			ImGui::Separator();
			ImGui::Separator();
			ImGui::Separator();

			ImGui::Text("////Globals");
			ImGui::Text("//Auto_Added_Globals");
			ImGui::Text("struct Vertex{");
			ImGui::Text("float3 position : POSITION;");
			ImGui::Text("float3 normal : NORMAL;");
			ImGui::Text("float3 binormal : BINORMAL;");
			ImGui::Text("float3 tangent : TANGENT;");
			ImGui::Text("float3 uv : TEXCOORD;");
			ImGui::Text("};");
			for (auto i : AutoAddGlobals) {
				ImGui::Text(i.second.c_str());
			}
			ImGui::Text("//");
			ImGui::InputTextMultilineQuick("T0", &Globals, &TextType);
			ImGui::Text("////");

			ImGui::Separator();
			ImGui::Separator();

			ImGui::Text("////Mandatory for output Shaders");
			ImGui::Text("//VertexShaders"); //add - hint: use 'Vertex' as your input structure type. Auto_Added_Globals shows the variables features for ALL vertex buffers loaded
			ImGui::InputTextMultilineQuick("T1", &VsString, &TextType);
			ImGui::Text("//PixelShaders");
			ImGui::InputTextMultilineQuick("T2", &PsString, &TextType);
			ImGui::Text("////");

			ImGui::Separator();
			ImGui::Separator();

			ImGui::Text("////Tesslation Shader content");
			ImGui::Text("//HullShaders - TODO");
			ImGui::InputTextMultilineQuick("T3", &HsString, &TextType);
			ImGui::Text("//DomainShaders - TODO");
			ImGui::InputTextMultilineQuick("T4", &DsString, &TextType);
			ImGui::Text("////");

			ImGui::Separator();
			ImGui::Separator();

			ImGui::Text("////GeometryShader Content");
			ImGui::Text("//GeometryShaders - TODO");
			ImGui::InputTextMultilineQuick("T5", &GsString, &TextType);
			ImGui::Text("////");

			ImGui::Separator();
			ImGui::Separator();
			
			ImGui::Text("////ComputeShader Content - TODO");
			ImGui::Text("//ComputeShaders - TODO");
			ImGui::InputTextMultilineQuick("T6", &CsString, &TextType);
			ImGui::Text("////");

			ImGui::Separator();
			ImGui::Separator();

			ImGui::Text("////MeshShader Content - TODO");
			ImGui::Text("//MeshShaders - TODO");
			ImGui::InputTextMultilineQuick("T7", &MeshsString, &TextType);
			ImGui::Text("//AmplificationShaders - TODO");
			ImGui::InputTextMultilineQuick("T8", &AmpsString, &TextType);
			ImGui::Text("////");

			ImGui::Separator();
			ImGui::Separator();

			ImGui::Text("////Ray-tracing Shader Content - TODO");
			ImGui::Text("//RayGen Shaders - TODO");
			ImGui::InputTextMultilineQuick("T9", &RayGensString, &TextType);
			ImGui::Text("//Miss Shaders - TODO");
			ImGui::InputTextMultilineQuick("T10", &MisssString, &TextType);
			ImGui::Text("//Hit Shaders - TODO");
			ImGui::InputTextMultilineQuick("T11", &HitsString, &TextType);
			ImGui::Text("////");

			ImGui::Separator();
			ImGui::Separator();
			ImGui::Separator();

			ImGui::End();
		}
	}

}MASTER_Editor_m;
struct MASTER_Pipeline : MASTER_Function_Inherit {

	void settingWindowSettingsMaker() {
		SettingWindowFlag = MASTER_IM_GUI_obj.WindowDrawFlagBuilder(
			false, false,
			false, false, false, false,
			false, false, false, false,
			false
		);
	}

	MASTER_Pipeline() {
		settingWindowSettingsMaker();
	}

	void BasicViewDraw(GroupData* GD) {
		if (DrawBasicWindow(this, GD, "Pipeline:")) {
			//TODO: essentially it looks like this:
			/*
			* //draw this demo and update every frame
			START ->
			Pipeline_Name_That_Runs_1 -> 
			Pipeline_Name_That_Runs_2 -> 
			Pipeline_Name_That_Runs_3 ->
			END

			New Pipeline [+]  //button
			------------------------
			Drop down menu of All pipelines - click to open the item 
			------------------------
			Delete This Pipeline [-] //button

			Order: [INT INPUT] //Hint, 0 is the first pipeline to run - if you put 2 zeros, it sets the other copies to 1 greater, and in order does this to all; shows current number

			Name: "What is shown in drop down pipeline menu as name"

			Compute Shaders [Check Box To Enable]:
			[Drop Down Menu]
			[Choose CS] __NULL__, Order: __0__ //-hint, 0 is first
			[+] //click plus to add compute shader, works in order given, NULL means nothing runs, it is no shader
			

			[VS] [Check Box To Enable] //hint - Vertex Shader; if disabled only ComputeShader runs
			[Choose VS] _____ //Click on button to open drop down menu of all names of made vertex shaders from Editor. Click and then it puts the VS name on the ____ position
			[Choose Vertex Buffer] ___ //choose from list -- Full Screen Quad is default
			[Drop Down Menu]
			{

			}

			[HS] [Check Box To Enable] //hint - Hull Shader
			[Choose HS] _____
			[Drop Down Menu]
			{

			}

			[DS] [Check Box To Enable] //hint - Domain Shader
			[Choose DS] _____
			[Drop Down Menu]
			{

			}

			[GS] [Check Box To Enable] //hint - Geometry Shader
			[Choose GS] _____
			[Drop Down Menu]
			{

			}

			[PS] [Check Box To Enable] //hint - Pixel Shader
			[Choose PS] _____ //checks if input works with VS selected
			[Drop Down Menu]
			{
			[Copy_Depth_Texture_After]
			[Copy_Output_Texture_After]
			}

			*/



			ImGui::End();
		}
	}

}MASTER_Pipeline_m;

	void GroupData::LinkToSettings() {
		WindowType = W_SETTING;

		ToDraw = [this]() { MASTER_Setting_m.BasicViewDraw(this); };
	}
	void GroupData::LinkToScene() {
		WindowType = W_SCENE;

		ToDraw = [this]() { MASTER_Scene_m.BasicViewDraw(this); };
	}
	void GroupData::LinkToEditor() {
		WindowType = W_EDITOR;

		ToDraw = [this]() { MASTER_Editor_m.BasicViewDraw(this); };
	}
	void GroupData::LinkToPipeline() {
		WindowType = W_Pipeline;

		ToDraw = [this]() { MASTER_Pipeline_m.BasicViewDraw(this); };
	}
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
		Add_New_Win_Type([](GroupData* GDV) { MASTER_Setting_m.BasicViewDraw(GDV); }, "Settings");
		Add_New_Win_Type([](GroupData* GDV) { MASTER_Scene_m.BasicViewDraw(GDV); }, "Scene");
		Add_New_Win_Type([](GroupData* GDV) { MASTER_Editor_m.BasicViewDraw(GDV); }, "Editor");
		Add_New_Win_Type([&](GroupData* GDV) { MASTER_Pipeline_m.BasicViewDraw(GDV); }, "Pipeline"); 

	}
