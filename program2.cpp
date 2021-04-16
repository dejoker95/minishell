#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctime>
#include <cstring>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <fcntl.h>
#include <cstring>
#include <sys/wait.h>
#include <string>

using namespace std;

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

// 기본 merge_sort 이지만 자식 프로세스가 반환한 길이보다  배열을 작게 쪼개지 않음
void merge_sort(int* arr, int left, int right, int part_length){
    if (left < right && (right - left + 1) > part_length){
        int mid = (left + right) / 2;
        merge_sort(arr, left, mid, part_length);
        merge_sort(arr, mid + 1, right, part_length);
        merge(arr, left, mid, right);
    }
}

void make_input(int filenum, int* arr, int start, int end){
    int num = end -start + 1;
    string fileName = "input" + to_string(filenum);
    ofstream out(fileName);
    out<< num << endl;
    for (int i = start; i <= end; i++){
        out << (int)arr[i] << " ";
    }
    out << endl;
    out.close();
}

int main(int argc, char* argv[]){
    int pNum = atoi(argv[1]);   // 프로세스 개수

    // merge sort 배열 받아오기
    int n;          // 배열 길이
    int* arr;       // 배열
    scanf("%d", &n);
    arr = new int[n];
    for (int i = 0; i < n; i++){
        scanf("%d", &arr[i]);
    }

    int part_length;
    part_length = n / pNum;
    
    for (int i = 1; i <= pNum; i++){
        if (i == pNum){
            make_input(i-1, arr, part_length * (i - 1), n - 1);
        }
        else{
            make_input(i-1, arr, part_length * (i - 1), part_length * i - 1);
        }
    }

    clock_t startTime = clock();

    // 자식 프로세스 만들어서 실행
    pid_t* children = new pid_t[pNum];
    
    for(int i = 0; i < pNum; i++){
        children[i] = fork();
        // 자식 프로세스의 표준입출력 바꿔주고 program1 실행
        if(children[i] == 0){
            char* args[] = {(char*)"./program1", NULL};
            string filename = "input" + to_string(i);
            int in = open(filename.c_str(), O_RDONLY);
            filename = "output" + to_string(i);
            int out = open(filename.c_str(), O_WRONLY|O_TRUNC|O_CREAT, S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP);
            dup2(in, 0);
            dup2(out, 1);
            close(in);
            close(out);
            execvp("./program1", args);
        }
    }

    // 부모 프로세스는 자식 프로세스들의 작업이 끝날 때까지 wait
    for(int i = 0; i < pNum; i++){
        waitpid(children[i], NULL, 0);
    }

    // 자식 프로세스들의 output을 원래 배열에 덮어씌운다
    int cnt = 0;
    for(int i = 0; i < pNum; i++){
        string in = "input" + to_string(i);
        string out = "output" + to_string(i);
        ifstream readInput;
        ifstream readOutput;
        
        readOutput.open(out);

        if (i == pNum - 1 && pNum != 1){
            for(int j = 0; j < n - part_length*(pNum - 1); j++){
                readOutput >> arr[cnt++];
            }
        }
        else{
            for(int j = 0; j < part_length; j++){
                readOutput >> arr[cnt++];
            }
        }
        readOutput.close();
        // 임시 생성 input, output 파일 삭제
        remove(in.c_str());
        remove(out.c_str());
    }

    // 부분 소팅된 배열을 부모 프로세스에서 마무리 병합정렬
    arr2 = new int[n];
    merge_sort(arr, 0, n-1, part_length);

    clock_t endTime = clock();

    // 최종 결과 출력
    for (int i = 0; i < n; i++){
        printf("%d ", arr[i]);
    }
    printf("\n");
    printf("%ld\n", (endTime - startTime));
}
