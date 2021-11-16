#include <iostream>
#include <fstream>
#include <chrono>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

/*formula for indices: [i][j] = i * noofcolumns + j*/
 
/*to time any function*/
struct Timer
{

    std::chrono::time_point<std::chrono::system_clock> start;
    std::chrono::duration<float> duration;

    Timer(){
         start = std::chrono::high_resolution_clock::now();
    }
    
    ~Timer(){

        auto end  = std::chrono::high_resolution_clock::now();
        duration = end - start;
        
        float ms = duration.count()*1000.00f;
        std::cout << "Finished in "<<ms<<" ms\n";
    }
};

/*defining a matrix*/
struct matrix
{
    int * size;
    int * arr;
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
    }else
        return nullptr;
}

/*multiply element*/
int * multiplyElement(int * index){

    /*get dimensions*/
    int * size1 = inputMatrices[0]->size;
    int * size2 = inputMatrices[1]->size;

    int * result = new int;
    * result = 0;
    int x,y; // 2 elements to multiply
    int k; //product of 2 elements

    
    for (int i = 0; i < size1[1]; i++)
    {
        x = inputMatrices[0]->arr[ index[0] * size1[1] + i];
        y = inputMatrices[1]->arr[i * size2[1] + index[1]];
        k = x * y;
        * result += k;
    }
    output_matrix->arr[ index[0] * output_matrix->size[1] + index[1]] = * result;
    return result;
}

/*function to call element multiplier - for 1st procedure*/
void * call_multiply_element(void * ptr){

    int * index = (int *)ptr;

    /*element*/
    int * trash = multiplyElement(index);
    delete(trash);
                  
    pthread_exit(0);

}

/*function to call element multiplier for each row - procedure 2*/
void * call_multiply_row(void * ptr){
    
    int * i = (int *)ptr;
    int * index = new int[2];
    index[0] = *i;
    
    /*row*/
    for(int j = 0; j < output_matrix->size[1]; j ++){
        /*element*/
        index[1] = j;
        multiplyElement(index);
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


void elements_as_threads();
void rows_as_threads();

int main(){

    ///*get file name*/
    //char * fileName =(char *) malloc(FILENAME_MAX*sizeof(char));
//
    //std::cout << "Enter the name of the input file: \n";
    //std::cin >> fileName;
//


    /*read matrices from file*/
    inputMatrices = matrices("input.txt");


    /*check if multiplication is possible*/
    if( inputMatrices[0]->size[1] != inputMatrices[1]->size[0] )
    {
        std::cout << "Invalid input: matrices cannot be multipled. Exiting...\n";
        return 0;
    }

    /*initialize output matrix*/
    output_matrix = new matrix;

    int * outsize = new int[2];
    outsize[0] = inputMatrices[0]->size[0];
    outsize[1] = inputMatrices[1]->size[1];
    output_matrix->size = outsize;

    elements_as_threads();
    rows_as_threads();

    /*cleanup*/
    delete(output_matrix->arr);
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

void elements_as_threads(){
    Timer  timer;
    int * index = new int[2];
    void * ptr = (void *) index;


    int noofthreads = inputMatrices[0]->size[0] * inputMatrices[1]->size[1];
    pthread_t tid[noofthreads];

    int k;//actual index of thread

    /*creating threads*/
    
    for(int i =0; i < output_matrix->size[0]; i++)
    {
        /*row*/
        index[0] = i;

        for (int j = 0; j < output_matrix->size[1]; j++){

            /*element*/
            k = i * output_matrix->size[1] + j; 
            index[1] = j; 

            if(pthread_create(&tid[ k],nullptr,&call_multiply_element,ptr)){
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
    delete(index);

    print_matrix(output_matrix);


    std::cout << "END1\t";
}

void rows_as_threads(){

    Timer timer;

    int noofthreads = output_matrix->size[0];
    pthread_t tid[noofthreads];


    int * i  = new int;
    
    for(int k = 0; k < noofthreads; k ++){
        *i = k;

        pthread_create(&tid[k],nullptr,&call_multiply_row,(void *)i);
    }


    for(int k = 0; k < noofthreads; k ++){
        *i = k;

        pthread_join(tid[k],nullptr);
    }
    
    print_matrix(output_matrix);

    std::cout << "END2\t";

    delete(i);
}