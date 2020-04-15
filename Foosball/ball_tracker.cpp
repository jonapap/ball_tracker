#include "ball_tracker.h"

#include "opencv2/imgproc.hpp"

namespace {
std::vector<cv::Point>& getMaxArea(std::vector<std::vector<cv::Point>>& list) {
	std::vector<cv::Point>* maxVector = nullptr;
	double maxArea = -1;
	for (auto it = list.begin(); it != list.end(); it++) {
		double itArea = contourArea(*it);
		if (itArea >= maxArea) {
			maxVector = &(*it);
			maxArea = itArea;
		}
	}
	return *maxVector;
}
} //anonymous namespace

namespace foosball {

BallTracker::BallTracker(const cv::Scalar& low, const cv::Scalar& high) : ballLowHSV(low), ballHighHSV(high) {
}

BallInformation BallTracker::getBallInformation() const {
	return ball;
}


TrackedState BallTracker::update(const cv::Mat& frame) {
	using namespace cv;

	if (frame.empty()) {
		state = TrackedState::ERROR;
		return state;
	}

	GaussianBlur(frame, transform, Size(11,11), 0);
	
	cvtColor(transform, transform, COLOR_BGR2HSV);

	inRange(transform, ballLowHSV, ballHighHSV, threshold);

	erode(threshold, threshold, Mat(), cv::Point(-1, -1), 20);
	dilate(threshold, threshold, Mat(), cv::Point(-1, -1), 20);
	
	std::vector<std::vector<cv::Point> > contours;
	findContours(threshold, contours, RETR_TREE, CHAIN_APPROX_NONE);

	bool foundCircle = false;
	if (contours.size() > 0) {
		std::vector<cv::Point> maxBall = getMaxArea(contours);

		float radius;
		Point2f center;
		minEnclosingCircle(maxBall, center, radius);
		
		ball.center = center;
		ball.radius = radius;

		foundCircle = true;

		state = TrackedState::TRACKED;
	}
	else {
		state = TrackedState::NOT_TRACKED;
	}

	return state;
}

void BallTracker::writeInformationOnImage(cv::Mat& image) {
	cv::circle(image, ball.center, (int)ball.radius, cv::Scalar(255, 255, 255));
	cv::circle(image, ball.center, 5, cv::Scalar(255, 255, 255), cv::FILLED);
}



} //namespace foosball
