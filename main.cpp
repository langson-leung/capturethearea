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
#pragma comment(lib, "C:/opencv/build/lib/Debug/opencv_core249d.lib")
#pragma comment(lib, "C:/opencv/build/lib/Debug/opencv_imgproc249d.lib")
#pragma comment(lib, "C:/opencv/build/lib/Debug/opencv_highgui249d.lib")

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
                cv::Mat global;
                cv::threshold(roiMat, global, 100, 255, CV_THRESH_BINARY_INV);
                //cv::adaptiveThreshold(roiMat, global, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, 25, 10); 
                cv::imshow("ROI", global);
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