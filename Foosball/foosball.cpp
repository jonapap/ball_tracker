#include "foosball.h"

#include "iostream"

#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"

#include "ball_tracker.h"

cv::VideoCapture cap(0);
foosball::BallTracker tracker(cv::Scalar(0,50,50), cv::Scalar(10,255,255));

int main() {

	cv::Mat frame;

	char key;
	do {
		cap >> frame;

		tracker.update(frame);

		tracker.writeInformationOnImage(frame);

		cv::imshow("Frame", frame);

		key = (char) cv::waitKey(1);

	} while (key != 'q' && key != 27);

	return 0;
}
