#include "Compute.h"
#include "VideoSource.h"
#include "View.h"
#include <iostream>


void Compute::BackgroundLoop() {
    while (! cap->imagesCaptured.quitting) {
        cv::Mat image = cap->imagesCaptured.Dequeue();

        if (view->imagesToDraw.size < view->imagesToDraw.Capacity()) {
            view->imagesToDraw.Enqueue(image);
        } else {
	    //std::cerr << "Compute dropped frame." << std::endl;
        }
    }
}

static void* ComputeThread(void* compute) {
    ((Compute*)compute)->BackgroundLoop();
    return 0;
}

void Compute::Start() {
    pthread_create(&thread, NULL, ComputeThread, this);
}

void Compute::Stop() {
    cap->imagesCaptured.Quit();
    pthread_join(thread, NULL);
}
