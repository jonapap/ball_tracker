#include "ball_information.h"

#include "cmath"

namespace balltracker {

BallInformation::BallInformation(cv::Point2f center, float speedx, float speedy, float radius) : center(center), speedx(speedx), speedy(speedy), radius(radius) {
}

float BallInformation::getSpeed() const {
	return sqrt(speedx * speedx + speedy * speedy);
}

float BallInformation::getAngle() const {
	return tan(speedy / speedx);
}

}