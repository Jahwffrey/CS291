#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;

int cameraNumber;
int width = 640;
int height = 480;

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
	
	while(true){
		capture >> captFeed;
		imshow("Capture",captFeed);
	}
	return 0;
}
