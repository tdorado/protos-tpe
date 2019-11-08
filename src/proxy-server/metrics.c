#include "include/metrics.h"

metrics_t init_metrics() {
  metrics_t ret = (metrics_t)malloc(sizeof(*ret));

  if (ret == NULL){
    perror("Error initializing metrics");
    exit(EXIT_FAILURE);
  }

  return ret;
}

void free_metrics(metrics_t metrics){
    free(metrics);
}