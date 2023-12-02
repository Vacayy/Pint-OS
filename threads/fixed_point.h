#define F (1<<14) // 16384

/* convert int(n) to fixed point */
int int_to_fp(int n); 
/* convert fixed point(x) to int (소수점 아래는 버림) */
int fp_to_int(int x);
/* convert fixed point(x) to int (반올림) */
int fp_to_int_round(int x);


// /* Add operation between FPs (return with FP)*/
// int add_fp(int x, int y);
// /* subtraction operation between FPs (x-y, return with FP)*/
// int sub_fp(int x, int y);
// /* multiplication operation between FPs (return with FP) */
// int multiply_fp(int x, int y);
// /* Division operation between FPs (x/y, return with FP) */
// int divide_fp(int x, int y);


int 
int_to_fp(int n) {
    int fp_n = n * F;
    return fp_n;
}

int 
fp_to_int(int x) {
    int int_x = (x / F);
    return int_x;
};

int 
fp_to_int_round(int x) {
    if (x >= 0){
        int int_x = (x+F/2)/F ;
        return int_x;
    } else {
        int int_x = (x-F/2)/F ;
        return int_x;
    }
};


// /* Add operation between FPs (return with FP)*/
// int 
// add_fp(int x, int y) {    
//     return add_fp;    
// }


// /* subtraction operation between FPs (x-y)*/
// int sub_fp(int x, int y);
// /* multiplication operation between FPs */
// int multiply_fp(int x, int y);
// /* Division operation between FPs (x/y) */
// int divide_fp(int x, int y);
