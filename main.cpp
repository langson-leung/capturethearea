//
// Requirements:
// 1) OpenCV 2.4+.
//
// NOTE: 
// 1) Add "C:\opencv\build\include" to the %AdditionalIncludeDirectories%
//    in Project Property.
// 2) Add "C:\opencv\build\bin\Debug" to %PATH% Environment Variables.
//
#include <opencv2/opencv.hpp>
#pragma comment(lib, "D:/OpenCV2.4.4/opencv/build/x86/vc10/lib/opencv_core244d.lib")
#pragma comment(lib, "D:/OpenCV2.4.4/opencv/build/x86/vc10/lib/opencv_imgproc244d.lib")
#pragma comment(lib, "D:/OpenCV2.4.4/opencv/build/x86/vc10/lib/opencv_highgui244d.lib")

static cv::Mat frameMat;
static enum MenuAction {
	MENU_ACTION_REF,
	MENU_ACTION_OBJ,
	MENU_ACTION_ROI,
} menuAction = MENU_ACTION_ROI;
static cv::Rect refButtonRect = cv::Rect( 40, 10, 15, 15);
static cv::Rect objButtonRect = cv::Rect( 95, 10, 15, 15);
static cv::Rect roiButtonRect = cv::Rect(145, 10, 15, 15);
static cv::Scalar refButtonColor = cv::Scalar(222, 222, 222);
static cv::Scalar objButtonColor = cv::Scalar(222, 222, 222);
static cv::Scalar roiButtonColor = cv::Scalar(222, 222, 222);

static enum MouseAction {
	MOUSE_ACTION_START = 0,
	MOUSE_ACTION_DRAG,
	MOUSE_ACTION_FINISH
} mouseAction = MOUSE_ACTION_FINISH;
static cv::Point ltPoint, rbPoint;

int RGBImgThr(cv::Mat& roiMat)
{
	//if (NULL == roiMat)
	//{
	//	return -1;
	//}

	double b0 = refButtonColor[0];
	double g0 = refButtonColor[1];
	double r0 = refButtonColor[2];
	double b1 = objButtonColor[0];
	double g1 = objButtonColor[1];
	double r1 = objButtonColor[2];

	double th = r1 * r1 + g1 * g1 + b1 * b1 - r0 * r0 - g0 * g0 - b0 * b0;
	double aR = 2 * (r1 - r0);
	double bG = 2 * (g1 - g0);
	double cB = 2 * (b1 - b0);
	for (int r = 0; r < roiMat.rows; ++r) 
	{
		for (int c = 0; c < roiMat.cols; ++c) 
		{
			uchar *p = roiMat.ptr(r, c);
			if(aR * p[2] + bG * p[1] + cB * p[0] >= th)
			{
				p[0] = 255;
				p[1] = 255;
				p[2] = 255;
			}
			else
			{
				p[0] = 0;
				p[1] = 0;
				p[2] = 0;
			}
		}
	}

	return 0;
}

double RGB2HPoint(cv::Scalar rgbPoint)
{
	double r = rgbPoint[2];
	double g = rgbPoint[1];
	double b = rgbPoint[0];

	double max = (r > g) ? (r > b ? r : b) : (g > b ? g : b);
	double min = (r < g) ? (r < b ? r : b) : (g < b ? g : b);
	double v = max;
	double s = v - min;
	double z = (s < 0.0000001 && s > -0.0000001) ? 1.0 : 0.0;
	s += z;
	double h = ((r == v) ? ((g-b)/s) : ( (g == v) ? (2+(b-r)/s) : (4+(r-g)/s )))/6; 
	h = h < -0.0000001 ? (h + 1) : h;
	h = ( (z == 1.0) ? 0.0 : 1.0 ) * h;

	return h;
}

int RGB2HImgThr(cv::Mat& roiMat)
{
	double h0 = RGB2HPoint(refButtonColor);
	double h1 = RGB2HPoint(objButtonColor);

	double th = 0.5 * (h0 + h1);
	if (h0 < h1)
	{
		for (int r = 0; r < roiMat.rows; ++r) 
		{
			for (int c = 0; c < roiMat.cols; ++c) 
			{
				uchar *p = roiMat.ptr(r, c);
				cv::Scalar pointScalar = cv::Scalar(p[0], p[1], p[2]);
				double h = RGB2HPoint(pointScalar);

				if(h >= th)
				{
					p[0] = 255;
					p[1] = 255;
					p[2] = 255;
				}
				else
				{
					p[0] = 0;
					p[1] = 0;
					p[2] = 0;
				}
			}
		}
	} 
	else
	{
		for (int r = 0; r < roiMat.rows; ++r) 
		{
			for (int c = 0; c < roiMat.cols; ++c) 
			{
				uchar *p = roiMat.ptr(r, c);
				cv::Scalar pointScalar = cv::Scalar(p[0], p[1], p[2]);
				double h = RGB2HPoint(pointScalar);

				if(h <= th)
				{
					p[0] = 255;
					p[1] = 255;
					p[2] = 255;
				}
				else
				{
					p[0] = 0;
					p[1] = 0;
					p[2] = 0;
				}
			}
		}
	}
	

	return 0;
}


static void onMouse(int ev, int x, int y, int flags, void*)
{
	switch (ev) {
	case cv::EVENT_LBUTTONDOWN:
		if (mouseAction == MOUSE_ACTION_FINISH) {
			mouseAction = MOUSE_ACTION_START;
		}
		break;
	case cv::EVENT_MOUSEMOVE:
		if (flags & cv::EVENT_FLAG_LBUTTON) {
			if (mouseAction == MOUSE_ACTION_START) {
				ltPoint.x = x;
				ltPoint.y = y;
				mouseAction = MOUSE_ACTION_DRAG;
			} else if (mouseAction == MOUSE_ACTION_DRAG) {
				rbPoint.x = x;
				rbPoint.y = y;
			}
		}
		break;
	case cv::EVENT_LBUTTONUP:
		if (refButtonRect.contains(cv::Point(x, y))) {
			menuAction = MENU_ACTION_REF;
		} else if (objButtonRect.contains(cv::Point(x, y))) {
			menuAction = MENU_ACTION_OBJ;
		} else if (roiButtonRect.contains(cv::Point(x, y))) {
			ltPoint = cv::Point(0, 0);
			rbPoint = cv::Point(frameMat.cols-1, frameMat.rows-1);
			menuAction = MENU_ACTION_ROI;
		} else if (mouseAction == MOUSE_ACTION_DRAG) {
			cv::Mat roiMat = frameMat(cv::Rect(ltPoint, rbPoint));
            if (roiMat.total() == 0) break;
			//cv::imshow("ROI", roiMat);
			
			int rgb[3] = {0, };
			for (int r = 0; r < roiMat.rows; ++r) {
				for (int c = 0; c < roiMat.cols; ++c) {
					uchar *p = roiMat.ptr(r, c);
					rgb[0] += p[0];
					rgb[1] += p[1];
					rgb[2] += p[2];
				}
			}
			rgb[0] /= roiMat.total();
			rgb[1] /= roiMat.total();
			rgb[2] /= roiMat.total();
			
			if (menuAction == MENU_ACTION_REF) {
				refButtonColor = cv::Scalar(rgb[0], rgb[1], rgb[2]);
			} else if (menuAction == MENU_ACTION_OBJ) {
				objButtonColor = cv::Scalar(rgb[0], rgb[1], rgb[2]);
			}
			mouseAction = MOUSE_ACTION_FINISH;
		}
		break;
	default: break;
	}
	std::cout << "event = " << flags << ", x = " << x << ", y = " << y << std::endl;
}

int main(int argc, char *argv[])
{
	int err = 0;

	cv::VideoCapture *vCapture = new cv::VideoCapture(0);
	if (!vCapture->isOpened()) {
		err = -9999;
		goto hell;
	}
	cv::namedWindow("CaptureScene");
	cv::namedWindow("ROI");
	cv::setMouseCallback("CaptureScene", onMouse, &frameMat);
	
	vCapture->read(frameMat);
	ltPoint = cv::Point(0, 0);
	rbPoint = cv::Point(frameMat.cols-1, frameMat.rows-1);

	do {
		vCapture->read(frameMat);

        if (menuAction == MENU_ACTION_ROI) {
            cv::Mat roiMat = frameMat(cv::Rect(ltPoint, rbPoint));
            if (roiMat.total() != 0) {
				//*****①RGB转HSV*****
				cv::Mat global = roiMat.clone();
				cv::cvtColor(roiMat, global, CV_BGR2HSV);

				//*****②提取H分量*****
				cv::Mat hImg(global.rows, global.cols, CV_8UC1);
				cv::Mat sImg = hImg.clone();
				cv::Mat vImg = hImg.clone();
				cv::vector<cv::Mat> hsv;
				hsv.push_back(hImg);
				hsv.push_back(sImg);
				hsv.push_back(vImg);
				cv::split(global, hsv);

				//*****③计算分割阈值*****
				double h0 = RGB2HPoint(refButtonColor);
				double h1 = RGB2HPoint(objButtonColor);
				double th = 255 * 0.5 * (h0 + h1);

				//*****④二值化*****
				int type = (h0 < h1) ? 0 : 1;
				cv::threshold(hImg, hImg, th, 255, type);

				//*****⑤抚平毛刺：闭运算+开运算*****
				cv::Mat bwImg = hImg.clone();
				cv::Mat element3(3, 3, CV_8U, cv::Scalar(1));
				morphologyEx(hImg, bwImg, CV_MOP_CLOSE, element3);
				morphologyEx(bwImg, hImg, CV_MOP_OPEN, element3);

				//*****⑥提取边界*****
				bwImg = hImg.clone();
				cv::vector<cv::vector<cv::Point>> contours;
				cv::findContours(bwImg, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
				cv::drawContours(roiMat, contours, -1, cv::Scalar(222, 0, 0));
				cv::imshow("ROI", hImg);


				//cv::Mat global = roiMat.clone();
				//cvtColor(roiMat, global, CV_BGR2HSV);
				//cv::Mat hImg(global.rows, global.cols, CV_8UC1);
				//cv::Mat sImg = hImg.clone();
				//cv::Mat vImg = hImg.clone();
				//cvSplit(&global, &hImg, &sImg, &vImg, NULL);
				////cvSplit(&global, &hImg, &sImg, &vImg, 0);
				//cv::threshold(hImg, hImg, 100, 255, CV_THRESH_BINARY_INV);
				////RGBImgThr(global);
				////RGB2HImgThr(global);
				//cv::imshow("ROI", hImg);
                //cv::threshold(roiMat, global, 100, 255, CV_THRESH_BINARY_INV);
                ////cv::adaptiveThreshold(roiMat, global, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, 25, 10); 
                //cv::imshow("ROI", global);
            }
        }
		
		cv::putText(frameMat, "REF", cv::Point(10, 20), 
			cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(222, 222, 222));
		cv::rectangle(frameMat, refButtonRect, refButtonColor, 
			menuAction == MENU_ACTION_REF ? 4 : CV_FILLED);
		cv::putText(frameMat, "OBJ", cv::Point(65, 20), 
			cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(222, 222, 222));
		cv::rectangle(frameMat, objButtonRect, objButtonColor, 
			menuAction == MENU_ACTION_OBJ ? 4 : CV_FILLED);
		cv::putText(frameMat, "ROI", cv::Point(120, 20), 
			cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(222, 222, 222));
		cv::rectangle(frameMat, roiButtonRect, roiButtonColor, 
			menuAction == MENU_ACTION_ROI ? 4 : CV_FILLED);

		if (menuAction == MENU_ACTION_ROI || mouseAction == MOUSE_ACTION_DRAG) {
			cv::rectangle(frameMat, ltPoint, rbPoint, cv::Scalar(10, 255, 10));
		}

		cv::imshow("CaptureScene", frameMat);
	} while (cv::waitKey(30) != 0x1b);

hell:
	cv::destroyAllWindows();
	if (vCapture) delete vCapture;
	return err;
}