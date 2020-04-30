#ifndef BALL_INFORMATION_H
#define BALL_INFORMATION_H

#include "chrono"

#include "opencv2/core.hpp"

namespace balltracker {

/*
Holds information about the ball.
Please note that units represents the ball on the image.
*/
class BallInformation {
public:
	BallInformation() = default;
	BallInformation(cv::Point2f center, float speedx, float speedy, float radius);

	cv::Point2f center;
	float speedx = 0; //speed is in distance/second
	float speedy = 0;
	float radius = 0;

	//holds when this information has been taken
	std::chrono::time_point<std::chrono::steady_clock> time;

	float getAngle() const;
	float getSpeed() const;
};

}

#endif