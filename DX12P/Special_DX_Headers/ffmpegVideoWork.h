#pragma once

#include <filesystem>
#include <fstream>



namespace fs = std::filesystem;
//TODO use pointers because faster - but RN its late at night and i'm feeling tired
struct FFMPEG_VIDEO_WORKER {

	//
	std::string filePathName;

	std::string filePathNameffmpeg;

	std::string filePathNameStore;
	//

	//
	std::string DirPathCache = "Directory.txt";
	std::string VidPathCache = "vidPath.txt";
	std::string FFMPEGPathCache = "ffmpeg.txt";
	//
	
	//toggle bools for compute
	bool CacheVideoImages = false; // takes and caches main images in split images folder
	bool ShowOutputPicture = true;
	int NumToShow = 1;
	int ComputePassCount = 3;
	bool ComputePixelChangeFrequency = false; //means motion happened
	bool ComputeRateOfChange = true; //has 3

	bool SaveTex = true;
	bool DontBuildVideo = false;

	bool CompileLazyIntrop = false;
	bool CompileSoftBodyIntrop = true;
	


	//dir file path additions
	std::string imageSplit = "ImageSplit\\";
	std::string musicSplit = "Music\\";
	std::string musicName = "%m.mp3";
	
	std::string pixFResult = "PixelFrequencyResult\\";

	std::string EndProduct = "EndProduct\\";
	std::string MovieOutDir = "MovieOutDir\\";
	std::string MovieOutName = "out.mp4";

	//

	void DeleteVideoCache(std::string* dir) {
		std::string FileSplitAllPath = *dir + imageSplit;
		std::string FileSplitMusic = *dir + musicSplit;
		
		if (fs::is_directory(FileSplitAllPath) == false) {
			fs::create_directory(FileSplitAllPath);
		}
		if (fs::is_directory(FileSplitMusic) == false) {
			fs::create_directory(FileSplitMusic);
		}



		if (fs::is_empty(FileSplitAllPath) == false) {
			fs::remove_all(FileSplitAllPath);
		}
		if (fs::is_empty(FileSplitMusic) == false) {
			fs::remove_all(FileSplitMusic);
		}



		if (fs::is_directory(FileSplitAllPath) == false) {
			fs::create_directory(FileSplitAllPath);
		}
		if (fs::is_directory(FileSplitMusic) == false) {
			fs::create_directory(FileSplitMusic);
		}
	}

	std::string ReturnFile(std::string* p) {
		std::ifstream f(*p);
		if (f) {
			std::string o;
			while (getline(f, o)) {
				// Output the text from the file
			}
			f.close();
			return std::string(o);
		}
		else {
			return "";
		}
	}

	void WriteNewFileString(std::string* path, std::string* p) {
		std::ofstream f;
		f.open(*path);
		f << (p->c_str());
		f.close();
	}

	void LoadImages(std::string path) {

	}
	void SetPathCache(std::string* dir, std::string* vidPath, std::string* ffmpeg) {
		WriteNewFileString(&DirPathCache, dir);
		WriteNewFileString(&VidPathCache, vidPath);
		WriteNewFileString(&FFMPEGPathCache, ffmpeg);
	}
	void SplitVideo(std::string* dir, std::string* vidPath, std::string* ffmpeg) {

		SetPathCache(dir, vidPath, ffmpeg);

		DeleteVideoCache(dir);
		std::string FileSplitAllPath = *dir + imageSplit;
		std::string FileSplitMusic = *dir + musicSplit;

		std::string qs = R"(")";
		std::string command = qs+*ffmpeg+qs + " -i " + qs+*vidPath + qs + " " + qs + FileSplitAllPath + "%"+ std::to_string(ZERO_FILE_COUNT)+"d."+ fNameEnd+qs;
		std::string commandM = qs + *ffmpeg + qs + " -i " + qs + *vidPath + qs + " " + qs + FileSplitMusic + "%m.mp3" + qs;

		system((qs+command+qs).c_str());
		system((qs + commandM + qs).c_str());

		std::cout << "\nDone\n";
	}

}FFMPEG;