#include "ball_tracker.h"

#include "opencv2/imgproc.hpp"

#include "iostream"


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

namespace balltracker {

std::chrono::duration<float> BallTracker::extrapolatingDuration = std::chrono::milliseconds(1500);
std::chrono::duration<float> BallTracker::speedDuration = std::chrono::milliseconds(500);


BallTracker::BallTracker(const cv::Scalar& low, const cv::Scalar& high, double inputFPS) : ballLowHSV(low), ballHighHSV(high), maxBufferSize((int)(inputFPS*speedDuration.count())) {
}

BallInformation BallTracker::getBallInformation() const {
	if (ballBuffer.empty()) {
		return BallInformation();
	}

	return ballBuffer.back();
}

BallInformation BallTracker::getExtrapolatedBallInformation() const {
	if (state != TrackedState::EXTRAPOLATING) {
		return BallInformation();
	}

	BallInformation extraBall = ballBuffer.back();

	auto now = std::chrono::steady_clock::now();
	auto deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(now - extraBall.time);

	extraBall.center.x = extraBall.center.x + extraBall.speedx * deltaTime.count();
	extraBall.center.y = extraBall.center.y + extraBall.speedy * deltaTime.count();

	extraBall.time = now;

	return extraBall;
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

	currentBall.time = now;


	state = TrackedState::NOT_TRACKED;
	if (!ballBuffer.empty()) {

		const BallInformation& pastBall = ballBuffer.front();

		auto deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(now - pastBall.time);
		std::cout << deltaTime.count() << std::endl;

		if (foundBall) {

			currentBall.speedx = (currentBall.center.x - pastBall.center.x) / deltaTime.count();
			currentBall.speedy = (currentBall.center.y - pastBall.center.y) / deltaTime.count();

			state = TrackedState::TRACKED;
		}
		else if (deltaTime < extrapolatingDuration) {
			state = TrackedState::EXTRAPOLATING;
		} 
		else {
			std::queue<BallInformation> empty;
			std::swap(ballBuffer, empty); //empty the buffer
			
			state = TrackedState::NOT_TRACKED;
		}
	}

	if (foundBall) {
		ballBuffer.push(currentBall);

		state = TrackedState::TRACKED;
	}

	if (ballBuffer.size() > maxBufferSize) {
		ballBuffer.pop();
	}

	return state;
}

void BallTracker::writeInformationOnImage(cv::Mat& image) {
	if (state == TrackedState::TRACKED) {
		BallInformation& ball = ballBuffer.back();
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
