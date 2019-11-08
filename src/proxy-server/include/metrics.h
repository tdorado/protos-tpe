#ifndef METRICS_H
#define METRICS_H

#include <stdio.h>
#include <stdlib.h>

struct metrics {
    int concurrent_connections;
    int total_connections;
    int bytes_transfered;
};

typedef struct metrics * metrics_t;

metrics_t init_metrics();
void free_metrics(metrics_t metrics);

#endif