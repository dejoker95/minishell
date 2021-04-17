#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <chrono>
#include <iostream>

using namespace std;

// sorting 중간 결과를 저장하기 위한 temporary 배열
int *arr2;

// 쪼개진 두 배열의 값을 비교해서 하나의 배열로 정렬하는 함수
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

// 재귀적으로 병합정렬을 실행하는 함수
void merge_sort(int* arr, int left, int right){
    if (left < right){
        int mid = (left + right) / 2;
        merge_sort(arr, left, mid);
        merge_sort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}


int main(){
    // input 받아오기
    int n;          // input의 길이
    int* arr;       // input을 저장할 배열
    scanf("%d", &n);

    arr = new int[n];
    arr2 = new int[n];

    for (int i = 0; i < n; i++){
        scanf("%d", &arr[i]);
    }

    // 시간 측정 시작 / sorting 실행
    auto start = chrono::high_resolution_clock::now();
    merge_sort(arr, 0, n-1);
    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
    long time = duration.count();
    // 결과 출력
    for (int i = 0; i < n; i++){
        printf("%d ", arr[i]);
    }
    printf("\n");

    printf("%ld\n", time);
}