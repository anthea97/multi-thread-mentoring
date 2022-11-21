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

typedef struct student{
   int student_ID;
   int num_helps;
   sem_t student_sleeping;
}student;


int main(int argc, char *argv[]){
    if(argc == 4){
        int n = atoi(argv[0]);
        int m = atoi(argv[1]);
        int total_chairs = atoi(argv[2]);
        int max_help = atoi(argv[3]);

        student* students = (student*) malloc(n * sizeof(student));
    }

    else{
        printf("Wrong number of arguments");
        exit(0);
    }

    return 0;
}