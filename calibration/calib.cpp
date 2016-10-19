#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <math.h>

#define RADS 57.2958

using namespace cv;

char *fileNames [] = {"../boards/1.jpg",	
	"../boards/2.jpg",
	"../boards/3.jpg",
	"../boards/4.jpg",
	"../boards/5.jpg",
	"../boards/6.jpg",
	"../boards/7.jpg",
	"../boards/8.jpg",
	"../boards/9.jpg",
	"../boards/10.jpg",
	"../boards/11.jpg"};

Mat camMatrix = Mat::eye(3,3,CV_64F);
Mat distCoeffs = Mat::zeros(4,1,CV_64F);
vector< vector<Point3f> > objPoints;
vector< vector<Point2f> > imgPoints;
CvSize boardSize = {8,6};//new CvSize(8,6);
CvSize imgSize = {640,480};

int main(int argc,char** argv){
	const int calibFilesNum = 11;
	
	//Set up object points
	for(int im = 0;im < calibFilesNum;im++){
		vector<Point3f> newVect;
		objPoints.push_back(newVect);
		for(int i = 0;i < 6;i++){
			for(int j = 0;j < 8;j++){
				Point3f val = Point3f(j,i,0);
				objPoints[im].push_back(val);
			}
		}
	}
	//Calibrate from image files checkerboard
	/*for(int i = 0;i < calibFilesNum;i++){
		Mat loadImage;
		Mat calibImage;
		loadImage = imread(fileNames[i],CV_LOAD_IMAGE_COLOR);
		resize(loadImage,calibImage,imgSize);
		vector<Point2f> corners;
		bool foundBoard = findChessboardCorners(calibImage,boardSize,corners);
		if(foundBoard){
			imgPoints.push_back(corners);
			std::cout << "Found board!\n";
		} else {
			std::cout << "Did not found board in " << fileNames[i]  <<"!\n";
		}
	}*/
	
	int foundNum = 0;
	VideoCapture capt;
	capt = VideoCapture(0);
	while(foundNum < 11){
		Mat loadImage;
		Mat calibImage;
		capt >> loadImage;
		resize(loadImage,calibImage,imgSize);
		imshow("Press space to capture image",calibImage);
		vector<Point2f> corners;
		int key = waitKey(10);
	       	if(key != -1){	
			bool foundBoard = findChessboardCorners(calibImage,boardSize,corners);
			if(foundBoard){
				imgPoints.push_back(corners);
				std::cout << "Found board!\n";
				foundNum++;
			} else {
				std::cout << "Did not found board!\n";
			}
		}
	}

	vector<Mat> rvecOut;
	vector<Mat> tvecOut;
	calibrateCamera(objPoints,imgPoints,imgSize,camMatrix,distCoeffs,rvecOut,tvecOut);
	
	//Reprojection error
	double totalError;
	double avgError;
	int pointNum;
	for(int i = 0;i < calibFilesNum;i++){
		double error;
		vector<Point2f> immPts;
		projectPoints(Mat(objPoints[i]),rvecOut[i],tvecOut[i],camMatrix,distCoeffs,immPts);
		error = norm(Mat(imgPoints[i]),Mat(immPts),CV_L2);
		totalError += (error * error);
		pointNum += objPoints[i].size();
	}
	
	avgError = std::sqrt(totalError/pointNum);
	std::cout << "Total error: " << totalError << "\nAvg Error: " << avgError << "\n";
	std::cout << "Camera Intrinsics:\n";
	for(int i = 0;i < 3;i++){
		for(int j = 0;j < 3;j++){
			std::cout << camMatrix.at<double>(i,j) << "\n";
		}
	}
	for(int i = 0;i < 4;i++){
		std::cout << distCoeffs.at<double>(i,0) << "\n";
	}

	return 0;
}
