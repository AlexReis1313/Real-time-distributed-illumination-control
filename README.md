## Description of the problem

The project’s objective is to design a decentralized and collaborative real-time control system
for lighting in an open workspace. The system features a network of luminaires, each consisting
of an LED, an LDR sensor, an Raspberry Pi Pico, and a CAN-BUS interface for network
communication.
In its initial stage, the project concentrates on setting up a controller for each light and
establishing a CAN-BUS communication framework. These actions provide a foundation for
moving towards an integrated, real-time control system across the network.
The first phase of the project involves creating a serial communication protocol that connects
the Raspberry Pi Pico microcontrollers to a computer interface. This protocol allows for real-
time adjustments and monitoring of the system’s parameters and performance.
Moving to the next phase, the previously established CAN-BUS protocol becomes oper-
ational, allowing for coordinated control among all the microcontrollers. Then, utilizing a
consensus algorithm, the system intelligently balances energy use with lighting needs by con-
sidering the impact of each luminaire on its surroundings. This method aims to minimize
energy consumption while ensuring adequate lighting levels, whether desks are in use or not.

<p align="center">
  <img src="https://github.com/duarte3333/Real-time-distributed-illumination-control/assets/76222459/5ebee8c6-d486-44c7-8e79-098269cdda52" alt="Image Description" style="width: 50%;">
</p>

## Setup

So, the primary goals of this project are the following:

  •   Minimize Energy Consumption: Achieve energy efficiency by adjusting the dimming
level of each LED to the minimum necessary to satisfy comfort requirements.

  •   Maximize User Comfort: Ensure the illumination is always at or above visibility
lower bounds, respond quickly to user commands and minimize fast illuminance variations
(flicker).

<p align="center">
  <img src="https://github.com/duarte3333/Real-time-distributed-illumination-control/assets/76222459/31d579b2-f07f-4a9f-a4d7-309bd53fea68" style="width: 40%;">
</p>

<p align="center">
  <img src="https://github.com/duarte3333/Real-time-distributed-illumination-control/assets/76222459/153eaa64-3d5d-46ea-abf8-ecdcd78bd67f" alt="Image Description" style="width: 50%;">
</p>

## Key Concepts

### What is CAN-Bus?

CAN-Bus (Controller Area Network Bus) is a communication system commonly used in vehicles and various types of machinery to allow microcontrollers and devices to communicate with each other without needing a central computer. Think of it like a method for various parts of a car—like the engine, brakes, and air conditioning—to talk to each other and coordinate functions without sending everything through a main computer. This system is efficient because it reduces wiring complexity and helps various components to work together effectively.

### What is a Control System?

A control system is a set of mechanical or electronic devices that manages, commands, directs, or regulates the behavior of other devices or systems. It typically consists of interconnected components that receive and process signals from sensors, then use this information to send commands to actuators or other output devices. Control systems are used everywhere from home heating systems (thermostats control temperature) to large industrial machines (where they manage and coordinate various parts of the process).

<p align="center">
  <img src="https://github.com/duarte3333/Real-time-distributed-illumination-control/assets/76222459/dfe081aa-1927-44f1-a2ea-1f43f77aaa1a" alt="Image Description" style="width: 50%;">
</p>

### What is an LDR?

LDR (Light Dependent Resistor) is an electronic component whose resistance changes with the amount of light falling on it. Essentially, the more light that hits the LDR, the lower its resistance. They are often used in devices where it is necessary to detect light levels, such as in automatic street lights, which turn on when it gets dark, or in security devices that need to detect changes in lighting.

### What is the Consensus Algorithm?

A consensus algorithm is a method used in computer systems to make sure all parts of a system agree on a single version of the truth, even if some parts fail or try to cause errors. It's like having a group decision-making process where everyone must agree before moving forward, ensuring that the entire system remains consistent and in sync.

These algorithms are particularly important in networks where multiple computers or devices need to work together seamlessly, like in blockchain technology for cryptocurrencies or in systems that manage database updates across many servers. The goal is to guarantee that all participants, or nodes, in the network agree on the current state of data or a transaction, despite potential disruptions.


