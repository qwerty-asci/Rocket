#include <iostream>
#include <string>
#include <random>
#include <chrono>

#include <Python.h>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <climits>
#include <math.h>
#include <utility>
#include <vector>

#ifndef g
#define g 9.81


#define theta_max 0.523598776
#define v_0_x 2.0
#define v_0_y 2.0
#define phi_0 acos(-1)/36.0
#define x_range 1.5
#define y_range 1.5


#define phi_max acos(-1)/3.0
#define area 10

#define FINISH_PENALIZATION -700.0
#define steps_actualization 50


#endif

using namespace std;
namespace py=pybind11;
using myclock=chrono::high_resolution_clock;


class Rocket{

private:

    mt19937 rng;
    uniform_real_distribution<double> rn{0,1.0};
    uniform_int_distribution<unsigned int> rni{0,3};


    double x,y,phi,u,v,w,t=0.0,theta;
    const double a=1.0,b=1.0,m=10.0,F_e=130.0,w_theta=2.0,h=0.001;
    int ignition,rotation;
    bool flag;


    double rotor(double local_time){

        // theta+=rotation*w_theta*h;

        // theta=(abs(theta)>=theta_max)?theta_max*theta/abs(theta):theta;

        return theta+rotation*w_theta*(local_time-this->t);

    }

    double * differential_ecuations_step(double t,double x,double y,double phi,double u, double v,double w){


        double * data=new double[6];

        double F=F_e*ignition;

        data[0]=u;
        // data[0]=-x*x*t+cos(t)*x*x*t;
        data[1]=v;
        data[2]=w;
        data[3]=-F*sin(phi+rotor(t))/m;
        data[4]=-g+F*cos(phi+rotor(t))/m;
        data[5]=6.0*F*b*sin(rotor(t))/(m*(a*a+b*b));

        return data;
    }

    void check(){

        flag=(abs(phi)>=phi_max || abs(x)>=area || abs(y)>=area)?false:true;

    }



public:

    Rocket();


    py::array_t<double> reset();
    py::array_t<double> step(int action);
    int sample();


};




Rocket::Rocket():flag(true){

    myclock::time_point d=myclock::now();
    myclock::duration d2=myclock::now()-d;
    this->rng.seed((unsigned)d2.count());

    x=0.0;
    y=0.0;
    phi=0.0;
    u=0.0;
    v=0.0;
    w=0.0;
    theta=0.0;
    ignition=0;
    rotation=0;

    this->reset();

}


py::array_t<double> Rocket::reset(){


    x=rn(rng)*x_range-x_range/2.0;
    y=rn(rng)*y_range-y_range/2.0;
    phi=rn(rng)*phi_0-phi_0/2.0;
    u=2.0*v_0_x*rn(rng)-v_0_x;
    v=2.0*v_0_y*rn(rng)-v_0_y;
    theta=(theta_max*rn(rng)-theta_max/2.0)*0.25;
    w=0.0;
    t=0.0;

    flag=true;

    py::array_t<double> arr(9);


    double * data=static_cast<double *>(arr.request().ptr);


    data[0]=x;
    data[1]=y;
    data[2]=phi;
    data[3]=u;
    data[4]=v;
    data[5]=w;
    data[6]=theta;
    data[7]=ignition;
    data[8]=rotation;


    return arr;
}





py::array_t<double> Rocket::step(int action){

    /*

        Actions:

        0->Do nothing
        1-> Activate the engine (if the engine is already activated and receive 1, the rocket will turn off the motor)
        2-> rotate the nozzle to the right (if the nozzle is rotating to one of the sides and the instruction to rotate is received again, the nozzle will stop the rotation)
        3-> rotate the nozzle to the left

    */


    py::array_t<double> arr;

    if(action==1){
        ignition=static_cast<int>((pow(-1,ignition)+1)/2.0);
    }else if(action==2){
        rotation=static_cast<int>((pow(-1,rotation)+1)/2.0);
    }else if(action==3){
        rotation=-static_cast<int>((pow(-1,rotation)+1)/2.0);
    }

    double * data=new double[11],* k1ptr,* k2ptr,* k3ptr,* k4ptr,r=0.0;

    if(flag){

        for(int i=0;i<steps_actualization && flag;i++){
            k1ptr=differential_ecuations_step(t,x,y,phi,u,v,w);

            k2ptr=differential_ecuations_step(t+h*0.5,x+0.5*k1ptr[0]*h,y+0.5*k1ptr[1]*h,phi+0.5*k1ptr[2]*h,u+0.5*k1ptr[3]*h,v+0.5*k1ptr[4]*h,w+0.5*k1ptr[5]*h);

            k3ptr=differential_ecuations_step(t+h*0.5,x+0.5*k2ptr[0]*h,y+0.5*k2ptr[1]*h,phi+0.5*k2ptr[2]*h,u+0.5*k2ptr[3]*h,v+0.5*k2ptr[4]*h,w+0.5*k2ptr[5]*h);

            k4ptr=differential_ecuations_step(t+h,x+k3ptr[0]*h,y+k3ptr[1]*h,phi+k3ptr[2]*h,u+k3ptr[3]*h,v+k3ptr[4]*h,w+k3ptr[5]*h);



            x+=h*(k1ptr[0]+2.0*k2ptr[0]+2.0*k3ptr[0]+k4ptr[0])/6.0;
            y+=h*(k1ptr[1]+2.0*k2ptr[1]+2.0*k3ptr[1]+k4ptr[1])/6.0;
            phi+=h*(k1ptr[2]+2.0*k2ptr[2]+2.0*k3ptr[2]+k4ptr[2])/6.0;
            u+=h*(k1ptr[3]+2.0*k2ptr[3]+2.0*k3ptr[3]+k4ptr[3])/6.0;
            v+=h*(k1ptr[4]+2.0*k2ptr[4]+2.0*k3ptr[4]+k4ptr[4])/6.0;
            w+=h*(k1ptr[5]+2.0*k2ptr[5]+2.0*k3ptr[5]+k4ptr[5])/6.0;

            theta+=rotation*w_theta*h;
            theta=(abs(theta)>=theta_max)?theta_max*theta/abs(theta):theta;


            t+=h;



            check();
        }

        r+=30.0*(1.0+t)/(1.0+pow(x*x+y*y,2))*abs(cos(phi));

        data[0]=x;
        data[1]=y;
        data[2]=phi;
        data[3]=u;
        data[4]=v;
        data[5]=w;
        data[6]=theta;
        data[7]=ignition;
        data[8]=rotation;
        data[9]=flag?r:FINISH_PENALIZATION;
        data[10]=static_cast<double>(flag);

        // cout<<ignition<<" "<<rotation<<endl;

        arr=py::array_t<double>(
            py::buffer_info(
                data,
                sizeof(double),
                py::format_descriptor<double>::format(),
                1,
                vector<ssize_t>{11},
                vector<ssize_t>{(long int)(sizeof(double))}
            )
        );



    }else{

        data[0]=x;
        data[1]=y;
        data[2]=phi;
        data[3]=u;
        data[4]=v;
        data[5]=w;
        data[6]=theta;
        data[7]=ignition;
        data[8]=rotation;
        data[9]=FINISH_PENALIZATION;
        data[10]=static_cast<double>(flag);

        // cout<<ignition<<" "<<rotation<<endl;

        arr=py::array_t<double>(
            py::buffer_info(
                data,
                sizeof(double),
                py::format_descriptor<double>::format(),
                1,
                vector<ssize_t>{11},
                vector<ssize_t>{(long int)(sizeof(double))}
            )
        );

    }

    return arr;

}






int Rocket::sample(){
    return rni(rng);
}




PYBIND11_MODULE(Rocket, m, py::mod_gil_not_used()) {


    py::class_<Rocket>(m,"Rocket","Enviroment for train a neural network to control a rocket").
    def(py::init()).
    def("sample",&Rocket::sample,"Draw a random sample from the possible actions that the rocket can take").
    def("step",&Rocket::step,"Takes a forward step in the integration following the Runge-Kutta method").
    def("reset",&Rocket::reset,"Reset the Enviroment introducing new random values for all the state parameters");

}
