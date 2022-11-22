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

sem_t chair_mutex, q_mutex, queue_fill, tutor_waiting;
int total_chairs, max_help, chairs_avail, total_students;

/* For Student */
typedef struct student{
   int student_ID;
   int num_helps;
}student;

student *stu_arr;
sem_t *student_sleeping[];

/* For Coordinator Queue */
int rear = -1;
int front = -1;
student *coord_queue;
void add(student st);
student* pop();


/* Student Thread Routine */
void student_routine(int id) {
    student *curr_student;
    curr_student = &stu_arr[id];
    curr_student->student_ID = id;
    curr_student->num_helps = 0;

    while (1) {
        if (curr_student->num_helps == max_help) {
            break;
        }
        if (chairs_avail < total_chairs) {
            // occupy chair
            sem_wait(&chair_mutex);
            chairs_avail--;
            sem_post(&chair_mutex);

            //add self to coordinator Queue
            sem_wait(&q_mutex);
            add(*curr_student);
            sem_post(&q_mutex);
            //Inform coordinator that queue is not empty
            sem_post(&queue_fill);

            // wait to be woken up for tutoring
            sem_wait(student_sleeping[id]);
            //Once woken up, get tutored
            curr_student->num_helps++;
            sleep(2000);

        } else {
            //do programming
            sleep(20);
        }

    }

}

void tutor_routine() {
/*
 * While(1){
//Wait for Tutor PQ to be filled
Wait(tutor_waiting)

//Remove student with highest priority from MLPQ
signal(student_sleeping[student_id])

//Tutor student
Sleep(0.2ms)

 */
}

void coord_routine() {
    student* curr_student;

    while(1){
    //should wait for Queue to fill up
    sem_wait(queue_fill);
    //Remove student from queue (FCFS)
    sem_wait(&q_mutex);
    curr_student = pop();
    sem_post(&q_mutex);
    //Add student to Tutor MLPQ
    //MLPQ[curr_student.nhelps].addatend()

    //Notify tutor that student is waiting
    //sem_post(tutor_waiting);
    }

}


int main(int argc, char *argv[]) {
    pthread_t *student_thread;
    pthread_t *tutor_thread;
    pthread_t coord_thread;
    int n, m, i, j;

    if (argc == 4) {
        n = atoi(argv[0]); //Number of students
        m = atoi(argv[1]); //Number of tutors
        total_chairs = atoi(argv[2]);
        max_help = atoi(argv[3]);
        total_students = n;
        stu_arr = (student *) malloc(n * sizeof(student));
        coord_queue = (student *) malloc(n * sizeof(student));

        student_thread = malloc(sizeof(pthread_t) * n);
        tutor_thread = malloc(sizeof(pthread_t) * m);

        //Create coordinator thread
        assert(pthread_create(&coord_thread[i], NULL, coord_routine, (void *) i) == 0);

        //Create student threads
        for (i = 0; i < n; i++) {
            assert(pthread_create(&student_thread[i], NULL, student_routine, (void *) i) == 0);
        }

        //Create tutor threads
        for (j = 0; j < m; j++) {
            assert(pthread_create(&tutor_thread[j], NULL, tutor_routine, (void *) j) == 0);
        }

    } else {
        printf("Wrong number of arguments");
        exit(-1);
    }

    return 0;
}

/* Coordinator Queue Helper Functions */

void add(student st) {
    if (rear == total_students - 1)
        printf("Queue Overflow n");
    else {
        if (front == -1)
            front = 0;
        printf("Insert student %d in queue : ", st.student_ID);
        scanf("%d", &st);
        rear = rear + 1;
        coord_queue[rear] = st;
    }
}

student* pop() {
    student* st;
    if (front == -1 || front > rear) {
        printf("Queue Underflow n");
        return NULL;
    } else {
        st = &coord_queue[front];
        printf("Element deleted from queue is : %dn", coord_queue[front].student_ID);
        front = front + 1;
        return st;
    }

}

