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

vector< vector<Point2f> > imgPoints;
vector< vector<Point3f> > objPoints; 

/*[48][3] =

	{0,0,0},{1,0,0},{2,0,0},{3,0,0},{4,0,0},{5,0,0},{6,0,0},{7,0,0},
	{0,1,0},{1,1,0},{2,1,0},{3,1,0},{4,1,0},{5,1,0},{6,1,0},{7,1,0},
	{0,2,0},{1,2,0},{2,2,0},{3,2,0},{4,2,0},{5,2,0},{6,2,0},{7,2,0},
	{0,3,0},{1,3,0},{2,3,0},{3,3,0},{4,3,0},{5,3,0},{6,3,0},{7,3,0},
	{0,4,0},{1,4,0},{2,4,0},{3,4,0},{4,4,0},{5,4,0},{6,4,0},{7,4,0},
	{0,5,0},{1,5,0},{2,5,0},{3,5,0},{4,5,0},{5,5,0},{6,5,0},{7,5,0}
};*/


int main(int argc,char** argv){
	if(argc != 2){
		std::cout << "Please provide camera number\n";
		return 0;
	} else {
		cameraNumber = atoi(argv[1]);
	}

	//InputArray inptArr = InputArray(objPoints);
	Size calibSize;
	VideoCapture capture;
	Mat captFeed;
	capture = VideoCapture(cameraNumber);

	//Set up object points
	/*for(int i = 0;i < 6;i++){
		vector<Point3f> newVect;
		objPoints.push_back(newVect);
		for(int j = 0;j < 8;j++){
			Point3f val = Point3f(j,i,0);
			objPoints[i].push_back(val);
		}
	}*/

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
	
	/*for(int i = 0;i < 48;i++){
		std::cout << "("<< objPoints[0][i].x << "," << objPoints[0][i].y << "," << objPoints[0][i].z << ")"; 
		if(i % 8 == 7) std::cout << "\n";
	}*/

	//Calibrate from image files checkerboard
	for(int i = 0;i < calibFilesNum;i++){
		Mat calibImage;
		calibImage = imread(fileNames[i],CV_LOAD_IMAGE_COLOR);
		vector<Point2f> corners;

		if(!calibImage.data){
			std::cout << "Failed to read " << fileNames[i] << "!!\n";
		} else {
			if(i == 0){
				calibSize = calibImage.size();
			}

			//imshow(fileNames[i],calibImage);
			bool foundBoard = findChessboardCorners(calibImage,boardSize,corners);
			if(foundBoard){
				imgPoints.push_back(corners);
				/*for(int j = 0;j < corners.size();j++){
					for(int k = -5;k < 6;k++){
						Vec3b & col = calibImage.at<Vec3b>(corners[j].y+k,corners[j].x);
						col[0] = 0;   //b
						col[1] = 0;   //g
						col[2] = 255; //r
					}
				}
				imshow(fileNames[i],calibImage);*/
			} else {
				std::cout << "Failed to find chessboard in " << fileNames[i] << "\n"; 
			}
		}
	}

	/*for(int i = 0;i < 48;i++){
		std::cout << "("<< imgPoints[0][i].x << "," << imgPoints[0][i].y << ",0)"; 
		if(i % 8 == 7) std::cout << "\n";
	}*/

	///float camMatrix[3][3];
	//OutputArray camMatrix = create(3,3,float);
	Mat camMatrix = Mat::eye(3,3,CV_64F);
	Mat distCoeffs = Mat::zeros(8,1,CV_64F);
	vector<Mat> rvecOut;
	vector<Mat> tvecOut;
	calibrateCamera(objPoints,imgPoints,calibSize,camMatrix,distCoeffs,rvecOut,tvecOut);

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
		pointNum += imgPoints[i].size();
	}
	
	avgError = std::sqrt(totalError/pointNum);
	std::cout << "Total error: " << totalError << "\nAvg Error: " << avgError << "\n";
	
	while(true){
		capture >> captFeed;
		imshow("Capture",captFeed);
	}
	

	return 0;
}
