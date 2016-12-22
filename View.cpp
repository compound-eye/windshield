#include "View.h"


View::View() {
    pthread_mutex_init(&dataMutex, NULL);
}

View::~View() {
    pthread_mutex_destroy(&dataMutex);
}

void View::SwapDataToDraw(Data& data) {
    Data x = data;

    pthread_mutex_lock(&dataMutex);
    data = dataToDraw;
    dataToDraw = x;
    pthread_mutex_unlock(&dataMutex);
}
