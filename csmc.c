/* Multi-Threaded Mentoring Center
 * ANTHEA ABREO - AXA210122, HIMA SAI KIRAN PRUDHIVI - HXP220011
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#include <stdint.h>

//#define DEBUG

sem_t chair_mutex, q_mutex, queue_fill, waiting_students_sem;
int total_chairs, max_help, chairs_avail, total_students, waiting_students, total_requests, tutor_rr, total_tutors;

/* For Student */
typedef struct student {
    int student_ID;
    int num_helps;
} student;

student *stu_arr;
sem_t *student_sleeping, *tutor_waiting;

/* For Coordinator Queue */
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
            break;
        }
        sem_wait(&chair_mutex);
        if (chairs_avail <= total_chairs && chairs_avail > 0) {
            // occupy chair
            chairs_avail--;
            printf("S: Student %d takes a seat. Empty chairs = %d.\n", curr_student->student_ID, chairs_avail);
            sem_post(&chair_mutex);

            //add self to coordinator Queue
            sem_wait(&q_mutex);
            add(curr_student->student_ID);
            sem_post(&q_mutex);
            //Inform coordinator that queue is not empty
            sem_post(&queue_fill);

            // wait to be woken up for tutoring
            sem_wait(&student_sleeping[curr_student->student_ID]);
            //Once woken up, get tutored
            curr_student->num_helps++;
            sleep(2000);

        } else {
            sem_post(&chair_mutex);
            //do programming
            // TODO: Random upto 2ms
            printf("S: Student %d found no empty chair. Will try again later.\n", curr_student->student_ID);
            sleep(20);
        }

    }

}

void tutor_routine(int id) {
#ifdef DEBUG
    printf("tutor_routine started for %d\n", id);
#endif

    while (1) {
        //Wait for Tutor PQ to be filled
        sem_wait(&tutor_waiting[id]);
        printf("WOKEEE %d\n", id);

//        //Remove student with highest priority from MLPQ
//        sem_post(&student_sleeping[0]);
//        printf("T: Student %d tutored by Tutor %d", 0, id);
//
//        //Tutor student
        sleep(2);
    }
}

void coord_routine() {
#ifdef DEBUG
    printf("coord_routine started\n");
#endif
    student *curr_student;

    while (1) {
        //should wait for Queue to fill up
        sem_wait(&queue_fill);
        //Remove student from queue (FCFS)
        sem_wait(&q_mutex);
        int studentID = pop();
        sem_post(&q_mutex);

        curr_student = &stu_arr[studentID];
        sem_wait(&waiting_students_sem);
        waiting_students += 1;
        total_requests += 1;
        printf("C: Student %d with priority %d added to the queue. Waiting students now = %d. Total requests = %d\n",
               curr_student->student_ID, curr_student->num_helps, waiting_students, total_requests);
        sem_post(&waiting_students_sem);
        //Add student to Tutor MLPQ
        //MLPQ[curr_student.nhelps].addatend()

        //Notify tutor that student is waiting
        int next_tutor = tutor_rr;
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

        sem_init(&chair_mutex, 0, 1);
        sem_init(&q_mutex, 0, 1);
        sem_init(&queue_fill, 0, 0);
        sem_init(&waiting_students_sem, 0, 1);

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
        student_thread = malloc(sizeof(pthread_t) * n);
        assert(student_thread != NULL);
        tutor_thread = malloc(sizeof(pthread_t) * m);
        assert(tutor_thread != NULL);

#ifdef DEBUG
        printf("stu_arr, student_sleeping, coord_queue, student_thread, tutor_thread malloc\n");
#endif

        for (i = 0; i < m; i++) {
            sem_init(&tutor_waiting[i], 0, 0);
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
        //Create tutor threads
        for (j = 0; j < m; j++) {
            assert(pthread_create(&tutor_thread[j], NULL, (void *(*)(void *)) tutor_routine,
                                  (void *) (uintptr_t) j) ==
                   0);
        }

        // Wait for coordinator thread
        assert(pthread_join(coord_thread, NULL) == 0);
        // Wait for student threads
        for (i = 0; i < n; i++) {
            assert(pthread_join(student_thread[i], NULL) == 0);
        }
        // Wait for tutor threads
        for (j = 0; j < m; j++) {
            assert(pthread_join(tutor_thread[j], NULL) == 0);
        }
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
    student *st = &stu_arr[studentID];
#ifdef DEBUG
    printf("Inserting student %d in queue at position %d\n", st->student_ID, rear);
#endif
    coord_queue[rear] = studentID;
    rear = (rear + 1) % total_chairs;
}

int pop() {
    int studentID = coord_queue[front];
    student *st = &stu_arr[studentID];

#ifdef DEBUG
    printf("Student deleted from queue is %d from position %d\n", st->student_ID, front);
#endif
    front = (front + 1) % total_chairs;
    return studentID;
}
