#include <stdio.h>

void input_scores(int *scores, int n) {
    for (int i = 0; i < n; i++) {
        int s;
        while (1) {
            printf("Enter score of student%d: ", i + 1);
            scanf("%d", &s);
            if (s >= 0 && s <= 100) break;
            printf("Invalid score. Please re-enter.\n");
        }
        scores[i] = s;
    }
}

