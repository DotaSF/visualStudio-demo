
/******************************/
/*        ����ƥ��Ͳ��        */
/******************************/

#include <opencv2/opencv.hpp>  
#include <iostream>  

using namespace std;
using namespace cv;

const int imageWidth = 640;                             //����ͷ�ķֱ���  
const int imageHeight = 360;
Size imageSize = Size(imageWidth, imageHeight);

Mat rgbImageL, grayImageL;
Mat rgbImageR, grayImageR;
Mat rectifyImageL, rectifyImageR;

Rect validROIL;//ͼ��У��֮�󣬻��ͼ����вü��������validROI����ָ�ü�֮�������  
Rect validROIR;

Mat mapLx, mapLy, mapRx, mapRy;     //ӳ���  
Mat Rl, Rr, Pl, Pr, Q;              //У����ת����R��ͶӰ����P ��ͶӰ����Q
Mat xyz;              //��ά����

Point origin;         //��갴�µ���ʼ��
Rect selection;      //�������ѡ��
bool selectObject = false;    //�Ƿ�ѡ�����

int blockSize = 0, uniquenessRatio = 0, numDisparities = 0;
Ptr<StereoBM> bm = StereoBM::create(16, 9);//Stereoģ����ר�����������Ӳ��

/*
���ȱ궨�õ�����Ĳ���
fx 0 cx
0 fy cy
0 0  1
*/
Mat cameraMatrixL = (Mat_<double>(3, 3) << 547.37320682070992, 0, 313.40703278703023,
	0, 544.24322148310353, 286.99617489104958,
	0, 0, 1);
//��Ӧmatlab���������궨����
Mat distCoeffL = (Mat_<double>(5, 1) << -0.51902186170644582, 0.41712502090127274, -0.0043865894471223840, 0.0031424640986452011, -0.25643488965975514);
//��ӦMatlab������i����������

Mat cameraMatrixR = (Mat_<double>(3, 3) << 544.27459114664964, 0, 292.42080812988888,
	0, 541.35927611595719, 274.38192875116101,
	0, 0, 1);
//��Ӧmatlab���������궨����

Mat distCoeffR = (Mat_<double>(5, 1) << -0.49474767445226042, 0.29520363940171490, -0.0034989229535530163, 0.0015218647234243009, -0.087720305761985445);
//��ӦMatlab����������������

Mat T = (Mat_<double>(3, 1) << -15.767802198499600, -0.054260053756460973, 0.55346203720146858);//Tƽ������
															 //��ӦMatlab����T����
Mat R = (Mat_<double>(3, 3) << 0.99942378320865377, 0.022374018497183980, -0.033868799651958839,
	-0.0021565376382288895, 0.99999473697849106, 0.0024239143411458257,
	0.033874044670168219, -0.0023494782997905872, 0.99942334826108714);//R ��ת����


	  /*****����ƥ��*****/
void stereo_match(int, void*)
{
	bm->setBlockSize(2 * blockSize + 5);     //SAD���ڴ�С��5~21֮��Ϊ��
	bm->setROI1(validROIL);
	bm->setROI2(validROIR);
	bm->setPreFilterCap(31);
	bm->setMinDisparity(0);  //��С�ӲĬ��ֵΪ0, �����Ǹ�ֵ��int��
	bm->setNumDisparities(numDisparities * 16 + 16);//�Ӳ�ڣ�������Ӳ�ֵ����С�Ӳ�ֵ֮��,���ڴ�С������16����������int��
	bm->setTextureThreshold(10);
	bm->setUniquenessRatio(uniquenessRatio);//uniquenessRatio��Ҫ���Է�ֹ��ƥ��
	bm->setSpeckleWindowSize(100);
	bm->setSpeckleRange(32);
	bm->setDisp12MaxDiff(-1);
	Mat disp, disp8;
	bm->compute(rectifyImageL, rectifyImageR, disp);//����ͼ�����Ϊ�Ҷ�ͼ
	disp.convertTo(disp8, CV_8U, 255 / ((numDisparities * 16 + 16)*16.));//��������Ӳ���CV_16S��ʽ
	reprojectImageTo3D(disp, xyz, Q, true); //��ʵ�������ʱ��ReprojectTo3D������X / W, Y / W, Z / W��Ҫ����16(Ҳ����W����16)�����ܵõ���ȷ����ά������Ϣ��
	xyz = xyz * 16;
	imshow("disparity", disp8);
}

/*****�������������ص�*****/
static void onMouse(int event, int x, int y, int, void*)
{
	if (selectObject)
	{
		selection.x = MIN(x, origin.x);
		selection.y = MIN(y, origin.y);
		selection.width = std::abs(x - origin.x);
		selection.height = std::abs(y - origin.y);
	}

	switch (event)
	{
	case EVENT_LBUTTONDOWN:   //�����ť���µ��¼�
		origin = Point(x, y);
		selection = Rect(x, y, 0, 0);
		selectObject = true;
		cout << origin << "in world coordinate is: " << xyz.at<Vec3f>(origin) << endl;
		break;
	case EVENT_LBUTTONUP:    //�����ť�ͷŵ��¼�
		selectObject = false;
		if (selection.width > 0 && selection.height > 0)
			break;
	}
}


/*****������*****/
int main()
{
	/*
	����У��
	*/
	stereoRectify(cameraMatrixL, distCoeffL, cameraMatrixR, distCoeffR, imageSize, R, T, Rl, Rr, Pl, Pr, Q, CALIB_ZERO_DISPARITY,
		0, imageSize, &validROIL, &validROIR);
	initUndistortRectifyMap(cameraMatrixL, distCoeffL, Rl, Pr, imageSize, CV_32FC1, mapLx, mapLy);
	initUndistortRectifyMap(cameraMatrixR, distCoeffR, Rr, Pr, imageSize, CV_32FC1, mapRx, mapRy);

	/*
	��ȡͼƬ
	*/
	rgbImageL = imread("D:/Binocular_Camera/left_pictures/0.jpg", CV_LOAD_IMAGE_COLOR);
	cvtColor(rgbImageL, grayImageL, CV_BGR2GRAY);
	rgbImageR = imread("D:/Binocular_Camera/right_pictures/0.jpg", CV_LOAD_IMAGE_COLOR);
	cvtColor(rgbImageR, grayImageR, CV_BGR2GRAY);

	imshow("ImageL Before Rectify", grayImageL);
	imshow("ImageR Before Rectify", grayImageR);

	/*
	����remap֮�����������ͼ���Ѿ����沢���ж�׼��
	*/
	remap(grayImageL, rectifyImageL, mapLx, mapLy, INTER_LINEAR);
	remap(grayImageR, rectifyImageR, mapRx, mapRy, INTER_LINEAR);

	/*
	��У�������ʾ����
	*/
	Mat rgbRectifyImageL, rgbRectifyImageR;
	cvtColor(rectifyImageL, rgbRectifyImageL, CV_GRAY2BGR);  //α��ɫͼ
	cvtColor(rectifyImageR, rgbRectifyImageR, CV_GRAY2BGR);

	//������ʾ
	//rectangle(rgbRectifyImageL, validROIL, Scalar(0, 0, 255), 3, 8);
	//rectangle(rgbRectifyImageR, validROIR, Scalar(0, 0, 255), 3, 8);
	imshow("ImageL After Rectify", rgbRectifyImageL);
	imshow("ImageR After Rectify", rgbRectifyImageR);

	//��ʾ��ͬһ��ͼ��
	Mat canvas;
	double sf;
	int w, h;
	sf = 600. / MAX(imageSize.width, imageSize.height);
	w = cvRound(imageSize.width * sf);
	h = cvRound(imageSize.height * sf);
	canvas.create(h, w * 2, CV_8UC3);   //ע��ͨ��

										//��ͼ�񻭵�������
	Mat canvasPart = canvas(Rect(w * 0, 0, w, h));                                //�õ�������һ����  
	resize(rgbRectifyImageL, canvasPart, canvasPart.size(), 0, 0, INTER_AREA);     //��ͼ�����ŵ���canvasPartһ����С  
	Rect vroiL(cvRound(validROIL.x*sf), cvRound(validROIL.y*sf),                //��ñ���ȡ������    
		cvRound(validROIL.width*sf), cvRound(validROIL.height*sf));
	//rectangle(canvasPart, vroiL, Scalar(0, 0, 255), 3, 8);                      //����һ������  
	cout << "Painted ImageL" << endl;

	//��ͼ�񻭵�������
	canvasPart = canvas(Rect(w, 0, w, h));                                      //��û�������һ����  
	resize(rgbRectifyImageR, canvasPart, canvasPart.size(), 0, 0, INTER_LINEAR);
	Rect vroiR(cvRound(validROIR.x * sf), cvRound(validROIR.y*sf),
		cvRound(validROIR.width * sf), cvRound(validROIR.height * sf));
	//rectangle(canvasPart, vroiR, Scalar(0, 0, 255), 3, 8);
	cout << "Painted ImageR" << endl;

	//���϶�Ӧ������
	for (int i = 0; i < canvas.rows; i += 16)
		line(canvas, Point(0, i), Point(canvas.cols, i), Scalar(0, 255, 0), 1, 8);
	imshow("rectified", canvas);

	/*
	����ƥ��
	*/
	namedWindow("disparity", CV_WINDOW_AUTOSIZE);
	// ����SAD���� Trackbar
	createTrackbar("BlockSize:\n", "disparity", &blockSize, 8, stereo_match);
	// �����Ӳ�Ψһ�԰ٷֱȴ��� Trackbar
	createTrackbar("UniquenessRatio:\n", "disparity", &uniquenessRatio, 50, stereo_match);
	// �����Ӳ�� Trackbar
	createTrackbar("NumDisparities:\n", "disparity", &numDisparities, 16, stereo_match);
	//�����Ӧ����setMouseCallback(��������, ���ص�����, �����ص������Ĳ�����һ��ȡ0)
	setMouseCallback("disparity", onMouse, 0);
	stereo_match(0, 0);

	waitKey(0);
	return 0;
}
