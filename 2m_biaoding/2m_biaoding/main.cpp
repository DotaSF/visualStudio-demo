//˫Ŀ����궨 
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <opencv2/opencv.hpp>
//#include <cv.h>
//#include <cv.hpp>

using namespace std;
using namespace cv;
//����ͷ�ķֱ���
const int imageWidth = 640;
const int imageHeight = 360;
//����Ľǵ���Ŀ
const int boardWidth = 8;
//����Ľǵ���Ŀ
const int boardHeight = 7;
//�ܵĽǵ���Ŀ
const int boardCorner = boardWidth * boardHeight;
//����궨ʱ��Ҫ���õ�ͼ��֡��
const int frameNumber = 15;
//�궨��ڰ׸��ӵĴ�С ��λ��mm
const int squareSize = 19;
//�궨������ڽǵ�
const Size boardSize = Size(boardWidth, boardHeight);
Size imageSize = Size(imageWidth, imageHeight);

Mat R, T, E, F;
//R��תʸ�� Tƽ��ʸ�� E�������� F��������
vector<Mat> rvecs; //R
vector<Mat> tvecs; //T
				   //��������������Ƭ�ǵ�����꼯��
vector<vector<Point2f>> imagePointL;
//�ұ������������Ƭ�ǵ�����꼯��
vector<vector<Point2f>> imagePointR;
//��ͼ��Ľǵ��ʵ�ʵ��������꼯��
vector<vector<Point3f>> objRealPoint;
//��������ĳһ��Ƭ�ǵ����꼯��
vector<Point2f> cornerL;
//�ұ������ĳһ��Ƭ�ǵ����꼯��
vector<Point2f> cornerR;

Mat rgbImageL, grayImageL;
Mat rgbImageR, grayImageR;

Mat intrinsic;
Mat distortion_coeff;
//У����ת����R��ͶӰ����P����ͶӰ����Q
Mat Rl, Rr, Pl, Pr, Q;
//ӳ���
Mat mapLx, mapLy, mapRx, mapRy;
Rect validROIL, validROIR;
//ͼ��У��֮�󣬻��ͼ����вü������У�validROI�ü�֮�������
/*���ȱ궨�õ���������ڲξ���
fx 0 cx
0 fy cy
0  0  1
*/
Mat cameraMatrixL = (Mat_<double>(3, 3) << 546.715, 0, 312.305,
	0, 543.498, 287.576,
	0, 0, 1);
//��õĻ������
Mat distCoeffL = (Mat_<double>(5, 1) << -0.519419, 0.42124, -0.00455925, 0.00373031, -0.259622);
/*���ȱ궨�õ���������ڲξ���
fx 0 cx
0 fy cy
0  0  1
*/
Mat cameraMatrixR = (Mat_<double>(3, 3) << 545.199, 0, 293.75,
	0, 542.428, 273.781,
	0, 0, 1);
Mat distCoeffR = (Mat_<double>(5, 1) << -0.494445, 0.287989, -0.00326898, 0.000664986, -0.0775125);

/*����궨����ģ���ʵ����������*/
void calRealPoint(vector<vector<Point3f>>& obj, int boardWidth, int boardHeight, int imgNumber, int squareSize)
{
	vector<Point3f> imgpoint;
	for (int rowIndex = 0; rowIndex < boardHeight; rowIndex++)
	{
		for (int colIndex = 0; colIndex < boardWidth; colIndex++)
		{
			imgpoint.push_back(Point3f(rowIndex * squareSize, colIndex * squareSize, 0));
		}
	}
	for (int imgIndex = 0; imgIndex < imgNumber; imgIndex++)
	{
		obj.push_back(imgpoint);
	}
}



void outputCameraParam(void)
{
	/*��������*/
	/*�������*/
	FileStorage fs("intrisics.yml", FileStorage::WRITE);
	if (fs.isOpened())
	{
		fs << "cameraMatrixL" << cameraMatrixL << "cameraDistcoeffL" << distCoeffL << "cameraMatrixR" << cameraMatrixR << "cameraDistcoeffR" << distCoeffR;
		fs.release();
		cout << "cameraMatrixL=:" << cameraMatrixL << endl << "cameraDistcoeffL=:" << distCoeffL << endl << "cameraMatrixR=:" << cameraMatrixR << endl << "cameraDistcoeffR=:" << distCoeffR << endl;
	}
	else
	{
		cout << "Error: can not save the intrinsics!!!!" << endl;
	}

	fs.open("extrinsics.yml", FileStorage::WRITE);
	if (fs.isOpened())
	{
		fs << "R" << R << "T" << T << "Rl" << Rl << "Rr" << Rr << "Pl" << Pl << "Pr" << Pr << "Q" << Q;
		cout << "R=" << R << endl << "T=" << T << endl << "Rl=" << Rl << endl << "Rr" << Rr << endl << "Pl" << Pl << endl << "Pr" << Pr << endl << "Q" << Q << endl;
		fs.release();
	}
	else
	{
		cout << "Error: can not save the extrinsic parameters\n";
	}

}


int main(int argc, char* argv[])
{
	Mat img;
	int goodFrameCount = 0;
	while (goodFrameCount < frameNumber)
	{
		char filename[100];
		/*��ȡ��ߵ�ͼ��*/
		sprintf_s(filename, "D:/Binocular_Camera/left_pictures/%d.jpg", goodFrameCount + 1);

		rgbImageL = imread(filename, CV_LOAD_IMAGE_COLOR);
		imshow("chessboardL", rgbImageL);
		cvtColor(rgbImageL, grayImageL, CV_BGR2GRAY);
		/*��ȡ�ұߵ�ͼ��*/
		sprintf_s(filename, "D:/Binocular_Camera/right_pictures/%d.jpg", goodFrameCount + 1);
		rgbImageR = imread(filename, CV_LOAD_IMAGE_COLOR);
		cvtColor(rgbImageR, grayImageR, CV_BGR2GRAY);

		bool isFindL, isFindR;
		isFindL = findChessboardCorners(rgbImageL, boardSize, cornerL);
		isFindR = findChessboardCorners(rgbImageR, boardSize, cornerR);
		if (isFindL == true && isFindR == true)
		{
			cornerSubPix(grayImageL, cornerL, Size(5, 5), Size(-1, 1), TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 20, 0.1));
			drawChessboardCorners(rgbImageL, boardSize, cornerL, isFindL);
			imshow("chessboardL", rgbImageL);
			imagePointL.push_back(cornerL);

			cornerSubPix(grayImageR, cornerR, Size(5, 5), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 20, 0.1));
			drawChessboardCorners(rgbImageR, boardSize, cornerR, isFindR);
			imshow("chessboardR", rgbImageR);
			imagePointR.push_back(cornerR);

			goodFrameCount++;
			cout << "the image" << goodFrameCount << " is good" << endl;
		}
		else
		{
			cout << "the image is bad please try again" << endl;
		}
		if (waitKey(10) == 'q')
		{
			break;
		}
	}

	//����ʵ�ʵ�У�������ά���꣬����ʵ�ʱ궨���ӵĴ�С������
	calRealPoint(objRealPoint, boardWidth, boardHeight, frameNumber, squareSize);
	cout << "cal real successful" << endl;

	//�궨����ͷ
	double rms = stereoCalibrate(objRealPoint, imagePointL, imagePointR,
		cameraMatrixL, distCoeffL,
		cameraMatrixR, distCoeffR,
		Size(imageWidth, imageHeight), R, T, E, F, CALIB_USE_INTRINSIC_GUESS,
		TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 100, 1e-5));

	cout << "Stereo Calibration done with RMS error = " << rms << endl;

	stereoRectify(cameraMatrixL, distCoeffL, cameraMatrixR, distCoeffR, imageSize, R, T, Rl,
		Rr, Pl, Pr, Q, CALIB_ZERO_DISPARITY, -1, imageSize, &validROIL, &validROIR);


	//�����У��ӳ��
	initUndistortRectifyMap(cameraMatrixL, distCoeffL, Rl, Pl, imageSize, CV_32FC1, mapLx, mapLy);
	initUndistortRectifyMap(cameraMatrixR, distCoeffR, Rr, Pr, imageSize, CV_32FC1, mapRx, mapRy);

	Mat rectifyImageL, rectifyImageR;
	cvtColor(grayImageL, rectifyImageL, CV_GRAY2BGR);
	cvtColor(grayImageR, rectifyImageR, CV_GRAY2BGR);

	imshow("Recitify Before", rectifyImageL);
	cout << "��Q1�˳�..." << endl;
	//����remap֮�����������ͼ���Ѿ����沢���ж�׼��
	Mat rectifyImageL2, rectifyImageR2;
	remap(rectifyImageL, rectifyImageL2, mapLx, mapLy, INTER_LINEAR);
	remap(rectifyImageR, rectifyImageR2, mapRx, mapRy, INTER_LINEAR);
	cout << "��Q2�˳�..." << endl;

	imshow("rectifyImageL", rectifyImageL2);
	imshow("rectifyImageR", rectifyImageR2);

	outputCameraParam();

	//��ʾУ�����
	Mat canvas;
	double sf;
	int w, h;
	sf = 600. / MAX(imageSize.width, imageSize.height);
	w = cvRound(imageSize.width * sf);
	h = cvRound(imageSize.height * sf);
	canvas.create(h, w * 2, CV_8UC3);

	//��ͼ�񻭵�������
	Mat canvasPart = canvas(Rect(0, 0, w, h));
	resize(rectifyImageL2, canvasPart, canvasPart.size(), 0, 0, INTER_AREA);
	Rect vroiL(cvRound(validROIL.x*sf), cvRound(validROIL.y*sf),
		cvRound(validROIL.width*sf), cvRound(validROIL.height*sf));
	rectangle(canvasPart, vroiL, Scalar(0, 0, 255), 3, 8);

	cout << "Painted ImageL" << endl;

	//��ͼ�񻭵�������
	canvasPart = canvas(Rect(w, 0, w, h));
	resize(rectifyImageR2, canvasPart, canvasPart.size(), 0, 0, INTER_LINEAR);
	Rect vroiR(cvRound(validROIR.x*sf), cvRound(validROIR.y*sf),
		cvRound(validROIR.width*sf), cvRound(validROIR.height*sf));
	rectangle(canvasPart, vroiR, Scalar(0, 255, 0), 3, 8);

	cout << "Painted ImageR" << endl;

	//���϶�Ӧ������
	for (int i = 0; i < canvas.rows; i += 16)
		line(canvas, Point(0, i), Point(canvas.cols, i), Scalar(0, 255, 0), 1, 8);

	imshow("rectified", canvas);

	cout << "wait key" << endl;
	waitKey(0);
	return 0;
}

