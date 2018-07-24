# Voltage-Data-Logger
Programmable analog voltage data logger project on FRDM KL25Z Dev board.

The system can be in one of four states:
•	CLI: Providing a command line interface with the UART.
•	Sampling: Sampling data with the ADC.
•	Waiting: Waiting to take the next sample.
•	Sending Data: Sending formatted data out the serial port.
which are indicated by lighting the LED and raising the corresponding debug signal to 1 (clearing others to 0).



