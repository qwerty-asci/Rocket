# Rocket DQN Environment

Development of an environment for training a neural network using reinforcement learning to control a rocket with unlimited fuel by adjusting the nozzle orientation and activating or deactivating the rocket ignition.

The environment is designed to train an agent to maintain the rocket close to the origin of the coordinate system by adjusting the nozzle orientation, $\theta$, which is limited to a maximum angle of $\frac{\pi}{6}$. The system dynamics are simulated by numerically solving the equations of motion using a fixed-step fourth-order Runge–Kutta (RK4) method. The generalized coordinates chosen are the position of the center of mass, $\overrightarrow{r}_{CM} = (x, y)$, and the rotation of the rocket about its center of mass, $\phi$.

The external force $F(t)$ corresponds to the thrust produced by the rocket when the ignition is activated. It is modeled as a constant force that is switched on and off depending on the ignition state. The agent can also change the orientation of the nozzle to control the direction in which the thrust acts, thereby controlling the rocket’s motion.

The equations of motion are:

$$
\ddot{x} = -\frac{F(t)}{m}\sin(\phi + \theta)
$$

$$
\ddot{y} = -g + \frac{F(t)}{m}\cos(\phi + \theta)
$$

$$
\ddot{\phi} = \frac{6F(t)b}{m(a^2 + b^2)}\sin(\theta)
$$

This control problem can be approached using Deep Q-Learning (DQN), where the agent selects one of the following discrete actions at each time step:

- **0**: No action
- **1**: Activate or deactivate the rocket ignition
- **2**: Rotate the nozzle to the right or stop its rotation
- **3**: Rotate the nozzle to the left or stop its rotation

Through interaction with the environment, the agent learns a policy that maximizes long-term rewards by keeping the rocket close to the origin of the coordinate system.

### State Representation

The state of the system is represented as:

| Variable        | Description                                    |
|-----------------|------------------------------------------------|
| `x`             | Position of the center of mass along the x-axis |
| `y`             | Position of the center of mass along the y-axis |
| `phi`           | Rotation angle about the center of mass        |
| `dot_x`         | Velocity of the center of mass along x-axis    |
| `dot_y`         | Velocity of the center of mass along y-axis    |
| `dot_phi`       | Angular velocity about the center of mass     |
| `theta`         | Nozzle orientation                             |
| Flag 1          | Ignition state (on/off)                        |
| Flag 2          | Nozzle rotation state (direction)              |

### Reward Function

The reward function is defined as:

$$
R(t) = \frac{30(1 + t)}{1 + (x^2 + y^2)^2} \lvert \cos(\phi) \rvert
$$

If the rocket fails during training, a penalty reward of –700 is assigned.
Each episode ends after 6000 steps or when the conditions $|x|, |y| < 10$ and $|\phi| < \frac{\pi}{3}$ are violated.

---

### Project Structure

A recommended project layout for clarity and maintainability:



```text
Rocket/
│
├── setup.py                  # Setup script to compile the C++ shared library
├── Makefile                  # Build commands for library and docs
├── requirements.txt          # Python dependencies
├── cpp/                      # C++ source files
│   └── rocket.cpp            # Core environment implementation
├── notebooks/                # Jupyter notebooks for training & testing
│   └── rocket.ipynb
├── models/                   # Saved neural network checkpoints
│   └── net.pth
├── build/                    # Output folder for compiled shared library
│   └── Rocket.so             # Compiled Python module
├── docs/                     # Documentation generated with Doxygen
│   └── html/
└── animaciones               # Examples animations of the trained rocket
```





- **`cpp/`** contains the core C++ environment.
- **`python/`** can include helper scripts, wrappers, or utilities for training.
- **`notebooks/`** contains Jupyter notebooks for training, visualization, and testing the agent.
- **`models/`** stores trained model checkpoints.
- **`docs/`** is generated automatically via Doxygen to document the C++ API.

---

### Installation and Usage

1. **Install dependencies**:

```bash
pip install -r requirements.txt
sudo apt-get update
sudo apt-get install -y build-essential
pip install pybind11
```

2. **Build the shared library:**

```bash
make
```

This will compile rocket.cpp and generate the Python module Rocket in-place.


3. Import the library in notebooks:

```python
import sys, os
sys.path.append(os.path.abspath("../build"))  # Adjust if the .so is in a different folder
import Rocket as rck

env = rck.Rocket()
state = env.reset()
action = env.sample()
next_state = env.step(action)

```

4. Run training: Open notebooks/rocket.ipynb and run the cells.


- Visualization

This shows the rocket being controlled by a trained neural network.


## Documentation

Run the following command to generate API documentation:
```bash
doxygen Doxyfile
```

Then open docs/html/index.html in a browser

## Results

![Network controlling the rocket](animaciones/animacion5.gif)
