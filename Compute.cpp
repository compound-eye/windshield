#include "Compute.h"
#include "VideoSource.h"
#include "View.h"
#include <iostream>


void Compute::BackgroundLoop() {
    while (! cap->imagesCaptured.quitting) {
        cv::Mat image(cap->imagesCaptured.Dequeue());
        view->SwapImageToDraw(image);
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
