#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
using namespace cv;
using namespace std;
Mat XUANZHUAN(Mat srcImage)
{
	Mat srcGray;
	cvtColor(srcImage, srcGray, CV_RGB2GRAY);
	const int nRows = srcGray.rows;
	const int nCols = srcGray.cols;
	//���㸵��Ҷ�任�ߴ�
	int cRows = getOptimalDFTSize(nRows);
	int cCols = getOptimalDFTSize(nCols);
	Mat sizeConvMat;
	copyMakeBorder(srcGray, sizeConvMat, 0, cRows - nRows, 0, cCols - nCols, BORDER_CONSTANT, Scalar::all(0));

	//ͼ��DFT�任
	//ͨ���齨��
	Mat groupMats[] = { Mat_<float>(sizeConvMat), Mat::zeros(sizeConvMat.size(), CV_32F) };
	Mat mergeMat;
	//����ҳ�ϳ�һ��2ͨ����mat  
	merge(groupMats, 2, mergeMat);
	//������ϳɵ�mat������ɢ����Ҷ�任��֧��ԭ�ز���������Ҷ�任���Ϊ������ͨ��1�����ʵ����ͨ��2������鲿��  
	dft(mergeMat, mergeMat);
	//�ѱ任�Ľ���ָ�����������ҳ�У������������  
	split(mergeMat, groupMats);
	//����Ҷ�仯��Ƶ�ʵķ�ֵ����ֵ���ڵ�һҳ��  
	magnitude(groupMats[0], groupMats[1], groupMats[0]);
	Mat magnitudeMat = groupMats[0].clone();
	//��һ����������ֵ��1
	magnitudeMat += Scalar::all(1);
	//����Ҷ�任�ķ���ֵ��Χ�󵽲��ʺ�����Ļ����ʾ����ֵ����Ļ����ʾΪ�׵㣬����ֵΪ�ڵ㣬  
	//�ߵ�ֵ�ı仯�޷���Ч�ֱ棬Ϊ������Ļ��͹�Գ��ߵ͵ı仯�������ԣ����ǿ����ö����߶����滻���Գ߶� 
	log(magnitudeMat, magnitudeMat);
	//��һ��
	normalize(magnitudeMat, magnitudeMat, 0, 1, CV_MINMAX);
	magnitudeMat.convertTo(magnitudeMat, CV_8UC1, 255, 0);
	//imshow("magnitudeMat2", magnitudeMat);
	//���·������ޣ�ʹ(0,0)�ƶ���ͼ�����ģ�  
	//����Ҷ�任֮ǰҪ��Դͼ�����(-1)^(x+y)���������Ļ�  
	//���ǶԸ���Ҷ�任����������Ļ�  
	int cx = magnitudeMat.cols / 2;
	int cy = magnitudeMat.rows / 2;
	Mat tmp;
	//Top-Left--Ϊÿһ�����޴���ROI  
	Mat q0(magnitudeMat, Rect(0, 0, cx, cy));
	//Top-Right  
	Mat q1(magnitudeMat, Rect(cx, 0, cx, cy));
	//Bottom-Left  
	Mat q2(magnitudeMat, Rect(0, cy, cx, cy));
	//Bottom-Right  
	Mat q3(magnitudeMat, Rect(cx, cy, cx, cy));
	//�������ޣ�(Top-Left with Bottom-Right)  
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);

	//�������ޣ���Top-Right with Bottom-Letf��  
	q1.copyTo(tmp);
	q2.copyTo(q1);
	tmp.copyTo(q2);

	Mat binaryMagnMat;
	threshold(magnitudeMat, binaryMagnMat, 155, 255, CV_THRESH_BINARY);
	vector<Vec2f> lines;
	binaryMagnMat.convertTo(binaryMagnMat, CV_8UC1, 255, 0);
	HoughLines(binaryMagnMat, lines, 1, CV_PI / 180, 100, 0, 0);
	cout << "lines.size:  " << lines.size() << endl;
	Mat houghMat(binaryMagnMat.size(), CV_8UC3);
	//���Ƽ����
	for (size_t i = 0; i < lines.size(); i++)
	{
		float rho = lines[i][0], theta = lines[i][1];
		Point pt1, pt2;
		//����任�����߱��ʽ
		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));
		line(houghMat, pt1, pt2, Scalar(0, 0, 255), 1, 8, 0);
	}
	imshow("houghMat", houghMat);
	float theta = 0;
	//����߽Ƕ��ж�
	for (size_t i = 0; i < lines.size(); i++)
	{
		float thetaTemp = lines[i][1] * 180 / CV_PI;
		if (thetaTemp > 0 && thetaTemp < 90)
		{
			theta = thetaTemp;
			break;
		}
	}
	//�Ƕ�ת��
	float angelT = nRows*tan(theta / 180 * CV_PI) / nCols;
	theta = atan(angelT) * 180 / CV_PI;
	cout << "theta: " << theta << endl;

	//ȡͼ������
	Point2f centerPoint = Point2f(nCols / 2, nRows / 2);
	double scale = 1;
	//������ת����
	Mat warpMat = getRotationMatrix2D(centerPoint, theta, scale);
	//����任
	Mat resultImage(srcGray.size(), srcGray.type());
	warpAffine(srcGray, resultImage, warpMat, resultImage.size());
	return resultImage;
}
int main()
{
	Mat srcImage2;
	Mat srcImage = imread("D:/360��ȫ���������/20161213140155704.jpg");
	resize(srcImage, srcImage2, Size(360, 360),0,0,INTER_LINEAR);
	if (srcImage.empty())
		return -1;
	imshow("srcImage2", srcImage2);
	Mat resultImage = XUANZHUAN(srcImage2);
	imshow("resultImage", resultImage);
	waitKey(0);
	return 0;

}


