#include <iostream>
#include <fstream>
#include <chrono>
 
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
int multiplyElement( matrix * * matrices,const int * index){
    /*get dimensions*/
    int * size1 = matrices[0]->size;
    int * size2 = matrices[1]->size;

    int result = 0;
    int x,y; // 2 elements to multiply
    int k; //product of 2 elements

    if (size1[1] == size2[0]) //checking if possible
    {
        for (int i = 0; i < size1[1]; i++)
        {
            x = matrices[0]->arr[ index[0] * size1[1] + i];
            y = matrices[1]->arr[i * size2[1] + index[1]];
            k = x * y;
            result += k;
        }
        return result;
        
        //TODO: bad return, change later
    }else
        return -1;

}


int main(){

    matrix * * x = matrices("input.txt");

    int index[] = {1,3};
    std::cout << multiplyElement(x , index);

    return 0;
}