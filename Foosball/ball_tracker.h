#ifndef BALL_TRACKER_H
#define BALL_TRACKER_H

#include "chrono"

#include "opencv2/core.hpp"

#include "ball_information.h"

namespace foosball {

enum class TrackedState {
	TRACKED, //ball is visible and all information is available
	EXTRAPOLATING, //ball is not visible but information is available from previous data
	NOT_TRACKED, //ball is not visible and information is not available or invalid
	ERROR //when in this state, accessing other propreties of the object will not be defined (e.g. when no frames are available)
};

class BallTracker {
public:
	BallTracker(const cv::Scalar& low, const cv::Scalar& high);

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
	Returns the ball information when the ball was last seen
	*/
	BallInformation getBallInformation() const;

	/*
	Uses previous ball information to extrapolate where the ball 
	is at the present time. This function can be called regardless 
	of the current state, but it may or may not give relevent data 
	if the state is not EXTRAPOLATING.
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
	BallInformation ball;
	bool previouslySeen = false;

	//How long to extrapolate the ball's position before switching to NOT_TRACKED
	static std::chrono::milliseconds extrapolatingDuration;
};

}
#endif // BALL_TRACKER_H
