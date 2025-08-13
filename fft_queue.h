#ifndef FFT_QUEUE_H_
#define FFT_QUEUE_H_

#define FFT_SIZE 1024

typedef struct
{
    float samples[FFT_SIZE];
    int index;
} fft_queue;

void push_fft_queue(fft_queue *queue, float sample) {
    if (queue->index >= FFT_SIZE)
    {
        return;
    }
    
    queue->samples[queue->index++] = sample;
}

void clear_fft_queue(fft_queue *queue) {
    queue->index = 0;
}

#endif