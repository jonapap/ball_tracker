#include "foosball.h"

#include "iostream"
#include "memory"
#include "string.h"

#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"

#include "ball_tracker.h"

cv::VideoCapture cap(0);
balltracker::BallTracker tracker(cv::Scalar(0,50,50), cv::Scalar(10,255,255));

int main(int argc, char *argv[]) {

	std::unique_ptr<cv::VideoWriter> video(nullptr);

	for (int i = 0; i < argc; ++i) {
		std::cout << argv[i] << std::endl;
		if (strcmp(argv[i], "-write") == 0) {
			video = std::make_unique<cv::VideoWriter>("output.avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), cap.get(cv::CAP_PROP_FPS), cv::Size((int)cap.get(cv::CAP_PROP_FRAME_WIDTH), (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT)));
		}
	}

	cv::Mat frame;

	char key;
	do {
		cap >> frame;

		tracker.update(frame);

		tracker.writeInformationOnImage(frame);

		cv::imshow("Frame", frame);

		if (video) {
			video->write(frame);
		}
		
		key = (char) cv::waitKey(1);

	} while (key != 'q' && key != 27);

	cap.release();
	
	if (video) {
		video->release();
	}

	cv::destroyAllWindows();

	return 0;
}
