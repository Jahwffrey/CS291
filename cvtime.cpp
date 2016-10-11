#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;

int cameraNumber;
int width = 640;
int height = 480;
const int calibFilesNum = 11;

char *fileNames [] = {"../boards/frame0054.jpg",	
"../boards/frame0124.jpg",
"../boards/frame0174.jpg",
"../boards/frame0249.jpg",
"../boards/frame0070.jpg",
"../boards/frame0141.jpg",
"../boards/frame0190.jpg",
"../boards/frame0274.jpg",
"../boards/frame0107.jpg",
"../boards/frame0161.jpg",
"../boards/frame0228.jpg"};

CvSize boardSize = {8,6};//new CvSize(8,6);

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
	
	//Calibrate from image files checkerboard
	for(int i = 0;i < calibFilesNum;i++){
		Mat calibImage;
		calibImage = imread(fileNames[i],CV_LOAD_IMAGE_COLOR);
		vector<Point2f> corners;

		if(!calibImage.data){
			std::cout << "Failed to read " << fileNames[i] << "!!\n";
		} else {
			//imshow(fileNames[i],calibImage);
			bool foundBoard = findChessboardCorners(calibImage,boardSize,corners);
			if(foundBoard){
				for(int j = 0;j < corners.size();j++){
					for(int k = -5;k < 6;k++){
						//calibImage.at<Vec3b>(corners[j].x,corners[j].y + k) = Vec3b(1,0,0);
						//calibImage.at<uchar>((int)corners[j].x,(int)corners[j].y + k) = 128;
						//Vec3b & col = calibImage.at<Vec3b>(10+k,1);
						Vec3b & col = calibImage.at<Vec3b>(corners[j].y+k,corners[j].x);
						col[0] = 0;
						col[1] = 0;
						col[2] = 255;
					}
				}
				imshow(fileNames[i],calibImage);
			} else {
				std::cout << "Failed to find chessboard in " << fileNames[i] << "\n"; 
			}
		}
	}
	

	
	while(true){
		capture >> captFeed;
		imshow("Capture",captFeed);
	}
	

	return 0;
}
