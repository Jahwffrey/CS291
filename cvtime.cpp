#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;

int cameraNumber;
int width = 640;
int height = 480;
const int calibFileNum = 11;

char *fileNames [] = {"frame0054.jpg",	
"frame0124.jpg",
"frame0174.jpg",
"frame0249.jpg",
"frame0070.jpg",
"frame0141.jpg",
"frame0190.jpg",
"frame0274.jpg",
"frame0107.jpg",
"frame0161.jpg",
"frame0228.jpg"};

int main(int argc,char** argv){
	if(argc != 2){
		std::cout << "Please provide camera number\n";
		return 0;
	} else {
		cameraNumber = atoi(argv[1]);
	}

	VideoCapture capture;
	Mat captFeed;
	capture = VideoCapture(cameraNumber);
	
	//Calibrate checkerboard

	while(true){
		capture >> captFeed;
		imshow("Capture",captFeed);
	}

	return 0;
}
