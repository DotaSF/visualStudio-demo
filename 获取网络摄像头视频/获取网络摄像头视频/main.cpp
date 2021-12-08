#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
using namespace cv;
using namespace std;
int main(int argc, char*argv[])
{
	Mat src, grayvideo, blurvideo, cannyvideo;
	VideoCapture video;
	const string address = "http://192.168.1.1:8080/?action=stream";
	if (video.open(address))
	{
		for (;;)
		{
			video >> src;
			//cvtColor(src, grayvideo, COLOR_BGR2GRAY);
			//blur(grayvideo, blurvideo, Size(7, 7));
			//Canny(blurvideo, cannyvideo, 0, 60, 3);
			//imshow("cannyvideo", cannyvideo);
			imshow("Cam", src);
			waitKey(1);
		}
	}
	return 0;
}