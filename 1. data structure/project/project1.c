#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define strlength 50 // 입력 가능한 파일명의 최대 길이. 적당히 잡음

typedef struct _RGB // r, g, b 값을 배열에 담기 위한 구조체를 선언
{
    int r;
    int g;
    int b;
} RGB;

int isitnum(int a);
int check_notnums(char *str);
int check_countofnums(char *str);
int check_placeofnums(char *str);
int data_integrity(FILE *pfile, int size);
void findMaxMin(int **arr_2d, int row, int column, int *max, int *min);
double loadGreyscale(double height, double max, double min);
void outputPPM(RGB **arr_2d, int row, int column, char *str);

int main()
{
    char str[strlength]; // 파일명 저장
    FILE *pfile;         // open한 file을 담음
    int row, column;     // 파일 내 데이터의 행, 열 크기
    int max, min;        // file data 내의 최대, 최소값
    int **input_2d;      // file의 값을 담기위한 배열의 포인터
    RGB **output_2d;     // 변환된 r, g, b 데이터를 담기 위한 배열의 포인터
    int i, j, temp;

    printf("파일명을 입력하세요. (파일명 형식 : map-input-ccc-rrr.dat) : ");
    fgets(str, strlength, stdin);
    str[strlen(str) - 1] = '\0';
    while ((check_notnums(str) && check_countofnums(str) && check_placeofnums(str)) == 0) // 파일명을 올바르게 입력했는지를 확인
    {
        printf("잘못된 형식의 입력입니다.\n\n");
        printf("파일명을 입력하세요. (파일명 형식 : map-input-ccc-rrr.dat) : ");
        fgets(str, strlength, stdin);
        str[strlen(str) - 1] = '\0';
    }

    pfile = fopen(str, "r"); // 파일 읽기 전용으로 open
    if (pfile == NULL)
    {
        printf("파일이 없습니다.");
        return 0;
    }

    sscanf(str, "map-input-%d-%d.dat", &column, &row); // 입력한 파일명에서 숫자값만 가져와 정수로 변환

    if (!data_integrity(pfile, row * column)) // 데이터 무결성 검사. 데이터에 문제가 있으면 프로그램 종료
    {
        fclose(pfile); // 파일 close
        return 0;
    }
    rewind(pfile); // 파일 포인터를 처음으로 옮김

    input_2d = (int **)malloc(sizeof(int *) * row);  // 2차원 int배열 생성 후 .dat 파일의 값을 입력
    output_2d = (RGB **)malloc(sizeof(RGB *) * row); // 2차원 RGB배열 생성
    for (i = 0; i < row; i++)
    {
        input_2d[i] = (int *)malloc(sizeof(int) * column);
        output_2d[i] = (RGB *)malloc(sizeof(RGB) * column);
        for (j = 0; j < column; j++)
            fscanf(pfile, "%d", &input_2d[i][j]);
    }
    fclose(pfile);

    findMaxMin(input_2d, row, column, &max, &min); // input_2d 내의 최대, 최소값을 찾아 max, min에 입력
    for (i = 0; i < row; i++)                      // RGB배열에 값을 입력
    {
        for (j = 0; j < column; j++)
        {
            temp = loadGreyscale((double)input_2d[i][j], (double)max, (double)min); // 이미지 출력을 위한 Greyscale 값을 계산
            output_2d[i][j].r = temp;
            output_2d[i][j].g = temp;
            output_2d[i][j].b = temp;
        }
    }
    outputPPM(output_2d, row, column, str); // RGB 배열 데이터를 ppm 포멧의 파일로 출력

    for (i = 0; i < row; i++) // 동적할당된 2차원 배열을 free
    {
        free(input_2d[i]);
        free(output_2d[i]);
    }
    free(input_2d);
    free(output_2d);

    return 0;
}

int isitnum(int a) // 문자가 숫자인지 확인
{
    return (a >= 48 && a <= 57); // 숫자 0 ~ 9는 아스키코드로 48 ~ 57임
}

int check_notnums(char *str) // 숫자를 제외한 모든 문자들이 형식과 같은지 확인
{
    char ref[16] = "map-input--.dat"; // 기준 파일명 형식. "map-input--.dat"의 문자 개수는 15개. +1은 끝의 널값
    char input[17];                   // 입력 파일명 형식. 배열 크기는 초과 입력을 체크하기 위해 +1
    int i = 0;
    int j = 0;

    while (str[i] != '\0' && j < 16) // 최대 16개의 데이터를 받음. 널값이 읽히면 break
    {
        if (isitnum(str[i]) == 0) // 숫자를 제외한 모든 char을 input에 입력
        {
            input[j] = str[i];
            j++;
        }
        i++;
    }
    input[j] = '\0'; // 끝 널값 삽입

    if (strcmp(ref, input) == 0) // strcmp은 문자열 ref와 input이 같으면 0을 반환함
        return 1;                // 조건문 참이면 1을 반환
    else
        return 0;
}

int check_countofnums(char *str) // 숫자문자열의 개수가 2개인지 확인
{
    int i = 1;
    int count = 0;

    if (isitnum(str[0]) == 1) // 맨 처음이 숫자이면 count++;
        count++;
    while (str[i] != '\0')
    {
        if ((isitnum(str[i - 1]) == 0) && (isitnum(str[i]) == 1)) // str[i - 1]은 숫자가 아니고, str[i]는 숫자이면 count++ 함 (rise에서 count++)
            count++;
        i++;
    }

    if (count == 2)
        return 1;
    else
        return 0;
}

int check_placeofnums(char *str) // 2개의 숫자문자열이 올바른 자리에 있는지 확인
{
    char *ptr = strchr(str, '-'); // 첫번째 '-'의 위치(주소)를 담음
    ptr = strchr(ptr + 1, '-');   // 두번째 '-'의 위치(주소)를 담음
    if (isitnum(*(ptr + 1)) == 0) // 두번째 '-' 다음의 문자가 숫자인지 확인
        return 0;                 // 숫자 아니면 0을 반환
    ptr = strchr(ptr + 1, '-');   // 세번째 '-'의 위치(주소)를 담음
    return isitnum(*(ptr + 1));   // 세번째 '-' 다음의 문자가 숫자인지 확인. 참이면 1을 반환
}

int data_integrity(FILE *pfile, int size) // 파일 데이터 무결성 검사
{
    int str[3] = {' ', ' ', ' '}; // EOF(-1)의 표현 때문에 char가 아닌 int에 받음
    int c0 = 1;                   // 참값
    int c1 = 1;                   // 참값
    int c2 = 0;                   // 0부터 카운트 시작

    str[2] = fgetc(pfile);
    while (str[2] != EOF)
    {
        c0 = c0 && (str[2] == ' ' || str[2] == '\n' || str[2] == '+' || str[2] == '-' || isitnum(str[2])); // 있으면 안되는 문자(공백문자, 부호, 숫자를 제외한 문자)가 있으면 c0는 0이 됨
        if (str[1] == '+' || str[1] == '-')                                                                // str[1]이 부호일때 str[0]에는 공백문자, str[2]에는 숫자가 있어야 함
            c1 = c1 && ((str[0] == ' ' || str[0] == '\n') && isitnum(str[2]));                             // 부호가 잘못된 위치에 있으면 c1은 0이 됨
        c2 = c2 + ((str[1] == ' ' || str[1] == '\n') && (str[2] != ' ' && str[2] != '\n'));                // 문자열의 개수를 count함. 공백문자를 기준으로 rise에서 count함
        str[0] = str[1];                                                                                   // 데이터 쉬프트
        str[1] = str[2];                                                                                   // 데이터 쉬프트
        str[2] = fgetc(pfile);
    }
    if (str[1] == '+' || str[1] == '-') // 마지막 입력값이 부호면 c1 = 0(부호가 잘못된 위치에 있습니다)가 됨
        c1 = 0;

    if (c0 == 0)
        printf("공백문자, 부호, 숫자를 제외한 문자가 입력되었습니다.\n");
    if (c1 == 0)
        printf("부호가 잘못된 위치에 있습니다.\n");
    if (c2 == size) // 파일명의 행, 열 값과 공백문자로 구분된 데이터의 개수가 같으면
    {
        c2 = 1; // 참값이 됨
    }
    else // 아니면
    {
        if (c2 > size)
            printf("데이터 개수가 초과되었습니다.\n"); // 문자열의 개수가 size보다 크면
        else
            printf("데이터 개수가 부족합니다.\n"); // 문자열의 개수가 size보다 작으면
        c2 = 0;                                    // 거짓값으로 만듬
    }

    return (c0 && c1 && c2); // 문제가 없으면 1, 있으면 0을 반환
}

void findMaxMin(int **arr_2d, int row, int column, int *max, int *min) // arr_2d의 최대, 최소값을 *max, *min에 입력하는 함수
{
    int i, j;

    *max = arr_2d[0][0];
    *min = arr_2d[0][0];
    for (i = 0; i < row; i++)
    {
        for (j = 0; j < column; j++)
        {
            if (arr_2d[i][j] > *max)
                *max = arr_2d[i][j];
            if (arr_2d[i][j] < *min)
                *min = arr_2d[i][j];
        }
    }
}

double loadGreyscale(double height, double max, double min) // shade of grey 계산 함수
{
    return ((height - min) / (max - min)) * 255; // shade of grey 계산식
}

void outputPPM(RGB **arr_2d, int row, int column, char *str) // 변환한 데이터를 ppm 포멧의 파일로 출력하는 함수
{
    int i, j;
    char s0[37];                  // 배열 크기 적당히 잡음
    char *ptr = strchr(str, '.'); // '.' 문자의 위치(주소)를 담음. '.'은 ".dat"의 점임
    *(ptr + 1) = 'p';             // d를 p로
    *(ptr + 2) = 'p';             // a를 p로
    *(ptr + 3) = 'm';             // t를 m으로
    *(ptr + 4) = '\0';            // 혹시 모르니깐 끝 널값 삽입. 이미 널값 있어 안해도 무방

    FILE *pfile = fopen(str, "w"); // 파일 쓰기 전용으로 open
    fputs("P3 ", pfile);           // 문자열을 파일에 쓰기
    sprintf(s0, "%d ", column);    // column 정수값을 문자열로 변환해 s0에 넣음
    fputs(s0, pfile);              // s0를 파일에 쓰기
    sprintf(s0, "%d ", row);       // row 정수값을 문자열로 변환해 s0에 넣음
    fputs(s0, pfile);              // s0를 파일에 쓰기
    fputs("255", pfile);           // 문자열을 파일에 쓰기

    for (i = 0; i < row; i++)
    {
        for (j = 0; j < column; j++)
        {
            sprintf(s0, " %d %d %d", arr_2d[i][j].r, arr_2d[i][j].g, arr_2d[i][j].b); // RGB값을 문자열로 변환해 s0에 입력
            fputs(s0, pfile);                                                         // s0를 파일에 쓰기
        }
    }

    printf("변환이 완료되었습니다. (출력 파일명 : %s)", str);
}