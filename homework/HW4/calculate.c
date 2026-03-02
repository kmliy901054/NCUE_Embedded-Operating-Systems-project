int find_max(int *scores, int n) {
    int max = scores[0];
    for (int i = 1; i < n; i++) {
        if (scores[i] > max) max = scores[i];
    }
    return max;
}

int find_min(int *scores, int n) {
    int min = scores[0];
    for (int i = 1; i < n; i++) {
        if (scores[i] < min) min = scores[i];
    }
    return min;
}

float calculate_average(int *scores, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += scores[i];
    }
    return (float)sum / n;
}

void count_ranges(int *scores, int n, int *q1, int *q2, int *q3, int *q4) {
    *q1 = *q2 = *q3 = *q4 = 0;
    for (int i = 0; i < n; i++) {
        int s = scores[i];
        if (s >= 76) (*q1)++;
        else if (s >= 51) (*q2)++;
        else if (s >= 26) (*q3)++;
        else (*q4)++;
    }
}

