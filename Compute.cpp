#include "Compute.h"
#include "VideoSource.h"
#include "View.h"


void Compute::BackgroundLoop() {
    while (! cap->imagesCaptured.quitting) {
        view->imagesToDraw.Enqueue(cap->imagesCaptured.Dequeue());
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
