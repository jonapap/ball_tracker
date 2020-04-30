#ifndef BALL_TRACKER_H
#define BALL_TRACKER_H

#include "chrono"
#include "queue"

#include "opencv2/core.hpp"

#include "ball_information.h"

namespace balltracker {

enum class TrackedState {
	TRACKED, //ball is visible and all information is available
	EXTRAPOLATING, //ball is not visible but information is available from previous data
	NOT_TRACKED, //ball is not visible and information is not available or invalid
	ERROR //when in this state, accessing other propreties of the object will not be defined (e.g. when no frames are available)
};

class BallTracker {
public:
	/*
	low and high represents the range of the ball's color in HSV
	inputFPS is the fps of the input video and is used to have a 
	specific duration when calculating the speed
	*/
	BallTracker(const cv::Scalar& low, const cv::Scalar& high, double inputFPS = 30);

	/*
	Designed to be called in a loop.
	This function will track the current position of the ball
	and update this instance to reflect the new information.

	The function returns a TrackedState variable representing
	the current state of the object. See TrackedState definition
	for more information on each state.
	*/
	TrackedState update(const cv::Mat& frame);

	/*
	Will draw a circle around the detected ball and add various
	information about the tracking state. This information
	is intendend to be shown to the user.
	*/
	void writeInformationOnImage(cv::Mat& image);


	/*
	Returns the ball information when the ball was last seen.
	If this is called when the state is NOT_TRACKED, it will
	return a default initialized BallInformation object
	*/
	BallInformation getBallInformation() const;

	/*
	Uses previous ball information to extrapolate where the ball 
	is at the present time. This function must be called when the
	state is EXTRAPOLATING, else invalid information will be returned
	*/
	BallInformation getExtrapolatedBallInformation() const;
	
private:
	//Lower and higher threshold in HSV for the ball
	cv::Scalar ballLowHSV;
	cv::Scalar ballHighHSV;

	cv::Mat transform, threshold;

	//Current tracking state
	TrackedState state = TrackedState::NOT_TRACKED;

	//Last known information of the ball
	//BallInformation ball;
	//bool previouslySeen = false;

	std::queue<BallInformation> ballBuffer;
	int maxBufferSize;

	//How long to extrapolate the ball's position before switching to NOT_TRACKED
	static std::chrono::duration<float> extrapolatingDuration;

	//Approximate duration between consecutive frames to get the ball's. This number
	//assumes that the constructor's inputFPS is correct and that update() will be called at this FPS
	static std::chrono::duration<float> speedDuration;
};

}
#endif // BALL_TRACKER_H
