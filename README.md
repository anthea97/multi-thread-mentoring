# multi-thread-mentoring

## Requirements
The computer science department runs a mentoring center (csmc) to help undergraduate students with their programming assignments.

- There are many coordinators and many tutors. 
- The waiting area has several chairs.
- Initially all chairs are empty
- Coordinator waits for student to arrive
- Tutor waits for notification from coordinator that student is available for tutoring or is busy tutoring
- When student arrives:

  If no chair is available, the student leaves(goes back to programming) and comes back later
  
    Otherwise student sits in an empty chairs and waits to be called for tutoring


A student is serviced based on their priority. The priority of a student is based on the number of times the student has taken help from a tutor.
A student visiting the center for the first time gets the highest priority.

The total number of students, the number of tutors, the number of chairs, and the number of times a student seeks a tutor’s help are passed as command line arguments.

Once a student thread takes the required number of helps from the tutors, it should terminate. 
Once all the student threads are terminated, the tutor threads, the coordinator thread, and the main program should be terminated.


## Implementation:
1. We model the student i.e. Create a student structure with attributes like student ID, #helps(priority), etc.
2. We use two shared data structures:
    
    One shared between Student and Coordinator

    One shared between Coordinator and Tutor

![](Drawing.png)