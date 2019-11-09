#ifndef METRICS_H
#define METRICS_H

typedef struct metrics * metrics_t;

struct metrics {
    int concurrent_connections;
    int total_connections;
    int bytes_transfered;
};

metrics_t init_metrics();
void free_metrics(metrics_t metrics);

#endif