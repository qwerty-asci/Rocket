/**
 * @file rocket.cpp
 * @brief Rocket environment implementation for DQN training.
 *
 * This file defines the Rocket class that simulates a controllable rocket
 * using discrete actions. Numerical integration is implemented using
 * 4th-order Runge-Kutta. The class is exposed to Python via pybind11.
 */

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

#define theta_max 0.523598776 ///< Maximum rocket tilt angle (rad)
#define v_0_x 2.0             ///< Initial velocity in x
#define v_0_y 2.0             ///< Initial velocity in y
#define phi_0 acos(-1)/36.0   ///< Small initial random angle
#define x_range 1.5           ///< Initial x position range
#define y_range 1.5           ///< Initial y position range

#define phi_max acos(-1)/3.0  ///< Absolute tilt limit
#define area 10               ///< Simulation area size

#define FINISH_PENALIZATION -700.0 ///< Penalty for going out of bounds
#define steps_actualization 50     ///< Number of internal Runge-Kutta steps
#endif

using namespace std;
namespace py = pybind11;
using myclock = chrono::high_resolution_clock;

/**
 * @class Rocket
 * @brief Defines the rocket environment.
 *
 * The rocket can receive discrete actions:
 * - 0: Do nothing
 * - 1: Toggle engine on/off
 * - 2: Rotate nozzle right
 * - 3: Rotate nozzle left
 *
 * Provides functions for:
 * - reset(): Reset the environment
 * - step(action): Advance one simulation step
 * - sample(): Get a random action
 */
class Rocket {
private:
    mt19937 rng; ///< Random number generator
    uniform_real_distribution<double> rn{0,1.0}; ///< Continuous random
    uniform_int_distribution<unsigned int> rni{0,3}; ///< Discrete random for actions

    // Physical states
    double x, y, phi, u, v, w, t = 0.0, theta;
    const double a = 1.0, b = 1.0, m = 10.0, F_e = 130.0, w_theta = 2.0, h = 0.001;
    int ignition, rotation;
    bool flag; ///< Indicates if rocket is within valid area

    /**
     * @brief Calculates the effect of nozzle rotation on angle.
     * @param local_time Current time
     * @return Angle modified by rotation
     */
    double rotor(double local_time) {
        return theta + rotation * w_theta * (local_time - this->t);
    }

    /**
     * @brief Calculates derivatives for Runge-Kutta integration step.
     * @param t Current time
     * @param x Position x
     * @param y Position y
     * @param phi Tilt angle
     * @param u Velocity x
     * @param v Velocity y
     * @param w Angular velocity
     * @return Pointer to array {dx, dy, dphi, du, dv, dw}
     */
    double* differential_ecuations_step(double t, double x, double y, double phi, double u, double v, double w) {
        double* data = new double[6];
        double F = F_e * ignition;

        data[0] = u; // dx/dt
        data[1] = v; // dy/dt
        data[2] = w; // dphi/dt
        data[3] = -F * sin(phi + rotor(t)) / m; // du/dt
        data[4] = -g + F * cos(phi + rotor(t)) / m; // dv/dt
        data[5] = 6.0 * F * b * sin(rotor(t)) / (m * (a*a + b*b)); // dw/dt

        return data;
    }

    /**
     * @brief Checks if the rocket is within bounds and tilt limit.
     */
    void check() {
        flag = (abs(phi) >= phi_max || abs(x) >= area || abs(y) >= area) ? false : true;
    }

public:
    Rocket(); ///< Constructor

    py::array_t<double> reset(); ///< Reset environment with random values
    py::array_t<double> step(int action); ///< Advance simulation one step given action
    int sample(); ///< Return a random action
};

// ------------------- Method Implementations -------------------

Rocket::Rocket() : flag(true) {
    myclock::time_point d = myclock::now();
    myclock::duration d2 = myclock::now() - d;
    this->rng.seed((unsigned)d2.count()); // Seed RNG with time-based value

    x = 0.0; y = 0.0; phi = 0.0;
    u = 0.0; v = 0.0; w = 0.0;
    theta = 0.0; ignition = 0; rotation = 0;

    this->reset();
}

py::array_t<double> Rocket::reset() {
    // Random initial positions and velocities
    x = rn(rng)*x_range - x_range/2.0;
    y = rn(rng)*y_range - y_range/2.0;
    phi = rn(rng)*phi_0 - phi_0/2.0;
    u = 2.0*v_0_x*rn(rng) - v_0_x;
    v = 2.0*v_0_y*rn(rng) - v_0_y;
    theta = (theta_max*rn(rng)-theta_max/2.0)*0.25;
    w = 0.0;
    t = 0.0;
    flag = true;

    py::array_t<double> arr(9);
    double* data = static_cast<double*>(arr.request().ptr);
    data[0] = x; data[1] = y; data[2] = phi;
    data[3] = u; data[4] = v; data[5] = w;
    data[6] = theta; data[7] = ignition; data[8] = rotation;

    return arr;
}

py::array_t<double> Rocket::step(int action) {
    py::array_t<double> arr;

    // Apply action
    if(action==1) ignition = static_cast<int>((pow(-1,ignition)+1)/2.0);
    else if(action==2) rotation = static_cast<int>((pow(-1,rotation)+1)/2.0);
    else if(action==3) rotation = -static_cast<int>((pow(-1,rotation)+1)/2.0);

    double* data = new double[11], *k1ptr, *k2ptr, *k3ptr, *k4ptr, r=0.0;

    if(flag) {
        // Runge-Kutta integration loop
        for(int i=0;i<steps_actualization && flag;i++){
            k1ptr=differential_ecuations_step(t,x,y,phi,u,v,w);
            k2ptr=differential_ecuations_step(t+h*0.5,x+0.5*k1ptr[0]*h,y+0.5*k1ptr[1]*h,phi+0.5*k1ptr[2]*h,u+0.5*k1ptr[3]*h,v+0.5*k1ptr[4]*h,w+0.5*k1ptr[5]*h);
            k3ptr=differential_ecuations_step(t+h*0.5,x+0.5*k2ptr[0]*h,y+0.5*k2ptr[1]*h,phi+0.5*k2ptr[2]*h,u+0.5*k2ptr[3]*h,v+0.5*k2ptr[4]*h,w+0.5*k2ptr[5]*h);
            k4ptr=differential_ecuations_step(t+h,x+k3ptr[0]*h,y+k3ptr[1]*h,phi+k3ptr[2]*h,u+k3ptr[3]*h,v+k3ptr[4]*h,w+k3ptr[5]*h);

            // Update states
            x += h*(k1ptr[0]+2*k2ptr[0]+2*k3ptr[0]+k4ptr[0])/6.0;
            y += h*(k1ptr[1]+2*k2ptr[1]+2*k3ptr[1]+k4ptr[1])/6.0;
            phi += h*(k1ptr[2]+2*k2ptr[2]+2*k3ptr[2]+k4ptr[2])/6.0;
            u += h*(k1ptr[3]+2*k2ptr[3]+2*k3ptr[3]+k4ptr[3])/6.0;
            v += h*(k1ptr[4]+2*k2ptr[4]+2*k3ptr[4]+k4ptr[4])/6.0;
            w += h*(k1ptr[5]+2*k2ptr[5]+2*k3ptr[5]+k4ptr[5])/6.0;

            // Update tilt angle
            theta += rotation*w_theta*h;
            theta = (abs(theta)>=theta_max) ? theta_max*theta/abs(theta) : theta;

            t += h;
            check();
        }

        // Reward calculation
        r += 30.0*(1.0+t)/(1.0+pow(x*x+y*y,2))*abs(cos(phi));

        data[0]=x; data[1]=y; data[2]=phi;
        data[3]=u; data[4]=v; data[5]=w;
        data[6]=theta; data[7]=ignition; data[8]=rotation;
        data[9]=flag ? r : FINISH_PENALIZATION;
        data[10]=static_cast<double>(flag);
    } else {
        // Rocket out of bounds
        data[0]=x; data[1]=y; data[2]=phi;
        data[3]=u; data[4]=v; data[5]=w;
        data[6]=theta; data[7]=ignition; data[8]=rotation;
        data[9]=FINISH_PENALIZATION;
        data[10]=static_cast<double>(flag);
    }

    // Wrap into numpy array
    arr = py::array_t<double>(
        py::buffer_info(
            data,
            sizeof(double),
            py::format_descriptor<double>::format(),
            1,
            vector<ssize_t>{11},
            vector<ssize_t>{(long int)(sizeof(double))}
        )
    );

    return arr;
}

int Rocket::sample() {
    return rni(rng); // Random discrete action
}

// ------------------- Pybind11 Binding -------------------

PYBIND11_MODULE(Rocket, m, py::mod_gil_not_used()) {
    py::class_<Rocket>(m,"Rocket","Environment for training a neural network to control a rocket")
        .def(py::init())
        .def("sample",&Rocket::sample,"Draw a random action from the possible rocket actions")
        .def("step",&Rocket::step,"Advance one step using Runge-Kutta integration")
        .def("reset",&Rocket::reset,"Reset the environment with new random state values");
}
