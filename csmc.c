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

sem_t chair_mutex, q_mutex;


typedef struct student{
   int student_ID;
   int num_helps;
   sem_t student_sleeping;
}student;

void student_routine(){

}

void tutor_routine(){

}

int main(int argc, char *argv[]){
    pthread_t *student_thread;
    pthread_t *tutor_thread;
    int n, m, total_chairs, max_help, i, j;
    student *stu_arr;

    if(argc == 4){
        n = atoi(argv[0]); //Number of students
        m = atoi(argv[1]); //Number of tutors
        total_chairs = atoi(argv[2]);
        max_help = atoi(argv[3]);

        stu_arr = (student*) malloc(n * sizeof(student));

        student_thread = malloc(sizeof(pthread_t) * n);
        tutor_thread = malloc(sizeof(pthread_t) * m);

        //Create student threads
        for(i = 0; i < n; i++) {
            assert(pthread_create(&student_thread[i], NULL, student_routine, (void *) i) == 0);
        }

        //Create tutor threads
        for(j = 0; j < m; j++) {
            assert(pthread_create(&tutor_thread[j], NULL, tutor_routine, (void *) j) == 0);
        }

    }

    else{
        printf("Wrong number of arguments");
        exit(-1);
    }

    return 0;
}