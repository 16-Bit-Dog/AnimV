#pragma once
#include <vector>
#include <string>
#include <functional>
#include <map>

struct GroupData;

UINT GLOBAL_WINDOW_ID = 0;

UINT GLOBAL_WINDOW_ID_I() {

	GLOBAL_WINDOW_ID += 1;
	return GLOBAL_WINDOW_ID - 1;
}
enum WIN_TYPE { // not used for now, but important to share all "scene" data for the most part
	W_SETTING = 1,
	W_SCENE = 2,
	W_EDITOR = 3,
	W_Pipeline = 4,
};
std::map<int, int> Win_Type_ID_Vector; // associated with Win_Type index for ease of adding more Win_type's
std::map<int, std::string> Win_Type_Name_Vector; // associated with Win_Type index for ease of adding more Win_type's
std::map<int, std::function<void(GroupData*)>> Win_Type_Initialization_Vector_Of_Type; //if int is equal, you run the function inside

int Add_New_Win_Type(std::function<void(GroupData*)> InitializationFunction, std::string name) { //returns new win type
	for (int i = Win_Type_ID_Vector.size() + 1; i > 0; i++) {
		if (Win_Type_ID_Vector.count(i) == 0) {

			Win_Type_ID_Vector[i] = i;
			Win_Type_Name_Vector[i] = name;
			Win_Type_Initialization_Vector_Of_Type[i] = InitializationFunction;

			return i;
		}
	}

	return 0;
}
//