#include <stdio.h>
#include <stdlib.h>

// function declarations
void input_scores(int *scores, int n);
void count_ranges(int *scores, int n, int *q1, int *q2, int *q3, int *q4);
int find_max(int *scores, int n);
int find_min(int *scores, int n);
float calculate_average(int *scores, int n);

int main() {
    int num_students;

    printf("Enter the number of students: ");
    scanf("%d", &num_students);
    
    printf("\nPlease type their scroes of these %d students\n\n",num_students);

    if (num_students <= 0) {
        printf("Invalid number of students.\n");
        return 1;
    }

    int *scores = malloc(num_students * sizeof(int));
    if (scores == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    input_scores(scores, num_students);

    int q1, q2, q3, q4;
    count_ranges(scores, num_students, &q1, &q2, &q3, &q4);
    int max = find_max(scores, num_students);
    int min = find_min(scores, num_students);
    float avg = calculate_average(scores, num_students);

    printf("\n==== Score Statistics ====\n");
    printf("A.Q1: %d., Q2: %d., Q3: %d., Q4: %d.\n",q1,q2,q3,q4);
    printf("B. Highest score: %d\n", max);
    printf("C. Lowest score: %d\n", min);
    printf("D. Average score: %.1f\n", avg);
    printf("\n\nP.S. Q1:76-100,Q2:51-75,Q3:26-50,Q4:0-25\n\n");    

    free(scores);
    return 0;
}

