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
    
    int x , y;

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
        std::cout << "Invalid input: matrices cannot be multipled\n";
        return 0;
    }

    /*initialize output matrix*/
    output_matrix = new matrix;

    int * outsize = new int[2];
    outsize[0] = inputMatrices[0]->size[0];
    outsize[1] = inputMatrices[1]->size[1];
    output_matrix->size = outsize;

    pthread_t trash;

    elements_as_threads();

    //elements_as_threads()


    print_matrix(output_matrix);

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
    Timer timer;
    intptr_t * index = new intptr_t[2];

    int noofthreads = inputMatrices[0]->size[0] * inputMatrices[1]->size[1];
    pthread_t tid[noofthreads];
    pthread_attr_t attr;
    void * status;
    int threads[noofthreads];
    int k;

    /*creating threads*/
    if(pthread_attr_init(&attr) != 0){
        perror("attr init");
        exit(-1);
    }

    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
    
    for(int i =0; i < inputMatrices[0]->size[0]; i++)
    {
        if(i > 0)break;
        /*row*/
        index[0] = i;

        for (int j = 0; j < inputMatrices[1]->size[1]; j++){
            if (j == 2)break;

            /*element*/
            k = i * inputMatrices[1]->size[1] + j; //actual index
            index[1] = j; 
            
            void * ptr = (void *) index;

            threads[k] = pthread_create(&tid[ k],&attr,call_multiply_element,ptr);

            if(threads[k]){
                std::cout << "Unable to create thread. exiting...\n";
                exit(-1);
            }
        } 
    }

    /*free attr and join*/
    int returnval;
    pthread_attr_destroy(&attr);
    for(int i = 0; i< 2; i++){//was noofthreads
        returnval = pthread_join(tid[i], &status);
        if(returnval){
            std::cout << "Unable to join thread. exiting...\n";
            exit(-1);
        }
    }
    delete(index);

    std::cout << "END1, status:\t"<<status<<"\n";
}