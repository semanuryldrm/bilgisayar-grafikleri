#ifndef FIFO_H_
#define FIFO_H_

#define FIFO_SIZE 20

typedef struct 
{
    float x1, y1, x2, y2;
} Raylib_Line;

Raylib_Line rline[FIFO_SIZE];

void push(Raylib_Line *fifo, Raylib_Line item);


#ifdef FIFO_H_IMPLEMENTATION_

void push(Raylib_Line *fifo, Raylib_Line item) {
    for (size_t i = FIFO_SIZE - 1; i > 0; i--)
    {
        fifo[i] = fifo[i - 1];
    }
    fifo[0] = item;
}

#endif


#endif
