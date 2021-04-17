#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <chrono>
#include <cstdlib>
#include <cstdio>
#include <pthread.h>

using namespace std;

// 쓰레드들이 변수에 접근할 수 있도록 글로벌 변수로 선언
int n, tNum, part_length;
int* arr;
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

// 각 쓰레드 id에 따라 실행할 파트를 정해주고 merge sort를 실행하는 함수
void* thread_merge_sort(void* arg){
    long thread_id = (long) arg;
    int left = thread_id * part_length;
    int right;
    if (thread_id == tNum - 1){
        right = n - 1;
    }
    else{
        right = (thread_id + 1) * part_length - 1;
    }
    merge_sort(arr, left, right);
    return NULL;
}   


// 기본 merge_sort 이지만 자식 프로세스가 반환한 길이보다  배열을 작게 쪼개지 않음
void merge_sort_last(int* arr, int left, int right, int part_length){
    if (left < right && (right - left + 1) > part_length){
        int mid = (left + right) / 2;
        merge_sort_last(arr, left, mid, part_length);
        merge_sort_last(arr, mid + 1, right, part_length);
        merge(arr, left, mid, right);
    }
}


int main(int argc, char* argv[]){
    tNum = atoi(argv[1]);   // 쓰레드 개수

    // merge sort 배열 받아오기
    scanf("%d", &n);
    arr = new int[n];
    arr2 = new int[n];
    for (int i = 0; i < n; i++){
        scanf("%d", &arr[i]);
    }

    // 쓰레드에 할당할 배열 파트 나누기, 방식은 program2와 같음
    part_length = n / tNum;
    // 시간 측정 시작
    auto start = chrono::high_resolution_clock::now();

    // 쓰레드 만들기
    pthread_t* threads = new pthread_t[tNum];
    for (long i = 0; i < tNum; i++){
        int rc = pthread_create(&threads[i], NULL, thread_merge_sort, (void *)i);
        if (rc) {
            printf("error");
        }
    }
    // 모든 쓰레드의 작업이 끝날 때까지 대기
    for (long i = 0; i < tNum; i++){
        pthread_join(threads[i], NULL);
    }

    // 부분 소팅된 배열을 부모 프로세스에서 마무리 병합정렬
    merge_sort_last(arr, 0, n-1, part_length);
    // 시간 측정 끝
    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
    long time = duration.count();

    // 최종 결과 출력
    for (int i = 0; i < n; i++){
        printf("%d ", arr[i]);
    }
    printf("\n");
    printf("%ld\n", time);
}
