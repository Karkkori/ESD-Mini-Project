# ESD Mini Project

## Introduction
This project integrates a Lilygo smartwatch with a Raspberry Pi to track and visualize hiking activities through a user-friendly Flask web application. The LILYGO watch captures real-time steps and traveled distance which are synchronized to the Rasberry Pi webserver utilizing the Bluetooth low energy connection. The webserver provides a web user interface that offers hikers insightful visualizations of their performance, showing the step count, traveled distance and burnt calories. The Lilygo smartwatch software and all the included files are found inside the Watch_TTGo_fw, and the Raspberry Pi software and related files can be found inside teh Rpi folder.

## Cloning the repository

To clone the repository, install the git following these instructions: https://github.com/git-guides/install-git. \
After installing the git, move to the folder in which you want to clone the respository and run the following command via terminal:
```
git clone https://github.com/Karkkori/ESD-Mini-Project.git
```
## Programming the LILYGO watch

For LILYGO watch programming, we recommed using VS Code with Platform IO extension. \
The Platform IO environment and project can be established following the instuctions in this video:\
https://www.youtube.com/watch?v=wUGADCnerCs \
The Watch_TTGo_fw folder includes all the required libraries and dependencies, and the watch can be programmed by modifying the main.cpp file in the src-folder.
The code can be uploaded to the watch by following the instuctions in the video linked above.

## Setting up the RPi 
To program and run the software in RPi, we recommend using the virtual environment.
After moving to the project folder, the virtual environment can be established by running the following command via terminal:
```
source .venv/bin/activate

```
## Installing dependencies for RPi
Before installing the project dependencies, please ensure, that the virtual environment is activated.\
Dependencies for this project can be installed via terminal:
```
pip install -r requirements.txt
```
If there are challenges installing pybluez library using the requirements.txt, run the following command in the terminal window in the virtual environment:
```
pip install git+https://github.com/pybluez/pybluez.git#egg=pybluez
```

## Running the RPi webserver
To run the the data receiver and webserver simultaneously, run the following command via terminal:
```
./start_app.sh
```
This command runs the start_app.sh which operates the receiver.py and wserver.py the same time providing the full webserver functionality.

