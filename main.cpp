#include <iostream>
#include <fstream>
#include <chrono>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#define FILENAME_MAX 4096

/*formula for indices: [i][j] = i * noofcolumns + j*/
 
/*to time any function*/
struct Timer
{

    std::chrono::time_point<std::chrono::system_clock> start;
    std::chrono::duration<float> duration;

    Timer(){
         start = std::chrono::high_resolution_clock::now();
    }
    
    float getTimeElapsed(){
        auto end  = std::chrono::high_resolution_clock::now();
        duration = end - start;
        
        return duration.count()*1000.00f;
    }
};

/*defining a matrix*/
struct matrix
{
    int * size;
    int * arr;
};

/*args for thread function*/
struct Index{
    int i , j ;
};

/*global vars*/
matrix * * inputMatrices; 
matrix * output_matrix;

/*read matrix*/
matrix * readmatrix(std::ifstream & fin){
    int x, y;


    /* get matrix size */
    fin >> x >> y;

    /*creaing one dimensional arr to improve performance*/
    int* arr = new int[x * y];

    for(int i =0;i<x;i++){
        for (int j = 0; j < y; j++)
            fin >> arr[i * y + j];
        
    }

    /*store this info*/
    int * size = new int[2];
    size[0] = x;
    size[1] = y;
    matrix * m = new matrix;

    m->size = size;
    m->arr = arr;

    return m;
}

/*read file*/
matrix * * matrices(const char * filename){
    std::ifstream fin;
    fin.open(filename);
    

    if (fin)
    {
        /*read 1st arr*/
        matrix * matrix1 = readmatrix(fin);

        /*repeat for 2nd arr*/
        matrix * matrix2 = readmatrix(fin);

        fin.close();

        /*store them*/
        matrix * * ms = new matrix * [2];
        ms[0] = matrix1;
        ms[1] = matrix2;


        return ms;
    }else{
        std::cout << "File not found.Exiting...\n";
        exit(-1);
    }

}

/*multiply element*/
int * multiplyElement(Index * index){

    /*get dimensions*/
    int * size1 = inputMatrices[0]->size;
    int * size2 = inputMatrices[1]->size;

    int * result = new int;
    * result = 0;
    int x,y; // 2 elements to multiply
    int k; //product of 2 elements

    
    for (int i = 0; i < size1[1]; i++)
    {
        x = inputMatrices[0]->arr[ index->i * size1[1] + i];
        y = inputMatrices[1]->arr[i * size2[1] + index->j];
        k = x * y;
        * result += k;
    }
    output_matrix->arr[ index->i * output_matrix->size[1] + index->j] = * result;
    return result;
}

/*function to call element multiplier - for 1st procedure*/
void * call_multiply_element(void * ptr){

    Index * index = (Index *)ptr;

    /*element*/
    multiplyElement(index);
                  
    pthread_exit(0);

}

/*function to call element multiplier for each row - procedure 2*/
void * call_multiply_row(void * ptr){
    
    int * i = (int *)ptr;
    Index index;
    index.i= *i;
    
    /*row*/
    for(int j = 0; j < output_matrix->size[1]; j ++){
        /*element*/
        index.j = j;
        multiplyElement(&index);
    }

    pthread_exit(0);
}

/*utility function to print matrix*/
void print_matrix(matrix * m){
    int x = m->size[0];
    int y = m->size[1];
    
    for(int i =0; i < x; i++){
        for(int j = 0; j < y ; j++){
            std::cout << m->arr[ i * y + j] << "\t";
        }
        std::cout << "\n";
    }
}


float elements_as_threads();
float rows_as_threads();

int main(){

    /*get file name*/
    char * fileName = new char[FILENAME_MAX];

    std::cout << "Enter the name of the input file: \n";
    std::cin >> fileName;

    /*read matrices from file*/
    inputMatrices = matrices(fileName);


    /*check if multiplication is possible*/
    if( inputMatrices[0]->size[1] != inputMatrices[1]->size[0] )
    {
        std::cout << "Invalid input: matrices cannot be multiplied. Exiting...\n";
        return 0;
    }

    /*initialize output matrix*/
    output_matrix = new matrix;

    int * outsize = new int[2];
    outsize[0] = inputMatrices[0]->size[0];
    outsize[1] = inputMatrices[1]->size[1];
    output_matrix->size = outsize;
    output_matrix->arr = new int[outsize[0] * outsize[1]];

    /*procedure 1 - by element*/
    std::cout<<"By element:\n";
    float time1 = elements_as_threads();
    print_matrix(output_matrix);
    std::cout << "\nEND1\t" << time1 << " ms\n";
    
    /*cleaning after procedure 1*/
    delete[] output_matrix->arr;
    
    /*a new empty matrix*/
    output_matrix->arr = new int[outsize[0] * outsize[1]];

    /*procedure 2 - by row*/
    std::cout<<"\nBy row:\n";
    float time2 = rows_as_threads();
    print_matrix(output_matrix);
    std::cout << "\nEND2\t" << time2 << " ms\n";

    /*cleanup*/
    delete[] output_matrix->arr;
    delete(output_matrix->size);
    delete(output_matrix);

    for(int i =0; i < 2; i++){
        delete(inputMatrices[i]->arr);
        delete(inputMatrices[i]->size);
        delete(inputMatrices[i]);
    }
    delete(inputMatrices);

    return 0;
}

float elements_as_threads(){
    int noofthreads = output_matrix->size[0] * output_matrix->size[1];
    Index * index = new Index[noofthreads];

    pthread_t tid[noofthreads];

    int k;//actual index of thread

    /*creating threads*/
    Timer  timer;

    for(int i =0; i < output_matrix->size[0]; i++)
    {
        /*row*/

        for (int j = 0; j < output_matrix->size[1]; j++){

            /*element*/
            k = i * output_matrix->size[1] + j; 
            index[k].i = i;
            index[k].j = j; 

            if(pthread_create(&tid[k],nullptr,call_multiply_element,&index[k])){
                std::cout << "Unable to create thread. exiting...\n";
                exit(-1);
            }
        } 
    }

    /*join*/
    for(int i = 0; i< noofthreads; i++){
        if(pthread_join(tid[i], nullptr)){
            std::cout << "Unable to join thread. exiting...\n";
            exit(-1);
        }
    }
    float time = timer.getTimeElapsed();
    delete[] index;
    return time;
}

float rows_as_threads(){
    /*returns time in ms*/

    int noofthreads = output_matrix->size[0];
    pthread_t tid[noofthreads];


    int * i = new int [noofthreads];

    Timer timer;

    for(int k = 0; k < noofthreads; k ++){
        i[k] = k;

        pthread_create(&tid[k],nullptr,call_multiply_row,&i[k]);
    }


    for(int k = 0; k < noofthreads; k ++){

        pthread_join(tid[k],nullptr);
    }
    
    float time = timer.getTimeElapsed();

    delete[] i;
    return time;
}