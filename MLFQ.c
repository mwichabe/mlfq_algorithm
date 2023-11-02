#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>

#define MAX_NUM_PROCESS 10
#define MAX_QUEUE_SIZE 10
#define MAX_PROCESS_NAME 5
#define MAX_GANTT_CHART 300

#define KEYWORD_TQ0 "tq0"
#define KEYWORD_TQ1 "tq1"
#define KEYWORD_PROCESS_TABLE_SIZE "process_table_size"
#define KEYWORD_PROCESS_TABLE "process_table"

#define SPACE_CHARS " \t"

struct Process {
    char name[MAX_PROCESS_NAME];
    int arrival_time;
    int burst_time;
    int remain_time;
};

void process_init(struct Process* p, char name[MAX_PROCESS_NAME], int arrival_time, int burst_time) {
    strcpy(p->name, name);
    p->arrival_time = arrival_time;
    p->burst_time = burst_time;
    p->remain_time = 0;
}

void process_table_print(struct Process* p, int size) {
    int i;
    printf("Process\tArrival\tBurst\n");
    for (i = 0; i < size; i++) {
        printf("%s\t%d\t%d\n", p[i].name, p[i].arrival_time, p[i].burst_time);
    }
}

struct GanttChartItem {
    char name[MAX_PROCESS_NAME];
    int duration;
};

void gantt_chart_update(struct GanttChartItem chart[MAX_GANTT_CHART], int* n, char name[MAX_PROCESS_NAME], int duration) {
    int i = *n;
    if (i > 0 && strcmp(chart[i - 1].name, name) == 0) {
        chart[i - 1].duration += duration;
    } else {
        strcpy(chart[i].name, name);
        chart[i].duration = duration;
        *n = i + 1;
    }
}

void gantt_chart_print(struct GanttChartItem chart[MAX_GANTT_CHART], int n) {
    int t = 0;
    int i = 0;
    printf("Gantt Chart = ");
    printf("%d ", t);
    for (i = 0; i < n; i++) {
        t = t + chart[i].duration;
        printf("%s %d ", chart[i].name, t);
    }
    printf("\n");
}

int tq0 = 0, tq1 = 0;
int process_table_size = 0;
struct Process process_table[MAX_NUM_PROCESS];

int is_blank(char* line) {
    char* ch = line;
    while (*ch != '\0') {
        if (!isspace(*ch))
            return 0;
        ch++;
    }
    return 1;
}

int is_skip(char* line) {
    if (is_blank(line))
        return 1;
    char* ch = line;
    while (*ch != '\0') {
        if (!isspace(*ch) && *ch == '#')
            return 1;
        ch++;
    }
    return 0;
}

void parse_tokens(char** argv, char* line, int* numTokens, char* delimiter) {
    int argc = 0;
    char* token = strtok(line, delimiter);
    while (token != NULL) {
        argv[argc++] = token;
        token = strtok(NULL, delimiter);
    }
    *numTokens = argc;
}

void parse_input() {
    FILE* fp = stdin;
    char* line = NULL;
    ssize_t nread;
    size_t len = 0;
    char* two_tokens[2];
    int numTokens = 0;
    char equal_plus_spaces_delimiters[5] = "";
    char process_name[MAX_PROCESS_NAME];
    int process_arrival_time = 0;
    int process_burst_time = 0;

    strcpy(equal_plus_spaces_delimiters, "=");
    strcat(equal_plus_spaces_delimiters, SPACE_CHARS);

    while ((nread = getline(&line, &len, fp)) != -1) {
        if (is_skip(line) == 0) {
            line = strtok(line, "\n");

            if (strstr(line, KEYWORD_TQ0)) {
                parse_tokens(two_tokens, line, &numTokens, equal_plus_spaces_delimiters);
                if (numTokens == 2) {
                    sscanf(two_tokens[1], "%d", &tq0);
                }
            } else if (strstr(line, KEYWORD_TQ1)) {
                parse_tokens(two_tokens, line, &numTokens, equal_plus_spaces_delimiters);
                if (numTokens == 2) {
                    sscanf(two_tokens[1], "%d", &tq1);
                }
            } else if (strstr(line, KEYWORD_PROCESS_TABLE_SIZE)) {
                parse_tokens(two_tokens, line, &numTokens, equal_plus_spaces_delimiters);
                if (numTokens == 2) {
                    sscanf(two_tokens[1], "%d", &process_table_size);
                }
            } else if (strstr(line, KEYWORD_PROCESS_TABLE)) {
                for (int i = 0; i < process_table_size; i++) {
                    getline(&line, &len, fp);
                    line = strtok(line, "\n");
                    sscanf(line, "%s %d %d", process_name, &process_arrival_time, &process_burst_time);
                    process_init(&process_table[i], process_name, process_arrival_time, process_burst_time);
                }
            }
        }
    }
}

void print_parsed_values() {
    printf("%s = %d\n", KEYWORD_TQ0, tq0);
    printf("%s = %d\n", KEYWORD_TQ1, tq1);
    printf("%s = \n", KEYWORD_PROCESS_TABLE);
    process_table_print(process_table, process_table_size);
}

// Implemented the MLFQ algorithm
void mlfq() {
    struct GanttChartItem gantt_chart[MAX_GANTT_CHART];
    int chart_size = 0;

    // Initialize queues
    struct Process queue0[MAX_QUEUE_SIZE];
    struct Process queue1[MAX_QUEUE_SIZE];
    int queue0_size = 0;
    int queue1_size = 0;

    // Initialize time
    int current_time = 0;

    // Add processes to the initial queues
    for (int i = 0; i < process_table_size; i++) {
        if (process_table[i].arrival_time == current_time) {
            if (queue0_size < MAX_QUEUE_SIZE) {
                queue0[queue0_size] = process_table[i];
                queue0_size++;
            }
        }
    }

    // Process the queues until all processes are executed
    while (queue0_size > 0 || queue1_size > 0) {
        // Execute processes in QUEUE 0
        while (queue0_size > 0) {
            struct Process current_process = queue0[0];
            if (current_process.burst_time <= tq0) {
                // Process completes within time quantum tq0
                current_time += current_process.burst_time;
                current_process.burst_time = 0;
                gantt_chart_update(gantt_chart, &chart_size, current_process.name, current_process.burst_time);
            } else {
                // Process is preempted
                current_time += tq0;
                current_process.burst_time -= tq0;
                gantt_chart_update(gantt_chart, &chart_size, current_process.name, tq0);
                // Ensure we don't exceed queue1's capacity
                if (queue1_size < MAX_QUEUE_SIZE) {
                    queue1[queue1_size] = current_process;
                    queue1_size++;
                }
            }
            // Remove the executed process from QUEUE 0
            for (int i = 0; i < queue0_size - 1; i++) {
                queue0[i] = queue0[i + 1];
            }
            queue0_size--;
        }

        // Execute processes in QUEUE 1
        while (queue1_size > 0) {
            struct Process current_process = queue1[0];
            if (current_process.burst_time <= tq1) {
                // Process completes within time quantum tq1
                current_time += current_process.burst_time;
                current_process.burst_time = 0;
                gantt_chart_update(gantt_chart, &chart_size, current_process.name, current_process.burst_time);
            } else {
                // Process is preempted
                current_time += tq1;
                current_process.burst_time -= tq1;
                gantt_chart_update(gantt_chart, &chart_size, current_process.name, tq1);
                // Move the process back to QUEUE 1 (no promotion in this simplified version)
                if (queue1_size < MAX_QUEUE_SIZE) {
                    queue1[queue1_size] = current_process;
                    queue1_size++;
                }
            }
            // Remove the executed process from QUEUE 1
            for (int i = 0; i < queue1_size - 1; i++) {
                queue1[i] = queue1[i + 1];
            }
            queue1_size--;
        }

        // Add new processes to QUEUE 0 that have arrived
        for (int i = 0; i < process_table_size; i++) {
            if (process_table[i].arrival_time == current_time) {
                if (queue0_size < MAX_QUEUE_SIZE) {
                    queue0[queue0_size] = process_table[i];
                    queue0_size++;
                }
            }
        }
    }

    // Display the Gantt chart
    gantt_chart_print(gantt_chart, chart_size);
}

int main() {
    parse_input();
    print_parsed_values();
    mlfq();
    return 0;
}
