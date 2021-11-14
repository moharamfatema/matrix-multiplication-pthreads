#include <iostream>
#include <fstream>
#include <chrono>
 

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


/*read from file*/
int* matrix(){
    std::ifstream fin;
    fin.open("input.txt");
    int x,y;
    int sizeofint = sizeof(int);

    if (fin)
    {
        Timer timer;

        /* get matrix size */
        fin >> x >> y;

        /*creaing one dimensional arr to improve performance*/

        int* result = new int[x * y];
        for(int i =0;i<x;i++){
            for (int j = 0; j < y; j++)
            {
                fin >> result[i * y + j];
            }
            
        }
     
        
        return result;
    }else
        return nullptr;
}


int main(){

    int * x = matrix();

    return 0;
}