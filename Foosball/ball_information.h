#ifndef BALL_INFORMATION_H
#define BALL_INFORMATION_H

#include "opencv2/core.hpp"

namespace foosball {

class BallInformation {
public:
	BallInformation() = default;
	BallInformation(cv::Point2f center, float speedx, float speedy, float radius);

	cv::Point2f center;
	float speedx;
	float speedy;
	float radius;

	float getAngle() const;
	float getSpeed() const;
};

}

#endif