# ESD Mini Project

## Introduction
This project integrates a Lilygo smartwatch with a Raspberry Pi to track and visualize hiking activities through a user-friendly Flask web application. By leveraging Bluetooth for data transmission, it captures real-time metrics like steps, calories burned, and distance traveled. The result is a comprehensive dashboard that offers hikers insightful visualizations of their performance, enhancing their outdoor experiences with technology. The Lilygo smartwatch software and all the included files are found inside the Watch_TTGo_fw and the Raspberry Pi software and related files can be found inside teh Rpi folder.

## Cloning the repository

To clone the repository, install the git following these instructions: https://github.com/git-guides/install-git. \
After installing the git, move to the folder in which you want to clone the respository and run the following command via terminal:
```
git clone https://github.com/Karkkori/ESD-Mini-Project.git
```
## Setting up the LILYGO watch programming environment

For LILYGO watch programming, we recommed using VS Code with Platform IO extension. \
The Platform IO environment and project can be established following the instuctions in this video:\
https://www.youtube.com/watch?v=wUGADCnerCs \
The Watch_TTGo_fw folder includes all the required libraries and dependencies.\
The watch is programmed by editing the main.cpp file found in the src-folder.
## Installing dependencies for RPi
To install this project, install dependencies via terminal:
```
pip install -r requirements.txt
```
## Usage
To use this project, enter following commands to terminal:

```
source .venv/bin/activate
./start_app.sh
```



## License
MIT License. See the [LICENSE](https://opensource.org/license/mit) file for more details.
