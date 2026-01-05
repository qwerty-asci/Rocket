Development of an environment for training a neural network using reinforcement learning to control a rocket with unlimited fuel by adjusting the nozzle orientation and activating or deactivating the rocket ignition.

The environment is designed to train an agent to maintain the rocket close to the origin of the coordinate system by adjusting the nozzle orientation, $`\theta`$, which is limited to a maximum angle of $`\frac{\pi}{6}`$. To this end, the system dynamics are simulated by numerically solving the equations of motion using a fixed-step fourth-order Runge–Kutta (RK4) method. The generalized coordinates chosen are the position of the center of mass, $`\overrightarrow{r}_{CM} = (x, y)`$, and the rotation of the rocket about its center of mass, $`\phi`$.

The external force $`F(t)`$ corresponds to the thrust produced by the rocket when the ignition is activated. It is modeled as a constant force that is switched on and off depending on the ignition state. Moreover, the agent can change the orientation of the nozzle to control the direction in which the thrust acts, thereby controlling the rocket’s motion.

The equations of motion are given by:

$`\ddot{x} = -\frac{F(t)}{m}\sin(\phi + \theta)`$

$`\ddot{y} = -g + \frac{F(t)}{m}\cos(\phi + \theta)`$

$`\ddot{\phi} = \frac{6F(t)b}{m(a^2 + b^2)}\sin(\theta)`$

This control problem can be addressed using Deep Q-Learning, where the agent selects one of the following discrete actions at each time step:

0: No action

1: Activate or deactivate the rocket ignition

2: Rotate the nozzle to the right or stop its rotation

3: Rotate the nozzle to the left or stop its rotation

Through interaction with the environment, the agent learns a policy that maximizes long-term rewards by keeping the rocket close to the origin of the coordinate system.

The state of the system is represented as:

$`x`$: Position of the center of mass along the $x$-axis

$`y`$: Position of the center of mass along the $y$-axis

$`\phi`$: Rotation angle about the center of mass

$`\dot{x}`$: Velocity of the center of mass along the $x$-axis

$`\dot{y}`$: Velocity of the center of mass along the $y$-axis

$`\dot{\phi}`$: Angular velocity about the center of mass

$`\theta`$: Nozzle orientation

Flag 1: Ignition state (activated or deactivated)

Flag 2: Nozzle rotation state (rotating and direction of rotation)

To train the model, the reward function is defined as:

$`R(t) = \frac{30(1 + t)}{1 + (x^2 + y^2)^2} \lvert \cos(\phi) \rvert`$

If the network fails during training, a penalty reward of –700 is assigned.

Each episode ends after 6000 steps, or when the conditions $`|x, y| < 10`$ and $`|\phi| < \frac{\pi}{3}`$ are satisfied.



![Network controlling the rocket](animacion5.gif)
