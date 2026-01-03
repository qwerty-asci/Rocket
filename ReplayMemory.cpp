#include <iostream>
#include <string>
#include <random>
#include <chrono>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <climits>
#include <vector>
#include <utility>



using namespace std;
namespace py=pybind11;
using myclock=chrono::high_resolution_clock;




#pragma GCC visibility push(hidden)
class ReplayMemory{

private:

    //Random number generator, that is going to be used to shuffle the list
    mt19937 rng;
    uniform_int_distribution<unsigned int> rni{0,UINT_MAX};

    py::array_t<double> * data;

    //Dimension of the replay memory
    unsigned long int * shuffle_list,length,width,cont=0,dimension=0,position=0;






public:

    ReplayMemory(unsigned long int,unsigned long int);
    void append(py::array_t<double>);
    unsigned long int len();
    void shuffle();
    py::array_t<double> batch(unsigned long int);
    ~ReplayMemory();


};





ReplayMemory::ReplayMemory(unsigned long int length, unsigned long int width):length(length),width(width){
    myclock::time_point d=myclock::now();
    myclock::duration d2=myclock::now()-d;
    this->rng.seed((unsigned)d2.count());

    try{
        data=new py::array_t<double>[length];
    }catch(bad_alloc& ba){
        cerr<<"The replay memory could not be created: "<<ba.what()<<endl;
        data=nullptr;
    }

    position=0;
}

void ReplayMemory::append(py::array_t<double> arr){
    this->data[cont]=arr;
    cont=(cont==(length-1))?0:cont+1;
    dimension=(dimension==(length))?dimension:dimension+1;
}

unsigned long int ReplayMemory::len(){
    return this->dimension;
}

ReplayMemory::~ReplayMemory(){

    if(data!=nullptr){
        delete[] data;
    }
    if(shuffle_list!=nullptr){
        delete[] shuffle_list;
    }
}





void ReplayMemory::shuffle(){

    unsigned long int * random_numbers,contador=0;

    try{
        this->shuffle_list=new unsigned long int[this->dimension];
        random_numbers=new unsigned long int[this->dimension];
    }catch(bad_alloc& ba){
        cerr<<"The replay memory could not be created: "<<ba.what()<<endl;
        this->shuffle_list=nullptr;
        random_numbers=nullptr;
    }

    random_numbers[0]=rni(rng);
    shuffle_list[0]=0;




    for(long long int i=1;i<(long long int)dimension;i++){
        random_numbers[i]=rni(rng);
        shuffle_list[i]=i;
        contador=i;
        for(long long int j=i-1;j>=0;j--){
            if(random_numbers[contador]>random_numbers[j]){

                swap(random_numbers[j],random_numbers[contador]);
                swap(shuffle_list[j],shuffle_list[contador]);
                contador=j;

            }else{
                break;
            }
        }

    }

    if(random_numbers!=nullptr){
        delete[] random_numbers;
    }
}


py::array_t<double> ReplayMemory::batch(unsigned long int bsize){


    if(this->shuffle_list==nullptr){
        this->shuffle();
    }


    int rest=(position+bsize<=dimension-1)?bsize:dimension-position;


    py::array_t<double> arr;

    arr=py::array_t<double>(py::buffer_info(
        new double[rest*width],
        sizeof(double),
        py::format_descriptor<double>::format(),
        2,
        vector<ssize_t>{(long int)rest,(long int)width},
        vector<ssize_t>{(long int)(sizeof(double)*width),(long int)(sizeof(double))}
    ));

    double * buff=static_cast<double*>(arr.request().ptr),* buff2;


    position+=rest;

    for(unsigned long int i=position-rest;i<position;i++){

        buff2=static_cast<double *>(data[shuffle_list[i]].request().ptr);


        memcpy(buff+(long int)(this->width*(i-position+rest)),buff2,(long int)(sizeof(double)*this->width));
    }

    if(position==this->dimension){
        position=0;
        delete[] this->shuffle_list;
        this->shuffle_list=nullptr;
    }



    return arr;
}


PYBIND11_MODULE(ReplayMemory, m, py::mod_gil_not_used()) {

    py::class_<ReplayMemory>(m,"ReplayMemory","Library to implement a replay memory more efficiently").
    def(py::init<unsigned long int,unsigned long int>()).
    def("append",&ReplayMemory::append,"Add a new element to the replay memory").
    def("len",&ReplayMemory::len,"Return the current dimension of the replay memory").
    def("shuffle",&ReplayMemory::shuffle,"Creates a copy of the replay memory with all the data shuffled").
    def("batch",&ReplayMemory::batch,"Return a batch size of the data");

}
