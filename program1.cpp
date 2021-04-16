#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctime>

int *arr2;

void merge(int* arr, int left, int mid, int right){
    int i = left;
    int j = mid + 1;
    int k = left;

    while (i <= mid && j <= right){
        if (arr[i] >= arr[j]){
            arr2[k++] = arr[i++];
        }
        else {
            arr2[k++] = arr[j++];
        }
    }

    if (i > mid){
        for (int t = j; t <= right; t++){
            arr2[k++] = arr[t];
        }
    }
    else {
        for (int t = i; t <= mid; t++){
            arr2[k++] = arr[t];
        }
    }
    for (int t = left; t <= right; t++){
        arr[t] = arr2[t];
    }
}

void merge_sort(int* arr, int left, int right){
    if (left < right){
        int mid = (left + right) / 2;
        merge_sort(arr, left, mid);
        merge_sort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

int main(){
    int n;
    int* arr;
    scanf("%d", &n);

    arr = new int[n];
    arr2 = new int[n];

    for (int i = 0; i < n; i++){
        scanf("%d", &arr[i]);
    }

    clock_t startTime = clock();
    merge_sort(arr, 0, n-1);
    clock_t endTime = clock();

    for (int i = 0; i < n; i++){
        printf("%d ", arr[i]);
    }
    printf("\n");

    printf("%ld\n", (endTime - startTime));
}