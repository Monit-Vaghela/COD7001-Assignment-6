#ifndef METRICS_H
#define METRICS_H

#include <time.h>

// Instruction count
extern long instruction_count;

// Time spent performing differnt things
extern clock_t stack_time;
extern clock_t memory_time;
extern clock_t alu_time;
extern clock_t control_time;
extern clock_t others;
extern clock_t mark_time;
extern clock_t sweep_time;


// GC Performance Metrics
extern long gc_runs;
extern long gc_marked_count;
extern long gc_swept_count;
extern long gc_objects_before;
extern long gc_objects_after;

// Total execution time
extern clock_t exec_start;
extern clock_t exec_end;

#endif
