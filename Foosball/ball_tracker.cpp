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

std::chrono::milliseconds BallTracker::extrapolatingDuration = std::chrono::seconds(1);


BallTracker::BallTracker(const cv::Scalar& low, const cv::Scalar& high) : ballLowHSV(low), ballHighHSV(high) {
}

BallInformation BallTracker::getBallInformation() const {
	return ball;
}

BallInformation BallTracker::getExtrapolatedBallInformation() const {
	BallInformation currentBall = ball;

	auto now = std::chrono::steady_clock::now();
	auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - ball.time);

	currentBall.center.x = ball.center.x + ball.speedx * deltaTime.count();
	currentBall.center.y = ball.center.y + ball.speedy * deltaTime.count();

	currentBall.time = now;

	return currentBall;
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

	bool foundBall = false;
	BallInformation currentBall;
	if (contours.size() > 0) {
		std::vector<cv::Point> maxBall = getMaxArea(contours);

		minEnclosingCircle(maxBall, currentBall.center, currentBall.radius);

		foundBall = true;
	}

	//next part calculates the ball speed based on previous information

	auto now = std::chrono::steady_clock::now();
	auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - ball.time);

	state = TrackedState::NOT_TRACKED;

	if (foundBall && previouslySeen) { //if we just found the ball and we have seen it previously
		currentBall.speedx = (currentBall.center.x - ball.center.x) / deltaTime.count();
		currentBall.speedy = (currentBall.center.y - ball.center.y) / deltaTime.count();

		state = TrackedState::TRACKED;
	}
	else if (!foundBall && previouslySeen) { //if the ball has not been found but it was seen previously
		if (deltaTime < extrapolatingDuration){ //if we are within the time limit
			state = TrackedState::EXTRAPOLATING;
		}
		else {
			previouslySeen = false;
			state = TrackedState::NOT_TRACKED;
		}
	}

	if (foundBall) {
		currentBall.time = now;
		ball = currentBall;

		previouslySeen = true;
	}

	return state;
}

void BallTracker::writeInformationOnImage(cv::Mat& image) {
	if (state == TrackedState::TRACKED) {
		cv::circle(image, ball.center, (int)ball.radius, cv::Scalar(255, 255, 255));
		cv::circle(image, ball.center, 5, cv::Scalar(255, 255, 255), cv::FILLED);
	
		cv::putText(image, "Tracking State : TRACKED", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0));
	}
	else if (state == TrackedState::EXTRAPOLATING) {
		BallInformation extraBall = getExtrapolatedBallInformation();

		cv::circle(image, extraBall.center, (int)extraBall.radius, cv::Scalar(255, 255, 255));
		cv::circle(image, extraBall.center, 5, cv::Scalar(255, 255, 255), cv::FILLED);


		cv::putText(image, "Tracking State : EXTRAPOLATING", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0));
	}
	else if (state == TrackedState::NOT_TRACKED) {
		cv::putText(image, "Tracking State : NOT_TRACKED", cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255));
	}


}



} //namespace foosball
