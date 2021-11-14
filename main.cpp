#include <iostream>
#include <fstream>
#include <chrono>
#include <pthread.h>
#include <unistd.h>
#include <cstdlib>

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
void * multiplyElement(void * indexptr){
    int * index  = (int * )indexptr;
    /*get dimensions*/
    int * size1 = inputMatrices[0]->size;
    int * size2 = inputMatrices[1]->size;

    int result = 0;
    int x,y; // 2 elements to multiply
    int k; //product of 2 elements

    if (size1[1] == size2[0]) //checking if possible
    {
        for (int i = 0; i < size1[1]; i++)
        {
            x = inputMatrices[0]->arr[ index[0] * size1[1] + i];
            y = inputMatrices[1]->arr[i * size2[1] + index[1]];
            k = x * y;
            result += k;
        }
        std::cout << result << "\t";
                
        //TODO: bad return, change later
    }
    pthread_exit(0);

}

/*procedure 1*/
int elements_as_threads(){
    /*start timer*/
    Timer timer;

    int noof_threads = inputMatrices[0]->size[0] * inputMatrices[1]->size[1];
    int returnval;

    int * index = new int[2];
    
    /*initialize threads and attributes*/
    pthread_t threads[noof_threads]; //to store thread id's
    pthread_attr_t attr;
    void * status;

    /*set as joinable thread*/
    //pthread_attr_init(&attr);
    //pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

    /*creating threads*/

    for(int i =0; i < inputMatrices[0]->size[0]; i++)
    {
        /*row*/
        for (int j = 0; j < inputMatrices[1]->size[1]; j++){
            /*element*/
            index[0] = i;
            index[1] = j; 
            returnval = pthread_create(&threads[i * inputMatrices[0]->size[1] + j], 0,multiplyElement, (void *) index);
            if (returnval)
                std::cout << "Error: unable to create thread, "<< returnval << "\n";
        }      
        std::cout << "\n";

    }

    /*free attr and wait for threads*/
    //pthread_attr_destroy(&attr);
    //for(int i = 0; i < noof_threads; i++){
    //    returnval = pthread_join(threads[i], &status);
    //    if(returnval)
    //        std::cout << "Error: unable to join thread, "<< returnval << "\n";
//
    //}

    std::cout << "END1\t";

    return 0;
}


int main(){

    ///*get file name*/
    //char * fileName =(char *) malloc(FILENAME_MAX*sizeof(char));
//
    //std::cout << "Enter the name of the input file: ";
    //std::cin >> fileName;
//


    /*read matrices from file*/
    inputMatrices = matrices("input.txt");

    elements_as_threads();

    
    return 0;
}