/* Multi-Threaded Mentoring Center
 * ANTHEA ABREO - AXA210122, HIMA SAI KIRAN PRUDHIVI - HXP220011
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#include <stdint.h>

// Uncomment for debug print statements
//#define DEBUG

sem_t chair_mutex, q_mutex, queue_fill, waiting_students_mutex, MLPQ_mutex;
sem_t students_tutored_now_mutex, kill_tutor_thread_mutex;
int total_chairs, max_help, chairs_avail, total_students, waiting_students, total_requests, tutor_rr, total_tutors;
int students_tutored_now, total_sessions_tutored, kill_tutor_thread;

// For Multi-level priority queue
int **MLPQ;
int *MLPQ_front;
int *MLPQ_rear;

// For Student
typedef struct student {
    int student_ID;
    int num_helps;
    int tutored_by;
} student;

student *stu_arr;
sem_t *student_sleeping, *tutor_waiting;

// For Coordinator Queue
int rear = 0;
int front = 0;
int *coord_queue;

void add(int studentID);

int pop();

/* Student Thread Routine */
void student_routine(int studentID) {
    student *curr_student = &stu_arr[studentID];
#ifdef DEBUG
    printf("student_routine started for %d\n", curr_student->student_ID);
#endif

    while (1) {
        if (curr_student->num_helps == max_help) {
            // Exit the student thread
            pthread_exit(NULL);
        }
        sem_wait(&chair_mutex);
        if (chairs_avail <= total_chairs && chairs_avail > 0) {
            // occupy chair
            chairs_avail--;
            printf("S: Student %d takes a seat. Empty chairs = %d.\n", curr_student->student_ID, chairs_avail);
            sem_post(&chair_mutex);

            // add self to coordinator Queue
            sem_wait(&q_mutex);
            add(curr_student->student_ID);
            sem_post(&q_mutex);

            // Inform coordinator that queue is not empty
            sem_post(&queue_fill);

            // wait to be woken up for tutoring
            sem_wait(&student_sleeping[curr_student->student_ID]);

            // Empty the chair
            sem_wait(&chair_mutex);
            chairs_avail += 1;
            sem_post(&chair_mutex);

            // Not waiting anymore
            sem_wait(&waiting_students_mutex);
            waiting_students--;
            sem_post(&waiting_students_mutex);

            // Once woken up, get tutored
            curr_student->num_helps++;
            usleep(200);

            // Done with tutoring
            printf("S: Student %d received help from Tutor %d.\n", curr_student->student_ID, curr_student->tutored_by);
        } else {
            sem_post(&chair_mutex);
            // Do programming
            printf("S: Student %d found no empty chair. Will try again later.\n", curr_student->student_ID);
            usleep(rand() % (2001));
        }
    }
}

void tutor_routine(int tutorID) {
#ifdef DEBUG
    printf("tutor_routine started for %d\n", tutorID);
#endif
    int curr_help_level, studentID;
    student *curr_student;

    while (1) {
        // Wait for Tutor PQ to be filled
        sem_wait(&tutor_waiting[tutorID]);

        // Check if the tutor was woken up to kill the thread
        sem_wait(&kill_tutor_thread_mutex);
        if (kill_tutor_thread == 1) {
            sem_post(&kill_tutor_thread_mutex);
            pthread_exit(NULL);
        }
        sem_post(&kill_tutor_thread_mutex);

        // Remove student with the highest priority from MLPQ
        sem_wait(&MLPQ_mutex);
        curr_help_level = 0;
        while (MLPQ_rear[curr_help_level] == MLPQ_front[curr_help_level] && curr_help_level < max_help) {
            curr_help_level += 1;
        }
        studentID = MLPQ[curr_help_level][MLPQ_front[curr_help_level]];
        MLPQ_front[curr_help_level] += 1;
        sem_post(&MLPQ_mutex);

        curr_student = &stu_arr[studentID];
        curr_student->tutored_by = tutorID;

        // Update count variables
        sem_wait(&students_tutored_now_mutex);
        students_tutored_now += 1;
        sem_post(&students_tutored_now_mutex);

        // Wake up student
        sem_post(&student_sleeping[studentID]);
        // Tutor student
        usleep(200);

        // Done with tutoring the student
        sem_wait(&students_tutored_now_mutex);
        students_tutored_now -= 1;
        total_sessions_tutored += 1;
        printf("T: Student %d tutored by Tutor %d. Students tutored now = %d. Total sessions tutored = %d.\n",
               studentID, tutorID, students_tutored_now, total_sessions_tutored);
        sem_post(&students_tutored_now_mutex);
    }
}

void coord_routine() {
#ifdef DEBUG
    printf("coord_routine started\n");
#endif
    student *curr_student;
    int studentID, next_tutor;

    while (1) {
        // If all requests are sent, kill the coordinator thread
        sem_wait(&waiting_students_mutex);
        if (total_requests == total_students * max_help) {
            sem_post(&waiting_students_mutex);
            pthread_exit(NULL);
        }
        sem_post(&waiting_students_mutex);

        //should wait for Queue to fill up
        sem_wait(&queue_fill);
        //Remove student from queue (FCFS)
        sem_wait(&q_mutex);
        studentID = pop();
        sem_post(&q_mutex);

        curr_student = &stu_arr[studentID];
        sem_wait(&waiting_students_mutex);
        waiting_students += 1;
        total_requests += 1;
        printf("C: Student %d with priority %d added to the queue. Waiting students now = %d. Total requests = %d.\n",
               curr_student->student_ID, curr_student->num_helps, waiting_students, total_requests);
        sem_post(&waiting_students_mutex);

        //Add student to Tutor MLPQ
        sem_wait(&MLPQ_mutex);
        MLPQ[curr_student->num_helps][MLPQ_rear[curr_student->num_helps]] = curr_student->student_ID;
        MLPQ_rear[curr_student->num_helps] += 1;
        sem_post(&MLPQ_mutex);

        //Notify tutor that student is waiting
        next_tutor = tutor_rr;
        tutor_rr = (tutor_rr + 1) % total_tutors;
        sem_post(&tutor_waiting[next_tutor]);
    }
}


int main(int argc, char *argv[]) {
    pthread_t *student_thread;
    pthread_t *tutor_thread;
    pthread_t coord_thread;
    int n, m, i, j;

    if (argc == 5) {
        n = atoi(argv[1]); //Number of students
        m = atoi(argv[2]); //Number of tutors
        total_chairs = atoi(argv[3]);
        chairs_avail = total_chairs;
        max_help = atoi(argv[4]);

        waiting_students = 0;
        total_requests = 0;
        tutor_rr = 0;
        students_tutored_now = 0;
        total_sessions_tutored = 0;
        kill_tutor_thread = 0;

        sem_init(&chair_mutex, 0, 1);
        sem_init(&q_mutex, 0, 1);
        sem_init(&queue_fill, 0, 0);
        sem_init(&waiting_students_mutex, 0, 1);
        sem_init(&MLPQ_mutex, 0, 1);
        sem_init(&students_tutored_now_mutex, 0, 1);
        sem_init(&kill_tutor_thread_mutex, 0, 1);

#ifdef DEBUG
        printf("students: %d, tutors: %d, total_chairs: %d, max_help: %d\n", n, m, total_chairs, max_help);
#endif

        total_students = n;
        total_tutors = m;
        stu_arr = (student *) malloc(n * sizeof(student));
        assert(stu_arr != NULL);
        student_sleeping = (sem_t *) malloc(n * sizeof(sem_t));
        assert(student_sleeping != NULL);
        tutor_waiting = (sem_t *) malloc(m * sizeof(sem_t));
        assert(tutor_waiting != NULL);
        coord_queue = (int *) malloc(total_chairs * sizeof(int));
        assert(coord_queue != NULL);
        student_thread = (pthread_t *) malloc(sizeof(pthread_t) * n);
        assert(student_thread != NULL);
        tutor_thread = (pthread_t *) malloc(sizeof(pthread_t) * m);
        assert(tutor_thread != NULL);

        // MLPQ initialization
        MLPQ = (int **) malloc(sizeof(int *) * max_help);
        assert(MLPQ != NULL);
        MLPQ_front = (int *) malloc(sizeof(int) * max_help);
        assert(MLPQ_front != NULL);
        MLPQ_rear = (int *) malloc(sizeof(int) * max_help);
        assert(MLPQ_rear != NULL);

        for (i = 0; i < max_help; i++) {
            MLPQ[i] = (int *) malloc(sizeof(int) * n);
            MLPQ_front[i] = 0;
            MLPQ_rear[i] = 0;
        }

#ifdef DEBUG
        printf("stu_arr, student_sleeping, coord_queue, student_thread, tutor_thread malloc\n");
#endif

        for (i = 0; i < m; i++) {
            sem_init(&tutor_waiting[i], 0, 0);
        }

        //Create tutor threads
        for (j = 0; j < m; j++) {
            assert(pthread_create(&tutor_thread[j], NULL, (void *(*)(void *)) tutor_routine,
                                  (void *) (uintptr_t) j) ==
                   0);
        }
        //Create coordinator thread
        assert(pthread_create(&coord_thread, NULL, (void *(*)(void *)) coord_routine, NULL) == 0);
        //Create student threads
        for (i = 0; i < n; i++) {
            sem_init(&student_sleeping[i], 0, 0);
            student *s = &stu_arr[i];
            s->student_ID = i;
            s->num_helps = 0;
            assert(pthread_create(&student_thread[i], NULL, (void *(*)(void *)) student_routine,
                                  (void *) (uintptr_t) i) == 0);
        }

        // Wait for student threads
        for (i = 0; i < n; i++) {
            assert(pthread_join(student_thread[i], NULL) == 0);
        }
#ifdef DEBUG
        printf("Student threads joined\n");
#endif
        // Wait for coordinator thread
        assert(pthread_join(coord_thread, NULL) == 0);
#ifdef DEBUG
        printf("Coordinator thread joined\n");
#endif

        kill_tutor_thread = 1;
        for (j = 0; j < m; j++) {
            sem_post(&tutor_waiting[j]);
        }
        // Wait for tutor threads
        for (j = 0; j < m; j++) {
            assert(pthread_join(tutor_thread[j], NULL) == 0);
        }
#ifdef DEBUG
        printf("Tutor threads joined\n");
#endif
    } else {
#ifdef DEBUG
        printf("Wrong number of arguments");
#endif
        exit(-1);
    }

    return 0;
}

/* Coordinator Queue Helper Functions */

void add(int studentID) {
    coord_queue[rear] = studentID;
    rear = (rear + 1) % total_chairs;
}

int pop() {
    int studentID = coord_queue[front];
    front = (front + 1) % total_chairs;
    return studentID;
}
